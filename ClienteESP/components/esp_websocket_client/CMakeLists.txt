idf_component_register(
    SRCS "esp_websocket_client.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_http_client 
            mbedtls 
            esp_event 
            esp_wifi 
            esp_netif 
            lwip
            tcp_transport
            esp_timer
) 