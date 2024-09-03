set(NO_MOLD ON)

if (NOT "${GODOT_ARCH}" STREQUAL "universal")
    message(FATAL_ERROR "${GODOT_ARCH} is an unsupported target architecture for ${GODOT_SYSTEM_NAME}")
endif ()

add_link_options(
    LINKER:-framework,Cocoa
    LINKER:-undefined,dynamic_lookup
)

add_compile_definitions(
    MACOS_ENABLED
    UNIX_ENABLED
)
