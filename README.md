# Hydrapet

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)

## Overview

**Hydrapet** is an automated water dispensing system designed to provide precise water measurements, monitor water levels, and manage water dispensing through a user-friendly interface. Leveraging the power of ESP32, FreeRTOS, and MQTT, Hydrapet offers seamless connectivity, real-time monitoring, and remote control capabilities. The system integrates various sensors and actuators to ensure efficient and reliable operation, making it ideal for applications such as pet hydration stations, automated watering systems, and more.

## Features

- **Weight Measurement:** Utilizes the HX711 load cell amplifier for accurate weight readings.
- **Motor Control:** Manages water dispensing through motor operations.
- **Water Level Monitoring:** Tracks the water level using a dedicated sensor.
- **MQTT Communication:** Publishes sensor data and receives remote commands via MQTT.
- **Wi-Fi Connectivity:** Connects to Wi-Fi networks with automatic reconnection attempts.
- **Alarm System:** Schedules and triggers alarms for specific events or conditions.
- **LED Indicators:** Provides visual feedback for system status and alerts.
- **User Interaction:** Incorporates buttons for manual controls and interactions.

## Hardware Requirements

- **Microcontroller:**
  - [ESP32 Development Board](https://www.espressif.com/en/products/socs/esp32) or compatible.
  
- **Sensors and Actuators:**
  - **HX711 Load Cell Amplifier:** For weight measurements.
  - **Load Cell:** To measure the weight of water.
  - **Motor:** Such as a pump or servo motor connected to GPIO15 for dispensing water.
  - **Water Level Sensor:** Connected to GPIO10 to monitor the water tank level.
  
- **Indicators and Controls:**
  - **LEDs:** For status and alert indications.
  - **Buttons:** For user interactions like pairing and manual controls.
  
- **Connectivity:**
  - **Wi-Fi Module:** Integrated within ESP32 for network connectivity.
  
- **Power Supply:**
  - Suitable power adapter matching the requirements of ESP32 and connected peripherals.
  
- **Miscellaneous:**
  - **Breadboard and Jumper Wires:** For prototyping and connections.
  - **Resistors, Capacitors, etc.:** As needed for circuit stability and functionality.

## Software Requirements

- **Development Framework:**
  - [ESP-IDF (Espressif IoT Development Framework)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)
  
- **Operating System:**
  - FreeRTOS (included with ESP-IDF)
  
- **Communication Protocols:**
  - MQTT (e.g., [Mosquitto](https://mosquitto.org/) broker)
  
- **Programming Languages:**
  - C
  
- **Additional Tools:**
  - [Doxygen](https://www.doxygen.nl/index.html) for documentation generation (optional)

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/yourusername/hydrapet.git
cd hydrapet
```

### 2. Set Up ESP-IDF

Follow the official [ESP-IDF Get Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) to install and configure the ESP-IDF environment on your system.

### 3. Configure the Project

Edit the `config.h` file to set your Wi-Fi credentials and other configuration parameters.

```c
// config.h 

#ifndef MAIN_CONFIG_H_
#define MAIN_CONFIG_H_

/** @brief Duration between each publish in milliseconds (60 seconds) */
#define PUBLISH_ALL_DURATION_TIME 	60000 /**< @brief 1 minute between refresh */

/** @brief SSID of the Wi-Fi network */
#define EXAMPLE_ESP_WIFI_SSID      	"Your_WiFi_SSID"         

/** @brief Password of the Wi-Fi network */
#define EXAMPLE_ESP_WIFI_PASS      	"Your_WiFi_Password"        

/** @brief Maximum number of connection retries */
#define EXAMPLE_ESP_MAXIMUM_RETRY  	10                

#endif /* MAIN_CONFIG_H_ */
```

Alternatively, you can use `menuconfig` for configuring the project:

```bash
idf.py menuconfig
```

### 4. Build and Flash

Connect your ESP32 development board to your computer and run the following commands:

```bash
idf.py build
idf.py flash
idf.py monitor
```

Ensure that the flashing process completes without errors and the ESP32 boots up successfully.

## Configuration

All configurable parameters are defined in the `config.h` file. You can adjust the following settings as needed:

- **Publish Interval:**
  - `PUBLISH_ALL_DURATION_TIME`: Sets the duration between each data publish cycle (default is 60,000 ms or 1 minute).

- **Wi-Fi Credentials:**
  - `EXAMPLE_ESP_WIFI_SSID`: Your Wi-Fi network's SSID.
  - `EXAMPLE_ESP_WIFI_PASS`: Your Wi-Fi network's password.

- **Connection Retries:**
  - `EXAMPLE_ESP_MAXIMUM_RETRY`: Maximum number of attempts to reconnect to Wi-Fi after a disconnection.

Ensure that sensitive information like Wi-Fi passwords is secured and not exposed in public repositories.

## Usage

### System Operation

1. **Startup:**
   - Upon powering up, Hydrapet initializes all modules, including Wi-Fi, MQTT, sensors, and actuators.
   - It attempts to connect to the configured Wi-Fi network. If the connection fails, it retries based on the maximum retry count.

2. **Data Publishing:**
   - Every minute (configurable), the system publishes the following data to the MQTT broker:
     - **Weight Measurement:** Current weight from the HX711 sensor.
     - **Timestamp:** Current system time.
     - **Button State:** Status of user buttons.
     - **LED State:** Current state of LEDs.
     - **Motor State:** Status of the motor (ON/OFF).

3. **Water Level Monitoring:**
   - Continuously monitors the water tank level. If the water level drops below 30%, it publishes an alert via MQTT and attempts to dispense water to reach the target weight.

4. **Alarms:**
   - Allows scheduling of alarms to perform specific actions at predefined times, such as dispensing water or other tasks.

5. **User Interactions:**
   - Buttons can be used to perform actions like pairing or manual control. LED indicators provide visual feedback for various statuses and alerts.

### MQTT Integration

Hydrapet communicates with an MQTT broker to publish sensor data and receive commands. Ensure that your MQTT broker is accessible and correctly configured.

- **Publishing Topics:**
  - `hydrapet0001/hydrapetinfo/all`: Publishes all sensor data.
  - `hydrapet0001/hydrapetinfo/water`: Publishes the current water state.
  - `hydrapet0001/hydrapetinfo/time`: Publishes the current system time.
  - `hydrapet0001/hydrapetinfo/watertanklevel`: Publishes the water tank level status.

- **Subscribing Topics:**
  - Various topics are subscribed for receiving commands, such as setting time, adjusting water levels, managing alarms, etc.

### LED Indicators

LEDs provide visual feedback for system statuses, alerts, and user interactions. Refer to the LED module documentation for specific LED behaviors and indicators.

### Buttons

User buttons allow manual interactions with the system, such as initiating water dispensing, resetting alarms, or pairing the device. Refer to the Buttons module documentation for detailed usage instructions.

## Project Structure

```
hydrapetv12/
├── main/
│   ├── main.c
│   ├── config.h
│   ├── mqtt.c
│   ├── mqtt.h
│   ├── wifi.c
│   ├── wifi.h
│   ├── motor.c
│   ├── motor.h
│   ├── alarms.c
│   ├── alarms.h
│   ├── water_level_sensor.c
│   ├── water_level_sensor.h
│   ├── led.c
│   ├── led.h
│   ├── buttons.c
│   ├── buttons.h
|   └──CMakeLists.txt
Makefile
README.md
```

- **hydrapetv12/**
 - **main/**: Contains all source (`.c`) and header (`.h`) files for the project, organized by functionality.
   - **main.c**: Entry point of the application.
   - **config.h**: Configuration parameters for the project.
   - **mqtt.c & mqtt.h**: MQTT client implementation and interface.
   - **wifi.c & wifi.h**: Wi-Fi connectivity implementation and interface.
   - **motor.c & motor.h**: Motor control implementation and interface.
   - **alarms.c & alarms.h**: Alarm management implementation and interface.
   - **water_level_sensor.c & water_level_sensor.h**: Water level sensor implementation and interface.
   - **led.c & led.h**: LED control implementation and interface.
   - **buttons.c & buttons.h**: Button handling implementation and interface.
   - **CMakeLists.txt & Makefile**: Build configuration files for compiling the project.
- **README.md**: Project documentation.

## Troubleshooting

- **Wi-Fi Connection Issues:**
  - Ensure that the SSID and password in `config.h` are correct.
  - Check the Wi-Fi signal strength and ensure the ESP32 is within range.
  - Verify that the Wi-Fi network supports WPA2-PSK authentication.

- **MQTT Publishing Failures:**
  - Confirm that the MQTT broker address is correct and accessible.
  - Ensure that the ESP32 has a stable internet connection.
  - Check MQTT broker logs for any authentication or connection issues.

- **Sensor Readings Inaccurate:**
  - Verify the connections of the HX711 load cell and ensure proper calibration.
  - Check for any physical obstructions or issues with the load cell.

- **Motor Not Operating:**
  - Ensure that the motor is correctly connected to GPIO15.
  - Check the power supply to the motor and ensure it's adequate.
  - Verify that the motor control logic in the `motor` module is functioning correctly.

- **LED Indicators Not Working:**
  - Confirm the connections of the LEDs to the designated GPIO pins.
  - Check the LED module initialization and control logic.

- **Alarms Not Triggering:**
  - Ensure that alarms are correctly added via MQTT commands.
  - Verify that the system time is correctly set.
  - Check the alarm task's operation and logs for any errors.

## Contributing

Contributions are welcome! If you'd like to enhance Hydrapet, please follow these guidelines:

1. **Fork the Repository:** Click the "Fork" button on the repository page.
2. **Create a Feature Branch:** `git checkout -b feature/YourFeature`
3. **Commit Your Changes:** `git commit -m "Add your feature"`
4. **Push to the Branch:** `git push origin feature/YourFeature`
5. **Open a Pull Request:** Submit a pull request detailing your changes and the reasons behind them.

Please ensure that your code adheres to the project's coding standards and includes appropriate documentation.

