// alarms.c

#include "alarms.h"
#include "mqtt.h"
#include "hx711.h"
#include "motor.h"
#include "led.h"
#include "driver/rtc_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

static const char *TAG = "ALARMS";

/** @brief List of alarms */
static Alarm_t alarms_list[MAX_ALARMS];

/** @brief Current count of alarms in the list */
static int alarms_count = 0;

/** @brief Mutex to protect access to the alarms list */
static SemaphoreHandle_t alarms_mutex = NULL;

/** @brief Forward declaration of the fill_water_to function */
extern void fill_water_to(int32_t target_weight);

/**
 * @brief Compares two time structures for equality.
 *
 * This function checks if all components of the provided `struct tm` instances are equal.
 *
 * @param a Pointer to the first time structure.
 * @param b Pointer to the second time structure.
 * @return `true` if both times are equal, `false` otherwise.
 */
static bool is_time_equal(const struct tm *a, const struct tm *b) {
    return (a->tm_year == b->tm_year) &&
           (a->tm_mon == b->tm_mon) &&
           (a->tm_mday == b->tm_mday) &&
           (a->tm_hour == b->tm_hour) &&
           (a->tm_min == b->tm_min) &&
           (a->tm_sec == b->tm_sec);
}

/**
 * @brief Initializes the alarms module.
 *
 * This function creates a mutex to protect the alarms list and starts the FreeRTOS task
 * responsible for monitoring and triggering alarms.
 */
void alarms_init(void) {
    // Create mutex
    alarms_mutex = xSemaphoreCreateMutex();
    if (alarms_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create alarms_mutex");
        return;
    }

    // Start alarms monitoring task
    xTaskCreate(alarms_task, "alarms_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "Alarms module initialized.");
}

/**
 * @brief Adds an alarm to the alarms list.
 *
 * This function adds a new alarm to the `alarms_list` if there is space available.
 * It ensures thread-safe access using a mutex.
 *
 * @param alarm Pointer to the `Alarm_t` structure to be added.
 * @return `true` if the alarm was successfully added, `false` if the list is full or an error occurred.
 */
bool add_alarm(const Alarm_t *alarm) {
    if (xSemaphoreTake(alarms_mutex, portMAX_DELAY) == pdTRUE) {
        if (alarms_count >= MAX_ALARMS) {
            ESP_LOGE(TAG, "Alarms list full, cannot add alarm.");
            xSemaphoreGive(alarms_mutex);
            return false;
        }

        // Add alarm to the list
        alarms_list[alarms_count] = *alarm;
        alarms_count++;
        ESP_LOGI(TAG, "Alarm added: %04d-%02d-%02dT%02d:%02d:%02d, target_weight=%ld",
                 alarm->timestamp.tm_year + 1900,
                 alarm->timestamp.tm_mon + 1,
                 alarm->timestamp.tm_mday,
                 alarm->timestamp.tm_hour,
                 alarm->timestamp.tm_min,
                 alarm->timestamp.tm_sec,
                 alarm->target_weight);
        xSemaphoreGive(alarms_mutex);
        return true;
    }
    return false;
}

/**
 * @brief Deletes an alarm from the alarms list based on its timestamp.
 *
 * This function searches for an alarm matching the provided timestamp and removes it
 * from the `alarms_list`. It ensures thread-safe access using a mutex.
 *
 * @param timestamp Pointer to the `struct tm` representing the time of the alarm to delete.
 * @return `true` if the alarm was successfully deleted, `false` if not found or an error occurred.
 */
bool delete_alarm(const struct tm *timestamp) {
    if (xSemaphoreTake(alarms_mutex, portMAX_DELAY) == pdTRUE) {
        for (int i = 0; i < alarms_count; i++) {
            if (is_time_equal(&alarms_list[i].timestamp, timestamp)) {
                // Remove alarm by shifting the rest
                for (int j = i; j < alarms_count -1; j++) {
                    alarms_list[j] = alarms_list[j+1];
                }
                alarms_count--;
                ESP_LOGI(TAG, "Alarm deleted: %04d-%02d-%02dT%02d:%02d:%02d",
                         timestamp->tm_year + 1900,
                         timestamp->tm_mon + 1,
                         timestamp->tm_mday,
                         timestamp->tm_hour,
                         timestamp->tm_min,
                         timestamp->tm_sec);
                xSemaphoreGive(alarms_mutex);
                return true;
            }
        }
        ESP_LOGE(TAG, "Alarm not found for deletion: %04d-%02d-%02dT%02d:%02d:%02d",
                 timestamp->tm_year + 1900,
                 timestamp->tm_mon + 1,
                 timestamp->tm_mday,
                 timestamp->tm_hour,
                 timestamp->tm_min,
                 timestamp->tm_sec);
        xSemaphoreGive(alarms_mutex);
        return false;
    }
    return false;
}

/**
 * @brief Retrieves all alarms and publishes them via MQTT.
 *
 * This function constructs a JSON payload containing all alarms in the `alarms_list`
 * and publishes it to the MQTT topic `hydrapet0001/update/get/alarms`.
 */
void get_alarms(void) {
    if (xSemaphoreTake(alarms_mutex, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take alarms_mutex");
        return;
    }

    // Start building JSON payload
    char json_payload[1024];
    size_t offset = 0;
    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "{\"alarms\": [");

    for (int i = 0; i < alarms_count; i++) {
        const Alarm_t *alarm = &alarms_list[i];
        offset += snprintf(json_payload + offset, sizeof(json_payload) - offset,
                         "{\"timestamp\": \"%04d-%02d-%02dT%02d:%02d:%02d\", \"target_weight\": %ld}%s",
                         alarm->timestamp.tm_year + 1900,
                         alarm->timestamp.tm_mon + 1,
                         alarm->timestamp.tm_mday,
                         alarm->timestamp.tm_hour,
                         alarm->timestamp.tm_min,
                         alarm->timestamp.tm_sec,
                         alarm->target_weight,
                         (i < alarms_count -1) ? "," : "");
        if (offset >= sizeof(json_payload)) {
            ESP_LOGE(TAG, "JSON payload buffer overflow");
            break;
        }
    }

    offset += snprintf(json_payload + offset, sizeof(json_payload) - offset, "]}");

    xSemaphoreGive(alarms_mutex);

    // Publish to hydrapet0001/update/get/alarms
    mqtt_publish("hydrapet0001/update/get/alarms", json_payload);
    ESP_LOGI(TAG, "Published alarms: %s", json_payload);
}

/**
 * @brief Task responsible for monitoring and triggering alarms.
 *
 * This FreeRTOS task continuously checks the current time against the alarms in the `alarms_list`.
 * When an alarm's time is due, it triggers the alarm by initiating the water filling process
 * and removes the alarm from the list.
 *
 * @param pvParameters Argument passed to the task (unused).
 */
void alarms_task(void *pvParameters) {
    while (1) {
        // Get current time
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);

        // Check for due alarms
        if (xSemaphoreTake(alarms_mutex, portMAX_DELAY) == pdTRUE) {
            for (int i = 0; i < alarms_count; i++) {
                Alarm_t *alarm = &alarms_list[i];
                // Convert alarm time to time_t
                time_t alarm_time = mktime(&alarm->timestamp);
                if (alarm_time <= now) {
                    // Alarm is due, trigger it
                    ESP_LOGI(TAG, "Triggering alarm: %04d-%02d-%02dT%02d:%02d:%02d, target_weight=%ld",
                             alarm->timestamp.tm_year + 1900,
                             alarm->timestamp.tm_mon + 1,
                             alarm->timestamp.tm_mday,
                             alarm->timestamp.tm_hour,
                             alarm->timestamp.tm_min,
                             alarm->timestamp.tm_sec,
                             alarm->target_weight);

                    // Copy alarm data
                    Alarm_t triggered_alarm = *alarm;

                    // Remove the alarm from the list by shifting
                    for (int j = i; j < alarms_count -1; j++) {
                        alarms_list[j] = alarms_list[j+1];
                    }
                    alarms_count--;
                    i--; // Adjust index after shifting

                    xSemaphoreGive(alarms_mutex);

                    // Trigger water filling
                    fill_water_to(triggered_alarm.target_weight);
                }
            }
            xSemaphoreGive(alarms_mutex);
        }

        // Sleep for 1 second before next check
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
