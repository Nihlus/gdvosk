
set(GODOT_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
string(TOLOWER ${GODOT_SYSTEM_NAME} GODOT_SYSTEM_NAME)
string(REPLACE darwin macos GODOT_SYSTEM_NAME ${GODOT_SYSTEM_NAME})

set(GODOT_ARCH ${CMAKE_SYSTEM_PROCESSOR})
string(TOLOWER ${GODOT_ARCH} GODOT_ARCH)
string(REPLACE amd64 x86_64 GODOT_ARCH ${GODOT_ARCH})
string(REPLACE i686 x86_32 GODOT_ARCH ${GODOT_ARCH})
string(REGEX REPLACE "^x86$" x86_32 GODOT_ARCH ${GODOT_ARCH})
string(REGEX REPLACE "^aarch64$" arm64 GODOT_ARCH ${GODOT_ARCH})
string(REGEX REPLACE "^armv7-a$" arm32 GODOT_ARCH ${GODOT_ARCH})
string(REGEX REPLACE "^arm$" arm32 GODOT_ARCH ${GODOT_ARCH})

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    set(GODOT_SYSTEM_NAME "web")
    set(GODOT_ARCH "wasm32")
endif ()

if (APPLE)
    set(GODOT_ARCH "universal")
endif ()
