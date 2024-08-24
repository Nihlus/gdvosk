# generate import library
if (MSVC)
    find_file(Vosk_SHARED_LIBRARY
        NAMES
            vosk
            libvosk
        PATHS
            ${CMAKE_CURRENT_LIST_DIR}/lib/${GODOT_SYSTEM_NAME}/${GODOT_ARCH}
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
        REQUIRED
    )

    set(Vosk_DEFINITION_FILE ${Vosk_SHARED_LIBRARY})
    string(REPLACE .dll .def Vosk_DEFINITION_FILE ${Vosk_DEFINITION_FILE})

    set(Vosk_LIB_FILE ${Vosk_SHARED_LIBRARY})
    string(REPLACE .dll .lib Vosk_LIB_FILE ${Vosk_LIB_FILE})

    execute_process(dumpbin /exports ${Vosk_SHARED_LIBRARY} > DEF_FILE.def)
    execute_process(lib /def:${Vosk_DEFINITION_FILE} /out:${Vosk_LIB_FILE} /machine:${CMAKE_SYSTEM_PROCESSOR})
endif ()

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
set_target_properties(Vosk
    PROPERTIES
        IMPORTED_LOCATION "${Vosk_LIBRARY}"
)

target_include_directories(Vosk
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

add_library(Vosk::Vosk ALIAS Vosk)

# Copy shared libraries
install(
        DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib/"
        DESTINATION "${INSTALL_DIR}/lib/"
        PATTERN ".*" EXCLUDE
)