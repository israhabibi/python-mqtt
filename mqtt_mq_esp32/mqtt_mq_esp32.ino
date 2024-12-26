#include <WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "credentials.h" // Include credentials

// MQTT Topics
const char* sensorTopic = "esp32/data";
const char* errorTopic = "esp32/error";

// WiFi and MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200, 60000); // Timezone offset in seconds (0 = UTC)

// MQ Sensors Pin Configuration
#define MQ7_PIN 34   // Analog pin for MQ7 (ESP32 ADC pin)
#define MQ135_PIN 35 // Analog pin for MQ135 (ESP32 ADC pin)
#define MQ2_PIN 36 // Analog pin for MQ2 (ESP32 ADC pin)

// OTA Web Server
AsyncWebServer server(80);

// Function Prototypes
float normalizeADCValue(int rawValue);
void connectWiFi();
void connectMQTT();
bool readSensorsAndPublish();
void publishError(const char* message);

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 with MQ Sensors and OTA!");

  // Connect to WiFi
  connectWiFi();

  // Set up NTP client
  timeClient.begin();

  // Set up MQTT client
  client.setServer(mqttServer, mqttPort);

  // Connect MQTT
  connectMQTT();

  // Initialize ADC for MQ sensors
  analogReadResolution(12); // ESP32 ADC has a 12-bit resolution (values 0-4095)

  // Set up ElegantOTA
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32 with ElegantOTA.");
  });

  ElegantOTA.begin(&server); // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server and OTA setup complete.");
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // Update NTP time
  timeClient.update();

  // Read sensors and publish data
  if (readSensorsAndPublish()) {
    Serial.println("Sensor data published successfully");
  }

  // Handle OTA loop
  ElegantOTA.loop();

  // Wait before next read
  delay(15000); // Adjust as needed
}

// Connect to WiFi
void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Connect to MQTT broker
void connectMQTT() {
  Serial.println("Connecting to MQTT...");
  while (!client.connected()) {
    if (client.connect("ESP32_MQ_Sensors", mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed MQTT connection, retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// Read MQ7 sensor (CO sensor)
float readMQ7() {
  int rawValue = analogRead(MQ7_PIN);
  return normalizeADCValue(rawValue); // Normalize to 0-100
}

// Read MQ135 sensor (Air Quality sensor)
float readMQ135() {
  int rawValue = analogRead(MQ135_PIN);
  return normalizeADCValue(rawValue); // Normalize to 0-100
}

// Read MQ2 sensor (Air Quality sensor)
float readMQ2() {
  int rawValue = analogRead(MQ2_PIN);
  return normalizeADCValue(rawValue); // Normalize to 0-100
}

// Normalize ADC value to 0-1000
float normalizeADCValue(int rawValue) {
  return (float)rawValue / 4095.0 * 1000.0; // Map 0-4095 to 0-100
}

// Publish sensor readings as JSON
bool readSensorsAndPublish() {
  float mq7Normalized = readMQ7();
  float mq135Normalized = readMQ135();
  float mq2Normalized = readMQ2();

  // Get current datetime
  String currentDateTime = timeClient.getFormattedTime();

  // Create JSON object for MQTT payload
  StaticJsonDocument<128> doc;
  doc["datetime"] = currentDateTime;
  doc["mq7"] = mq7Normalized;
  doc["mq135"] = mq135Normalized;
  doc["mq2"] = mq2Normalized;

  char jsonBuffer[128];
  serializeJson(doc, jsonBuffer);

  // Publish JSON data to MQTT topic
  if (client.publish(sensorTopic, jsonBuffer)) {
    Serial.println("Published sensor data to MQTT:");
    Serial.println(jsonBuffer);
    return true;
  } else {
    Serial.println("Failed to publish sensor data");
    return false;
  }
}

// Publish error message to MQTT
void publishError(const char* message) {
  StaticJsonDocument<64> errorDoc;
  errorDoc["error"] = message;

  char errorBuffer[64];
  serializeJson(errorDoc, errorBuffer);

  client.publish(errorTopic, errorBuffer);
  Serial.println("Published error message to MQTT");
}
