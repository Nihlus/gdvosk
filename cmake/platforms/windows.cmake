if ("${GODOT_ARCH}" STREQUAL "x86_64")
    set(SYSTEM_TARGET "x86_64-w64-mingw32")
    set(SYSTEM_MARCH "x86-64")
elseif ("${GODOT_ARCH}" STREQUAL "x86_32")
    set(SYSTEM_TARGET "i686-w64-mingw32")
    set(SYSTEM_MARCH "i686")
else()
    message(FATAL_ERROR "${GODOT_ARCH} is an unsupported target architecture for ${GODOT_SYSTEM_NAME}")
endif ()

add_compile_options(
    -Wwrite-strings
    -static-libgcc
    -static-libstdc++
)

add_compile_definitions(
    WINDOWS_ENABLED
)
