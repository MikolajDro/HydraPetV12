# HYDRAPET

HYDRAPET is a smart water dispensing system built using the ESP32 microcontroller. It leverages MQTT for seamless communication, enabling remote monitoring and control of water levels, alarms, and other system functionalities. The system integrates various components such as load cells (HX711), motors for water dispensing, LEDs for status indication, and buttons for user interaction.

## Table of Contents

1. [Features](#features)
2. [Hardware Requirements](#hardware-requirements)
3. [Software Requirements](#software-requirements)
4. [Installation](#installation)
5. [Configuration](#configuration)
6. [Usage](#usage)
    - [MQTT Topics](#mqtt-topics)
    - [Example MQTT Commands](#example-mqtt-commands)
7. [Project Structure](#project-structure)
8. [Troubleshooting](#troubleshooting)
9. [Contributing](#contributing)
10. [License](#license)
11. [Contact](#contact)

---

## Features

- **Remote Time Setting:** Synchronize device time via MQTT.
- **Water Level Control:** Set and get current water levels remotely.
- **Alarm Management:** Add, retrieve, and delete alarms for specific water dispensing times.
- **Automatic Water Dispensing:** Automatically dispense water at scheduled times.
- **Weight Taring:** Reset the load cell to account for tare weight.
- **Status Monitoring:** Retrieve comprehensive device status, including button states, LED states, and motor states.
- **Error Handling:** Notify via MQTT if water dispensing fails.

---

## Hardware Requirements

To set up HYDRAPET, you'll need the following hardware components:

- **ESP32 Development Board**
- **HX711 Load Cell Amplifier Module**
- **Load Cell (e.g., 5kg)**
- **Motor (for water dispensing)**
- **LEDs (for status indication)**
- **Push Buttons (for user interaction)**
- **Power Supply (appropriate for ESP32 and peripherals)**
- **Connecting Wires and Breadboard (for prototyping)**
- **Optional:** Enclosure for housing components

### Pin Configuration

Ensure the following pin connections between the ESP32 and peripherals:

- **HX711:**
  - `DT` to GPIO XX
  - `SCK` to GPIO YY
- **Motor:**
  - Control pins to GPIO AA and BB
- **LEDs:**
  - Anode to GPIO CC through a resistor
  - Cathode to GND
- **Buttons:**
  - One side to GPIO DD
  - Other side to GND

*Replace `XX`, `YY`, `AA`, `BB`, `CC`, and `DD` with actual GPIO numbers based on your setup.*

---

## Software Requirements

- **ESP-IDF (Espressif IoT Development Framework)**
- **CMake and Ninja Build Systems**
- **Python 3.x**
- **Git**
- **MQTT Broker (e.g., Mosquitto)**
- **Optional:** MQTT client tools like `mosquitto_pub` and `mosquitto_sub` for testing

### Installing ESP-IDF

Follow the official [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) to install the development environment.

---

## Installation

1. **Clone the Repository**

   ```bash
   git clone https://github.com/yourusername/hydrapet.git
   cd hydrapet
   ```

2. **Set Up ESP-IDF Environment**

   Source the ESP-IDF environment script:

   ```bash
   source $HOME/esp/esp-idf/export.sh
   ```

3. **Build the Project**

   ```bash
   idf.py build
   ```

4. **Flash the Firmware to ESP32**

   Connect your ESP32 board via USB and execute:

   ```bash
   idf.py -p /dev/ttyUSB0 flash
   ```

   *Replace `/dev/ttyUSB0` with the appropriate serial port.*

5. **Monitor the Logs**

   ```bash
   idf.py -p /dev/ttyUSB0 monitor
   ```

---

## Configuration

### MQTT Broker Setup

HYDRAPET communicates using MQTT. You can use a public broker like `test.mosquitto.org` for testing or set up a private broker for production.

**Using Mosquitto Broker:**

- **Install Mosquitto:**

  ```bash
  sudo apt-get update
  sudo apt-get install mosquitto mosquitto-clients
  ```

- **Start Mosquitto Service:**

  ```bash
  sudo systemctl start mosquitto
  ```

- **Enable Mosquitto to Start on Boot:**

  ```bash
  sudo systemctl enable mosquitto
  ```

### Configure MQTT Broker in Code

Edit the `mqtt.c` file to set your MQTT broker address:

```c
// mqtt.c

esp_mqtt_client_config_t mqtt_cfg = {
    .broker = {
        .address.uri = "mqtt://your_broker_address:1883", // Replace with your broker
    },
};
```

Rebuild and flash the firmware after making changes.

---

## Usage

HYDRAPET uses various MQTT topics to interact with the device. Below is a detailed description of each topic along with example commands.

### MQTT Topics

Refer to the [HYDRAPET MQTT Topics Documentation](./MQTT_Topics_Documentation.md) for comprehensive details on each topic.

### Example MQTT Commands

Below are examples of how to interact with HYDRAPET using MQTT commands via `mosquitto_pub` and `mosquitto_sub`.

#### 1. Set System Time

**Topic:** `hydrapet0001/update/set/time`

**Payload:**

```json
{
    "timestamp": "2025-01-26T14:30:00"
}
```

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/time" -m '{"timestamp": "2025-01-26T14:30:00"}'
```

#### 2. Get System Time

**Topic:** `hydrapet0001/update/get/time`

**Payload:** None

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/time" -n
```

**Subscribe to Receive Response:**

```bash
mosquitto_sub -h broker.mosquitto.org -t "hydrapet0001/update/get/time"
```

#### 3. Set Water Level

**Topic:** `hydrapet0001/update/set/water`

**Payload:**

```
250
```

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/water" -m "250"
```

#### 4. Get Water Level

**Topic:** `hydrapet0001/update/get/water`

**Payload:** None

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/water" -n
```

**Subscribe to Receive Response:**

```bash
mosquitto_sub -h broker.mosquitto.org -t "hydrapet0001/update/get/water"
```

#### 5. Get Device Status

**Topic:** `hydrapet0001/update/get/status`

**Payload:** None

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/status" -n
```

**Subscribe to Receive Response:**

```bash
mosquitto_sub -h broker.mosquitto.org -t "hydrapet0001/update/get/status"
```

#### 6. Add an Alarm

**Topic:** `hydrapet0001/update/set/alarm`

**Payload:**

```json
{
    "timestamp": "2025-01-27T10:00:00",
    "target_weight": 200
}
```

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/alarm" -m '{"timestamp": "2025-01-27T10:00:00", "target_weight": 200}'
```

#### 7. Get All Alarms

**Topic:** `hydrapet0001/update/get/alarms`

**Payload:** None

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/alarms" -n
```

**Subscribe to Receive Response:**

```bash
mosquitto_sub -h broker.mosquitto.org -t "hydrapet0001/update/get/alarms"
```

#### 8. Delete an Alarm

**Topic:** `hydrapet0001/update/del/alarm`

**Payload:**

```json
{
    "timestamp": "2025-01-27T10:00:00"
}
```

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/del/alarm" -m '{"timestamp": "2025-01-27T10:00:00"}'
```

#### 9. Pour Water to Specific Weight

**Topic:** `hydrapet0001/update/put/pourwater`

**Payload:**

```
250
```

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/put/pourwater" -m "250"
```

**Or in JSON Format:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/put/pourwater" -m '{"target_weight": 250}'
```

#### 10. Set Tare

**Topic:** `hydrapet0001/update/set/tare`

**Payload:** Any value (e.g., `1` or `"tare"`)

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/tare" -m "1"
```

**Or:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/tare" -m "tare"
```

---

## Project Structure

```
hydrapet/
├── main/
│   ├── main.c
│   ├── mqtt.c
│   ├── mqtt.h
│   ├── alarms.c
│   ├── alarms.h
│   ├── hx711.c
│   ├── hx711.h
│   ├── motor.c
│   ├── motor.h
│   ├── led.c
│   ├── led.h
│   ├── buttons.c
│   ├── buttons.h
│   ├── ...
├── CMakeLists.txt
├── README.md
├── MQTT_Topics_Documentation.md
├── LICENSE
└── ...
```

- **main.c:** Entry point of the application, initializes modules and starts tasks.
- **mqtt.c / mqtt.h:** Handles MQTT communication, subscribing to topics and publishing messages.
- **alarms.c / alarms.h:** Manages alarm creation, deletion, and monitoring.
- **hx711.c / hx711.h:** Interfaces with the HX711 load cell for weight measurements and tare functionality.
- **motor.c / motor.h:** Controls the motor responsible for dispensing water.
- **led.c / led.h:** Manages LED indicators for system status.
- **buttons.c / buttons.h:** Handles button inputs for user interactions.
- **CMakeLists.txt:** Build configuration for ESP-IDF.
- **MQTT_Topics_Documentation.md:** Detailed documentation of MQTT topics and their usage.
- **LICENSE:** Licensing information.

---

## Troubleshooting

- **Cannot Connect to Wi-Fi:**
  - Verify Wi-Fi credentials in your code.
  - Ensure ESP32 is within range of the Wi-Fi router.
  - Check for interference or network issues.

- **MQTT Connection Issues:**
  - Ensure the MQTT broker address and port are correct.
  - Verify network connectivity.
  - Check broker logs for any connection refusals or errors.

- **Unexpected Weight Readings:**
  - Calibrate the HX711 load cell.
  - Ensure the load cell is properly connected and not overloaded.
  - Verify the tare functionality is working correctly.

- **Motor Not Dispensing Water:**
  - Check motor connections and power supply.
  - Ensure motor control pins are correctly configured.
  - Verify motor is functional by testing manually.

- **LEDs Not Indicating Status:**
  - Check LED connections and resistor values.
  - Ensure correct GPIO pins are used in the code.

- **Alarms Not Triggering:**
  - Verify system time is correctly set.
  - Ensure alarms are added with correct timestamps.
  - Check logs for any errors related to alarm management.

---

## Contributing

Contributions are welcome! To contribute to HYDRAPET:

1. **Fork the Repository**
2. **Create a New Branch**

   ```bash
   git checkout -b feature/YourFeature
   ```

3. **Commit Your Changes**

   ```bash
   git commit -m "Add your feature"
   ```

4. **Push to the Branch**

   ```bash
   git push origin feature/YourFeature
   ```

5. **Open a Pull Request**

Please ensure your code follows the project's coding standards and includes appropriate comments and documentation.

---

## License

This project is licensed under the [MIT License](./LICENSE).

---

## Contact

For any questions, issues, or feature requests, please open an issue on the [GitHub repository](https://github.com/yourusername/hydrapet/issues) or contact the maintainer at [your.email@example.com](mailto:your.email@example.com).

---

# MQTT_Topics_Documentation.md

# HYDRAPET MQTT Topics Documentation

This documentation provides detailed information about the MQTT topics used in the HYDRAPET project. Each topic serves a specific purpose, enabling remote control and monitoring of the HYDRAPET system.

## Table of Contents

1. [Overview](#overview)
2. [List of MQTT Topics](#list-of-mqtt-topics)
    - [1. `hydrapet0001/update/set/time`](#1-hydrapet0001updatesettime)
    - [2. `hydrapet0001/update/get/time`](#2-hydrapet0001updategettime)
    - [3. `hydrapet0001/update/set/water`](#3-hydrapet0001updatesetwater)
    - [4. `hydrapet0001/update/get/water`](#4-hydrapet0001updategetwater)
    - [5. `hydrapet0001/update/get/status`](#5-hydrapet0001updategetstatus)
    - [6. `hydrapet0001/update/set/alarm`](#6-hydrapet0001updatesetalarm)
    - [7. `hydrapet0001/update/get/alarms`](#7-hydrapet0001updategetalarms)
    - [8. `hydrapet0001/update/del/alarm`](#8-hydrapet0001updatedelalarm)
    - [9. `hydrapet0001/update/put/pourwater`](#9-hydrapet0001updateputpourwater)
    - [10. `hydrapet0001/update/set/tare`](#10-hydrapet0001updatesettare)
3. [Example MQTT Commands](#example-mqtt-commands)
4. [Notes](#notes)

---

## Overview

HYDRAPET utilizes MQTT (Message Queuing Telemetry Transport) for lightweight, efficient communication between the ESP32-based device and external clients or controllers. The system subscribes to specific topics to receive commands and publishes status updates or responses on other topics.

---

## List of MQTT Topics

### 1. `hydrapet0001/update/set/time`

**Description:**

Sets the system time of the HYDRAPET device.

**Payload Formats:**

- **JSON:**

    ```json
    {
        "timestamp": "YYYY-MM-DDTHH:MM:SS"
    }
    ```

- **Plain Text:**

    ```
    YYYY-MM-DDTHH:MM:SS
    ```

**Example Messages:**

- **JSON:**

    ```bash
    mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/time" -m '{"timestamp": "2025-01-26T14:30:00"}'
    ```

- **Plain Text:**

    ```bash
    mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/time" -m "2025-01-26T14:30:00"
    ```

**Device Response:**

- Logs indicating time has been set successfully or errors if the format is incorrect.

---

### 2. `hydrapet0001/update/get/time`

**Description:**

Requests the current system time from the HYDRAPET device.

**Payload Formats:**

- **None:** This is a request topic; no payload is required.

**Example Message:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/time" -n
```

**Device Response:**

- Publishes the current time on the same topic:

    ```json
    {
        "current_time": "2025-01-26T14:30:00"
    }
    ```

---

### 3. `hydrapet0001/update/set/water`

**Description:**

Sets the target water weight. The device will start dispensing water to reach the specified weight.

**Payload Formats:**

- **Plain Text:**

    ```
    <target_weight>
    ```

    - `<target_weight>`: Integer representing weight in grams (e.g., `250`).

**Example Message:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/water" -m "250"
```

**Device Response:**

- Logs indicating the dispensing process has started.
- If dispensing fails, publishes `"empty"` on `hydrapet0001/hydrapetinfo/watertank`.

---

### 4. `hydrapet0001/update/get/water`

**Description:**

Requests the current water weight from the HYDRAPET device.

**Payload Formats:**

- **None:** This is a request topic; no payload is required.

**Example Message:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/water" -n
```

**Device Response:**

- Publishes the current water weight on the same topic:

    ```json
    {
        "water_state": 250
    }
    ```

---

### 5. `hydrapet0001/update/get/status`

**Description:**

Requests the comprehensive status of the HYDRAPET device, including weight, time, button state, LED state, and motor state.

**Payload Formats:**

- **None:** This is a request topic; no payload is required.

**Example Message:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/status" -n
```

**Device Response:**

- Publishes the device status on the same topic:

    ```json
    {
        "weight": 250,
        "timestamp": "2025-01-26T14:30:00",
        "button_state": "RELEASED",
        "led_state": "ON",
        "motor_state": "OFF"
    }
    ```

---

### 6. `hydrapet0001/update/set/alarm`

**Description:**

Adds a new alarm to the HYDRAPET system. The alarm specifies a timestamp and an optional target water weight.

**Payload Formats:**

- **JSON:**

    ```json
    {
        "timestamp": "YYYY-MM-DDTHH:MM:SS",
        "target_weight": 200
    }
    ```

    - `target_weight` is optional. If not provided, defaults to `200` grams.

**Example Message:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/alarm" -m '{"timestamp": "2025-01-27T10:00:00", "target_weight": 200}'
```

**Device Response:**

- Logs indicating the alarm has been added successfully or errors if the format is incorrect.

---

### 7. `hydrapet0001/update/get/alarms`

**Description:**

Requests the list of all scheduled alarms from the HYDRAPET device.

**Payload Formats:**

- **None:** This is a request topic; no payload is required.

**Example Message:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/alarms" -n
```

**Device Response:**

- Publishes the list of alarms on the same topic:

    ```json
    {
        "alarms": [
            {
                "timestamp": "2025-01-27T10:00:00",
                "target_weight": 200
            },
            {
                "timestamp": "2025-01-28T12:30:00",
                "target_weight": 250
            }
        ]
    }
    ```

---

### 8. `hydrapet0001/update/del/alarm`

**Description:**

Deletes an existing alarm based on the specified timestamp.

**Payload Formats:**

- **JSON:**

    ```json
    {
        "timestamp": "YYYY-MM-DDTHH:MM:SS"
    }
    ```

**Example Message:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/del/alarm" -m '{"timestamp": "2025-01-27T10:00:00"}'
```

**Device Response:**

- Logs indicating the alarm has been deleted successfully or errors if the alarm was not found.

---

### 9. `hydrapet0001/update/put/pourwater`

**Description:**

Initiates the water dispensing process to reach a specified target weight.

**Payload Formats:**

- **Plain Text:**

    ```
    <target_weight>
    ```

    - `<target_weight>`: Integer representing weight in grams (e.g., `250`).

- **JSON:**

    ```json
    {
        "target_weight": 250
    }
    ```

**Example Messages:**

- **Plain Text:**

    ```bash
    mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/put/pourwater" -m "250"
    ```

- **JSON:**

    ```bash
    mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/put/pourwater" -m '{"target_weight": 250}'
    ```

**Device Response:**

- Logs indicating the dispensing process has started.
- If dispensing fails, publishes `"empty"` on `hydrapet0001/hydrapetinfo/watertank`.

---

### 10. `hydrapet0001/update/set/tare`

**Description:**

Resets the load cell's tare weight, calibrating the system to the current state without any load.

**Payload Formats:**

- **Any Value:** The content of the message is irrelevant; any payload will trigger the tare process.

**Example Messages:**

- **Plain Text (Number):**

    ```bash
    mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/tare" -m "1"
    ```

- **Plain Text (Text):**

    ```bash
    mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/tare" -m "tare"
    ```

**Device Response:**

- Logs indicating the tare process has started and completed successfully.

---

## Example MQTT Commands

Below are example commands using `mosquitto_pub` and `mosquitto_sub` to interact with HYDRAPET.

### 1. Subscribe to Receive All Responses

To monitor all responses from HYDRAPET, subscribe to relevant topics.

```bash
mosquitto_sub -h broker.mosquitto.org -t "hydrapet0001/#"
```

### 2. Set System Time

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/time" -m '{"timestamp": "2025-01-26T14:30:00"}'
```

### 3. Get System Time

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/time" -n
```

### 4. Set Water Level

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/water" -m "250"
```

### 5. Get Water Level

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/water" -n
```

### 6. Get Device Status

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/status" -n
```

### 7. Add an Alarm

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/alarm" -m '{"timestamp": "2025-01-27T10:00:00", "target_weight": 200}'
```

### 8. Get All Alarms

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/alarms" -n
```

### 9. Delete an Alarm

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/del/alarm" -m '{"timestamp": "2025-01-27T10:00:00"}'
```

### 10. Pour Water to Specific Weight

- **Plain Text:**

    ```bash
    mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/put/pourwater" -m "250"
    ```

- **JSON:**

    ```bash
    mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/put/pourwater" -m '{"target_weight": 250}'
    ```

### 11. Set Tare

- **Plain Text (Number):**

    ```bash
    mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/tare" -m "1"
    ```

- **Plain Text (Text):**

    ```bash
    mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/tare" -m "tare"
    ```

---

## Notes

- **Message Formats:** Ensure that the payloads are correctly formatted as per the topic requirements. Incorrect formats may lead to unexpected behavior or errors.

- **Time Synchronization:** For alarm functionalities to work correctly, ensure that the system time is accurately set, preferably using NTP synchronization.

- **MQTT Security:** Consider securing your MQTT broker with authentication and encryption (TLS) for production environments to prevent unauthorized access.

- **Resource Management:** The ESP32 has limited resources. Ensure that your MQTT broker and other services are optimized to handle the expected load.

- **Logging:** Monitor device logs for debugging and to ensure all functionalities are operating as expected.

- **Extensibility:** The project is modular, allowing for easy addition of new features or integration with other sensors and actuators.

---
