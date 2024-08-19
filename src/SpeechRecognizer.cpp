//
// Created by jarl on 2024-08-16.
//

#include "SpeechRecognizer.h"
#include "vosk/VoskRecognizer.h"

#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <chrono>

using namespace std::chrono;
using namespace godot;

void SpeechRecognizer::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_recording_bus_name"), &SpeechRecognizer::get_recording_bus_name);
    ClassDB::bind_method(D_METHOD("set_recording_bus_name"), &SpeechRecognizer::set_recording_bus_name);
    ADD_PROPERTY
    (
        PropertyInfo(Variant::STRING, "recording_bus_name", PROPERTY_HINT_NONE),
        "set_recording_bus_name",
        "get_recording_bus_name"
    );

    ClassDB::bind_method(D_METHOD("get_recording_effect_index"), &SpeechRecognizer::get_recording_effect_index);
    ClassDB::bind_method(D_METHOD("set_recording_effect_index"), &SpeechRecognizer::set_recording_effect_index);
    ADD_PROPERTY
    (
        PropertyInfo(Variant::INT, "recording_effect_index", PROPERTY_HINT_NONE),
        "set_recording_effect_index",
        "get_recording_effect_index"
    );

    ClassDB::bind_method(D_METHOD("get_vosk_model"), &SpeechRecognizer::get_vosk_model);
    ClassDB::bind_method(D_METHOD("set_vosk_model"), &SpeechRecognizer::set_vosk_model);
    ADD_PROPERTY
    (
        PropertyInfo(Variant::OBJECT, "vosk_model", PROPERTY_HINT_RESOURCE_TYPE, "VoskModel"),
        "set_vosk_model",
        "get_vosk_model"
    );

    ClassDB::bind_method(D_METHOD("get_silence_timeout"), &SpeechRecognizer::get_silence_timeout);
    ClassDB::bind_method(D_METHOD("set_silence_timeout"), &SpeechRecognizer::set_silence_timeout);
    ADD_PROPERTY
    (
        PropertyInfo(Variant::FLOAT, "silence_timeout", PROPERTY_HINT_NONE),
        "set_silence_timeout",
        "get_silence_timeout"
    );

    ADD_SIGNAL(MethodInfo("partial_result", PropertyInfo(Variant::DICTIONARY, "data")));
    ADD_SIGNAL(MethodInfo("result", PropertyInfo(Variant::DICTIONARY, "data")));
    ADD_SIGNAL(MethodInfo("final_result", PropertyInfo(Variant::DICTIONARY, "data")));

    // callables
    ClassDB::bind_method(D_METHOD("worker_main"), &SpeechRecognizer::worker_main);
    ClassDB::bind_method(D_METHOD("update_bus_data"), &SpeechRecognizer::update_bus_data);
}

void SpeechRecognizer::_exit_tree()
{
    stop_voice_recognition();
}

PackedStringArray SpeechRecognizer::_get_configuration_warnings() const
{
    PackedStringArray warnings = Node::_get_configuration_warnings();
    if (_recording_bus_index < 0)
    {
        warnings.append("No valid audio bus has been configured");
    }
    else if (_recording_effect == nullptr)
    {
        warnings.append("No valid recording effect has been detected on the configured bus");
    }
    else if (_vosk_model == nullptr)
    {
        warnings.append("No valid Vosk model has been loaded");
    }

    return warnings;
}

void SpeechRecognizer::_ready()
{
    auto connection_result = AudioServer::get_singleton()->connect
    (
        "bus_layout_changed",
        callable_mp(this, &SpeechRecognizer::update_bus_data)
    );

    if (connection_result != OK)
    {
        // TODO: warn
    }

    _model_semaphore.instantiate();
    _model_semaphore->post();

    _bus_semaphore.instantiate();
    _bus_semaphore->post();

    update_bus_data();
}

void SpeechRecognizer::set_recording_bus_name(const StringName& recording_bus_name)
{
    _recording_bus_name = recording_bus_name;

    if (is_node_ready())
    {
        update_bus_data();
    }
}

const StringName& SpeechRecognizer::get_recording_bus_name() const
{
    return _recording_bus_name;
}

void SpeechRecognizer::set_recording_effect_index(int recording_effect_index)
{
    _recording_effect_index = recording_effect_index;

    if (is_node_ready())
    {
        update_bus_data();
    }
}

int SpeechRecognizer::get_recording_effect_index() const
{
    return _recording_effect_index;
}


void SpeechRecognizer::set_silence_timeout(float silence_timeout)
{
    _silence_timeout = round<nanoseconds>(duration<float>(silence_timeout));;
}

float SpeechRecognizer::get_silence_timeout() const
{
    return duration_cast<duration<float>>(_silence_timeout.load()).count();
}

void SpeechRecognizer::set_vosk_model(const godot::Ref<gdvosk::VoskModel>& vosk_model)
{
    _model_semaphore->wait();

    _vosk_model = vosk_model;
    update_vosk_data();

    _model_semaphore->post();
}

godot::Ref<gdvosk::VoskModel> SpeechRecognizer::get_vosk_model() const
{
    return _vosk_model;
}

void SpeechRecognizer::update_bus_data()
{
    auto* audio_server = AudioServer::get_singleton();

    _recording_bus_index = audio_server->get_bus_index(_recording_bus_name);
    if (_recording_bus_index < 0)
    {
        _bus_semaphore->wait();
        {
            _recording_effect.unref();
        }
        _bus_semaphore->post();

        update_configuration_warnings();
        return;
    }

    if (audio_server->get_bus_effect_count(_recording_bus_index) < 1)
    {
        _bus_semaphore->wait();
        {
            _recording_effect.unref();
        }
        _bus_semaphore->post();

        update_configuration_warnings();
        return;
    }

    auto effect = audio_server->get_bus_effect(_recording_bus_index, _recording_effect_index);
    auto recording_effect = cast_to<AudioEffectRecord>(effect.ptr());

    if (recording_effect == nullptr)
    {
        _bus_semaphore->wait();
        {
            _recording_effect.unref();
        }
        _bus_semaphore->post();

        update_configuration_warnings();
        return;
    }

    _bus_semaphore->wait();
    {
        _recording_effect = effect;
    }
    _bus_semaphore->post();

    update_configuration_warnings();
}

void SpeechRecognizer::update_vosk_data()
{
    stop_voice_recognition();

    if (_vosk_model != nullptr)
    {
        start_voice_recognition();
    }

    update_configuration_warnings();
}

void SpeechRecognizer::stop_voice_recognition()
{
    _should_worker_run = false;
    if (_worker.is_valid() && _worker->is_alive())
    {
        _worker->wait_to_finish();
        _worker.unref();
    }
}

void SpeechRecognizer::start_voice_recognition()
{
    if (_worker.is_valid() && _worker->is_alive())
    {
        // TODO: maybe raise an error here
        return;
    }

    _should_worker_run = true;

    _worker.instantiate();
    _worker->start(callable_mp(this, &SpeechRecognizer::worker_main));
}

void SpeechRecognizer::worker_main()
{
    constexpr auto interval_usec = duration_cast<microseconds>(milliseconds(200));

    std::optional<Dictionary> partial_result;
    std::optional<steady_clock::time_point> no_change_time_start;

    while (_should_worker_run)
    {
        OS::get_singleton()->delay_usec(interval_usec.count());

        Ref<AudioStreamWAV> recording;
        _bus_semaphore->wait();
        {
            if (_recording_effect == nullptr || !_recording_effect->is_recording_active())
            {
                continue;
            }

            recording = _recording_effect->get_recording();
            if (!recording.is_valid())
            {
                continue;
            }
        }
        _bus_semaphore->post();

        Ref<gdvosk::VoskRecognizer> recognizer;
        recognizer.instantiate();

        Error setup;
        _model_semaphore->wait();
        {
            setup = recognizer->setup(_vosk_model, static_cast<float>(recording->get_mix_rate()));
        }
        _model_semaphore->post();

        if (setup != OK)
        {
            continue;
        }

        auto accept_waveform = recognizer->accept_waveform(recording);
        switch (accept_waveform)
        {
            case ERR_BUSY:
            {
                auto new_partial_result = recognizer->get_partial_result();
                if (!partial_result.has_value() || partial_result != new_partial_result)
                {
                    partial_result = new_partial_result;
                    no_change_time_start = steady_clock::now();

                    call_deferred("emit_signal", "partial_result", *partial_result);
                }

                break;
            }
            case OK:
            {
                call_deferred("emit_signal", "result", recognizer->get_result());
                break;
            }
            case FAILED:
            default:
            {
                continue;
            }
        }

        auto now = steady_clock::now();
        if (no_change_time_start.has_value() && (now - *no_change_time_start > _silence_timeout.load()))
        {
            call_deferred("emit_signal", "final_result", recognizer->get_final_result());

            partial_result = std::nullopt;
            no_change_time_start = std::nullopt;
        }
    }
}

SpeechRecognizer::SpeechRecognizer()
{
    _model_semaphore.instantiate();
    _model_semaphore->post();

    _bus_semaphore.instantiate();
    _bus_semaphore->post();
}
