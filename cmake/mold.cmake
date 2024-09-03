# Enable the use of Mold to speed up linking
find_program(mold
    NAMES mold
)

if (mold AND NOT NO_MOLD)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_link_options(
            LINKER:-fuse-ld=mold
        )
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        if (CMAKE_CROSSCOMPILING)
            add_compile_options(
                -B/usr/libexec/mold
            )
        else ()
            add_link_options(
                LINKER:-fuse-ld=mold
            )
        endif ()
    endif ()
endif ()
