idf_component_register(
    SRCS "mqtts.c"
    INCLUDE_DIRS  "." 
    REQUIRES mqtt protocol_examples_common nvs_flash json
    EMBED_TXTFILES  ca.crt client.crt client.key)