// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#ifndef VOSKRECOGNIZER_H
#define VOSKRECOGNIZER_H

#include "VoskModel.h"

#include <godot_cpp/classes/ref_counted.hpp>
#include <vosk/vosk_api.h>

#include "VoskSpeakerModel.h"

namespace gdvosk
{
    class VoskRecognizer final : public godot::RefCounted
    {
        GDCLASS(VoskRecognizer, godot::RefCounted)

        ::VoskRecognizer* _recognizer;

        godot::Ref<VoskModel> _model;
        float _sample_rate;
        godot::Ref<VoskSpeakerModel> _speaker_model;
        godot::PackedStringArray _grammar;

        int _max_alternatives = 1;
        bool _include_words_in_output = false;
        bool _include_words_in_partial_output = false;
        bool _use_nlsml_output = false;

    public:
        godot::Error setup
        (
            const godot::Ref<VoskModel>& model,
            float sample_rate,
            const godot::Ref<VoskSpeakerModel>& speaker_model = nullptr
        );

        godot::Error setup
        (
            const godot::Ref<VoskModel>& model,
            float sample_rate,
            const godot::PackedStringArray& grammar
        );

        [[nodiscard]] godot::Ref<VoskSpeakerModel> get_speaker_model() const;
        void set_speaker_model(const godot::Ref<VoskSpeakerModel>& speaker_model);

        [[nodiscard]] int get_max_alternatives() const;
        void set_max_alternatives(int max_alternatives);

        [[nodiscard]] bool get_include_words_in_output() const;
        void set_include_words_in_output(bool include_words_in_output);

        [[nodiscard]] bool get_include_words_in_partial_output() const;
        void set_include_words_in_partial_output(bool include_words_in_partial_output);

        [[nodiscard]] bool get_use_nlsml_output() const;
        void set_use_nlsml_output(bool use_nlsml_output);

    private:
        void update_recognizer_parameters();
    };
}

#endif //VOSKRECOGNIZER_H
