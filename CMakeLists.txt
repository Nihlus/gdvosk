# SPDX-License-Identifier: Unlicense

cmake_minimum_required(VERSION 3.22)

# Add paths to modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# Turn on link time optimization for everything
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

# Build universal lib on macOS
# Note that CMAKE_OSX_ARCHITECTURES must be set before project().
if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "")
endif ()

# Main project information
project(gdvosk
    LANGUAGES CXX
    VERSION 1.0.0
)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE ${CMAKE_CURRENT_LIST_DIR}/sample/addons)
endif()

include(godot-helpers)
include(ccache)

# emscripten needs some massaging
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

    set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-s SIDE_MODULE=1")
    set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-s SIDE_MODULE=1")
endif ()

# Create our library
add_library(${PROJECT_NAME} SHARED)

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

set(INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/")
message(STATUS ${INSTALL_DIR})

install(
    TARGETS ${PROJECT_NAME}
    LIBRARY
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

add_subdirectory(extern/godot-cpp)

set_target_properties(godot-cpp
    PROPERTIES
        CXX_VISIBILITY_PRESET hidden
)

add_subdirectory(thirdparty/vosk)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        godot-cpp
        Vosk::Vosk
)
