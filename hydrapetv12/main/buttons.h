#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>

/**
 * @brief Inicjalizacja przycisków
 */
void buttons_init(void);

/**
 * @brief Zadanie obsługujące przycisk pair
 *
 * @param arg argument
 */
void buttons_task(void *arg);

/**
 * @brief Pobranie stanu przycisku pair
 */
bool pair_button_state(void);

#endif // BUTTONS_H
