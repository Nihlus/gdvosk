set(NO_MOLD ON)

if (NOT "${GODOT_ARCH}" STREQUAL "universal")
    message(FATAL_ERROR "${GODOT_ARCH} is an unsupported target architecture for ${GODOT_SYSTEM_NAME}")
endif ()

add_compile_definitions(
    IOS_ENABLED
    UNIX_ENABLED
)
