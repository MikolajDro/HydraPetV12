#ifndef LED_H
#define LED_H

#include <stdbool.h>

/**
 * @brief Inicjalizacja diody
 */
void led_init(void);

/**
 * @brief Ustawienie stanu diody
 *
 * @param on true = zapal, false = zgaś
 */
void led_set_state(bool on);


bool led_get_state(void);

/**
 * @brief Miganie diodą co 500 ms, do momentu aż funkcja się zatrzyma
 */
void led_blink_task(void *pvParameters);

void led_blink_once(void);

// Funkcja migania diodą co 500 ms przez np. 5 sekund
void led_blink_pair(void);

#endif // LED_H
