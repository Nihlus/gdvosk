option(WASM_ENABLE_THREADS "Enables thread support in WASM builds" ON)

set(NO_MOLD ON)

if (NOT "${GODOT_ARCH}" STREQUAL "wasm32")
    message(FATAL_ERROR "${GODOT_ARCH} is an unsupported target architecture for ${GODOT_SYSTEM_NAME}")
endif ()

set(SYSTEM_TARGET "wasm32-unknown-wasi")


add_compile_options(
    -msimd128
)

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-sSIDE_MODULE=1")
set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-sSIDE_MODULE=1")

# other stuff ported from Godot
if (WASM_ENABLE_THREADS)
    add_compile_options(
        -pthread
    )

    add_link_options(
        -pthread
    )
endif ()

add_compile_options(
    -sSUPPORT_LONGJMP=wasm
)

add_link_options(
    -sSUPPORT_LONGJMP=wasm
    -sWASM_BIGINT
)

add_compile_definitions(
    WEB_ENABLED
    UNIX_ENABLED
)