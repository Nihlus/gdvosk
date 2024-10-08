# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.22)

# Add paths to modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# Always build position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Turn on link time optimization for everything
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED)
if (IPO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif ()

# Default to hidden all the time
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)

# Main project information
project(gdvosk
    LANGUAGES CXX
    VERSION 1.0.0
)

include(ccache)

include(godot-helpers)

include(platforms/${GODOT_SYSTEM_NAME} OPTIONAL)
include(mold)
include(trim-sections)

if (NOT "${GODOT_SYSTEM_NAME}" STREQUAL "web")
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        if ("${GODOT_SYSTEM_NAME}" STREQUAL "windows")
            add_compile_options(
                -Og # we have major size issues when compiling for Windows, so we need some optimizations even in debug mode
            )
        else ()
            add_compile_options(
                -O0
                -ggdb3
            )
        endif ()
    else ()
        add_compile_options(
            -O3
        )
    endif ()
endif ()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(GDVOSK_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/sample/addons)
else ()
    set(GDVOSK_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif()

# Create our library
if ("${GODOT_SYSTEM_NAME}" STREQUAL "ios")
    add_library(${PROJECT_NAME} STATIC)
else ()
    add_library(${PROJECT_NAME} SHARED)
endif ()

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_17
)

set(LIB_DIR "lib/${GODOT_SYSTEM_NAME}/${GODOT_ARCH}")

# BUILD_OUTPUT_DIR is where we put the resulting library (in the build directory)
set(BUILD_OUTPUT_DIR "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/")

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN true
        RUNTIME_OUTPUT_DIRECTORY "${BUILD_OUTPUT_DIR}/${LIB_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${BUILD_OUTPUT_DIR}/${LIB_DIR}"
)

if (NOT DEFINED CMAKE_DEBUG_POSTFIX)
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            DEBUG_POSTFIX "-d"
    )
endif ()

# Copy over additional files from the support_files directory
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/support-files"
    ${BUILD_OUTPUT_DIR}
)

# Create and include version info file from git
include(GitVersionInfo)

add_subdirectory(src)

set(INSTALL_DIR "${GDVOSK_INSTALL_PREFIX}/${PROJECT_NAME}/")

message(STATUS "Installing to: ${INSTALL_DIR}")

install(
    TARGETS ${PROJECT_NAME}
    LIBRARY
        DESTINATION ${INSTALL_DIR}/${LIB_DIR}
    ARCHIVE
        DESTINATION ${INSTALL_DIR}/${LIB_DIR}
    RUNTIME
        DESTINATION ${INSTALL_DIR}/${LIB_DIR}
)

# Copy over support files
install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/support-files/"
        DESTINATION ${INSTALL_DIR}
        PATTERN ".*" EXCLUDE
)

add_subdirectory(gdextension)

# godot-cpp
# From here: https://github.com/godotengine/godot-cpp
if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/extern/godot-cpp/Makefile")
    message(
        FATAL_ERROR
        "[${PROJECT_NAME}] The godot-cpp submodule was not downloaded. Please update submodules: git submodule update --init --recursive."
    )
endif ()

set(GODOT_CPP_SYSTEM_HEADERS ON CACHE BOOL "" FORCE)

option(BUILD_SHARED_LIBS "Build and link with libraries." OFF)

add_subdirectory(extern EXCLUDE_FROM_ALL)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        godot-cpp
        vosk
)
