/*
 * water_level_sensor.h
 *
 *  Created on: 27 sty 2025
 *      Author: mikol
 */

#ifndef MAIN_WATER_LEVEL_SENSOR_H_
#define MAIN_WATER_LEVEL_SENSOR_H_

#include <stdbool.h>

/**
 * @brief Retrieves the current state of the water level sensor.
 *
 * @return `true` if the water level is below 30%, `false` otherwise.
 */
bool water_level_sensor_state(void);

/**
 * @brief Initializes the water level sensor GPIO configuration.
 *
 * Configures the tank level sensor GPIO pin as an input with a pull-up resistor.
 * This setup ensures reliable reading of the water level status.
 */
void water_level_sensor_init(void);

/**
 * @brief Publishes the current water tank level status via MQTT.
 *
 * This function checks the state of the water level sensor and publishes an appropriate
 * message to the MQTT topic `hydrapet0001/hydrapetinfo/watertanklevel`.
 *
 * @note The payload is a simple string indicating whether the water level is below 30% or full.
 */
void mqtt_publish_water_tank_level(void);

/**
 * @brief FreeRTOS task for monitoring the water level sensor.
 *
 * This task continuously monitors the water level sensor state. When a transition
 * from high to low is detected (indicating that the water level has dropped below 30%),
 * it logs the event, attempts to reconnect to Wi-Fi if disconnected, blinks an LED for
 * visual feedback, and publishes the water tank level status via MQTT.
 *
 * @param pvParameters Pointer to task parameters (unused).
 */
void water_level_sensor_task(void *pvParameters);

#endif /* MAIN_WATER_LEVEL_SENSOR_H_ */
