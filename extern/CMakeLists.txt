include(FetchContent)
include(ExternalProject)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# Godot
add_subdirectory(godot-cpp)
set_target_properties(godot-cpp
    PROPERTIES
        CXX_VISIBILITY_PRESET hidden
)

# openfst
set(OPENFST_BIN OFF CACHE INTERNAL "")
set(OPENFST_FAR ON CACHE INTERNAL "")
set(OPENFST_NGRAM_FSTS ON CACHE INTERNAL "")
set(OPENFST_LOOKAHEAD_FSTS ON CACHE INTERNAL "")

FetchContent_Declare(
    openfst
    GIT_REPOSITORY https://github.com/Nihlus/openfst
    GIT_TAG d7705ac568807c84361f68f2a205b7e6e81b3fb0
    OVERRIDE_FIND_PACKAGE
)

# Kaldi
set(CMAKE_POLICY_DEFAULT_CMP0146 OLD)
set(CMAKE_POLICY_DEFAULT_CMP0148 OLD)
set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

set(KALDI_BUILD_EXE OFF CACHE INTERNAL "")
set(KALDI_BUILD_TEST OFF CACHE INTERNAL "")

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")

endif ()

if (MINGW)
    FetchContent_Declare(
        kaldi
        GIT_REPOSITORY https://github.com/alphacep/kaldi
        GIT_TAG 8b7fecf92b329b7253d9d1cd97898c8b13f9aef3 # "vosk-android" branch head
        OVERRIDE_FIND_PACKAGE
    )
elseif ("${GODOT_ARCH}" STREQUAL "wasm32")
    FetchContent_Declare(
        kaldi
        GIT_REPOSITORY https://github.com/alphacep/kaldi
        GIT_TAG bc5baf14231660bd50b7d05788865b4ac6c34481 # "vosk" branch where the WASM patch applies
        PATCH_COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/patches/kaldi.patch
        UPDATE_DISCONNECTED 1
        OVERRIDE_FIND_PACKAGE
    )
else ()
    FetchContent_Declare(
        kaldi
        GIT_REPOSITORY https://github.com/alphacep/kaldi
        GIT_TAG bc5baf14231660bd50b7d05788865b4ac6c34481 # "vosk" branch head
        OVERRIDE_FIND_PACKAGE
    )
endif ()

if (NOT APPLE)
    # OpenBLAS
    if (MSVC)
        # not enabled by default on MSVC with OpenBLAS
        enable_language(ASM)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "")
    endif ()

    set(CMAKE_POLICY_DEFAULT_CMP0054 OLD)
    set(CMAKE_POLICY_DEFAULT_CMP0048 NEW) # OK, BLAS sets version properties after project call

    set(BUILD_TESTING OFF CACHE INTERNAL "")
    set(C_LAPACK ON CACHE INTERNAL "")

    if ("${GODOT_ARCH}" STREQUAL "x86_32" OR "${GODOT_ARCH}" STREQUAL "x86_64")
        if (ANDROID)
            set(TARGET ATOM CACHE INTERNAL "")
            set(ARM_SOFTFP_ABI 1 CACHE INTERNAL "")
        else ()
            if ("${GODOT_ARCH}" STREQUAL "x86_64")
                set(DYNAMIC_ARCH 1 CACHE INTERNAL "")
                set(TARGET NEHALEM CACHE INTERNAL "")
            else ()
                set(DYNAMIC_ARCH 0 CACHE INTERNAL "")
                set(TARGET NEHALEM CACHE INTERNAL "")
            endif ()
        endif ()

        if ("${GODOT_ARCH}" STREQUAL "x86_64")
            set(BINARY 64 CACHE INTERNAL "")
        else ()
            set(BINARY 32 CACHE INTERNAL "")
        endif ()
    elseif ("${GODOT_ARCH}" STREQUAL "arm32")
        set(TARGET ARMV7 CACHE INTERNAL "")
        if ("${GODOT_SYSTEM_NAME}" STREQUAL "android")
            set(ARM_SOFTFP_ABI 1 CACHE INTERNAL "")
        endif ()
    elseif ("${GODOT_ARCH}" STREQUAL "arm64")
        set(TARGET ARMV8 CACHE INTERNAL "")
    elseif ("${GODOT_ARCH}" STREQUAL "wasm32")
        set(TARGET RISCV64_GENERIC CACHE INTERNAL "")
    endif ()

    set(USE_LOCKING 1 CACHE INTERNAL "")
    set(USE_THREAD 0 CACHE INTERNAL "")

    FetchContent_Declare(
        OpenBLAS
            GIT_REPOSITORY https://github.com/xianyi/OpenBLAS
            GIT_TAG v0.3.28
    )

    if (ANDROID)
        add_compile_options(
            -Wno-format-security
        )
    endif ()
endif ()

# vosk
FetchContent_Declare(
    vosk-api
    GIT_REPOSITORY https://github.com/alphacep/vosk-api
    GIT_TAG v0.3.50
)

FetchContent_MakeAvailable(openfst)

FetchContent_GetProperties(openfst
    BINARY_DIR OPENFST_BINARY_DIR
)

if (ANDROID OR MINGW)
    target_compile_definitions(fst
        PUBLIC
            FST_NO_DYNAMIC_LINKING
    )
endif ()

if (NOT APPLE)
    FetchContent_MakeAvailable(OpenBLAS)
endif ()

# Kaldi screws with the install prefix, so we'll have to work around this
set(OLD_CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

FetchContent_MakeAvailable(kaldi)

if (NOT APPLE)
    target_compile_definitions(kaldi-base
        PUBLIC
            HAVE_OPENBLAS
    )

    target_link_libraries(kaldi-base
        PRIVATE
            $<BUILD_INTERFACE:openblas>
    )
endif ()

unset(CMAKE_INSTALL_PREFIX)
unset(CMAKE_INSTALL_PREFIX CACHE)

set(CMAKE_INSTALL_PREFIX ${OLD_CMAKE_INSTALL_PREFIX})

FetchContent_MakeAvailable(vosk-api)

# fix up vosk
target_sources(vosk
    PRIVATE
        ${vosk-api_SOURCE_DIR}/src/postprocessor.cc
)

target_include_directories(vosk
    INTERFACE
        ${vosk-api_SOURCE_DIR}/src
)

if (ANDROID)
    target_link_libraries(vosk
        PRIVATE
            log
    )
endif ()


