// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT


#ifndef VOSKSPEAKERMODEL_H
#define VOSKSPEAKERMODEL_H

#include <godot_cpp/classes/resource.hpp>
#include <vosk/vosk_api.h>

namespace gdvosk
{
    class VoskSpeakerModel final : public godot::Resource
    {
        GDCLASS(VoskSpeakerModel, godot::Resource)

        VoskSpkModel* _model;

    public:
        ~VoskSpeakerModel() override;

        godot::Error load(const godot::String& path);

        [[nodiscard]] VoskSpkModel* get_ptr() const;
    };
}

#endif //VOSKSPEAKERMODEL_H
