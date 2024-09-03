set(NO_MOLD ON)

if ("${GODOT_ARCH}" STREQUAL "arm64")
    set(SYSTEM_TARGET "aarch64-linux-android${ANDROID_NATIVE_API_LEVEL}")
    set(SYSTEM_MARCH "armv8-a")
elseif ("${GODOT_ARCH}" STREQUAL "arm32")
    set(SYSTEM_TARGET "armv7a-linux-androideabi${ANDROID_NATIVE_API_LEVEL}")
    set(SYSTEM_MARCH "armv7-a")
    add_compile_options(
        -mfpu=neon
        -mfloat-abi=softfp
    )
elseif ("${GODOT_ARCH}" STREQUAL "x86_64")
    set(SYSTEM_TARGET "x86_64-linux-android${ANDROID_NATIVE_API_LEVEL}")
    set(SYSTEM_MARCH "x86-64")
elseif ("${GODOT_ARCH}" STREQUAL "x86_32")
    set(SYSTEM_TARGET "i686-linux-android${ANDROID_NATIVE_API_LEVEL}")
    set(SYSTEM_MARCH "i686")
    add_compile_options(
        -mstackrealign
    )
else ()
    message(FATAL_ERROR "${GODOT_ARCH} is an unsupported target architecture for ${GODOT_SYSTEM_NAME}")
endif ()

add_compile_options(
    --target=${SYSTEM_TARGET}
    -march=${SYSTEM_MARCH}
)

add_compile_definitions(
    ANDROID_ENABLED
    UNIX_ENABLED
)
