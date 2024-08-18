// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#include "VoskModelResourceLoader.h"

using namespace godot;

PackedStringArray VoskModelResourceLoader::_get_recognized_extensions() const
{
    return { "vosk", "voskspk" };
}

bool VoskModelResourceLoader::_handles_type(const StringName& p_type) const
{
    return p_type == StringName("VoskModel") || p_type == StringName("VoskSpeakerModel");
}

String VoskModelResourceLoader::_get_resource_type(const String& p_path) const
{
    if (p_path.get_extension() == "vosk")
    {
        return "VoskModel";
    }

    return "VoskSpeakerModel";
}

Variant VoskModelResourceLoader::_load
(
    const String& p_path,
    const String& p_original_path,
    bool p_use_sub_threads,
    int32_t p_cache_mode
) const
{
    return ResourceFormatLoader::_load(p_path, p_original_path, p_use_sub_threads, p_cache_mode);
}
