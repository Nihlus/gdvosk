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
    add_compile_options(kaldi-base
        -msimd128
    )
endif ()

FetchContent_Declare(
    kaldi
    GIT_REPOSITORY https://github.com/alphacep/kaldi
    GIT_TAG bc5baf14231660bd50b7d05788865b4ac6c34481 # "vosk" branch head
    OVERRIDE_FIND_PACKAGE
)

if (NOT APPLE)
    # OpenBLAS
    if (MSVC)
        # not enabled by default on MSVC with OpenBLAS
        enable_language(ASM)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "")
    endif ()

    set(CMAKE_POLICY_DEFAULT_CMP0054 OLD)
    set(CMAKE_POLICY_DEFAULT_CMP0048 NEW) # OK, BLAS sets version properties after project call

    set(ONLY_CBLAS 1 CACHE INTERNAL "")

    if ("${GODOT_ARCH}" STREQUAL "x86_32" OR "${GODOT_ARCH}" STREQUAL "x86_64")
        set(DYNAMIC_ARCH 1 CACHE INTERNAL "")
        set(TARGET NEHALEM CACHE INTERNAL "")
    elseif ("${GODOT_ARCH}" STREQUAL "wasm32")
        set(TARGET GENERIC CACHE INTERNAL "")
    endif ()

    set(USE_LOCKING 1 CACHE INTERNAL "")
    set(USE_THREAD 0 CACHE INTERNAL "")

    if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
        set(ARCH generic CACHE INTERNAL "")
    endif ()

    FetchContent_Declare(
        OpenBLAS
        GIT_REPOSITORY https://github.com/xianyi/OpenBLAS
        GIT_TAG v0.3.20
    )

    # CLAPACK
    FetchContent_Declare(
        clapack
        GIT_REPOSITORY https://github.com/alphacep/clapack
        GIT_TAG eb94c6c9e91808f48e346161c09ee9ce7770c065
    )
endif ()

# vosk
FetchContent_Declare(
    vosk-api
    GIT_REPOSITORY https://github.com/alphacep/vosk-api
    GIT_TAG v0.3.50
)

FetchContent_MakeAvailable(openfst)

if (NOT APPLE)
    FetchContent_MakeAvailable(OpenBLAS clapack)
endif ()

# Kaldi screws with the install prefix, so we'll have to work around this
set(OLD_CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

FetchContent_MakeAvailable(kaldi)

if (NOT APPLE)
    target_link_libraries(kaldi-base
        PRIVATE
            $<BUILD_INTERFACE:openblas>
            $<BUILD_INTERFACE:blas>
            $<BUILD_INTERFACE:f2c>
            $<BUILD_INTERFACE:lapack>
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

# fix up openfst linkage with kaldi
ExternalProject_Add(
    openfst
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_deps
        SOURCE_DIR ${openfst_SOURCE_DIR}
        BINARY_DIR ${openfst_BINARY_DIR}
        CONFIGURE_COMMAND bash -c "autoreconf -i ${openfst_SOURCE_DIR} && ${openfst_SOURCE_DIR}/configure CFLAGS='-g -O3' --prefix=${openfst_BINARY_DIR} --enable-static --enable-shared --enable-far --enable-ngram-fsts --enable-lookahead-fsts --with-pic --disable-bin"
        BUILD_COMMAND bash -c "make -j && make install"
        BUILD_BYPRODUCTS
            ${openfst_BINARY_DIR}/lib/libfst.a
            ${openfst_BINARY_DIR}/lib/libfstngram.a
)

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

