// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#include "gdvosk.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

#include "SpeechRecognizer.h"
#include "vosk/VoskModelResourceLoader.h"
#include "vosk/VoskRecognizer.h"

using namespace godot;
using namespace gdvosk;

namespace gdvosk
{
    static Ref<VoskModelResourceLoader> _model_loader = nullptr;
}

void initialize_gdvosk_module(ModuleInitializationLevel p_level) 
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) 
    {
        return;
    }

    GDREGISTER_CLASS(VoskModelResourceLoader);

    _model_loader.instantiate();
    ResourceLoader::get_singleton()->add_resource_format_loader(_model_loader);

    GDREGISTER_CLASS(gdvosk::VoskModel);
    GDREGISTER_CLASS(VoskSpeakerModel);

    GDREGISTER_CLASS(gdvosk::VoskRecognizer);

    GDREGISTER_CLASS(SpeechRecognizer);
}

void uninitialize_gdvosk_module(ModuleInitializationLevel p_level) 
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) 
    {
        return;
    }

    _model_loader.unref();
}

extern "C"
{
    GDExtensionBool GDE_EXPORT gdvosk_library_init
    (
        GDExtensionInterfaceGetProcAddress p_get_proc_address, 
        const GDExtensionClassLibraryPtr p_library, 
        GDExtensionInitialization* r_initialization
    ) 
    {
        GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_gdvosk_module);
        init_obj.register_terminator(uninitialize_gdvosk_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}

