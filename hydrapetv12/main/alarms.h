// alarms.h

#ifndef ALARMS_H
#define ALARMS_H

#include <stdint.h>   /**< @brief For int32_t */
#include <time.h>     /**< @brief For struct tm */
#include <stdbool.h>  /**< @brief For bool */

/** @brief Maximum number of alarms that can be stored */
#define MAX_ALARMS 1000

/**
 * @brief Structure representing an alarm.
 *
 * Each alarm consists of a timestamp indicating when it should trigger
 * and a target weight that specifies the desired weight to fill.
 */
typedef struct {
    struct tm timestamp;      /**< @brief Time when the alarm should trigger */
    int32_t target_weight;    /**< @brief Target weight to achieve, default is 200g */
} Alarm_t;

/**
 * @brief Initializes the alarms module.
 *
 * Sets up necessary resources such as mutexes and starts the monitoring task.
 */
void alarms_init(void);

/**
 * @brief Adds an alarm to the alarms list.
 *
 * @param alarm Pointer to the `Alarm_t` structure to be added.
 * @return `true` if the alarm was successfully added, `false` otherwise.
 */
bool add_alarm(const Alarm_t *alarm);

/**
 * @brief Deletes an alarm from the alarms list based on its timestamp.
 *
 * @param timestamp Pointer to the `struct tm` representing the time of the alarm to delete.
 * @return `true` if the alarm was successfully deleted, `false` otherwise.
 */
bool delete_alarm(const struct tm *timestamp);

/**
 * @brief Retrieves all alarms and publishes them via MQTT.
 *
 * Constructs a JSON payload containing all alarms and publishes it to the MQTT topic.
 */
void get_alarms(void);

/**
 * @brief Task responsible for monitoring and triggering alarms.
 *
 * This FreeRTOS task continuously checks the current time against the alarms in the `alarms_list`.
 * When an alarm's time is due, it triggers the alarm by initiating the water filling process
 * and removes the alarm from the list.
 *
 * @param pvParameters Argument passed to the task (unused).
 */
void alarms_task(void *pvParameters);

#endif // ALARMS_H
