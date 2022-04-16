
FILE(GLOB_RECURSE controller_sources
${CMAKE_CURRENT_SOURCE_DIR}/fan/*.c
${CMAKE_CURRENT_SOURCE_DIR}/feeder/*.c
${CMAKE_CURRENT_SOURCE_DIR}/led/*.c
${CMAKE_CURRENT_SOURCE_DIR}/motor/*.c
${CMAKE_CURRENT_SOURCE_DIR}/valve/*.c
)


idf_component_register(
    SRCS ${controller_sources}
    INCLUDE_DIRS  "."
    REQUIRES 74hc595)