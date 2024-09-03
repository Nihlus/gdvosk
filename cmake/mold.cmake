# Enable the use of Mold to speed up linking
find_program(mold
    NAMES mold
)

if (mold AND NOT NO_MOLD)
    add_link_options(
        LINKER:-fuse-ld=mold
    )
endif ()
