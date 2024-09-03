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
set(OPENFST_EXTRA_FLAGS "")
set(OPENFST_EXTRA_CONFIGURE_FLAGS "")
set(OPENFST_TARGET "")

if (CMAKE_CROSSCOMPILING)
    if (ANDROID)
        if ("${GODOT_ARCH}" STREQUAL "arm64")
            set(OPENFST_TARGET "aarch64-linux-android")
        elseif ("${GODOT_ARCH}" STREQUAL "arm32")
            set(OPENFST_TARGET "armv7a-linux-androideabi")
        elseif ("${GODOT_ARCH}" STREQUAL "x86_64")
            set(OPENFST_TARGET "x86_64-linux-android")
        elseif ("${GODOT_ARCH}" STREQUAL "x86_32")
            set(OPENFST_TARGET "i686-linux-android")
        endif ()

        add_compile_options(
            --target=${OPENFST_TARGET}${ANDROID_PLATFORM}
        )

        set(OPENFST_EXTRA_FLAGS "${OPENFST_EXTRA_FLAGS} -DFST_NO_DYNAMIC_LINKING")
        set(OPENFST_EXTRA_CONFIGURE_FLAGS "${OPENFST_EXTRA_CONFIGURE_FLAGS} --host=${OPENFST_TARGET}")
    elseif (MINGW)
        set(OPENFST_EXTRA_FLAGS "${OPENFST_EXTRA_FLAGS} -DFST_NO_DYNAMIC_LINKING")
        set(OPENFST_EXTRA_CONFIGURE_FLAGS "${OPENFST_EXTRA_CONFIGURE_FLAGS} --host=x86_64-w64-mingw32")
    elseif ("${GODOT_ARCH}" STREQUAL "wasm32")
        set(OPENFST_EXTRA_CONFIGURE_FLAGS "${OPENFST_EXTRA_CONFIGURE_FLAGS} --host=wasm32-unknown-wasi")
    endif ()
endif ()

if (ANDROID OR MINGW)
    add_compile_definitions(
        FST_NO_DYNAMIC_LINKING
    )
endif ()

if (APPLE)
    set(OPENFST_EXTRA_FLAGS "${OPENFST_EXTRA_FLAGS} -arch x86_64 -arch arm64")
endif ()

FetchContent_Declare(
    openfst
    GIT_REPOSITORY https://github.com/alphacep/openfst
    GIT_TAG 18e94e63870ebcf79ebb42b7035cd3cb626ec090
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
    add_compile_options(
        -msimd128
    )
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
        set(DYNAMIC_ARCH 1 CACHE INTERNAL "")
        set(TARGET NEHALEM CACHE INTERNAL "")
    elseif ("${GODOT_ARCH}" STREQUAL "arm32")
        set(TARGET ARMV7 CACHE INTERNAL "")
        set(ARM_SOFTFP_ABI 1 CACHE INTERNAL "")
    elseif ("${GODOT_ARCH}" STREQUAL "arm64")
        set(TARGET ARMV8 CACHE INTERNAL "")
        set(ARM_SOFTFP_ABI 1 CACHE INTERNAL "")
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

        if ("${GODOT_ARCH}" STREQUAL "arm32")
            add_compile_options(
                -mfloat-abi=softfp
                -mfpu=neon
            )
        endif ()
    endif ()
endif ()

# vosk
FetchContent_Declare(
    vosk-api
    GIT_REPOSITORY https://github.com/alphacep/vosk-api
    GIT_TAG v0.3.50
)

FetchContent_MakeAvailable(openfst)

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

get_directory_property(ALL_COMPILE_FLAGS COMPILE_OPTIONS)
string(REPLACE ";" " " ALL_COMPILE_FLAGS "${ALL_COMPILE_FLAGS}")

get_directory_property(ALL_LINK_FLAGS LINK_OPTIONS)
string(REPLACE ";" " " ALL_LINK_FLAGS "${ALL_LINK_FLAGS}")
string(REPLACE "LINKER:" "" ALL_LINK_FLAGS "${ALL_LINK_FLAGS}")
string(REPLACE "," " " ALL_LINK_FLAGS "${ALL_LINK_FLAGS}")

string(REPLACE "--gc-sections" "" ALL_LINK_FLAGS "${ALL_LINK_FLAGS}")
string(REPLACE "--as-needed" "" ALL_LINK_FLAGS "${ALL_LINK_FLAGS}")
string(REPLACE "--exclude-libs=ALL" "" ALL_LINK_FLAGS "${ALL_LINK_FLAGS}")

# fix up openfst linkage with kaldi
if ("${GODOT_ARCH}" STREQUAL "wasm32")
    ExternalProject_Add(
        openfst
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_deps
            SOURCE_DIR ${openfst_SOURCE_DIR}
            BINARY_DIR ${openfst_BINARY_DIR}
            CONFIGURE_COMMAND bash -c "autoreconf -i ${openfst_SOURCE_DIR} && ${EMSCRIPTEN_ROOT_PATH}/emconfigure ${openfst_SOURCE_DIR}/configure CFLAGS='${OPENFST_EXTRA_FLAGS} ${ALL_COMPILE_FLAGS} -g -O3' CXXFLAGS='${OPENFST_EXTRA_FLAGS} ${ALL_COMPILE_FLAGS} -g -O3' LDFLAGS='${ALL_LINK_FLAGS}' --prefix=${openfst_BINARY_DIR} --disable-shared --enable-static --enable-far --enable-ngram-fsts --enable-lookahead-fsts --with-pic --disable-bin ${OPENFST_EXTRA_CONFIGURE_FLAGS}"
            BUILD_COMMAND bash -c "${EMSCRIPTEN_ROOT_PATH}/emmake make -j && ${EMSCRIPTEN_ROOT_PATH}/emmake make install"
            BUILD_BYPRODUCTS
            ${openfst_BINARY_DIR}/lib/libfst.a
            ${openfst_BINARY_DIR}/lib/libfstngram.a
    )
else ()
    set(OPENFST_EXPLICIT_COMPILER "")
    if (NOT APPLE)
        set (OPENFST_EXPLICIT_COMPILER "CC='${CMAKE_C_COMPILER}' CXX='${CMAKE_CXX_COMPILER}'")
    endif ()

    ExternalProject_Add(
        openfst
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_deps
            SOURCE_DIR ${openfst_SOURCE_DIR}
            BINARY_DIR ${openfst_BINARY_DIR}
            CONFIGURE_COMMAND bash -c "autoreconf -i ${openfst_SOURCE_DIR} && ${openfst_SOURCE_DIR}/configure ${OPENFST_EXPLICIT_COMPILER} CFLAGS='${OPENFST_EXTRA_FLAGS} ${ALL_COMPILE_FLAGS} -g -O3' CXXFLAGS='${OPENFST_EXTRA_FLAGS} ${ALL_COMPILE_FLAGS} -g -O3' LDFLAGS='${ALL_LINK_FLAGS}' --prefix=${openfst_BINARY_DIR} --disable-shared --enable-static --enable-far --enable-ngram-fsts --enable-lookahead-fsts --with-pic --disable-bin ${OPENFST_EXTRA_CONFIGURE_FLAGS}"
            BUILD_COMMAND bash -c "make -j && make install"
            BUILD_BYPRODUCTS
                ${openfst_BINARY_DIR}/lib/libfst.a
                ${openfst_BINARY_DIR}/lib/libfstngram.a
    )
endif ()

add_library(fst STATIC IMPORTED GLOBAL)
add_library(fstngram STATIC IMPORTED GLOBAL)

add_dependencies(fst openfst)
add_dependencies(fstngram openfst)

set_target_properties(fst
    PROPERTIES
        IMPORTED_LOCATION ${openfst_BINARY_DIR}/lib/libfst.a
)

set_target_properties(fstngram
    PROPERTIES
        IMPORTED_LOCATION ${openfst_BINARY_DIR}/lib/libfstngram.a
)

file(MAKE_DIRECTORY ${openfst_BINARY_DIR}/include)
target_include_directories(fst
    INTERFACE
        ${openfst_BINARY_DIR}/include
)

target_include_directories(fstngram
        INTERFACE
        ${openfst_BINARY_DIR}/include
)

target_link_directories(fst
    INTERFACE
        ${openfst_BINARY_DIR}/lib
        ${openfst_BINARY_DIR}/lib/fst
)

target_link_directories(fstngram
    INTERFACE
        ${openfst_BINARY_DIR}/lib
        ${openfst_BINARY_DIR}/lib/fst
)

add_dependencies(kaldi-base openfst)
target_compile_features(kaldi-base
    PUBLIC
        cxx_std_17
)
