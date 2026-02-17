# Main application component for ESP-IDF builds.
#
# For PlatformIO ESP-IDF: Berry is provided via lib_deps; port files
# (src/berry/*.c) are compiled here alongside application code.
#
# For pure idf.py: Berry is built as the berry_lang component (requires
# git submodule); port files are compiled there, so they are excluded
# from this component to avoid duplicate symbols.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

# When Berry submodule is present, port files are compiled inside the
# berry_lang component — exclude them here to avoid duplicate symbols.
set(BERRY_SUBMODULE "${CMAKE_SOURCE_DIR}/components/berry_lang/berry-lang/src/berry.h")
if(EXISTS "${BERRY_SUBMODULE}")
    list(FILTER app_sources EXCLUDE REGEX ".*/berry/.*\\.c$")
endif()

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS "." "berry"
    REQUIRES berry_lang json esp_http_server app_update
)
