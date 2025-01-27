#include "buttons.h"
#include "led.h"
#include "wifi.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"

static const char *TAG = "BUTTONS";


#define USER_BUTTON_PIN  GPIO_NUM_4  // User button

static bool s_user_button_state = false;  // false - released, true - pressed

bool user_button_state(void)
{
    return !gpio_get_level(USER_BUTTON_PIN);
}

void buttons_init(void)
{    
    gpio_config_t io_conf_user_button = {
        .pin_bit_mask = (1ULL << USER_BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = true,
        .pull_down_en = false,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf_user_button);
    ESP_LOGI(TAG, "Button initialized.");
}

void buttons_task(void *arg)
{
    bool last_state = true;

    while (1) {
        bool current_state = user_button_state();
        if (current_state != last_state) {
            if (current_state == true) {
                ESP_LOGI(TAG, "User Button PRESSED!");

				led_blink_pair();

				while(!is_wifi_connected()){
	                // Ponowna próba połączenia z internetem
	                if (wifi_reconnect()) {
	                    ESP_LOGI(TAG, "Successfully connected to Wi-Fi");
	                } else {
	                    ESP_LOGI(TAG, "Connection not established, trying connect again");
	                }
	                vTaskDelay(pdMS_TO_TICKS(200));
	            }

            } else {
                ESP_LOGI(TAG, "Pair Button RELEASED!");
            }
            last_state = current_state;
            s_user_button_state = current_state;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
