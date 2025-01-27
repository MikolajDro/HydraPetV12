// hx711.h

#ifndef HX711_H
#define HX711_H

#include <stdint.h>
#include <time.h>

/**
 * @brief Structure representing a weight measurement.
 */
typedef struct {
    int32_t weight;          /**< @brief Weight in raw units */
    struct tm timestamp;     /**< @brief Timestamp of the measurement */
} Measurement;

/** @brief Size of the circular measurement buffer */
#define MEASUREMENT_BUFFER_SIZE 1000

// Deklaracje funkcji

/**
 * @brief Initializes the HX711 sensor and related peripherals.
 *
 * Configures GPIO pins, initializes mutexes, and performs tare calibration.
 */
void hx711_init(void);

/**
 * @brief Retrieves the current water weight.
 *
 * Reads raw data from the HX711 sensor, applies tare calibration,
 * converts it to grams, and adds the measurement to the buffer.
 *
 * @return Water weight in grams as a signed 32-bit integer.
 */
int32_t get_water_weight(void);

/**
 * @brief Performs tare calibration to set the current weight as zero.
 *
 * Reads the raw weight and sets it as the tare offset.
 */
void tare(void);

/**
 * @brief Adds a weight measurement to the circular buffer.
 *
 * Creates a Measurement struct with the current weight and timestamp,
 * then adds it to the buffer in a thread-safe manner.
 *
 * @param weight The weight value to add to the buffer.
 */
void add_measurement(int32_t weight);

/**
 * @brief Reads the oldest measurement from the buffer.
 *
 * Retrieves the oldest measurement from the circular buffer in a thread-safe manner.
 *
 * @param measurement Pointer to a Measurement struct where the data will be stored.
 * @return `1` if a measurement was successfully read, `0` otherwise.
 */
int read_measurement(Measurement *measurement);

/**
 * @brief Retrieves and prints all measurements from the buffer.
 *
 * Continuously reads measurements from the buffer and prints their weight and timestamp
 * until the buffer is empty.
 */
void get_all_measurements(void); // Opcjonalnie, funkcja do odczytu wszystkich pomiarów

/**
 * @brief FreeRTOS task for filling water to a specified weight.
 *
 * Monitors the water weight and controls the motor to fill water until the target weight is reached.
 * Handles error conditions such as insufficient weight gain over a period.
 *
 * @param pvParameters Pointer to fill_water_params_t containing the target weight.
 */
void hx711_task(void *pvParameters);

#endif // HX711_H
