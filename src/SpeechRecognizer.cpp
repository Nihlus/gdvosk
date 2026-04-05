// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT


#include "SpeechRecognizer.h"
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

    ADD_SIGNAL(MethodInfo("partial_result", PropertyInfo(Variant::STRING, "text")));
    ADD_SIGNAL
    (
        MethodInfo
        (
            "result",
            PropertyInfo(Variant::FLOAT, "confidence"),
            PropertyInfo(Variant::STRING, "text")
        )
    );
}

SpeechRecognizer::SpeechRecognizer()
{
    _recognizer.instantiate();
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

void SpeechRecognizer::_process(double p_delta)
{
    if (_effect == nullptr)
    {
        return;
    }

    if (!_recognizer->is_set_up())
    {
        return;
    }

    auto samples = _effect->get_buffer(_effect->get_frames_available());
    switch (_recognizer->accept_samples(samples))
    {
        case ERR_BUSY:
        {
            auto new_partial_result = _recognizer->get_partial_result();
            if (!_partial_result.has_value() || _partial_result != new_partial_result)
            {
                _partial_result = new_partial_result;
                _no_change_time_start = steady_clock::now();

                auto result = _partial_result->get("partial", "");
                if (result != "")
                {
                    emit_signal("partial_result", result);
                }
            }

            break;
        }
        case OK:
        {
            auto result = _recognizer->get_final_result()
                .get("alternatives",  TypedArray<Dictionary>());

            if (result.get_type() == Variant::Type::ARRAY)
            {
                auto array = static_cast<TypedArray<Dictionary>>(result);
                auto dictionary = array.get(0);

                auto confidence = dictionary.get("confidence");
                auto text = dictionary.get("text");

                emit_signal("result", confidence, text);
            }

            break;
        }
        case FAILED:
        default:
        {
        }
    }
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

void SpeechRecognizer::set_vosk_model(const Ref<VoskModel>& vosk_model)
{
    _vosk_model = vosk_model;

    if (!_recognizer.is_valid())
    {
        update_configuration_warnings();
        return;
    }

    auto mix_rate = ProjectSettings::get_singleton()->get_setting("audio/driver/mix_rate", 44100);
    _recognizer->setup(_vosk_model, mix_rate);

    update_configuration_warnings();
}

Ref<gdvosk::VoskModel> SpeechRecognizer::get_vosk_model() const
{
    return _vosk_model;
}

void SpeechRecognizer::update_bus_data()
{
    auto* audio_server = AudioServer::get_singleton();

    _recording_bus_index = audio_server->get_bus_index(_recording_bus_name);
    if (_recording_bus_index < 0)
    {
        _effect.unref();

        update_configuration_warnings();
        return;
    }

    if (audio_server->get_bus_effect_count(_recording_bus_index) < 1)
    {
        _effect.unref();

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
            _effect = effect;

            update_configuration_warnings();
            return;
        }
    }

    _effect.unref();

    update_configuration_warnings();
}
