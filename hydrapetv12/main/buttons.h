#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>

/**
 * @brief Initializes the buttons.
 *
 * Configures the GPIO pins associated with the buttons.
 */
void buttons_init(void);

/**
 * @brief Task that handles the pair button.
 *
 * Monitors the pair button state and performs actions based on button events.
 *
 * @param arg Argument passed to the task.
 */
void buttons_task(void *arg);

/**
 * @brief Retrieves the state of the pair button.
 *
 * @return `true` if the pair button is pressed, `false` otherwise.
 */
bool user_button_state(void);

#endif // BUTTONS_H
