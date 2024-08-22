// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#ifndef SPEECHRECOGNIZER_H
#define SPEECHRECOGNIZER_H

#include <condition_variable>
#include <memory>
#include <optional>
#include <variant>
#include <queue>
#include <godot_cpp/classes/audio_effect_record.hpp>
#include <godot_cpp/classes/audio_effect_capture.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/thread.hpp>

#include <vosk/vosk_api.h>
#include <godot_cpp/classes/semaphore.hpp>
#include "vosk/VoskModel.h"

class SpeechRecognizer : public godot::Node
{
    GDCLASS(SpeechRecognizer, godot::Node)

    std::atomic_bool _should_worker_run = false;
    godot::Ref<godot::Thread> _worker = nullptr;

    int _recording_bus_index = 0;
    godot::Ref<godot::AudioEffectCapture> _effect;

    godot::Ref<godot::Semaphore> _model_semaphore = nullptr;
    godot::Ref<godot::Semaphore> _bus_semaphore = nullptr;

    /*
     * Properties
     */
    godot::StringName _recording_bus_name = "";
    int _recording_effect_index = 0;
    godot::Ref<gdvosk::VoskModel> _vosk_model = nullptr;
    std::atomic<std::chrono::nanoseconds> _silence_timeout = std::chrono::nanoseconds(2000000000);

protected:
    static void _bind_methods();

public:
    explicit SpeechRecognizer();

    void _exit_tree() override;

    [[nodiscard]] godot::PackedStringArray _get_configuration_warnings() const override;

    void _ready() override;

    void set_recording_bus_name(const godot::StringName& recording_bus_name);
    [[nodiscard]] const godot::StringName& get_recording_bus_name() const;

    void set_recording_effect_index(int recording_effect_index);
    [[nodiscard]] int get_recording_effect_index() const;

    void set_vosk_model(const godot::Ref<gdvosk::VoskModel>& vosk_model);
    [[nodiscard]] godot::Ref<gdvosk::VoskModel> get_vosk_model() const;

    void set_silence_timeout(float silence_timeout);
    [[nodiscard]] float get_silence_timeout() const;

private:
    void update_bus_data();
    void update_vosk_data();

    void stop_voice_recognition();
    void start_voice_recognition();

    void worker_main();

};

#endif //SPEECHRECOGNIZER_H
