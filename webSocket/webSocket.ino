#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define DHTPIN 32  // Pin donde está conectado el DHT22
#define DHTTYPE DHT22
#define ANALOG_PIN 36  // Pin analogo conexion MQ-8
#define LDR_PIN 34     //Pin analogo sensor Luz
#define LED_VENTILADOR 4
#define LED_BOMBA 17
#define LED_AUTO 18


const char* ssid = "FamiliaAngaritaAlva";
const char* password = "8+t#Uhf5?9T3";
const char* ws_server = "ws://192.168.1.2:8080";  // Dirección WebSocket del servidor

DHT dht(DHTPIN, DHTTYPE);
using namespace websockets;
WebsocketsClient client;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando a WiFi...");
  }

  Serial.println("Conectado a WiFi");
  dht.begin();
  pinMode(LED_VENTILADOR, OUTPUT);
  pinMode(LED_BOMBA, OUTPUT);
  pinMode(LED_AUTO, OUTPUT);

  client.onMessage([](WebsocketsMessage message) {
    Serial.println("Mensaje recibido del servidor: " + message.data());

    // ✅ Corregido: Crear un documento JSON dinámico
    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, message.data());

    if (error) {
      Serial.println("Error al parsear JSON");
      return;
    }

    String device = doc["device"];
    String state = doc["state"];

    // Control de dispositivos con los pines correctos
    if (device == "ventilador") {
      digitalWrite(LED_VENTILADOR, state == "ON" ? HIGH : LOW);
      Serial.println(state == "ON" ? "Ventilador ENCENDIDO" : "Ventilador APAGADO");
    } else if (device == "bomba") {
      digitalWrite(LED_BOMBA, state == "ON" ? HIGH : LOW);
      Serial.println(state == "ON" ? "Bomba ENCENDIDA" : "Bomba APAGADA");
    } else if (device == "modo_auto") {
      digitalWrite(LED_AUTO, state == "ON" ? HIGH : LOW);
      Serial.println(state == "ON" ? "Modo automático ACTIVADO" : "Modo automático DESACTIVADO");
    }
  });

  while (!client.connect(ws_server)) {
    Serial.println("Fallo en la conexión WebSocket. Reintentando en 5 segundos...");
    delay(5000);
  }

  Serial.println("Conectado al servidor WebSocket");
}

void loop() {
  if (client.available()) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    int analogValue = analogRead(ANALOG_PIN);
    float voltage = analogValue * (3.3 / 4095.0);
    float concentracionHidrogeno = map(analogValue, 0, 4095, 0, 1000);

    //lecturas LUZ
    int luz = analogRead(LDR_PIN);

    if (isnan(h) || isnan(t)) {
      Serial.println("Error leyendo el sensor DHT");
      return;
    }

    String data = "{\"temperatura\": " + String(t) + ", \"humedad\": " + String(h) + ", \"hidrogeno\": " + String(concentracionHidrogeno) + ", \"luz\": " + String(luz) + "}";
    client.send(data);
    Serial.println("Datos enviados: " + data);
  }

  client.poll();
  delay(2000);
}
