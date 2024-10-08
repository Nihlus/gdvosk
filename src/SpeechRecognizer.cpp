// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT


#include "SpeechRecognizer.h"
#include "helpers/semaphore_lock.h"
#include "vosk/VoskRecognizer.h"

#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace std::chrono;
using namespace godot;
using namespace gdvosk;

void SpeechRecognizer::_bind_methods()
{
    REGISTER_GODOT_PROPERTY(Variant::STRING, recording_bus_name)
    REGISTER_GODOT_PROPERTY_WITH_HINT(Variant::OBJECT, vosk_model, PROPERTY_HINT_RESOURCE_TYPE, "VoskModel")
    REGISTER_GODOT_PROPERTY(Variant::FLOAT, silence_timeout)

    ADD_SIGNAL(MethodInfo("partial_result", PropertyInfo(Variant::DICTIONARY, "data")));
    ADD_SIGNAL(MethodInfo("result", PropertyInfo(Variant::DICTIONARY, "data")));
    ADD_SIGNAL(MethodInfo("final_result", PropertyInfo(Variant::DICTIONARY, "data")));
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
    else if (_effect == nullptr)
    {
        warnings.append("No valid capture effect has been detected on the configured bus");
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

StringName SpeechRecognizer::get_recording_bus_name() const
{
    return _recording_bus_name;
}

void SpeechRecognizer::set_silence_timeout(float silence_timeout)
{
    _silence_timeout = round<microseconds>(duration<float>(silence_timeout));
}

float SpeechRecognizer::get_silence_timeout() const
{
    return duration_cast<duration<float>>(_silence_timeout.load()).count();
}

void SpeechRecognizer::set_vosk_model(const godot::Ref<gdvosk::VoskModel>& vosk_model)
{
    semaphore_lock lock(_model_semaphore);

    _vosk_model = vosk_model;
    update_vosk_data();
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
        {
            semaphore_lock lock(_bus_semaphore);
            _effect.unref();
        }

        update_configuration_warnings();
        return;
    }

    if (audio_server->get_bus_effect_count(_recording_bus_index) < 1)
    {
        {
            semaphore_lock lock(_bus_semaphore);
            _effect.unref();
        }

        update_configuration_warnings();
        return;
    }

    // find effect
    for (auto i = 0; i < audio_server->get_bus_effect_count(_recording_bus_index); ++i)
    {
        auto effect = audio_server->get_bus_effect(_recording_bus_index, i);

        auto capture_effect = cast_to<AudioEffectCapture>(effect.ptr());
        if (capture_effect != nullptr)
        {
            {
                semaphore_lock lock(_bus_semaphore);
                _effect = effect;
            }

            update_configuration_warnings();
            return;
        }
    }

    {
        semaphore_lock lock(_bus_semaphore);
        _effect.unref();
    }

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
    if (_worker.is_valid())
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

    auto callable = callable_mp(this, &SpeechRecognizer::worker_main).bind
    (
        _bus_semaphore,
        _model_semaphore
    );

    _worker->start(callable);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "performance-unnecessary-value-param"
void SpeechRecognizer::worker_main
(
    godot::Ref<godot::Semaphore> bus_semaphore,
    godot::Ref<godot::Semaphore> model_semaphore
)
{
    constexpr auto interval_usec = duration_cast<microseconds>(milliseconds(100));

    auto mix_rate = ProjectSettings::get_singleton()->get_setting("audio/driver/mix_rate", 44100);

    std::optional<Dictionary> partial_result;
    std::optional<steady_clock::time_point> no_change_time_start;

    bool has_set_up = false;
    Ref<gdvosk::VoskRecognizer> recognizer;
    recognizer.instantiate();

    while (_should_worker_run)
    {
        OS::get_singleton()->delay_usec(interval_usec.count());

        //
        PackedVector2Array data;
        {
            semaphore_lock lock(bus_semaphore);
            if (_effect == nullptr)
            {
                continue;
            }

            auto samples = _effect->get_buffer(_effect->get_frames_available());
            data = samples;
        }

        if (!has_set_up)
        {
            Error setup;
            {
                semaphore_lock lock(model_semaphore);
                setup = recognizer->setup(_vosk_model, static_cast<float>(mix_rate));
            }

            if (setup != OK)
            {
                continue;
            }

            has_set_up = true;
        }

        auto accept_waveform = recognizer->accept_samples(data);

        switch (accept_waveform)
        {
            case ERR_BUSY:
            {
                auto new_partial_result = recognizer->get_partial_result();
                if (!partial_result.has_value() || partial_result != new_partial_result)
                {
                    partial_result = new_partial_result;
                    no_change_time_start = steady_clock::now();

                    if (partial_result->get("partial", "") != "")
                    {
                        call_deferred("emit_signal", "partial_result", *partial_result);
                    }
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
            partial_result = std::nullopt;
            no_change_time_start = std::nullopt;

            auto final_result = recognizer->get_final_result();
            auto final_alternatives = static_cast<Array>(final_result.get("alternatives", Array()));

            if (final_alternatives.is_empty())
            {
                continue;
            }

            auto final_alternative = static_cast<Dictionary>(final_alternatives[0]);

            if (final_alternative.get("text", "") == "")
            {
                continue;
            }

            call_deferred("emit_signal", "final_result", final_result);
        }
    }
}
#pragma clang diagnostic pop

SpeechRecognizer::SpeechRecognizer()
{
    _model_semaphore.instantiate();
    _model_semaphore->post();

    _bus_semaphore.instantiate();
    _bus_semaphore->post();
}

SpeechRecognizer::~SpeechRecognizer()
{
    stop_voice_recognition();
}
