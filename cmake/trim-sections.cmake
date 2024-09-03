if (NOT APPLE)
    add_compile_options(
        -ffunction-sections
        -fdata-sections
    )

    if (NOT "${GODOT_SYSTEM_NAME}" STREQUAL "web")
        add_link_options(
            LINKER:--gc-sections
            LINKER:--as-needed
            LINKER:--exclude-libs=ALL
        )
    else ()
        add_link_options(
            LINKER:--gc-sections
        )
    endif ()
else()
    add_link_options(
            LINKER:-dead_strip
    )
endif()
