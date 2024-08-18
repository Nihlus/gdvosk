// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT


#include "VoskSpeakerModel.h"

#include <godot_cpp/classes/project_settings.hpp>

using namespace gdvosk;
using namespace godot;

VoskSpeakerModel::~VoskSpeakerModel()
{
    if (_model != nullptr)
    {
        vosk_spk_model_free(_model);
    }
}

Error VoskSpeakerModel::load(const String& path)
{
    auto globalized_path = ProjectSettings::get_singleton()->globalize_path(path);
    if (globalized_path == "")
    {
        return ERR_FILE_BAD_PATH;
    }

    auto model = vosk_spk_model_new(globalized_path.ascii());
    if (model == nullptr)
    {
        return ERR_FILE_CORRUPT;
    }

    if (_model != nullptr)
    {
        vosk_spk_model_free(_model);
    }

    _model = model;
    return OK;
}

VoskSpkModel* VoskSpeakerModel::get_ptr() const
{
    return _model;
}
