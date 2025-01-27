// water_level_sensor.c

#include "water_level_sensor.h"
#include "led.h"
#include "wifi.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hx711.h"
#include "mqtt.h"
#include "esp_log.h"
#include <errno.h>

static const char *TAG = "WATER LVL SENSOR";

#define TANK_LEVEL_SENSOR_PIN GPIO_NUM_10  /**< @brief GPIO number for the tank level sensor */

/**
 * @brief Retrieves the current state of the water level sensor.
 *
 * This function reads the GPIO level of the tank level sensor pin.
 *
 * @return `true` if the water level is below 30%, `false` otherwise.
 */
bool water_level_sensor_state(void)
{
    return gpio_get_level(TANK_LEVEL_SENSOR_PIN);
}

/**
 * @brief Initializes the water level sensor GPIO configuration.
 *
 * Configures the tank level sensor GPIO pin as an input with a pull-up resistor.
 * This setup ensures reliable reading of the water level status.
 */
void water_level_sensor_init(void)
{
    gpio_config_t io_conf_tank_sensor = {
        .pin_bit_mask = (1ULL << TANK_LEVEL_SENSOR_PIN), /**< @brief Bitmask for the tank level sensor pin */
        .mode = GPIO_MODE_INPUT,                            /**< @brief Set GPIO mode to input */
        .pull_up_en = true,                                 /**< @brief Enable pull-up resistor */
        .pull_down_en = false,                              /**< @brief Disable pull-down resistor */
        .intr_type = GPIO_INTR_DISABLE,                     /**< @brief Disable GPIO interrupts */
    };
    gpio_config(&io_conf_tank_sensor);
    ESP_LOGI(TAG, "Water level sensor initialized.");
}

/**
 * @brief Publishes the current water tank level status via MQTT.
 *
 * This function checks the state of the water level sensor and publishes an appropriate
 * message to the MQTT topic `hydrapet0001/hydrapetinfo/watertanklevel`.
 *
 * @note The payload is a simple string indicating whether the water level is below 30% or full.
 */
void mqtt_publish_water_tank_level(void) {
    if (water_level_sensor_state() == 1) {
        mqtt_publish("hydrapet0001/hydrapetinfo/watertanklevel", "Below 30%");
    }
    else {
        mqtt_publish("hydrapet0001/hydrapetinfo/watertanklevel", "Water tank is full");
    }
}

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
void water_level_sensor_task(void *pvParameters)
{
    bool prev_state = true; // Initial state with pull-up resistor enabled (1)
    while (true)
    {
        bool current_state = water_level_sensor_state();
        if (current_state && !prev_state)
        {
            // Water level has dropped below 30% (falling edge detected)
            ESP_LOGI(TAG, "Water level is below 30 percent");
            
            if(!is_wifi_connected())
            {
                ESP_LOGW(TAG, "Wi-Fi is not connected! (trying to connect again)");
                while(!wifi_reconnect()){
                     vTaskDelay(pdMS_TO_TICKS(200));
                }
            }
            led_blink_once();

            mqtt_publish_water_tank_level();
        }
        
        prev_state = current_state;

        vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100 ms before next check
    }
}
