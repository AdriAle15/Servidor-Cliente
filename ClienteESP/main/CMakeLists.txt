idf_component_register(
    SRCS "tcp_client_main.c"
    INCLUDE_DIRS "."
    REQUIRES esp_websocket_client nvs_flash protocol_examples_common esp_wifi driver json
)