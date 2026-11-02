/*
  Monitoring Suhu, Kelembaban & VPD - DHT11 + ESP8266 + MQTT (PubSubClient)

  Library (Library Manager):
    - PubSubClient            (Nick O'Leary)
    - DHT sensor library      (Adafruit)
    - Adafruit Unified Sensor (Adafruit)
  Board: NodeMCU 1.0 (ESP-12E Module)  -> package "esp8266" by ESP8266 Community

  Wiring (modul DHT11 3-pin):
    VCC  -> 3V3
    GND  -> GND
    DATA -> D2 (GPIO4)
  Catatan: DHT11 4-pin polos butuh resistor pull-up 10k antara VCC dan DATA.
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ====== KONFIGURASI (SESUAIKAN) ======
const char* WIFI_SSID   = "HelloWorld!";
const char* WIFI_PASS   = "1sampai9";
const char* MQTT_SERVER = "192.168.198.15";   // IP laptop/PC yang menjalankan Mosquitto
const uint16_t MQTT_PORT = 1883;

const char* CLIENT_ID    = "esp8266-dht11-01";
const char* TOPIC_DATA   = "iot/dht11/data";
const char* TOPIC_STATUS = "iot/dht11/status";

const unsigned long PUBLISH_INTERVAL = 5000;  // ms (DHT11 maks. ~1 pembacaan/detik)

#define DHTPIN  4
#define DHTTYPE DHT11

// =====================================

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastPublish = 0;

// VPD (kPa) - rumus Tetens, suhu daun dianggap sama dengan suhu udara.
// Rumus yang sama dipakai backend (server.js) dan MySQL (kolom vpd).
float calcVpd(float t, float rh) {
  float svp = 0.6108 * exp((17.27 * t) / (t + 237.3));   // tekanan uap jenuh (kPa)
  return svp * (1.0 - rh / 100.0);
}

void setupWifi() {
  Serial.printf("\nMenghubungkan ke WiFi %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi terhubung. IP: ");
  Serial.println(WiFi.localIP());
}

void reconnectMqtt() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT broker... ");
    // Last Will: broker otomatis mengirim "offline" jika ESP8266 terputus tiba-tiba
    if (client.connect(CLIENT_ID, TOPIC_STATUS, 1, true, "offline")) {
      Serial.println("berhasil");
      client.publish(TOPIC_STATUS, "online", true);  // retained
    } else {
      Serial.printf("gagal, rc=%d. Coba lagi dalam 3 detik\n", client.state());
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setupWifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) setupWifi();
  if (!client.connected()) reconnectMqtt();
  client.loop();

  unsigned long now = millis();
  if (now - lastPublish >= PUBLISH_INTERVAL) {
    lastPublish = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();   // Celsius

    if (isnan(h) || isnan(t)) {
      Serial.println("Gagal membaca sensor DHT11, cek wiring.");
      return;
    }

    float vpd = calcVpd(t, h);

    char payload[96];
    snprintf(payload, sizeof(payload),
             "{\"temperature\":%.1f,\"humidity\":%.1f,\"vpd\":%.2f}", t, h, vpd);

    if (client.publish(TOPIC_DATA, payload)) {
      Serial.printf("Publish [%s]: %s\n", TOPIC_DATA, payload);
    } else {
      Serial.println("Publish gagal");
    }
  }
}
