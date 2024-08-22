// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#ifndef VOSKRECOGNIZER_H
#define VOSKRECOGNIZER_H

#include "VoskModel.h"

#include <godot_cpp/classes/ref_counted.hpp>
#include <vosk/vosk_api.h>
#include <godot_cpp/classes/audio_stream_wav.hpp>

#include "VoskSpeakerModel.h"

namespace gdvosk
{
    class VoskRecognizer final : public godot::RefCounted
    {
        GDCLASS(VoskRecognizer, godot::RefCounted)

        ::VoskRecognizer* _recognizer = nullptr;

        godot::Ref<VoskModel> _model = nullptr;
        float _sample_rate = 0.0;
        godot::Ref<VoskSpeakerModel> _speaker_model = nullptr;
        godot::PackedStringArray _grammar = { };

        int _max_alternatives = 1;
        bool _include_words_in_output = false;
        bool _include_words_in_partial_output = false;
        bool _use_nlsml_output = false;

    protected:
        static void _bind_methods();

    public:
        godot::Error setup
        (
            const godot::Ref<VoskModel>& model,
            float sample_rate,
            const godot::Ref<VoskSpeakerModel>& speaker_model = nullptr
        );

        godot::Error setup_with_grammar
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

        godot::Error accept_stream(const godot::Ref<godot::AudioStreamWAV>& stream);

        godot::Error accept_samples(const godot::PackedVector2Array& samples);

        godot::Dictionary get_result();

        godot::Dictionary get_partial_result();

        godot::Dictionary get_final_result();

        void reset();

    private:
        void update_recognizer_parameters();

        static godot::PackedByteArray mix_stereo_to_mono(const godot::PackedByteArray& data);
        static godot::PackedFloat32Array mix_stereo_to_mono(const godot::PackedVector2Array& data);
        static godot::Dictionary parse_dictionary(const godot::String& data);
    };
}

#endif //VOSKRECOGNIZER_H
