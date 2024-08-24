if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    find_file(Vosk_LIBRARY
        NAMES
            vosk.js
            libvosk.js
        PATHS
            ${CMAKE_CURRENT_LIST_DIR}/lib/${GODOT_SYSTEM_NAME}/${GODOT_ARCH}
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
        REQUIRED
    )

    add_library(Vosk INTERFACE IMPORTED GLOBAL)

    set_target_properties(Vosk
        PROPERTIES
            LINK_FLAGS "--js-library ${Vosk_LIBRARY}"
    )
else ()
    find_library(Vosk_LIBRARY
        NAMES
            vosk
            libvosk
        PATHS
            ${CMAKE_CURRENT_LIST_DIR}/lib/${GODOT_SYSTEM_NAME}/${GODOT_ARCH}
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
        REQUIRED
    )

    add_library(Vosk SHARED IMPORTED GLOBAL)

    if (MSVC)
        set(Vosk_LIB ${Vosk_LIBRARY})
        string(REPLACE .lib .dll Vosk_LIB ${Vosk_LIB})

        set_target_properties(Vosk
                PROPERTIES
                IMPORTED_IMPLIB "${Vosk_LIBRARY}"
                IMPORTED_LOCATION "${Vosk_LIB}"
        )
    else ()
        set_target_properties(Vosk
                PROPERTIES
                IMPORTED_LOCATION "${Vosk_LIBRARY}"
        )
    endif ()
endif ()


target_include_directories(Vosk
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

add_library(Vosk::Vosk ALIAS Vosk)

# Copy shared libraries
install(
        DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib/${GODOT_SYSTEM_NAME}/${GODOT_ARCH}/"
        DESTINATION "${INSTALL_DIR}/lib/${GODOT_SYSTEM_NAME}/${GODOT_ARCH}/"
        PATTERN ".*" EXCLUDE
)