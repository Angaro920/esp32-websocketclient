#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <DHT.h>

#define DHTPIN 32    // Pin donde está conectado el DHT22
#define DHTTYPE DHT22

#define ANALOG_PIN 36 // Pin analogo conexion MQ-8

#define LDR_PIN 34 //Pin analogo sensor Luz

const char* ssid = "FamiliaAngaritaAlva";
const char* password = "8+t#Uhf5?9T3";
const char* ws_server = "ws://192.168.1.63:8080"; // Dirección WebSocket del servidor

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

  client.onMessage([](WebsocketsMessage message) {
    Serial.println("Mensaje del servidor: " + message.data());
  });

  if (client.connect(ws_server)) {
    Serial.println("Conectado al servidor WebSocket");
  } else {
    Serial.println("Fallo en la conexión WebSocket");
  }
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

    String data = "{\"temperatura\": " + String(t) + ", \"humedad\": " + String(h) + ", \"hidrogeno\": "+ String(concentracionHidrogeno) + ", \"luz\": "+ String(luz) +"}";
    client.send(data);
    Serial.println("Datos enviados: " + data);
  }

  client.poll(); // Mantén viva la conexión WebSocket
  delay(2000);   // Envía datos cada 2 segundos
}
