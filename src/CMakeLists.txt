# SPDX-License-Identifier: Unlicense

target_sources( ${PROJECT_NAME}
    PRIVATE
        gdvosk.h
	gdvosk.cpp
)

target_include_directories( ${PROJECT_NAME}
    PRIVATE
        "src"
)
