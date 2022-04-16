idf_component_register(
    SRCS "smart_config.c"
    INCLUDE_DIRS  "."
    REQUIRES wpa_supplicant nvs_flash)