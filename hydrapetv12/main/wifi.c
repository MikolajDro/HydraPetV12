// wifi.c

#include "wifi.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "led.h"
#include "config.h"
#include <string.h>

static const char *TAG = "WIFI";

static bool s_connected = false;
static bool s_connect_attempt = false;

#define EXAMPLE_ESP_MAXIMUM_RETRY  10                /**< @brief Maximum number of connection retries */

static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0                      /**< @brief Event bit for successful connection */
#define WIFI_FAIL_BIT      BIT1                      /**< @brief Event bit for failed connection */

/** @brief Variable to track the Wi-Fi connection status */
static bool wifi_connected = false;

/**
 * @brief Checks if the device is currently connected to Wi-Fi.
 *
 * @return `true` if connected, `false` otherwise.
 */
bool is_wifi_connected(void)
{
    return wifi_connected;
}

/**
 * @brief Sets the Wi-Fi connection status.
 *
 * @param is_conn `true` to mark as connected, `false` otherwise.
 */
void set_is_wifi_connected(bool is_conn)
{
    wifi_connected = is_conn;
}

/**
 * @brief Handles Wi-Fi and IP events.
 *
 * This function is called by the ESP-IDF event loop upon various Wi-Fi and IP events.
 * It manages connection attempts, retries, and updates the connection status accordingly.
 *
 * @param arg Argument passed to the handler.
 * @param event_base The base of the event (e.g., WIFI_EVENT, IP_EVENT).
 * @param event_id The ID of the specific event.
 * @param event_data Data associated with the event.
 */
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        s_connect_attempt = true;
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Attempting to reconnect to AP (%d)", s_retry_num);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"Connection fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        s_connected = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * @brief Initializes the Wi-Fi in station mode and attempts to connect to the specified network.
 *
 * This function initializes NVS flash, sets up the Wi-Fi event handlers, configures the Wi-Fi
 * settings (SSID and password), and starts the Wi-Fi connection process. It waits for a
 * successful connection or a failure based on the maximum number of retries.
 *
 * @return `true` if the connection was successful, `false` otherwise.
 */
bool wifi_init_sta(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    // Wait for connection
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            pdMS_TO_TICKS(10000)); // Timeout 10 seconds

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Wi-Fi connected!");
        return true;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to Wi-Fi");
        return false;
    } else {
        ESP_LOGE(TAG, "Timeout waiting for a Wi-Fi connection");
        return false;
    }
}

/**
 * @brief Attempts to reconnect to the Wi-Fi network.
 *
 * This function resets the retry counter and initiates a new connection attempt.
 * It waits for a successful connection or a failure based on the maximum number of retries.
 *
 * @return `true` if the reconnection was successful, `false` otherwise.
 */
bool wifi_reconnect(void)
{
    ESP_LOGI(TAG, "Trying to connect to Wi-Fi again");
    s_retry_num = 0;
    esp_wifi_connect();

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            pdMS_TO_TICKS(10000)); // Timeout 10 seconds

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to Wi-Fi (retrying)");
        return true;
    } else {
        ESP_LOGI(TAG, "Failed to connect to Wi-Fi (retrying)");
        return false;
    }
}

/**
 * @brief Initializes the Wi-Fi connection process.
 *
 * This function attempts to initialize and connect to the Wi-Fi network. If the initial
 * connection fails, it enters a loop to retry the connection, blinking the LED as feedback.
 * After a certain number of failed attempts, it halts the system.
 */
void wifi_init(void){        
    set_is_wifi_connected( wifi_init_sta() );
    
    int i=0;
    // Check if Wi-Fi connection was successful
    while(!is_wifi_connected())
    {
        ESP_LOGW(TAG, "Connection not established, trying again...");
        set_is_wifi_connected( wifi_init_sta() );
        
        vTaskDelay(pdMS_TO_TICKS(1000));
        led_blink_once();
        if(i > 10)
        {
            i++;
            while(1){
                ; // Halt the system
            }
        }
    }
}
