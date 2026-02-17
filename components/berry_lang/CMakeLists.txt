# Berry language ESP-IDF component
#
# For pure idf.py builds, Berry source is expected as a git submodule:
#   git submodule add https://github.com/berry-lang/berry.git components/berry_lang/berry-lang
#   git submodule update --init
#
# For PlatformIO ESP-IDF builds, Berry is managed via lib_deps and this
# component registers as an empty placeholder (no submodule needed).

set(BERRY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/berry-lang")
set(PROJECT_BERRY_DIR "${CMAKE_SOURCE_DIR}/src/berry")

if(EXISTS "${BERRY_DIR}/src/berry.h")
    # Berry source available via git submodule — build the full component.

    file(GLOB BERRY_CORE_SRCS "${BERRY_DIR}/src/*.c")

    idf_component_register(
        SRCS
            ${BERRY_CORE_SRCS}
            "${PROJECT_BERRY_DIR}/be_port.c"
            "${PROJECT_BERRY_DIR}/be_modtab.c"
        INCLUDE_DIRS
            "${PROJECT_BERRY_DIR}"
            "${BERRY_DIR}/src"
            "${BERRY_DIR}/generate"
    )

    # Run Berry COC tool to regenerate constant tables if marker is missing.
    set(BERRY_GENERATE_DIR "${BERRY_DIR}/generate")
    set(BERRY_MARKER "${BERRY_GENERATE_DIR}/be_const_strtab.h")
    set(BERRY_COC_TOOL "${BERRY_DIR}/tools/coc/coc")

    if(NOT EXISTS "${BERRY_MARKER}")
        find_package(Python3 COMPONENTS Interpreter QUIET)
        if(Python3_FOUND)
            add_custom_command(
                OUTPUT "${BERRY_MARKER}"
                COMMAND ${Python3_EXECUTABLE} "${BERRY_COC_TOOL}"
                    -o "${BERRY_GENERATE_DIR}"
                    "${BERRY_DIR}/src"
                    "${BERRY_DIR}/default"
                    -c "${PROJECT_BERRY_DIR}/berry_conf.h"
                WORKING_DIRECTORY "${BERRY_DIR}"
                COMMENT "Berry: generating constant tables"
            )
            add_custom_target(berry_generate DEPENDS "${BERRY_MARKER}")
            add_dependencies(${COMPONENT_LIB} berry_generate)
        else()
            message(WARNING "Python3 not found — cannot regenerate Berry constant tables. "
                            "Using pre-generated files from the Berry repository.")
        endif()
    endif()

else()
    # Berry source not found — register an empty component.
    # PlatformIO provides Berry via lib_deps; this placeholder satisfies
    # the REQUIRES dependency in src/CMakeLists.txt.
    idf_component_register()
endif()
