// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#include "VoskRecognizer.h"

#include <godot_cpp/classes/json.hpp>

using namespace godot;
using namespace gdvosk;

Error gdvosk::VoskRecognizer::setup
(
    const Ref<VoskModel>& model,
    float sample_rate,
    const Ref<VoskSpeakerModel>& speaker_model
)
{
    if (_recognizer != nullptr)
    {
        vosk_recognizer_free(_recognizer);
        _recognizer = nullptr;
    }

    _model = model;
    _sample_rate = sample_rate;
    _speaker_model = speaker_model;

    _recognizer = speaker_model != nullptr
            ? vosk_recognizer_new_spk(_model->get_ptr(), _sample_rate, _speaker_model->get_ptr())
            : vosk_recognizer_new(_model->get_ptr(), _sample_rate);

    if (_recognizer == nullptr)
    {
        return FAILED;
    }

    update_recognizer_parameters();

    return OK;
}

Error gdvosk::VoskRecognizer::setup_with_grammar
(
    const godot::Ref<VoskModel>& model,
    float sample_rate,
    const godot::PackedStringArray& grammar
)
{
    if (_recognizer != nullptr)
    {
        vosk_recognizer_free(_recognizer);
        _recognizer = nullptr;
    }

    _model = model;
    _sample_rate = sample_rate;
    _speaker_model.unref();

    auto json = JSON::stringify(grammar);

    _recognizer = vosk_recognizer_new_grm(_model->get_ptr(), _sample_rate, json.ascii());

    if (_recognizer == nullptr)
    {
        return FAILED;
    }

    update_recognizer_parameters();

    return OK;
}

Ref<VoskSpeakerModel> gdvosk::VoskRecognizer::get_speaker_model() const
{
    return _speaker_model;
}

void gdvosk::VoskRecognizer::set_speaker_model(const Ref<VoskSpeakerModel>& speaker_model)
{
    _speaker_model = speaker_model;

    if (_recognizer != nullptr)
    {
        vosk_recognizer_set_spk_model(_recognizer, _speaker_model->get_ptr());
    }
}

int gdvosk::VoskRecognizer::get_max_alternatives() const
{
    return _max_alternatives;
}

void gdvosk::VoskRecognizer::set_max_alternatives(int max_alternatives)
{
    _max_alternatives = max_alternatives;

    if (_recognizer != nullptr)
    {
        vosk_recognizer_set_max_alternatives(_recognizer, _max_alternatives);
    }
}

bool gdvosk::VoskRecognizer::get_include_words_in_output() const
{
    return _include_words_in_output;
}

void gdvosk::VoskRecognizer::set_include_words_in_output(bool include_words_in_output)
{
    _include_words_in_output = include_words_in_output;

    if (_recognizer != nullptr)
    {
        vosk_recognizer_set_words(_recognizer, _include_words_in_output ? 1 : 0);
    }
}

bool gdvosk::VoskRecognizer::get_include_words_in_partial_output() const
{
    return _include_words_in_partial_output;
}

void gdvosk::VoskRecognizer::set_include_words_in_partial_output(bool include_words_in_partial_output)
{
    _include_words_in_partial_output = include_words_in_partial_output;

    if (_recognizer != nullptr)
    {
        vosk_recognizer_set_partial_words(_recognizer, _include_words_in_partial_output ? 1 : 0);
    }
}

bool gdvosk::VoskRecognizer::get_use_nlsml_output() const
{
    return _use_nlsml_output;
}

void gdvosk::VoskRecognizer::set_use_nlsml_output(bool use_nlsml_output)
{
    _use_nlsml_output = use_nlsml_output;

    if (_recognizer != nullptr)
    {
        vosk_recognizer_set_nlsml(_recognizer, use_nlsml_output ? 1 : 0);
    }
}

void gdvosk::VoskRecognizer::update_recognizer_parameters()
{
    if (_recognizer == nullptr)
    {
        return;
    }

    if (_speaker_model != nullptr)
    {
        vosk_recognizer_set_spk_model(_recognizer, _speaker_model->get_ptr());
    }

    vosk_recognizer_set_max_alternatives(_recognizer, _max_alternatives);
    vosk_recognizer_set_words(_recognizer, _include_words_in_output);
    vosk_recognizer_set_partial_words(_recognizer, _include_words_in_partial_output);
    vosk_recognizer_set_nlsml(_recognizer, _use_nlsml_output);
}

godot::Error gdvosk::VoskRecognizer::accept_stream(const Ref<godot::AudioStreamWAV>& stream)
{
    auto data = stream->is_stereo()
        ? mix_stereo_to_mono(stream->get_data())
        : stream->get_data();

    auto* ptr = reinterpret_cast<const char*>(data.ptr());

    auto result = vosk_recognizer_accept_waveform(_recognizer, ptr, static_cast<int>(data.size()));
    if (result >= 1)
    {
        return OK;
    }

    if (result == 0)
    {
        // this is a bit of a hack... it's not really an error, but the recognizer is busy decoding the current
        // utterance and you should continue giving it data until it either says OK or FAILED. You can retrieve the
        // current best guess using partial_result if ERR_BUSY is returned.
        return ERR_BUSY;
    }

    return FAILED;
}

godot::Error gdvosk::VoskRecognizer::accept_samples(const PackedVector2Array& samples)
{
    auto data = mix_stereo_to_mono(samples);

    auto result = vosk_recognizer_accept_waveform_f(_recognizer, data.ptr(), static_cast<int>(data.size()));
    if (result >= 1)
    {
        return OK;
    }

    if (result == 0)
    {
        // this is a bit of a hack... it's not really an error, but the recognizer is busy decoding the current
        // utterance and you should continue giving it data until it either says OK or FAILED. You can retrieve the
        // current best guess using partial_result if ERR_BUSY is returned.
        return ERR_BUSY;
    }

    return FAILED;
}

godot::PackedByteArray gdvosk::VoskRecognizer::mix_stereo_to_mono(const PackedByteArray& data)
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
        const auto left_channel = *reinterpret_cast<const int16_t*>(&data.ptr()[i]);
        const auto right_channel = *reinterpret_cast<const int16_t*>(&data.ptr()[i + 2]);

        const auto mixed = (int32_t(left_channel) + right_channel) / 2;

        reinterpret_cast<int16_t*>(output.ptrw())[output_index] = static_cast<int16_t>(mixed);
        output_index += 1;
    }

    return output;
}

godot::PackedFloat32Array gdvosk::VoskRecognizer::mix_stereo_to_mono(const PackedVector2Array& data)
{
    PackedFloat32Array output;
    output.resize(data.size());

    for (auto i = 0; i < data.size(); ++i)
    {
        auto& sample = data[i];
        auto mixed = Math::clamp((sample.x + sample.y) / 2.0f, -1.0f, 1.0f);

        // vosk expects -32768 to 32768, not -1 to 1
        output[i] = mixed * 32768;
    }

    return output;
}

godot::Dictionary gdvosk::VoskRecognizer::get_result()
{
    auto result = vosk_recognizer_result(_recognizer);
    if (result == nullptr)
    {
        return { };
    }

    return parse_dictionary(result);
}

godot::Dictionary gdvosk::VoskRecognizer::get_partial_result()
{
    auto result = vosk_recognizer_partial_result(_recognizer);
    if (result == nullptr)
    {
        return { };
    }

    return parse_dictionary(result);
}

godot::Dictionary gdvosk::VoskRecognizer::get_final_result()
{
    auto result = vosk_recognizer_final_result(_recognizer);
    if (result == nullptr)
    {
        return { };
    }

    return parse_dictionary(result);
}

godot::Dictionary gdvosk::VoskRecognizer::parse_dictionary(const String& data)
{
    Ref<JSON> parsed;
    parsed.instantiate();

    auto parse_result = parsed->parse(String(data));
    if (parse_result != OK)
    {
        return { };
    }

    if (parsed->get_data().get_type() != Variant::DICTIONARY)
    {
        return { };
    }

    return parsed->get_data();
}

void gdvosk::VoskRecognizer::reset()
{
    vosk_recognizer_reset(_recognizer);
}

void gdvosk::VoskRecognizer::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("setup"), &VoskRecognizer::setup);
    ClassDB::bind_method(D_METHOD("setup_with_grammar"), &VoskRecognizer::setup_with_grammar);
    ClassDB::bind_method(D_METHOD("accept_stream"), &VoskRecognizer::accept_stream);
    ClassDB::bind_method(D_METHOD("accept_samples"), &VoskRecognizer::accept_samples);
    ClassDB::bind_method(D_METHOD("get_result"), &VoskRecognizer::get_result);
    ClassDB::bind_method(D_METHOD("get_partial_result"), &VoskRecognizer::get_partial_result);
    ClassDB::bind_method(D_METHOD("get_final_result"), &VoskRecognizer::get_final_result);
    ClassDB::bind_method(D_METHOD("reset"), &VoskRecognizer::reset);

    ClassDB::bind_method(D_METHOD("set_speaker_model"), &VoskRecognizer::set_speaker_model);
    ClassDB::bind_method(D_METHOD("get_speaker_model"), &VoskRecognizer::get_speaker_model);
    ADD_PROPERTY
    (
        PropertyInfo(Variant::OBJECT, "speaker_model", PROPERTY_HINT_RESOURCE_TYPE, "VoskSpeakerModel"),
        "set_speaker_model",
        "get_speaker_model"
    );

    ClassDB::bind_method(D_METHOD("set_max_alternatives"), &VoskRecognizer::set_max_alternatives);
    ClassDB::bind_method(D_METHOD("get_max_alternatives"), &VoskRecognizer::get_max_alternatives);
    ADD_PROPERTY
    (
        PropertyInfo(Variant::INT, "max_alternatives"),
        "set_max_alternatives",
        "get_max_alternatives"
    );

    ClassDB::bind_method(D_METHOD("set_include_words_in_output"), &VoskRecognizer::set_include_words_in_output);
    ClassDB::bind_method(D_METHOD("get_include_words_in_output"), &VoskRecognizer::get_include_words_in_output);
    ADD_PROPERTY
    (
        PropertyInfo(Variant::BOOL, "include_words_in_output"),
        "set_include_words_in_output",
        "get_include_words_in_output"
    );

    ClassDB::bind_method(D_METHOD("set_include_words_in_partial_output"), &VoskRecognizer::set_include_words_in_partial_output);
    ClassDB::bind_method(D_METHOD("get_include_words_in_partial_output"), &VoskRecognizer::get_include_words_in_partial_output);
    ADD_PROPERTY
    (
        PropertyInfo(Variant::BOOL, "include_words_in_partial_output"),
        "set_include_words_in_partial_output",
        "get_include_words_in_partial_output"
    );

    ClassDB::bind_method(D_METHOD("set_use_nlsml_output"), &VoskRecognizer::set_use_nlsml_output);
    ClassDB::bind_method(D_METHOD("get_use_nlsml_output"), &VoskRecognizer::get_use_nlsml_output);
    ADD_PROPERTY
    (
        PropertyInfo(Variant::BOOL, "use_nlsml_output"),
        "set_use_nlsml_output",
        "get_use_nlsml_output"
    );
}
