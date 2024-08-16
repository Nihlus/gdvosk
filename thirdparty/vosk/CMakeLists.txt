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