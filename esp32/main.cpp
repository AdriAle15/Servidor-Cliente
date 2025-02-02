#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// Configuración WiFi
const char* WIFI_SSID = "Yolymell";
const char* WIFI_PASS = "0802785477";

// Configuración WebSocket
const char* WS_HOST = "192.168.1.5"; // Cambia esto a la IP de tu servidor
const int WS_PORT = 80;

// Definición de pines para LEDs
#define LED_PIN_1 12  // GPIO12
#define LED_PIN_2 14  // GPIO14
#define LED_PIN_3 27  // GPIO27

WebSocketsClient webSocket;
bool isConnected = false;

void handleWebSocketMessage(uint8_t * payload) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload);
  
  if (error) {
    Serial.println("Error parsing JSON");
    return;
  }

  if (doc["type"] == "toggle_device") {
    const char* identifier = doc["identifier"];
    bool state = doc["state"] == "on";
    int pin = -1;
    
    Serial.print("Toggle recibido para: ");
    Serial.print(identifier);
    Serial.print(" estado: ");
    Serial.println(state ? "on" : "off");

    // Determinar qué LED controlar basado en el identificador
    if (strcmp(identifier, "led_1") == 0) {
      pin = LED_PIN_1;
    } else if (strcmp(identifier, "led_2") == 0) {
      pin = LED_PIN_2;
    } else if (strcmp(identifier, "led_3") == 0) {
      pin = LED_PIN_3;
    }

    if (pin != -1) {
      digitalWrite(pin, state ? HIGH : LOW);
      
      // Enviar confirmación
      StaticJsonDocument<200> response;
      response["type"] = "state_update";
      response["identifier"] = identifier;
      response["state"] = state ? "on" : "off";
      response["ip"] = WiFi.localIP().toString();
      
      String jsonString;
      serializeJson(response, jsonString);
      Serial.print("Enviando confirmación: ");
      Serial.println(jsonString);
      webSocket.sendTXT(jsonString);
    }
  }
}

void sendDeviceInfo(const char* id) {
  String ipAddress = WiFi.localIP().toString();
  
  // Notificar conexión del dispositivo
  StaticJsonDocument<200> status;
  status["type"] = "device_connected";
  status["identifier"] = id;
  status["ip"] = ipAddress;
  
  String jsonString;
  serializeJson(status, jsonString);
  Serial.print("Enviando mensaje de conexión: ");
  Serial.println(jsonString);
  webSocket.sendTXT(jsonString.c_str());
  
  // Enviar estado inicial
  StaticJsonDocument<200> state;
  state["type"] = "state_update";
  state["identifier"] = id;
  state["state"] = "off";
  state["ip"] = ipAddress;
  
  serializeJson(state, jsonString);
  Serial.print("Enviando estado inicial: ");
  Serial.println(jsonString);
  webSocket.sendTXT(jsonString);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("Desconectado del WebSocket");
      isConnected = false;
      break;
      
    case WStype_CONNECTED:
      Serial.println("Conectado al WebSocket");
      Serial.print("IP Local: ");
      Serial.println(WiFi.localIP().toString());
      isConnected = true;
      
      // Enviar información de los LEDs con un pequeño delay entre cada uno
      delay(100);
      sendDeviceInfo("led_1");
      delay(100);
      sendDeviceInfo("led_2");
      delay(100);
      sendDeviceInfo("led_3");
      break;
      
    case WStype_TEXT:
      Serial.print("Mensaje recibido: ");
      Serial.println((char*)payload);
      handleWebSocketMessage(payload);
      break;

    case WStype_ERROR:
      Serial.println("Error en WebSocket");
      break;

    case WStype_PING:
      Serial.println("Ping recibido");
      break;

    case WStype_PONG:
      Serial.println("Pong recibido");
      break;
  }
}

void connectToWiFi() {
  Serial.println("Conectando a WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFallo al conectar WiFi");
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);
  
  // Configurar pines de LEDs
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(LED_PIN_3, OUTPUT);
  
  // Estado inicial: apagados
  digitalWrite(LED_PIN_1, LOW);
  digitalWrite(LED_PIN_2, LOW);
  digitalWrite(LED_PIN_3, LOW);
  
  // Conectar WiFi
  connectToWiFi();

  // Configurar WebSocket con más opciones
  webSocket.begin(WS_HOST, WS_PORT, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(15000, 3000, 2);
  Serial.println("Conectando al servidor WebSocket...");
}

void loop() {
  webSocket.loop();

  static unsigned long lastReconnectAttempt = 0;
  const unsigned long reconnectInterval = 5000; // 5 segundos

  // Si no está conectado y ha pasado el intervalo de reconexión
  if (!isConnected && (millis() - lastReconnectAttempt > reconnectInterval)) {
    lastReconnectAttempt = millis();
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Conexión WiFi perdida. Reconectando...");
      connectToWiFi();
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Intentando reconectar WebSocket...");
      webSocket.begin(WS_HOST, WS_PORT, "/");
    }
  }
} 