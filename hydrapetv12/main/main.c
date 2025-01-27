// main.c

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_wifi.h"

#include "mqtt.h"         
#include "hx711.h"        
#include "led.h"          
#include "buttons.h"      
#include "driver/uart.h"  
#include "wifi.h"
#include "motor.h"
#include "alarms.h"
#include "water_level_sensor.h"
#include "config.h"

static const char *TAG = "MAIN";

/**
 * @brief Task responsible for publishing sensor data periodically.
 *
 * This FreeRTOS task runs indefinitely, performing the following actions in a loop:
 * 1. Checks if the device is connected to Wi-Fi.
 * 2. If connected, reads the current weight from the HX711 sensor.
 * 3. Retrieves the current system time.
 * 4. Publishes the collected data to various MQTT topics.
 * 5. Blinks an LED for visual feedback.
 * 6. Waits for the defined duration before repeating the process.
 *
 * @param pvParameters Pointer to task parameters (unused).
 */
static void publish_task(void *pvParameters)
{
    while (true)
    {
        // Check if connected to Wi-Fi
        while (!is_wifi_connected())
        {
            ESP_LOGW(TAG, "Wi-Fi connection not established yet");
            vTaskDelay(pdMS_TO_TICKS(2000)); 
        }

        // Connected to Wi-Fi, proceed to read data
        int32_t weight = get_water_weight();  // Read weight from HX711
        time_t now = 0;
        time(&now);
        struct tm timeinfo = {0};
        localtime_r(&now, &timeinfo);

        // Publish to "hydrapet0001/hydrapetinfo/all"
        mqtt_publish_all(weight, 
                        timeinfo, 
                        user_button_state(), 
                        led_get_state(), 
                        get_motor_state());

        // Publish water state to "hydrapet0001/hydrapetinfo/water"
        mqtt_publish_water_state(weight);

        // Publish current time to "hydrapet0001/hydrapetinfo/time"
        mqtt_publish_current_time(timeinfo);
        
        // Publish water tank level status
        mqtt_publish_water_tank_level();

        // Blink LED after publishing
        led_blink_once();

        // Wait for the defined duration before the next publish cycle
        vTaskDelay(pdMS_TO_TICKS(PUBLISH_ALL_DURATION_TIME));
    }
}

/**
 * @brief Application entry point.
 *
 * This function initializes all necessary modules and peripherals, sets up MQTT callbacks,
 * and creates FreeRTOS tasks for publishing data, monitoring water level sensors, and handling buttons.
 */
void app_main(void)
{
    // Initialize NVS (Non-Volatile Storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize LED module
    led_init();
    
    // Set Wi-Fi logging level to ERROR to reduce verbosity
    esp_log_level_set("wifi", ESP_LOG_ERROR);

    // Initialize Wi-Fi connection
    wifi_init();    

    // Initialize MQTT client
    mqtt_init();
    
    // Initialize motor control
    motor_init();
    
    // Initialize alarms module
    alarms_init();

    // Initialize buttons module
    buttons_init();

    // Initialize HX711 weight sensor
    hx711_init();

    // Create tasks

    /**
     * @brief Task for publishing sensor data periodically.
     *
     * Publishes weight, time, button state, LED state, and motor state every minute.
     */
    xTaskCreate(publish_task, "publish_task", 4096, NULL, 5, NULL);

    /**
     * @brief Task for handling MQTT incoming messages.
     *
     * Sets up the callback function to handle messages received on subscribed MQTT topics.
     */
    mqtt_set_message_callback(mqtt_message_handler);

    /**
     * @brief Task for monitoring the water level sensor.
     *
     * Continuously monitors the water level and publishes status updates when the water level drops below 30%.
     */
    xTaskCreate(water_level_sensor_task, "water_level_sensor_task", 2048, NULL, 5, NULL);
    
    /**
     * @brief Task for handling user button interactions.
     *
     * Monitors the state of user buttons and performs actions based on button presses.
     */
    xTaskCreate(buttons_task, "user_button_task", 2048, NULL, 5, NULL);
    
    // Infinite loop to keep the main task alive
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10000000000)); // Delay for an extended period
    }
}
