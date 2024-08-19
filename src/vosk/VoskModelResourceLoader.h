// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#ifndef VOSKMODELRESOURCELOADER_H
#define VOSKMODELRESOURCELOADER_H

#include <godot_cpp/classes/resource_format_loader.hpp>

namespace gdvosk
{
    class VoskModelResourceLoader final : public godot::ResourceFormatLoader
    {
        GDCLASS(VoskModelResourceLoader, godot::ResourceFormatLoader)

    protected:
        static void _bind_methods();

    public:
        [[nodiscard]] godot::PackedStringArray _get_recognized_extensions() const override;

        [[nodiscard]] bool _handles_type(const godot::StringName& p_type) const override;

        [[nodiscard]] godot::String _get_resource_type(const godot::String& p_path) const override;

        [[nodiscard]] godot::Variant _load
                (
                        const godot::String& p_path,
                        const godot::String& p_original_path,
                        bool p_use_sub_threads,
                        int32_t p_cache_mode
                ) const override;
    };
}

#endif //VOSKMODELRESOURCELOADER_H
