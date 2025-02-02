#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "cJSON.h"

#define WS_SERVER_URI "ws://192.168.1.6:80"
#define RETRY_TIMEOUT_MS (5000)

// Definición de pines para LEDs
#define LED_PIN_1 12  // GPIO12
#define LED_PIN_2 14  // GPIO14
#define LED_PIN_3 27  // GPIO27

static const char *TAG = "WebSocket_Client";

// Estructura para mantener el estado de los LEDs
typedef struct {
    bool led_1_state;
    bool led_2_state;
    bool led_3_state;
} led_state_t;

static led_state_t led_state = {
    .led_1_state = false,
    .led_2_state = false,
    .led_3_state = false
};

// Función para inicializar los GPIOs
static void init_gpio(void) {
    gpio_reset_pin(LED_PIN_1);
    gpio_reset_pin(LED_PIN_2);
    gpio_reset_pin(LED_PIN_3);
    
    gpio_set_direction(LED_PIN_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_PIN_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_PIN_3, GPIO_MODE_OUTPUT);
    
    gpio_set_level(LED_PIN_1, 0);
    gpio_set_level(LED_PIN_2, 0);
    gpio_set_level(LED_PIN_3, 0);
}

// Función para enviar información del dispositivo
static void send_device_info(esp_websocket_client_handle_t client, const char* id) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "device_connected");
    cJSON_AddStringToObject(root, "identifier", id);
    cJSON_AddStringToObject(root, "ip", "192.168.1.6"); // Reemplazar con IP real

    char *json_str = cJSON_Print(root);
    esp_websocket_client_send_text(client, json_str, strlen(json_str), portMAX_DELAY);
    ESP_LOGI(TAG, "Enviando info del dispositivo: %s", json_str);
    
    free(json_str);
    cJSON_Delete(root);

    // Enviar estado inicial
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "state_update");
    cJSON_AddStringToObject(root, "identifier", id);
    cJSON_AddStringToObject(root, "state", "off");
    cJSON_AddStringToObject(root, "ip", "192.168.1.6"); // Reemplazar con IP real

    json_str = cJSON_Print(root);
    esp_websocket_client_send_text(client, json_str, strlen(json_str), portMAX_DELAY);
    ESP_LOGI(TAG, "Enviando estado inicial: %s", json_str);
    
    free(json_str);
    cJSON_Delete(root);
}

// Función para actualizar el estado de un LED
static void update_led_state(const char* identifier, bool state) {
    int pin = -1;
    
    if (strcmp(identifier, "led_1") == 0) {
        pin = LED_PIN_1;
        led_state.led_1_state = state;
    } else if (strcmp(identifier, "led_2") == 0) {
        pin = LED_PIN_2;
        led_state.led_2_state = state;
    } else if (strcmp(identifier, "led_3") == 0) {
        pin = LED_PIN_3;
        led_state.led_3_state = state;
    }

    if (pin != -1) {
        gpio_set_level(pin, state);
    }
}

// Función para manejar mensajes recibidos
static void handle_websocket_message(esp_websocket_client_handle_t client, const char* data) {
    cJSON *root = cJSON_Parse(data);
    if (root == NULL) {
        ESP_LOGE(TAG, "Error parsing JSON");
        return;
    }

    cJSON *type = cJSON_GetObjectItem(root, "type");
    if (cJSON_IsString(type) && strcmp(type->valuestring, "toggle_device") == 0) {
        cJSON *identifier = cJSON_GetObjectItem(root, "identifier");
        cJSON *state = cJSON_GetObjectItem(root, "state");

        if (cJSON_IsString(identifier) && cJSON_IsString(state)) {
            bool new_state = (strcmp(state->valuestring, "on") == 0);
            update_led_state(identifier->valuestring, new_state);

            // Enviar confirmación
            cJSON *response = cJSON_CreateObject();
            cJSON_AddStringToObject(response, "type", "state_update");
            cJSON_AddStringToObject(response, "identifier", identifier->valuestring);
            cJSON_AddStringToObject(response, "state", new_state ? "on" : "off");
            cJSON_AddStringToObject(response, "ip", "192.168.1.6"); // Reemplazar con IP real

            char *json_str = cJSON_Print(response);
            esp_websocket_client_send_text(client, json_str, strlen(json_str), portMAX_DELAY);
            ESP_LOGI(TAG, "Enviando confirmación: %s", json_str);
            
            free(json_str);
            cJSON_Delete(response);
        }
    }

    cJSON_Delete(root);
}

void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    esp_websocket_client_handle_t client = (esp_websocket_client_handle_t)handler_args;
    
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Conectado al WebSocket Server");
            // Enviar información de los dispositivos
            vTaskDelay(pdMS_TO_TICKS(100));
            send_device_info(client, "led_1");
            vTaskDelay(pdMS_TO_TICKS(100));
            send_device_info(client, "led_2");
            vTaskDelay(pdMS_TO_TICKS(100));
            send_device_info(client, "led_3");
            break;

        case WEBSOCKET_EVENT_DATA:
            ESP_LOGI(TAG, "Mensaje recibido: %.*s", data->data_len, (char *)data->data_ptr);
            char *message = malloc(data->data_len + 1);
            memcpy(message, data->data_ptr, data->data_len);
            message[data->data_len] = '\0';
            handle_websocket_message(client, message);
            free(message);
            break;

        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "Desconectado del WebSocket Server");
            break;

        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGE(TAG, "Error en WebSocket");
            break;
    }
}

void websocket_client_task(void *pvParameters)
{
    esp_websocket_client_config_t ws_config = {
        .uri = WS_SERVER_URI,
        .reconnect_timeout_ms = RETRY_TIMEOUT_MS,  // Tiempo de reconexión
        .disable_auto_reconnect = false,           // Habilitar reconexión automática
        .transport = WEBSOCKET_TRANSPORT_OVER_TCP  // Especificar transporte TCP
    };

    esp_websocket_client_handle_t client = esp_websocket_client_init(&ws_config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Error al inicializar el cliente WebSocket");
        vTaskDelete(NULL);
        return;
    }

    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    esp_err_t ret = esp_websocket_client_start(client);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al iniciar el cliente WebSocket");
        esp_websocket_client_destroy(client);
        vTaskDelete(NULL);
        return;
    }

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Este código nunca se ejecutará en esta implementación
    esp_websocket_client_stop(client);
    esp_websocket_client_destroy(client);
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando aplicación...");

    // Inicializar GPIOs
    init_gpio();

    // Inicializar NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Inicializar la pila TCP/IP y el sistema de eventos
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Inicializar y conectar al WiFi
    ESP_LOGI(TAG, "Conectando al WiFi...");
    ESP_ERROR_CHECK(example_connect());

    // Iniciar la tarea del cliente WebSocket
    xTaskCreate(websocket_client_task, "websocket_client", 8192, NULL, 5, NULL);
}
