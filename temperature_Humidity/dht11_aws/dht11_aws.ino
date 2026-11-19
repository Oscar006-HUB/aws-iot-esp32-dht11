#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <DHT.h>
#include <ArduinoJson.h>

// ==================== HARDWARE CONFIGURATION ====================
#define DHTPIN 18        // GPIO pin connected to DHT11 Data pin[cite: 1]
#define DHTTYPE DHT11     // DHT 11 Sensor Type[cite: 1]

// ==================== NETWORK CONFIGURATION ====================
const char* WIFI_SSID        = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD    = "YOUR_WIFI_PASSWORD";
const char* AWS_IOT_ENDPOINT = "YOUR_ENDPOINT-ats.iot.REGION.amazonaws.com";
const char* MQTT_TOPIC          = "lab/climate/dht11_01";
const char* DEVICE_ID           = "ESP32_DHT11_Node_01";

// Simple Calibration Offset[cite: 1]
const float TEMP_OFFSET = 0.0;
const float HUM_OFFSET  = 0.0;

// ==================== AWS CERTIFICATES ====================
// Paste raw text contents from your downloaded certificate files inside the quotes[cite: 1]
static const char AWS_CERT_CA[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
<paste your certificate here>
-----END CERTIFICATE-----
)KEY";

static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
<paste your certificate here>
-----END CERTIFICATE-----
)KEY";

static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
<paste your rsa private key here>
-----END RSA PRIVATE KEY-----
)KEY";

// ==================== OBJECTS ====================
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure net;
MQTTClient client(256);
unsigned long lastPublish = 0;

void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");
}

void connectAWS() {
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  Serial.print("Connecting to AWS IoT Core");
  while (!client.connect(DEVICE_ID)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to AWS IoT Core!");
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  connectWiFi();
  connectAWS();
}

void loop() {
  client.loop();

  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!client.connected()) connectAWS();

  // Publish JSON telemetry every 5 to 10 seconds[cite: 1]
  unsigned long currentMillis = millis();
  if (currentMillis - lastPublish >= 5000) {
    lastPublish = currentMillis;

    float rawTemp = dht.readTemperature();
    float rawHum  = dht.readHumidity();

    if (isnan(rawTemp) || isnan(rawHum)) {
      Serial.println("Failed to read from DHT11 sensor!");
      return;
    }

    float calTemp = rawTemp + TEMP_OFFSET;
    float calHum  = rawHum + HUM_OFFSET;

    // JSON Payload containing device ID, telemetry values, and timestamps[cite: 1]
    StaticJsonDocument<256> doc;
    doc["device_id"]     = DEVICE_ID;
    doc["sensor_type"]   = "DHT11";
    doc["temperature_C"] = serialized(String(calTemp, 1));
    doc["humidity_pct"]  = serialized(String(calHum, 1));
    doc["timestamp_ms"]  = currentMillis;

    char payload[256];
    serializeJson(doc, payload);

    client.publish(MQTT_TOPIC, payload);
    Serial.print("Published to AWS: ");
    Serial.println(payload);
  }
}