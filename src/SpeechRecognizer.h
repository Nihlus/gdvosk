// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#ifndef SPEECHRECOGNIZER_H
#define SPEECHRECOGNIZER_H

#include <chrono>
#include <memory>
#include <optional>
#include <godot_cpp/classes/audio_effect_record.hpp>
#include <godot_cpp/classes/audio_effect_capture.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/node.hpp>

#include <godot_cpp/classes/semaphore.hpp>
#include "vosk/VoskModel.h"
#include "helpers/auto_property.h"

namespace gdvosk
{
    /**
     * Acts as a continuous speech recognizer, producing results via signals over time via a background thread.
     */
    class SpeechRecognizer : public godot::Node
    {
        GDCLASS(SpeechRecognizer, godot::Node)

        /**
         * Holds the index of the recording bus.
         */
        int _recording_bus_index = 0;

        /**
         * Holds a reference to the capture effect on the recording bus.
         */
        godot::Ref<godot::AudioEffectCapture> _effect;

        /**
         * Holds a reference to the Vosk recognizer.
         */
        godot::Ref<VoskRecognizer> _recognizer;

        /**
         * Holds the partially recognized result, if any.
         */
        std::optional<godot::Dictionary> _partial_result;

        /**
         * Holds the time at which the speech stopped changing, if any.
         */
        std::optional<std::chrono::steady_clock::time_point> _no_change_time_start;

        /**
         * Gets or sets the name of the recording bus.
         */
        GODOT_PROPERTY(godot::StringName, recording_bus_name, "")

        /**
         * Gets or sets the Vosk language model to use.
         */
        GODOT_PROPERTY(godot::Ref<gdvosk::VoskModel>, vosk_model, nullptr)

    protected:
        static void _bind_methods();

    public:
        explicit SpeechRecognizer();

        void _ready() override;
        void _process(double p_delta) override;
        [[nodiscard]] godot::PackedStringArray _get_configuration_warnings() const override;

    private:
        void update_bus_data();
    };
}

#endif //SPEECHRECOGNIZER_H
