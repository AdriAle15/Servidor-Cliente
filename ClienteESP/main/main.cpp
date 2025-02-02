#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include "cJSON.h"
#include "esp_websocket_client.h"
#include "esp_timer.h"
#include <inttypes.h>

// Definiciones de HIGH y LOW
#define HIGH 1
#define LOW 0

// Declaración explícita de app_main
#if defined(__cplusplus)
extern "C" {
#endif

void app_main(void);

#if defined(__cplusplus)
}
#endif

static const char *TAG = "LED_CLIENT";

// Configuración WiFi
const char* WIFI_SSID = "Yolymell";
const char* WIFI_PASS = "0802785477";

// Configuración WebSocket
const char* WS_HOST = "192.168.1.4";  // Cambia esto a la IP de tu servidor
const int WS_PORT = 80;
const char* WS_PATH = "/";  // Agregar la ruta explícita

// Definir pines para los LEDs
#define LED_PIN_1 GPIO_NUM_12
#define LED_PIN_2 GPIO_NUM_14
#define LED_PIN_3 GPIO_NUM_27

static esp_websocket_client_handle_t client;
static bool is_connected = false;

// Función para enviar información del dispositivo
static void send_device_info(const char* id) {
    if (!is_connected) return;

    esp_netif_ip_info_t ip_info;
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_get_ip_info(netif, &ip_info);
    char ip_str[16];
    sprintf(ip_str, IPSTR, IP2STR(&ip_info.ip));

    // Crear mensaje JSON
    cJSON *status = cJSON_CreateObject();
    cJSON_AddStringToObject(status, "type", "device_connected");
    cJSON_AddStringToObject(status, "identifier", id);
    cJSON_AddStringToObject(status, "ip", ip_str);

    char *json_str = cJSON_Print(status);
    ESP_LOGI(TAG, "Enviando mensaje de conexión: %s", json_str);
    esp_websocket_client_send_text(client, json_str, strlen(json_str), portMAX_DELAY);
    free(json_str);
    cJSON_Delete(status);

    // Enviar estado inicial
    cJSON *state = cJSON_CreateObject();
    cJSON_AddStringToObject(state, "type", "state_update");
    cJSON_AddStringToObject(state, "identifier", id);
    cJSON_AddStringToObject(state, "state", "off");
    cJSON_AddStringToObject(state, "ip", ip_str);

    json_str = cJSON_Print(state);
    ESP_LOGI(TAG, "Enviando estado inicial: %s", json_str);
    esp_websocket_client_send_text(client, json_str, strlen(json_str), portMAX_DELAY);
    free(json_str);
    cJSON_Delete(state);
}

// Manejador de mensajes WebSocket
static void handle_websocket_message(uint8_t *payload) {
    cJSON *doc = cJSON_Parse((const char *)payload);
    if (doc == NULL) {
        ESP_LOGE(TAG, "Error parsing JSON");
        return;
    }

    if (cJSON_IsString(cJSON_GetObjectItem(doc, "type")) && strcmp(cJSON_GetObjectItem(doc, "type")->valuestring, "toggle_device") == 0) {
        const char* identifier = cJSON_GetObjectItem(doc, "identifier")->valuestring;
        bool state = strcmp(cJSON_GetObjectItem(doc, "state")->valuestring, "on") == 0;
        gpio_num_t pin = GPIO_NUM_NC;

        ESP_LOGI(TAG, "Toggle recibido para: %s estado: %s", identifier, state ? "on" : "off");

        // Determinar qué LED controlar basado en el identificador
        if (strcmp(identifier, "led_1") == 0) {
            pin = LED_PIN_1;
        } else if (strcmp(identifier, "led_2") == 0) {
            pin = LED_PIN_2;
        } else if (strcmp(identifier, "led_3") == 0) {
            pin = LED_PIN_3;
        }

        if (pin != GPIO_NUM_NC) {
            gpio_set_level(pin, state ? HIGH : LOW);
            
            // Enviar confirmación
            esp_netif_ip_info_t ip_info;
            esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
            esp_netif_get_ip_info(netif, &ip_info);
            char ip_str[16];
            sprintf(ip_str, IPSTR, IP2STR(&ip_info.ip));

            cJSON *response = cJSON_CreateObject();
            cJSON_AddStringToObject(response, "type", "state_update");
            cJSON_AddStringToObject(response, "identifier", identifier);
            cJSON_AddStringToObject(response, "state", state ? "on" : "off");
            cJSON_AddStringToObject(response, "ip", ip_str);

            char *json_str = cJSON_Print(response);
            ESP_LOGI(TAG, "Enviando confirmación: %s", json_str);
            esp_websocket_client_send_text(client, json_str, strlen(json_str), portMAX_DELAY);
            free(json_str);
            cJSON_Delete(response);
        }
    }

    cJSON_Delete(doc);
}

// Eventos WebSocket
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;

    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
        {
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
            is_connected = true;
            send_device_info("led_1");
            send_device_info("led_2");
            send_device_info("led_3");
            break;
        }

        case WEBSOCKET_EVENT_DISCONNECTED:
        {
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
            is_connected = false;
            break;
        }

        case WEBSOCKET_EVENT_DATA:
        {
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
            if (data->data_ptr && data->data_len > 0) {
                ESP_LOGI(TAG, "Received data: %.*s", data->data_len, (char *)data->data_ptr);
                if (data->op_code == WS_TRANSPORT_OPCODES_TEXT) {
                    handle_websocket_message((uint8_t *)data->data_ptr);
                }
            }
            break;
        }

        case WEBSOCKET_EVENT_ERROR:
        {
            ESP_LOGE(TAG, "WEBSOCKET_EVENT_ERROR");
            is_connected = false;
            break;
        }

        default:
            ESP_LOGI(TAG, "Received WebSocket event: %ld", (long)event_id);
            break;
    }
}

// Inicialización de GPIO
static void init_gpio(void) {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL<<LED_PIN_1) | (1ULL<<LED_PIN_2) | (1ULL<<LED_PIN_3);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    gpio_set_level(LED_PIN_1, 0);
    gpio_set_level(LED_PIN_2, 0);
    gpio_set_level(LED_PIN_3, 0);
}

// Evento handler para WiFi
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Retry connecting to the AP");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR ", trying to connect to WebSocket server", IP2STR(&event->ip_info.ip));
        
        // Iniciar conexión WebSocket
        esp_websocket_client_config_t websocket_cfg = {};
        char uri[100];
        snprintf(uri, sizeof(uri), "ws://%s:%d%s", WS_HOST, WS_PORT, WS_PATH);
        ESP_LOGI(TAG, "Connecting to WebSocket URI: %s", uri);
        
        websocket_cfg.uri = uri;
        websocket_cfg.disable_auto_reconnect = false;
        websocket_cfg.task_stack = 8192;
        websocket_cfg.buffer_size = 4096;
        websocket_cfg.transport = WEBSOCKET_TRANSPORT_OVER_TCP;
        websocket_cfg.user_agent = "esp32";
        websocket_cfg.headers = NULL;
        websocket_cfg.ping_interval_sec = 10;
        websocket_cfg.disable_pingpong_discon = true;
        websocket_cfg.network_timeout_ms = 30000;  // Aumentado a 30 segundos

        client = esp_websocket_client_init(&websocket_cfg);
        if (client == NULL) {
            ESP_LOGE(TAG, "Error initializing WebSocket client");
            return;
        }

        ESP_LOGI(TAG, "Starting WebSocket client");
        esp_err_t start_err = esp_websocket_client_start(client);
        if (start_err != ESP_OK) {
            ESP_LOGE(TAG, "Error starting WebSocket client: %d (%s)", 
                     start_err, esp_err_to_name(start_err));
            return;
        }
    }
}

// Inicialización WiFi
static void wifi_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {};
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    
    memcpy(wifi_config.sta.ssid, WIFI_SSID, strlen(WIFI_SSID));
    memcpy(wifi_config.sta.password, WIFI_PASS, strlen(WIFI_PASS));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

// Implementación de app_main
void app_main(void)
{
    // Inicializar NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Inicializar componentes
    init_gpio();
    wifi_init();

    ESP_LOGI(TAG, "ESP32 LED Client iniciado");
    
    static int64_t last_reconnect_attempt = 0;
    const int64_t reconnect_interval = 5000; // 5 segundos

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        int64_t now = esp_timer_get_time() / 1000;  // Convertir microsegundos a milisegundos
        
        if (!is_connected && (now - last_reconnect_attempt > reconnect_interval)) {
            last_reconnect_attempt = now;
            ESP_LOGI(TAG, "Intentando reconectar WebSocket...");
            
            if (client != NULL) {
                esp_websocket_client_stop(client);
                esp_websocket_client_destroy(client);
                client = NULL;
            }
            
            // Reiniciar cliente WebSocket con la URI completa
            esp_websocket_client_config_t websocket_cfg = {};
            char uri[100];
            snprintf(uri, sizeof(uri), "ws://%s:%d%s", WS_HOST, WS_PORT, WS_PATH);
            websocket_cfg.uri = uri;
            websocket_cfg.disable_auto_reconnect = false;
            websocket_cfg.task_stack = 8192;
            websocket_cfg.buffer_size = 1024;
            websocket_cfg.transport = WEBSOCKET_TRANSPORT_OVER_TCP;
            websocket_cfg.path = WS_PATH;
            websocket_cfg.port = WS_PORT;
            websocket_cfg.ping_interval_sec = 15;
            websocket_cfg.disable_pingpong_discon = false;

            client = esp_websocket_client_init(&websocket_cfg);
            if (client != NULL) {
                esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, NULL);
                esp_websocket_client_start(client);
            }
        }
    }
} 