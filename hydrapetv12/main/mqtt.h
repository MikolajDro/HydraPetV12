// mqtt.h

#ifndef MQTT_H
#define MQTT_H

#include <stdint.h>   // For int32_t
#include <time.h>     // For struct tm
#include <stdbool.h>  // For bool

/**
 * @brief Initializes the MQTT client.
 *
 * Configures the MQTT client with broker settings, registers event handlers,
 * and starts the MQTT client.
 */
void mqtt_init(void);

/**
 * @brief Publishes a message to a specific MQTT topic.
 *
 * Sends the provided payload to the designated MQTT topic.
 *
 * @param topic The MQTT topic to publish to.
 * @param payload The message payload to publish.
 */
void mqtt_publish(const char *topic, const char *payload);

/**
 * @brief Publishes all relevant data to a specific MQTT topic.
 *
 * Constructs a JSON payload containing the current weight, timestamp,
 * button state, LED state, and motor state, then publishes it.
 *
 * @param weight The measured weight.
 * @param timestamp The timestamp of the measurement.
 * @param button_state The state of the user button.
 * @param led_state The state of the LED.
 * @param pin15_state The state of pin 15 (motor).
 */
void mqtt_publish_all(int32_t weight, struct tm timestamp, bool button_state, bool led_state, bool pin15_state);

/**
 * @brief Publishes the current water state to a specific MQTT topic.
 *
 * Constructs a JSON payload with the water state and publishes it.
 *
 * @param water_state The current state of the water as an integer.
 */
void mqtt_publish_water_state(int water_state);

/**
 * @brief Publishes the current system time to a specific MQTT topic.
 *
 * Constructs a JSON payload with the current time and publishes it.
 *
 * @param timestamp The current timestamp.
 */
void mqtt_publish_current_time(struct tm timestamp);

/**
 * @brief Sets the callback function to handle incoming MQTT messages.
 *
 * Assigns a user-defined callback function that will be invoked upon receiving MQTT messages.
 *
 * @param callback The callback function that takes the topic and message as parameters.
 */
void mqtt_set_message_callback(void (*callback)(const char *, const char *));

/**
 * @brief Type definition for the MQTT message callback function.
 *
 * The callback function should accept two parameters: the topic and the message.
 *
 * @param topic The MQTT topic of the received message.
 * @param message The MQTT message payload.
 */
typedef void (*mqtt_callback_t)(const char *topic, const char *message);

/**
 * @brief Callback function to handle incoming MQTT messages.
 *
 * Determines the topic of the received message and invokes the appropriate handler.
 *
 * @param topic The MQTT topic of the received message.
 * @param message The MQTT message payload.
 */
void mqtt_message_handler(const char *topic, const char *message);

/**
 * @brief Initiates the process of filling water to a specified weight.
 *
 * Allocates memory for the task parameters and creates the FreeRTOS task
 * responsible for monitoring and controlling the filling process.
 *
 * @param target_weight The desired weight to achieve in grams.
 */
void fill_water_to(int32_t target_weight);

#endif // MQTT_H
