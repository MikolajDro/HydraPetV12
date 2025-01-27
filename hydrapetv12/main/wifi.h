#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>

/**
 * @brief Inicjalizacja Wi-Fi
 *
 * @return Zwraca true, jeśli połączenie się powiodło, w przeciwnym wypadku false.
 */
bool wifi_init_sta(void);

/**
 * @brief Próba ponownego połączenia
 *
 * @return true, jeśli udało się połączyć, false w przeciwnym wypadku
 */
bool wifi_reconnect(void);

#endif // WIFI_H
