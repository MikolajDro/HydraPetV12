#ifndef LED_H
#define LED_H

#include <stdbool.h>

/**
 * @brief Initializes the LED.
 *
 * Configures the GPIO pin associated with the LED as an output and ensures it is turned off.
 */
void led_init(void);

/**
 * @brief Sets the state of the LED.
 *
 * Turns the LED on or off based on the provided parameter.
 *
 * @param on `true` to turn the LED on, `false` to turn it off.
 */
void led_set_state(bool on);

/**
 * @brief Retrieves the current state of the LED.
 *
 * @return `true` if the LED is on, `false` otherwise.
 */
bool led_get_state(void);

/**
 * @brief Task to blink the LED every 500 milliseconds.
 *
 * This FreeRTOS task toggles the LED state every 500 ms, creating a blinking effect.
 *
 * @param pvParameters Parameter passed to the task.
 */
void led_blink_task(void *pvParameters);

/**
 * @brief Blinks the LED once.
 *
 * Turns the LED on for 200 ms and then turns it off for 200 ms to create a single blink.
 */
void led_blink_once(void);

/**
 * @brief Blinks the LED in pairs.
 *
 * Blinks the LED every 500 ms for approximately 5 seconds (10 cycles).
 */
void led_blink_pair(void);

#endif // LED_H
