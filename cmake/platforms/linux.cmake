if ("${GODOT_ARCH}" STREQUAL "arm64")
    set(SYSTEM_TARGET "aarch64-linux-gnu")
    set(SYSTEM_MARCH "armv8-a")
elseif ("${GODOT_ARCH}" STREQUAL "arm32")
    set(SYSTEM_TARGET "armv7a-linux-gnu")
    set(SYSTEM_MARCH "armv7-a")
elseif ("${GODOT_ARCH}" STREQUAL "x86_64")
    set(SYSTEM_TARGET "x86_64-linux-gnu")
    set(SYSTEM_MARCH "x86-64")
elseif ("${GODOT_ARCH}" STREQUAL "x86_32")
    set(SYSTEM_TARGET "i686-linux-gnu")
    set(SYSTEM_MARCH "i686")
endif ()

add_compile_options(
    -Wwrite-strings
)

add_compile_options(
    --target=${SYSTEM_TARGET}
    -march=${SYSTEM_MARCH}
)

add_link_options(
    LINKER:--target=${SYSTEM_TARGET}
    LINKER:-march=${SYSTEM_MARCH}
)

add_compile_definitions(
    LINUX_ENABLED
    UNIX_ENABLED
)