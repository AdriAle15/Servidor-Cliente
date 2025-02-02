idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi
            nvs_flash
            driver
            cJSON
            mbedtls
            esp_websocket_client
)

set_source_files_properties(main.cpp 
    PROPERTIES 
    COMPILE_FLAGS "-x c++"
)
