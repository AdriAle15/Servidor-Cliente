#include <string.h>
#include "esp_websocket_client.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_transport.h"
#include "esp_transport_tcp.h"
#include "esp_transport_ssl.h"

static const char *TAG = "WEBSOCKET_CLIENT";

ESP_EVENT_DEFINE_BASE(WEBSOCKET_EVENTS);

struct esp_websocket_client {
    esp_websocket_client_config_t config;
    int sock;
    bool is_connected;
    TaskHandle_t task_handle;
    QueueHandle_t event_queue;
    esp_event_handler_t event_handler;
    void *event_handler_arg;
};

static void websocket_client_task(void *pv) {
    esp_websocket_client_handle_t client = (esp_websocket_client_handle_t)pv;
    esp_websocket_event_data_t event = {
        .client = client,
        .data_ptr = NULL,
        .data_len = 0,
        .op_code = 0,
        .event_id = WEBSOCKET_EVENT_CONNECTED,
        .user_context = client->config.user_context
    };

    // Aquí iría la implementación real del protocolo WebSocket
    // Por ahora solo simulamos la conexión
    client->is_connected = true;
    esp_event_post(WEBSOCKET_EVENTS, WEBSOCKET_EVENT_CONNECTED, &event, sizeof(event), portMAX_DELAY);

    while (client->is_connected) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelete(NULL);
}

esp_websocket_client_handle_t esp_websocket_client_init(const esp_websocket_client_config_t *config) {
    esp_websocket_client_handle_t client = calloc(1, sizeof(struct esp_websocket_client));
    if (client == NULL) {
        return NULL;
    }

    memcpy(&client->config, config, sizeof(esp_websocket_client_config_t));
    client->sock = -1;
    client->is_connected = false;
    
    return client;
}

esp_err_t esp_websocket_client_start(esp_websocket_client_handle_t client) {
    if (client == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    xTaskCreate(websocket_client_task, "websocket_client", 4096, client, 5, &client->task_handle);
    return ESP_OK;
}

esp_err_t esp_websocket_client_stop(esp_websocket_client_handle_t client) {
    if (client == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    client->is_connected = false;
    return ESP_OK;
}

esp_err_t esp_websocket_client_destroy(esp_websocket_client_handle_t client) {
    if (client == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_websocket_client_stop(client);
    free(client);
    return ESP_OK;
}

esp_err_t esp_websocket_client_send_text(esp_websocket_client_handle_t client, const char *data, int len, TickType_t timeout) {
    if (client == NULL || !client->is_connected) {
        return ESP_ERR_INVALID_STATE;
    }

    // Aquí iría la implementación real del envío de datos
    ESP_LOGI(TAG, "Sending: %.*s", len, data);
    return ESP_OK;
}

bool esp_websocket_client_is_connected(esp_websocket_client_handle_t client) {
    return (client != NULL && client->is_connected);
}

esp_err_t esp_websocket_register_events(esp_websocket_client_handle_t client, esp_websocket_event_id_t event, esp_event_handler_t event_handler, void *event_handler_arg) {
    if (client == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    client->event_handler = event_handler;
    client->event_handler_arg = event_handler_arg;
    return ESP_OK;
} 