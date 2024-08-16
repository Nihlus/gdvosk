// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#include "gdvosk.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "SpeechRecognizer.h"

using namespace godot;

void initialize_gdvosk_module(ModuleInitializationLevel p_level) 
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) 
    {
        return;
    }

    GDREGISTER_CLASS(SpeechRecognizer);
}

void uninitialize_gdvosk_module(ModuleInitializationLevel p_level) 
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) 
    {
        return;
    }
}

extern "C" {
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

