// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#ifndef VOSKMODEL_H
#define VOSKMODEL_H

#include <godot_cpp/classes/resource.hpp>
#include <vosk/vosk_api.h>

namespace gdvosk
{
    class VoskModel final : public godot::Resource
    {
        GDCLASS(VoskModel, godot::Resource)

        ::VoskModel* _model = nullptr;

    protected:
        static void _bind_methods();

    public:
        ~VoskModel() override;

        [[nodiscard]] int find_word(const godot::String& word) const;

        godot::Error load(const godot::String& path);

        [[nodiscard]] ::VoskModel* get_ptr() const;
    };
}

#endif //VOSKMODEL_H
