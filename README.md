# HYDRAPET MQTT Topics Documentation

This documentation provides a comprehensive overview of all MQTT topics utilized in the HYDRAPET project. It categorizes the topics based on their roles—**Subscribed Topics** (topics the device listens to) and **Published Topics** (topics the device sends messages to). Each topic includes a description, payload format, and example commands for ease of use.

## Table of Contents

1. [Overview](#overview)
2. [Subscribed MQTT Topics](#subscribed-mqtt-topics)
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
3. [Published MQTT Topics](#published-mqtt-topics)
    - [1. `hydrapet0001/hydrapetinfo/watertanklevel`](#1-hydrapet0001hydrapetinfowatertanklevel)
    - [2. `hydrapet0001/hydrapetinfo/watertank`](#2-hydrapet0001hydrapetinfowatertank)
4. [Example MQTT Commands](#example-mqtt-commands)
5. [Notes](#notes)

---

## Overview

HYDRAPET leverages MQTT (Message Queuing Telemetry Transport) for efficient and lightweight communication between the ESP32-based device and external clients or controllers. The system subscribes to specific topics to receive commands and publishes messages to other topics to relay status updates and alerts.

---

## Subscribed MQTT Topics

Subscribed topics are those that HYDRAPET listens to for receiving commands or requests from external clients.

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

- Logs indicating successful time setting or errors if the format is incorrect.

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

## Published MQTT Topics

Published topics are those that HYDRAPET sends messages to, providing status updates, alerts, or other relevant information.

### 1. `hydrapet0001/hydrapetinfo/watertanklevel`

**Description:**

Publishes the current status of the water tank level. The message will be:

- `"Below 30%"` if the variable `s_water_level_state` is `0`.
- `"Water tank is full"` otherwise.

**Payload Formats:**

- **Plain Text:**

    ```
    Below 30%
    ```

    or

    ```
    Water tank is full
    ```

**Automatic Publishing:**

The system automatically publishes to this topic based on the state of `s_water_level_state`. This can be configured to occur at regular intervals (e.g., every minute) or triggered by specific events such as changes in water level.

**Example Messages:**

- **Below 30%:**

    ```bash
    mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/hydrapetinfo/watertanklevel" -m "Below 30%"
    ```

- **Water Tank is Full:**

    ```bash
    mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/hydrapetinfo/watertanklevel" -m "Water tank is full"
    ```

**Use Case:**

Subscribers can monitor this topic to receive real-time updates on the water tank's status, enabling timely actions such as refilling or alerting users.

---

### 2. `hydrapet0001/hydrapetinfo/watertank`

**Description:**

Publishes alerts related to the water tank status, particularly when dispensing fails.

**Payload Formats:**

- **Plain Text:**

    ```
    empty
    ```

**Device Response:**

- Publishes `"empty"` when the dispensing process fails, indicating that the water tank might be empty or the dispensing mechanism is malfunctioning.

**Example Message:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/hydrapetinfo/watertank" -m "empty"
```

**Use Case:**

Subscribers can listen to this topic to receive critical alerts regarding the water tank's condition, ensuring immediate attention to potential issues.

---

## Example MQTT Commands

Below are examples of how to interact with HYDRAPET using MQTT commands via `mosquitto_pub` and `mosquitto_sub`.

### 1. Subscribe to Receive All Responses

To monitor all responses from HYDRAPET, subscribe to relevant topics.

```bash
mosquitto_sub -h broker.mosquitto.org -t "hydrapet0001/#"
```

### 2. Set System Time

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/time" -m '{"timestamp": "2025-01-26T14:30:00"}'
```

**Or:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/time" -m "2025-01-26T14:30:00"
```

### 3. Get System Time

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/time" -n
```

**Subscribe to Receive Response:**

```bash
mosquitto_sub -h broker.mosquitto.org -t "hydrapet0001/update/get/time"
```

### 4. Set Water Level

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/water" -m "250"
```

### 5. Get Water Level

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/water" -n
```

**Subscribe to Receive Response:**

```bash
mosquitto_sub -h broker.mosquitto.org -t "hydrapet0001/update/get/water"
```

### 6. Get Device Status

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/status" -n
```

**Subscribe to Receive Response:**

```bash
mosquitto_sub -h broker.mosquitto.org -t "hydrapet0001/update/get/status"
```

### 7. Add an Alarm

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/set/alarm" -m '{"timestamp": "2025-01-27T10:00:00", "target_weight": 200}'
```

### 8. Get All Alarms

**Command:**

```bash
mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/update/get/alarms" -n
```

**Subscribe to Receive Response:**

```bash
mosquitto_sub -h broker.mosquitto.org -t "hydrapet0001/update/get/alarms"
```

### 9. Delete an Alarm

**Command:**

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

### 12. Monitor Water Tank Level

**Subscribe to Water Tank Level:**

```bash
mosquitto_sub -h broker.mosquitto.org -t "hydrapet0001/hydrapetinfo/watertanklevel"
```

**Device Publishes:**

- If `s_water_level_state` is `0`:

    ```
    Below 30%
    ```

- Otherwise:

    ```
    Water tank is full
    ```

**Example Publish Commands (for testing purposes):**

- **Below 30%:**

    ```bash
    mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/hydrapetinfo/watertanklevel" -m "Below 30%"
    ```

- **Water Tank is Full:**

    ```bash
    mosquitto_pub -h broker.mosquitto.org -t "hydrapet0001/hydrapetinfo/watertanklevel" -m "Water tank is full"
    ```

---

## Notes

1. **Message Formats:**
   - Ensure that the payloads are correctly formatted as per the topic requirements. Incorrect formats may lead to unexpected behavior or errors.
   - For setting time, water levels, and alarms, JSON format is preferred for clarity and extensibility.

2. **Water Tank Level Monitoring:**
   - The `hydrapet0001/hydrapetinfo/watertanklevel` topic provides real-time status of the water tank.
   - Ensure that `s_water_level_state` is correctly updated in your code based on sensor readings.

3. **Synchronization of Time:**
   - Accurate system time is crucial for alarm functionalities. Consider integrating SNTP for automatic time synchronization.

4. **MQTT Security:**
   - Public brokers like `broker.mosquitto.org` are suitable for testing. For production, use a secure, private broker with authentication and TLS encryption.

5. **Resource Management:**
   - ESP32 has limited resources. Optimize MQTT message sizes and frequency to prevent resource exhaustion.

6. **Error Handling:**
   - The system logs errors for debugging. Consider adding additional MQTT notifications for critical errors.

7. **Extensibility:**
   - The modular design allows for easy addition of new features, such as integrating more sensors or actuators.

8. **Testing:**
   - Thoroughly test each MQTT topic and its corresponding functionality in a controlled environment before deploying to production.

9. **Documentation:**
   - Keep the documentation updated as new features and topics are added to ensure clarity for users and developers.

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
│   ├── water_level.c
│   ├── water_level.h
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
- **water_level.c / water_level.h:** Monitors and updates the water tank level status.
- **CMakeLists.txt:** Build configuration for ESP-IDF.
- **README.md:** Project overview and setup instructions.
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

- **Water Tank Level Not Publishing Correct Status:**
  - Ensure `s_water_level_state` is correctly updated in the code based on sensor readings.
  - Verify the publishing function is being called appropriately.

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

## Contact

For any questions, issues, or feature requests, please open an issue on the [GitHub repository](https://github.com/yourusername/hydrapet/issues).
