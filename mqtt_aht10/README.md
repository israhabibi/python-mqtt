# Multi-Sensor ESP8266 MQTT Integration

This project integrates multiple sensors with an ESP8266 microcontroller to collect and publish sensor data to an MQTT broker. The sensors include AHT10/AHT20, DHT22, and an LDR. The data is published in JSON format to specified MQTT topics.

## Features
- Reads temperature and humidity from AHT10/AHT20 and DHT22 sensors.
- Reads light intensity using an LDR.
- Publishes sensor data to an MQTT broker in JSON format.
- Implements watchdog timer for stability.
- Handles sensor read timeouts with error messages and device restarts.

## Requirements
- **Hardware**:
  - ESP8266 microcontroller
  - AHT10 or AHT20 sensor
  - DHT22 sensor
  - LDR (Light Dependent Resistor)
  - WiFi connectivity
- **Software**:
  - Arduino IDE with ESP8266 board support
  - Required libraries:
    - `ESP8266WiFi`
    - `Adafruit_AHTX0`
    - `PubSubClient`
    - `NTPClient`
    - `WiFiUdp`
    - `DHT`
    - `ArduinoJson`
- MQTT Broker (e.g., Mosquitto, AWS IoT Core, etc.)

## Setup Instructions

### 1. Clone the Repository
```bash
git clone <repository-url>
cd <repository-folder>
```

### 2. Install Arduino Libraries
Install the following libraries via the Arduino Library Manager:
- Adafruit AHTX0
- PubSubClient
- NTPClient
- ArduinoJson
- DHT

### 3. Add Credentials
Create a `credentials.h` file in the project directory and add your WiFi and MQTT credentials:
```cpp
#ifndef CREDENTIALS_H
#define CREDENTIALS_H

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqttServer = "YOUR_MQTT_BROKER_ADDRESS";
const int mqttPort = 1883;
const char* mqttUser = "YOUR_MQTT_USERNAME";
const char* mqttPassword = "YOUR_MQTT_PASSWORD";

#endif
```

### 4. Wiring Diagram
Connect the sensors to the ESP8266 as follows:
- **DHT22**: Data pin to GPIO2 (D4 on NodeMCU), VCC to 3.3V, GND to GND.
- **AHT10/AHT20**: SDA to D2 (GPIO4), SCL to D1 (GPIO5), VCC to 3.3V, GND to GND.
- **LDR**: Connect one side to A0 and the other side to a resistor (10kΩ) connected to GND. The junction of the LDR and resistor connects to 3.3V.

### 5. Upload Code
- Open the project in Arduino IDE.
- Select the appropriate ESP8266 board under **Tools > Board**.
- Select the correct COM port under **Tools > Port**.
- Compile and upload the code.

### 6. Run the Device
- Power the ESP8266 with a USB cable or a 5V power source.
- Monitor the serial output using the Serial Monitor in the Arduino IDE (baud rate: 115200).

### 7. MQTT Topics
- **Sensor Data**: Data is published to `sensor/data` in JSON format:
  ```json
  {
    "datetime": "2024-12-27T12:34:56",
    "aht10": {
      "temperature": 25.5,
      "humidity": 60.0
    },
    "dht22": {
      "temperature": 26.0,
      "humidity": 55.0
    },
    "ldr": {
      "value": 512,
      "voltage": 1.65
    }
  }
  ```
- **Timeout/Error**: Errors are published to `hello/topic`:
  ```json
  {"error": "AHT10 sensor timeout"}
  ```

## Troubleshooting
1. **WiFi Connection Issues**:
   - Ensure the SSID and password in `credentials.h` are correct.
   - Check WiFi signal strength near the ESP8266.

2. **MQTT Connection Issues**:
   - Verify the MQTT broker address, port, username, and password in `credentials.h`.
   - Ensure the MQTT broker is running and accessible.

3. **Sensor Errors**:
   - Check the wiring and connections for each sensor.
   - Ensure the sensors are compatible and functioning.

4. **Watchdog Timer Resets**:
   - Verify that the ESP8266 is not stuck in loops and is feeding the watchdog timer regularly.

## License
This project is open-source and available under the [MIT License](LICENSE).

## Acknowledgments
Special thanks to the developers of the libraries used in this project.

## Contributing
Feel free to open issues or submit pull requests to improve this project.

```

This `README.md` provides clear setup instructions and usage details for your project. Let me know if you need further customization!