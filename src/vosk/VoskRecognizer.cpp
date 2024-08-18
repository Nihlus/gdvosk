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

    _recognizer = vosk_recognizer_new(_model->get_ptr(), _sample_rate);

    if (_recognizer == nullptr)
    {
        return FAILED;
    }

    update_recognizer_parameters();

    return OK;
}

Error gdvosk::VoskRecognizer::setup
(
    const Ref<VoskModel>& model,
    float sample_rate,
    const PackedStringArray& grammar
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
        vosk_recognizer_set_nlsml(_recognizer, use_nlsml_output);
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
