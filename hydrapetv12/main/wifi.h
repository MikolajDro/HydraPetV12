#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>

/**
 * @brief Initializes the Wi-Fi in station mode and attempts to connect to the specified network.
 *
 * This function initializes NVS flash, sets up the Wi-Fi event handlers, configures the Wi-Fi
 * settings (SSID and password), and starts the Wi-Fi connection process. It waits for a
 * successful connection or a failure based on the maximum number of retries.
 *
 * @return `true` if the connection was successful, `false` otherwise.
 */
bool wifi_init_sta(void);

/**
 * @brief Attempts to reconnect to the Wi-Fi network.
 *
 * This function resets the retry counter and initiates a new connection attempt.
 * It waits for a successful connection or a failure based on the maximum number of retries.
 *
 * @return `true` if the reconnection was successful, `false` otherwise.
 */
bool wifi_reconnect(void);

/**
 * @brief Checks if the device is currently connected to Wi-Fi.
 *
 * @return `true` if connected, `false` otherwise.
 */
bool is_wifi_connected(void);

/**
 * @brief Sets the Wi-Fi connection status.
 *
 * @param is_conn `true` to mark as connected, `false` otherwise.
 */
void set_is_wifi_connected(bool is_conn);

/**
 * @brief Initializes the Wi-Fi connection process.
 *
 * This function attempts to initialize and connect to the Wi-Fi network. If the initial
 * connection fails, it enters a loop to retry the connection, blinking the LED as feedback.
 * After a certain number of failed attempts, it halts the system.
 */
void wifi_init(void);

#endif // WIFI_H
