
FILE(GLOB_RECURSE sensor_source
${CMAKE_CURRENT_SOURCE_DIR}/ct/*.c
${CMAKE_CURRENT_SOURCE_DIR}/humidity_temperature/*.c
${CMAKE_CURRENT_SOURCE_DIR}/light/*.c
${CMAKE_CURRENT_SOURCE_DIR}/ph_ec/*.c
${CMAKE_CURRENT_SOURCE_DIR}/soil_moisture/*.c
${CMAKE_CURRENT_SOURCE_DIR}/water_level/*.c
${CMAKE_CURRENT_SOURCE_DIR}/water_flow/*.c
)


idf_component_register(
    SRCS ${sensor_source}
    INCLUDE_DIRS  "."
    REQUIRES )