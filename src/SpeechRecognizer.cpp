//
// Created by jarl on 2024-08-16.
//

#include "SpeechRecognizer.h"

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

    ClassDB::bind_method(D_METHOD("get_vosk_model_path"), &SpeechRecognizer::get_vosk_model_path);
    ClassDB::bind_method(D_METHOD("set_vosk_model_path"), &SpeechRecognizer::set_vosk_model_path);
    ADD_PROPERTY
    (
        PropertyInfo(Variant::STRING, "vosk_model_path", PROPERTY_HINT_DIR),
        "set_vosk_model_path",
        "get_vosk_model_path"
    );

    ClassDB::bind_method(D_METHOD("get_silence_timeout"), &SpeechRecognizer::get_silence_timeout);
    ClassDB::bind_method(D_METHOD("set_silence_timeout"), &SpeechRecognizer::set_silence_timeout);
    ADD_PROPERTY
    (
        PropertyInfo(Variant::FLOAT, "silence_timeout", PROPERTY_HINT_NONE),
        "set_silence_timeout",
        "get_silence_timeout"
    );

    ADD_SIGNAL(MethodInfo("partial_result", PropertyInfo(Variant::STRING, "json")));
    ADD_SIGNAL(MethodInfo("final_result", PropertyInfo(Variant::STRING, "json")));

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

void SpeechRecognizer::set_vosk_model_path(const String& vosk_model_path)
{
    _vosk_model_path = vosk_model_path;

    update_vosk_data();
}

const godot::String& SpeechRecognizer::get_vosk_model_path() const
{
    return _vosk_model_path;
}

void SpeechRecognizer::set_silence_timeout(float silence_timeout)
{
    _silence_timeout = round<nanoseconds>(duration<float>(silence_timeout));;
}

float SpeechRecognizer::get_silence_timeout() const
{
    return duration_cast<duration<float>>(_silence_timeout.load()).count();
}

void SpeechRecognizer::update_bus_data()
{
    auto* audio_server = AudioServer::get_singleton();

    _recording_bus_index = audio_server->get_bus_index(_recording_bus_name);
    if (_recording_bus_index < 0)
    {
        _recording_effect.unref();

        update_configuration_warnings();
        return;
    }

    if (audio_server->get_bus_effect_count(_recording_bus_index) < 1)
    {
        _recording_effect.unref();

        update_configuration_warnings();
        return;
    }

    auto effect = audio_server->get_bus_effect(_recording_bus_index, _recording_effect_index);
    auto recording_effect = cast_to<AudioEffectRecord>(effect.ptr());

    if (recording_effect == nullptr)
    {
        _recording_effect.unref();

        update_configuration_warnings();
        return;
    }

    _recording_effect.reference_ptr(recording_effect);
    update_configuration_warnings();
}

void SpeechRecognizer::update_vosk_data()
{
    stop_voice_recognition();

    auto globalized_path = ProjectSettings::get_singleton()->globalize_path(_vosk_model_path);
    _vosk_model = std::shared_ptr<VoskModel>(vosk_model_new(globalized_path.ascii()), &vosk_model_free);

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

    std::optional<String> partial_result;
    std::optional<steady_clock::time_point> no_change_time_start;

    auto model = _vosk_model;
    while (_should_worker_run)
    {
        OS::get_singleton()->delay_usec(interval_usec.count());

        // TODO: is direct access of the recording effect thread safe?
        if (_recording_effect == nullptr || !_recording_effect->is_recording_active())
        {
            continue;
        }

        auto recording = _recording_effect->get_recording();
        if (!recording.is_valid())
        {
            continue;
        }

        auto recognizer = std::shared_ptr<VoskRecognizer>
        (
            vosk_recognizer_new(model.get(), static_cast<float>(recording->get_mix_rate())),
            vosk_recognizer_free
        );

        auto data = recording->is_stereo()
            ? mix_stereo_to_mono(recording->get_data())
            : recording -> get_data();

        auto* ptr = reinterpret_cast<const char*>(data.ptr());
        if (vosk_recognizer_accept_waveform(recognizer.get(), ptr, static_cast<int>(data.size())) == 0)
        {
            auto new_partial_result = String(vosk_recognizer_partial_result(recognizer.get()));
            if (!partial_result.has_value() || partial_result != new_partial_result)
            {
                partial_result = new_partial_result;
                no_change_time_start = steady_clock::now();

                call_deferred("emit_signal", "partial_result", *partial_result);
            }
        }

        auto now = steady_clock::now();
        if (no_change_time_start.has_value() && (now - *no_change_time_start > _silence_timeout.load()))
        {
            auto final_result = String(vosk_recognizer_final_result(recognizer.get()));
            call_deferred("emit_signal", "final_result", final_result);

            partial_result = std::nullopt;
            no_change_time_start = std::nullopt;
        }
    }
}

PackedByteArray SpeechRecognizer::mix_stereo_to_mono(const PackedByteArray& data)
{
    if (data.size() % 4 != 0)
    {
        PackedByteArray silent;
        silent.resize(24);

        return silent;
    }

    PackedByteArray output;
    output.resize(data.size() / 2);

    auto output_index = 0;
    for (auto i = 0; i < data.size(); i += 4)
    {
        const auto left_channel = static_cast<int16_t>(data[i]);
        const auto right_channel = static_cast<int16_t>(data[i + 2]);

        const auto mixed = static_cast<int16_t>((left_channel + right_channel) / 2);

        output[output_index] = static_cast<uint8_t>(mixed & 0xFF00);
        output[output_index + 1] = static_cast<uint8_t>((mixed & 0x00FF) << 8);

        output_index += 2;
    }

    return output;
}
