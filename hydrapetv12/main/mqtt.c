// mqtt.c

#include "mqtt.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>
#include "hx711.h"
#include "motor.h"
#include "led.h"
#include "buttons.h"
#include <sys/time.h>
#include <errno.h>
#include "alarms.h"

/** @brief Tag used for ESP logging */
static const char *TAG = "MQTT";

/** @brief Handle for the MQTT client */
static esp_mqtt_client_handle_t mqtt_client = NULL;

/** @brief Global callback function for handling incoming MQTT messages */
static mqtt_callback_t global_mqtt_callback = NULL;

/**
 * @brief Handler for MQTT events.
 *
 * This function is called by the MQTT client library upon various MQTT events such as
 * connection, disconnection, and incoming data. It manages subscriptions and invokes
 * the global callback for incoming messages.
 *
 * @param handler_args Arguments passed to the handler.
 * @param base The base of the event.
 * @param event_id The ID of the event.
 * @param event_data Data associated with the event.
 */
static void mqtt_event_handler_cb(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            // Subscribe to relevant topics upon connection
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/set/time", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/get/time", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/set/water", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/get/water", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/get/status", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/set/alarm", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/get/alarms", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/del/alarm", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/put/pourwater", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/set/tare", 0);
            ESP_LOGI(TAG, "MQTT topic subscriptions completed");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_DATA:
            // Handle incoming MQTT data
            if (global_mqtt_callback != NULL) {
                // Copy topic and message to buffers
                char topic[128];
                char message[256];
                memset(topic, 0, sizeof(topic));
                memset(message, 0, sizeof(message));

                // Copy topic
                strncpy(topic, event->topic, event->topic_len);
                topic[event->topic_len] = '\0';

                // Copy message
                strncpy(message, event->data, event->data_len);
                message[event->data_len] = '\0';

                // Invoke the callback
                global_mqtt_callback(topic, message);
            }
            break;
        // Handle other events as needed
        default:
            break;
    }
}

/**
 * @brief Initializes the MQTT client and starts it.
 *
 * Configures the MQTT client with the broker URI, registers the event handler,
 * and starts the MQTT client.
 */
void mqtt_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = "mqtt://test.mosquitto.org:1883", // Change to your own broker if needed
            },
        },
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return;
    }

    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler_cb, NULL);
    esp_mqtt_client_start(mqtt_client);
    ESP_LOGI(TAG, "MQTT initialized.");
}

/**
 * @brief Publishes a message to a specific MQTT topic.
 *
 * @param topic The MQTT topic to publish to.
 * @param payload The message payload to publish.
 */
void mqtt_publish(const char *topic, const char *payload)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return;
    }

    esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);
    ESP_LOGI(TAG, "MQTT publish: %s -> %s", topic, payload);
}

/**
 * @brief Publishes all relevant data to a specific MQTT topic.
 *
 * This function constructs a JSON payload containing the current weight, timestamp,
 * button state, LED state, and motor state, then publishes it to the designated topic.
 *
 * @param weight The measured weight.
 * @param timestamp The timestamp of the measurement.
 * @param button_state The state of the user button.
 * @param led_state The state of the LED.
 * @param pin15_state The state of pin 15 (motor).
 */
void mqtt_publish_all(int32_t weight, struct tm timestamp, bool button_state, bool led_state, bool pin15_state)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return;
    }

    char topic[50];
    char payload[300]; // Increased size to accommodate all data

    // Topic: hydrapet/get/all
    snprintf(topic, sizeof(topic), "hydrapet0001/hydrapetinfo/all");

    // Create JSON payload with data
    snprintf(payload, sizeof(payload),
             "{\"weight\": %ld, \"timestamp\": \"%04d-%02d-%02dT%02d:%02d:%02d\", \"button_state\": \"%s\", \"led_state\": \"%s\", \"motor_state\": \"%s\"}",
             weight,
             timestamp.tm_year + 1900, timestamp.tm_mon + 1, timestamp.tm_mday,
             timestamp.tm_hour, timestamp.tm_min, timestamp.tm_sec,
             button_state ? "PRESSED" : "RELEASED",
             led_state ? "ON" : "OFF",
             pin15_state ? "ON" : "OFF");

    esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);
    ESP_LOGI(TAG, "MQTT publish: %s -> %s", topic, payload);
}

/**
 * @brief Publishes the current water state to a specific MQTT topic.
 *
 * Constructs a JSON payload with the water state and publishes it.
 *
 * @param water_state The current state of the water as an integer.
 */
void mqtt_publish_water_state(int water_state)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return;
    }

    char topic[50];
    char payload[50];

    // Topic: hydrapet0001/hydrapetinfo/water
    snprintf(topic, sizeof(topic), "hydrapet0001/hydrapetinfo/water");

    // Create JSON payload with water state
    snprintf(payload, sizeof(payload), "{\"water_state\": %d}", water_state);

    esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);
    ESP_LOGI(TAG, "MQTT publish: %s -> %s", topic, payload);
}

/**
 * @brief Publishes the current system time to a specific MQTT topic.
 *
 * Constructs a JSON payload with the current time and publishes it.
 *
 * @param timestamp The current timestamp.
 */
void mqtt_publish_current_time(struct tm timestamp)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return;
    }

    char topic[50];
    char payload[100];

    // Topic: hydrapet0001/hydrapetinfo/time
    snprintf(topic, sizeof(topic), "hydrapet0001/hydrapetinfo/time");

    // Create JSON payload with current time
    snprintf(payload, sizeof(payload), "{\"current_time\": \"%04d-%02d-%02dT%02d:%02d:%02d\"}",
             timestamp.tm_year + 1900, timestamp.tm_mon + 1, timestamp.tm_mday,
             timestamp.tm_hour, timestamp.tm_min, timestamp.tm_sec);

    esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);
    ESP_LOGI(TAG, "MQTT publish: %s -> %s", topic, payload);
}

/**
 * @brief Sets the callback function to handle incoming MQTT messages.
 *
 * @param callback The callback function that takes the topic and message as parameters.
 */
void mqtt_set_message_callback(void (*callback)(const char *, const char *))
{
    global_mqtt_callback = callback;
    ESP_LOGI(TAG, "MQTT message callback set.");
}

/**
 * @brief Retrieves the state of pin 15.
 *
 * This function uses the motor state function to determine the state of pin 15.
 *
 * @return `true` if pin 15 is active (motor on), `false` otherwise.
 */
bool get_pin15_state(void) {
    // Uses function from motor.h
    return get_motor_state();
}

/**
 * @brief Sets the system time based on a provided string.
 *
 * Parses a time string in the format "YYYY-MM-DDTHH:MM:SS" and sets the system time.
 *
 * @param time_str The time string to parse and set.
 * @return `ESP_OK` on success, `ESP_ERR_INVALID_ARG` on parsing errors, or `ESP_FAIL` on system call failures.
 */
static esp_err_t set_system_time(const char *time_str) {
    struct tm tm_time;
    memset(&tm_time, 0, sizeof(struct tm));

    // Parse the string into a tm structure
    // Assumes format "YYYY-MM-DDTHH:MM:SS"
    if (sscanf(time_str, "%d-%d-%dT%d:%d:%d",
               &tm_time.tm_year,
               &tm_time.tm_mon,
               &tm_time.tm_mday,
               &tm_time.tm_hour,
               &tm_time.tm_min,
               &tm_time.tm_sec) != 6) {
        ESP_LOGE(TAG, "Invalid time format: %s", time_str);
        return ESP_ERR_INVALID_ARG;
    }

    // Adjust values
    tm_time.tm_year -= 1900; // tm_year: years since 1900
    tm_time.tm_mon -= 1;     // tm_mon: months since January [0-11]

    // Convert tm structure to epoch time
    time_t epoch_time = mktime(&tm_time);
    if (epoch_time == -1) {
        ESP_LOGE(TAG, "Error converting time");
        return ESP_ERR_INVALID_ARG;
    }

    // Set system time
    struct timeval tv;
    tv.tv_sec = epoch_time;
    tv.tv_usec = 0;
    if (settimeofday(&tv, NULL) != 0) {
        ESP_LOGE(TAG, "Failed to set time: %s", strerror(errno));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "System time set to: %s", time_str);
    return ESP_OK;
}

/**
 * @brief Handles the "pourwater" MQTT message to initiate water filling.
 *
 * Parses the incoming message to extract the target weight and starts the water filling process.
 *
 * @param message The received MQTT message containing the target weight.
 */
static void handle_pourwater(const char *message) {
    ESP_LOGI(TAG, "Handling pourwater with message: %s", message);

    int target_weight = 0;

    // Check if the message starts with '{', suggesting JSON format
    if (message[0] == '{') {
        ESP_LOGI(TAG, "Parsing message as JSON.");

        // Look for the key "target_weight"
        const char *key = "\"target_weight\"";
        char *key_ptr = strstr(message, key);
        if (key_ptr != NULL) {
            // Look for the colon after the key
            char *colon_ptr = strstr(key_ptr, ":");
            if (colon_ptr != NULL) {
                // Parse the value after the colon
                // Assumes the value is an integer
                if (sscanf(colon_ptr, ":%d", &target_weight) == 1) {
                    ESP_LOGI(TAG, "Parsed target_weight from JSON: %d g", target_weight);
                } else {
                    ESP_LOGE(TAG, "Failed to parse target_weight value from JSON.");
                    return;
                }
            } else {
                ESP_LOGE(TAG, "Colon not found after key target_weight.");
                return;
            }
        } else {
            ESP_LOGE(TAG, "Key \"target_weight\" not found in JSON message.");
            return;
        }
    } else {
        ESP_LOGI(TAG, "Parsing message as integer.");
        // Parse the message as a simple integer
        target_weight = atoi(message);
        if (target_weight <= 0) {
            ESP_LOGE(TAG, "Invalid target_weight value: %s", message);
            return;
        }
        ESP_LOGI(TAG, "Parsed target_weight as int: %d g", target_weight);
    }

    // Validate the target_weight
    if (target_weight <= 0) {
        ESP_LOGE(TAG, "Invalid target_weight after parsing: %d", target_weight);
        return;
    }

    // Initiate water filling
    ESP_LOGI(TAG, "Starting water filling to weight: %d g", target_weight);
    fill_water_to(target_weight);
}

/**
 * @brief Handles the "tare" MQTT message to perform tare calibration.
 *
 * Executes the tare function and provides visual feedback via LEDs.
 *
 * @param message The received MQTT message (content is not used in this handler).
 */
static void handle_tare(const char *message) {
    ESP_LOGI(TAG, "Handling tare with message: %s", message);
    
    led_blink_once();
    
    tare();

    ESP_LOGI(TAG, "Tare function has been called.");
}

/**
 * @brief Callback function to handle incoming MQTT messages.
 *
 * Determines the topic of the received message and invokes the appropriate handler.
 *
 * @param topic The MQTT topic of the received message.
 * @param message The MQTT message payload.
 */
void mqtt_message_handler(const char *topic, const char *message)
{
    ESP_LOGI(TAG, "Received message on topic: %s -> %s", topic, message);

    // Handle different topics
    if (strcmp(topic, "hydrapet0001/update/set/time") == 0) {
        ESP_LOGI(TAG, "Setting time based on message: %s", message);

        esp_err_t err = set_system_time(message);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Time has been successfully set.");
        } else {
            ESP_LOGE(TAG, "Failed to set time.");
        }
    }
    else if (strcmp(topic, "hydrapet0001/update/get/time") == 0) {
        // Example: Sending current time
        ESP_LOGI(TAG, "Sending current time upon request");
        // Get current time and send it
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        mqtt_publish_current_time(timeinfo);
    }
    else if (strcmp(topic, "hydrapet0001/update/set/water") == 0) {
        // Example: Setting water state
        ESP_LOGI(TAG, "Setting water state based on message: %s", message);
        // You can update the water state here, e.g., by controlling a valve
        // Convert message to int
        int target_weight = (int )atoi(message);
        
        if (target_weight > 0) {
            fill_water_to(target_weight);
        } else {
            ESP_LOGE(TAG, "Invalid target_weight value: %s", message);
        }
    }
    else if (strcmp(topic, "hydrapet0001/update/get/water") == 0) {
        // Example: Sending current water state
        ESP_LOGI(TAG, "Sending current water state upon request");
        // Get current water state and send it
        // Assuming water state is weight, adjust as needed
        int water_state = get_water_weight(); // Adjust to actual function if different
        mqtt_publish_water_state(water_state);
    }
    else if (strcmp(topic, "hydrapet0001/update/get/status") == 0) {
        // Example: Sending device status
        ESP_LOGI(TAG, "Sending device status upon request");
        // Gather all device status information and send it
        int32_t weight = get_water_weight();
        time_t now;
        struct tm current_time;
        time(&now);
        localtime_r(&now, &current_time);
        bool button_state = user_button_state();
        bool led_state = led_get_state(); // Assuming led_get_state() is available
        bool pin15_state = get_pin15_state();

        mqtt_publish_all(weight, current_time, button_state, led_state, pin15_state);
    }
    else if (strcmp(topic, "hydrapet0001/update/set/alarm") == 0) {
        // Setting an alarm
        ESP_LOGI(TAG, "Setting alarm based on message: %s", message);
        
        // Parse JSON
        char timestamp_str[25];
        int target_weight = 200; // Default value

        // Assume message is in JSON format:
        // {"timestamp": "YYYY-MM-DDTHH:MM:SS", "target_weight": 200}
        sscanf(message, "{\"timestamp\": \"%24[^\"]\", \"target_weight\": %d}", timestamp_str, &target_weight);

        // Parse timestamp_str into struct tm
        struct tm alarm_time;
        memset(&alarm_time, 0, sizeof(struct tm));
        if (sscanf(timestamp_str, "%d-%d-%dT%d:%d:%d",
                   &alarm_time.tm_year,
                   &alarm_time.tm_mon,
                   &alarm_time.tm_mday,
                   &alarm_time.tm_hour,
                   &alarm_time.tm_min,
                   &alarm_time.tm_sec) != 6) {
            ESP_LOGE(TAG, "Invalid alarm time format: %s", timestamp_str);
            return;
        }

        // Adjust values
        alarm_time.tm_year -= 1900; // tm_year: years since 1900
        alarm_time.tm_mon -= 1;     // tm_mon: months since January [0-11]

        // Add alarm
        Alarm_t new_alarm;
        new_alarm.timestamp = alarm_time;
        new_alarm.target_weight = target_weight;

        if (add_alarm(&new_alarm)) {
            ESP_LOGI(TAG, "Alarm has been added.");
        } else {
            ESP_LOGE(TAG, "Failed to add alarm.");
        }
    }
    else if (strcmp(topic, "hydrapet0001/update/get/alarms") == 0) {
        // Retrieving list of alarms
        ESP_LOGI(TAG, "Retrieving list of alarms upon request.");
        get_alarms();
    }
    else if (strcmp(topic, "hydrapet0001/update/del/alarm") == 0) {
        // Deleting an alarm
        ESP_LOGI(TAG, "Deleting alarm based on message: %s", message);
        
        // Parse JSON
        char timestamp_str[25];
        sscanf(message, "{\"timestamp\": \"%24[^\"]\"}", timestamp_str);

        // Parse timestamp_str into struct tm
        struct tm del_alarm_time;
        memset(&del_alarm_time, 0, sizeof(struct tm));
        if (sscanf(timestamp_str, "%d-%d-%dT%d:%d:%d",
                   &del_alarm_time.tm_year,
                   &del_alarm_time.tm_mon,
                   &del_alarm_time.tm_mday,
                   &del_alarm_time.tm_hour,
                   &del_alarm_time.tm_min,
                   &del_alarm_time.tm_sec) != 6) {
            ESP_LOGE(TAG, "Invalid alarm deletion time format: %s", timestamp_str);
            return;
        }
        // Adjust values
        del_alarm_time.tm_year -= 1900; // tm_year: years since 1900
        del_alarm_time.tm_mon -= 1;     // tm_mon: months since January [0-11]

        // Delete alarm
        if (delete_alarm(&del_alarm_time)) {
            ESP_LOGI(TAG, "Alarm has been deleted.");
        } else {
            ESP_LOGE(TAG, "Failed to delete alarm.");
        }
    }
    else if (strcmp(topic, "hydrapet0001/update/put/pourwater") == 0) {
        // Handle pouring water based on received weight
        handle_pourwater(message);
    }
    else if (strcmp(topic, "hydrapet0001/update/set/tare") == 0) {
        // Handle tare
        handle_tare(message);
    }
}
