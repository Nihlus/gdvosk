# SPDX-License-Identifier: Unlicense

add_custom_target(gdextension
    SOURCES
        template.gdextension.in
)

add_dependencies(${PROJECT_NAME} gdextension)

# Generate our project's .gdextension file from the template
configure_file(template.gdextension.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}.gdextension)

# Install the gdextension file from the build directory
install(
    FILES ${BUILD_OUTPUT_DIR}/${PROJECT_NAME}.gdextension
    DESTINATION ${INSTALL_DIR}
)
