// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#ifndef VOSKRECOGNIZER_H
#define VOSKRECOGNIZER_H

#include "VoskModel.h"

#include "../helpers/auto_property.h"
#include <godot_cpp/classes/ref_counted.hpp>
#include <vosk_api.h>
#include <godot_cpp/classes/audio_stream_wav.hpp>

#include "VoskSpeakerModel.h"

namespace gdvosk
{
    /**
     * Provides access to a Vosk recognizer as a normal Godot reference-counted object.
     */
    class VoskRecognizer final : public godot::RefCounted
    {
        GDCLASS(VoskRecognizer, godot::RefCounted)

        /**
         * Holds the underlying pointer to the recognizer.
         */
        ::VoskRecognizer* _recognizer = nullptr;

        /**
         * Holds the Vosk model currently in use.
         */
        godot::Ref<VoskModel> _model = nullptr;

        /**
         * Gets or sets the Vosk speaker model to use, if any.
         */
        GODOT_PROPERTY(godot::Ref<VoskSpeakerModel>, speaker_model, nullptr)

        /**
         * Gets or sets the maximum allowed number of alternative phrases to be returned by the recognizer.
         */
        GODOT_PROPERTY(int, max_alternatives, 1)

        /**
         * Gets or sets a value indicating whether words with start and end times should be returned in output.
         */
        GODOT_PROPERTY(bool, include_words_in_output, false)

        /**
         * Gets or sets a value indicating whether words with start and end times should be returned in partial output.
         */
        GODOT_PROPERTY(bool, include_words_in_partial_output, false)

        /**
         * Gets or sets a value indicating whether the output should be in NLSML format, instead of the default JSON.
         */
        GODOT_PROPERTY(bool, use_nlsml_output, false)

    public:
        /**
         * Sets up the recognizer with the given model, sample rate, and an optional speaker model.
         * @param model The language model.
         * @param sample_rate The sample rate of audio to be processed.
         * @param speaker_model The speaker model.
         * @return The result of the operation.
         */
        godot::Error setup
        (
            const godot::Ref<VoskModel>& model,
            float sample_rate,
            const godot::Ref<VoskSpeakerModel>& speaker_model = nullptr
        );

        /**
         * Sets up the recognizer with the given model, sample rate, and an optional set of words expected to be
         * recognized by the recognizer.
         * @param model The language model.
         * @param sample_rate The sample rate of audio to be processed.
         * @param grammar The grammar.
         * @return The result of the operation.
         */
        godot::Error setup_with_grammar
        (
            const godot::Ref<VoskModel>& model,
            float sample_rate,
            const godot::PackedStringArray& grammar
        );

        /**
         * Accepts a stream of audio data, transcribing the audio within it. The audio is expected to be in 16-bit
         * signed PCM format and can be either mono or stereo. Stereo audio will be mixed to mono before processing.
         * @param stream The stream.
         * @return OK if a complete sentence was recognized and a final result is available, ERR_BUSY if the audio was
         * accepted and a partial result is available, and FAILED if the audio was not accepted.
         */
        godot::Error accept_stream(const godot::Ref<godot::AudioStreamWAV>& stream);

        /**
         * Accepts a set of audio data samples, transcribing the audio within it. The audio is expected to be in 32-bit
         * floating-point PCM format and can be either mono or stereo. Stereo audio will be mixed to mono before
         * processing.
         * @param samples The audio samples.
         * @return OK if a complete sentence was recognized and a final result is available, ERR_BUSY if the audio was
         * accepted and a partial result is available, and FAILED if the audio was not accepted.
         */
        godot::Error accept_samples(const godot::PackedVector2Array& samples);

        /**
         * Gets the result of the current transcription. If no result is available yet, this method will block until a
         * set amount of silence has been detected.
         * @return A dictionary containing the parsed results.
         */
        godot::Dictionary get_result();

        /**
         * Gets the result of the current transcription. If no result is available yet, this method will return the
         * current best guess at what's being said.
         * @return A dictionary containing the parsed results.
         */
        godot::Dictionary get_partial_result();

        /**
         * Gets the result of the current transcription. If no result is available yet, this method will block and flush
         * the remaining audio through the processor.
         * @return A dictionary containing the parsed results.
         */
        godot::Dictionary get_final_result();

        /**
         * Resets the recognizer so transcription can continue from scratch.
         */
        void reset();

    protected:
        static void _bind_methods();

    private:
        void update_recognizer_parameters();

        static godot::PackedByteArray mix_stereo_to_mono(const godot::PackedByteArray& data);
        static godot::PackedFloat32Array mix_stereo_to_mono(const godot::PackedVector2Array& data);
        static godot::Dictionary parse_json_as_dictionary(const godot::String& data);
    };
}

#endif //VOSKRECOGNIZER_H
