// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#ifndef SPEECHRECOGNIZER_H
#define SPEECHRECOGNIZER_H

#include <condition_variable>
#include <chrono>
#include <memory>
#include <optional>
#include <variant>
#include <queue>
#include <godot_cpp/classes/audio_effect_record.hpp>
#include <godot_cpp/classes/audio_effect_capture.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/thread.hpp>

#include <vosk_api.h>
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
         * Holds the control variable for the background processing thread.
         */
        std::atomic_bool _should_worker_run = false;

        /**
         * Holds the background thread.
         */
        godot::Ref<godot::Thread> _worker = nullptr;

        /**
         * Holds the index of the recording bus.
         */
        int _recording_bus_index = 0;

        /**
         * Holds a reference to the capture effect on the recording bus.
         */
        godot::Ref<godot::AudioEffectCapture> _effect;

        /**
         * Holds a semaphore used for synchronization with the background thread when accessing the Vosk model.
         */
        godot::Ref<godot::Semaphore> _model_semaphore = nullptr;

        /**
         * Holds a semaphore used for synchronization with the background thread when accessing audio bus objects.
         */
        godot::Ref<godot::Semaphore> _bus_semaphore = nullptr;

        /**
         * Gets or sets the name of the recording bus.
         */
        GODOT_PROPERTY(godot::StringName, recording_bus_name, "")

        /**
         * Gets or sets the Vosk language model to use.
         */
        GODOT_PROPERTY(godot::Ref<gdvosk::VoskModel>, vosk_model, nullptr)

        /**
         * Holds the backing data for the silence timeout in microseconds.
         */
        std::atomic<std::chrono::microseconds> _silence_timeout =
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(2));

    protected:
        static void _bind_methods();

    public:
        /**
         * Initializes a new instance of the SpeechRecognizer class.
         */
        explicit SpeechRecognizer();

        ~SpeechRecognizer() override;

        void set_silence_timeout(float silence_timeout);
        [[nodiscard]] float get_silence_timeout() const;

        void _ready() override;
        void _exit_tree() override;
        [[nodiscard]] godot::PackedStringArray _get_configuration_warnings() const override;

    private:
        void update_bus_data();
        void update_vosk_data();

        void stop_voice_recognition();
        void start_voice_recognition();

        void worker_main
        (
            godot::Ref<godot::Semaphore> bus_semaphore,
            godot::Ref<godot::Semaphore> model_semaphore
        );
    };
}

#endif //SPEECHRECOGNIZER_H
