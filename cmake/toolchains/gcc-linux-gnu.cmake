option(TOOLCHAIN_PREFIX "The architecture prefix to use for the GCC toolchain." "x86_64-linux-gnu")

set(CMAKE_SYSTEM_NAME Linux)

# cross compilers to use for C, C++
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)

execute_process(
    COMMAND ${CMAKE_C_COMPILER} -dumpmachine
    OUTPUT_VARIABLE GCC_DUMPMACHINE
)

string(REGEX REPLACE "-.+$" "" CMAKE_SYSTEM_PROCESSOR "${GCC_DUMPMACHINE}")

# target environment on the build host system
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

# modify default behavior of FIND_XXX() commands
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
