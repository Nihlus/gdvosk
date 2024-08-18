// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#include "VoskModel.h"

#include <godot_cpp/classes/project_settings.hpp>

using namespace gdvosk;
using namespace godot;

gdvosk::VoskModel::~VoskModel()
{
    if (_model != nullptr)
    {
        vosk_model_free(_model);
        _model = nullptr;
    }
}

int gdvosk::VoskModel::find_word(const String& word) const
{
    return vosk_model_find_word(_model, word.ascii());
}

Error gdvosk::VoskModel::load(const String& path)
{
    auto globalized_path = ProjectSettings::get_singleton()->globalize_path(path);
    if (globalized_path == "")
    {
        return ERR_FILE_BAD_PATH;
    }

    auto model = vosk_model_new(globalized_path.ascii());
    if (model == nullptr)
    {
        return ERR_FILE_CORRUPT;
    }

    if (_model != nullptr)
    {
        vosk_model_free(_model);
    }

    _model = model;
    return OK;
}

::VoskModel* gdvosk::VoskModel::get_ptr() const
{
    return _model;
}
