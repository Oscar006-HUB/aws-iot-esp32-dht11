# AWS IoT Core + ESP32 DHT11

ESP32 firmware that publishes DHT11 temperature and humidity readings to **AWS IoT Core** over MQTT with mutual TLS.

## Contents

| Path | Description |
|------|-------------|
| `temperature_Humidity/dht11_aws/dht11_aws.ino` | ESP32 sketch: WiFi, TLS, MQTT publish of JSON telemetry |
| `Screenshot_edited2.png` | AWS IoT MQTT test client receiving data |

## Setup

1. In AWS IoT Core, create a **Thing**, then download its device certificate, private key and the Amazon Root CA.
2. Attach a policy that allows `iot:Connect` and `iot:Publish` on your topic.
3. In `dht11_aws.ino`, fill in:
   - `WIFI_SSID` / `WIFI_PASSWORD`
   - `AWS_IOT_ENDPOINT` (shown under *Settings* in the AWS IoT console)
   - `AWS_CERT_CA`, `AWS_CERT_CRT`, `AWS_CERT_PRIVATE`: paste the PEM contents
4. Flash it to the ESP32 and subscribe to `lab/climate/dht11_01` in the MQTT test client.

> **Security:** certificates and private keys are deliberately excluded from this repository (see `.gitignore`). Never commit them.

## Libraries

`WiFiClientSecure`, [MQTT (256dpi)](https://github.com/256dpi/arduino-mqtt), DHT sensor library, ArduinoJson.

---

Author: **Prince Oscar Mwinnuo** ([@Oscar006-HUB](https://github.com/Oscar006-HUB))
