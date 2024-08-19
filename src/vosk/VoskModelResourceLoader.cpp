// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/zip_reader.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include "VoskModelResourceLoader.h"
#include "VoskModel.h"
#include "VoskSpeakerModel.h"

using namespace godot;
using namespace gdvosk;

PackedStringArray gdvosk::VoskModelResourceLoader::_get_recognized_extensions() const
{
    return { "vosk", "voskspk" };
}

bool gdvosk::VoskModelResourceLoader::_handles_type(const StringName& p_type) const
{
    return p_type == StringName("VoskModel") || p_type == StringName("VoskSpeakerModel");
}

String gdvosk::VoskModelResourceLoader::_get_resource_type(const String& p_path) const
{
    if (p_path.get_extension() == "vosk")
    {
        return "VoskModel";
    }

    return "VoskSpeakerModel";
}

Variant gdvosk::VoskModelResourceLoader::_load
(
    const String& p_path,
    const String& p_original_path,
    bool p_use_sub_threads,
    int32_t p_cache_mode
) const
{
    auto extracted_models_root = String("user://gdvosk/models");

    auto make_root_dir = DirAccess::make_dir_recursive_absolute(extracted_models_root);
    if (make_root_dir != OK)
    {
        return make_root_dir;
    }

    auto model_name = p_path.get_file().replace("." + p_path.get_extension(), "");
    auto extracted_model_path = extracted_models_root.path_join(model_name);

    auto extracted_model_dir = DirAccess::open(extracted_model_path);
    if (!extracted_model_dir.is_valid())
    {
        // extract resource
        Ref<ZIPReader> reader;
        reader.instantiate();

        auto open = reader->open(p_path);
        if (open != OK)
        {
            return open;
        }

        auto files = reader->get_files();
        for (const auto& file : files)
        {
            if (file.ends_with("/"))
            {
                // just a folder, ignore
                continue;
            }

            auto output_path = extracted_models_root.path_join(file);
            if (FileAccess::file_exists(output_path))
            {
                continue;
            }

            auto output_folder = output_path.get_base_dir();

            auto make_output_folder = DirAccess::make_dir_recursive_absolute(output_folder);
            if (make_output_folder != OK)
            {
                return make_output_folder;
            }

            auto bytes = reader->read_file(file);
            auto output_file = FileAccess::open(output_path, FileAccess::ModeFlags::WRITE);
            if (!output_file.is_valid() || !output_file->is_open())
            {
                return ERR_FILE_CANT_WRITE;
            }

            output_file->store_buffer(bytes);
            output_file->close();
        }
    }

    auto type = p_path.get_extension();
    if (type == "vosk")
    {
        Ref<gdvosk::VoskModel> model;
        model.instantiate();

        model->load(extracted_model_path);

        return model;
    }

    if (type == "voskspk")
    {
        Ref<gdvosk::VoskSpeakerModel> model;
        model.instantiate();

        model->load(extracted_model_path);

        return model;
    }

    return ResourceFormatLoader::_load(p_path, p_original_path, p_use_sub_threads, p_cache_mode);
}

void gdvosk::VoskModelResourceLoader::_bind_methods()
{
}
