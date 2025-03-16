#include <ESP8266WiFi.h>
#include <Adafruit_AHTX0.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h> // JSON library
#include "credentials.h"      // Include credentials

const char* sensorTopic = "sensor/data";
const char* timeoutTopic = "sensor/error";

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_AHTX0 aht;
Adafruit_BMP085 bmp;

// NTP Configuration
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200, 60000); // Timezone offset in seconds (0 = UTC)

// LDR Pin Configuration
#define LDR_PIN A0       // Analog pin A0 for LDR

void setup() {
  Serial.begin(115200);
  Serial.println("Sensor integration demo!");

  // Initialize AHT sensor
  if (!aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");

  // Initialize BMP180 sensor
  if (!bmp.begin()) {
    Serial.println("Could not find BMP180? Check wiring");
    while (1) delay(10);
  }
  Serial.println("BMP180 sensor found");

  // Connect to WiFi
  connectWiFi();

  // Set up NTP client
  timeClient.begin();

  // Set up MQTT client
  client.setServer(mqttServer, mqttPort);
  connectMQTT();

  // Enable watchdog timer with a 10-second timeout
  ESP.wdtEnable(10000); // 10 seconds
}

void loop() {
  ESP.wdtFeed(); // Reset the watchdog timer

  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // Update time
  timeClient.update();

  // Get sensor readings with timeout
  if (readSensorsAndPublish()) {
    ESP.wdtFeed(); // Feed WDT after successful sensor reading
  }

  // Wait before sending next data
  delay(15000); // Adjust as needed
}

// Connect to WiFi
void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    ESP.wdtFeed(); // Feed WDT while waiting for WiFi
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Connect to MQTT broker
void connectMQTT() {
  Serial.println("Connecting to MQTT...");
  while (!client.connected()) {
    if (client.connect("MultiSensor_Client", mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed MQTT connection, retrying in 5 seconds...");
      delay(5000);
      ESP.wdtFeed(); // Feed WDT while retrying MQTT
    }
  }
}

// Read sensors and publish JSON data
bool readSensorsAndPublish() {
  unsigned long startTime = millis();

  sensors_event_t humidityAHT, tempAHT;

  // Read AHT10 sensor
  while (!aht.getEvent(&humidityAHT, &tempAHT)) {
    if (millis() - startTime > 10000) { // Timeout after 10 seconds
      Serial.println("AHT10 sensor read timeout!");

      // Publish timeout message to the topic "hello/topic"
      const char* timeoutMessage = "{\"error\": \"AHT10 sensor timeout\"}";
      client.publish(timeoutTopic, timeoutMessage);
      Serial.println("Published timeout message to MQTT: hello/topic");

      // Restart ESP
      Serial.println("Restarting ESP due to AHT10 timeout...");
      delay(1000); // Give some time for MQTT message to be sent
      ESP.restart();
    }
    ESP.wdtFeed(); // Feed WDT while waiting for sensor data
    delay(100);    // Small delay to avoid blocking completely
  }

  // Read LDR value
  int ldrValue = analogRead(LDR_PIN);
  float ldrVoltage = ldrValue * (3.3 / 1023.0); // Convert ADC value to voltage

  // Read BMP180 sensor
  float bmpTemperature = bmp.readTemperature();
  float bmpPressure = bmp.readPressure() / 100.0; // Convert to hPa

  // Get current datetime
  String currentDateTime = getCurrentDateTime();

  // Create JSON object for MQTT payload
  StaticJsonDocument<256> doc;
  doc["datetime"] = currentDateTime;
  doc["aht10"]["temperature"] = tempAHT.temperature;
  doc["aht10"]["humidity"] = humidityAHT.relative_humidity;
  doc["ldr"]["value"] = ldrValue;
  doc["ldr"]["voltage"] = ldrVoltage;
  doc["bmp180"]["temperature"] = bmpTemperature;
  doc["bmp180"]["pressure"] = bmpPressure;

  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  // Publish JSON data to MQTT topic
  client.publish(sensorTopic, jsonBuffer);
  Serial.println("Published sensor data to MQTT");
  Serial.println(jsonBuffer);

  return true;
}

// Get current datetime as a string
String getCurrentDateTime() {
  return timeClient.getFormattedTime();
}
