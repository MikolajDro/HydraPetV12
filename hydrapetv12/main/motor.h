// motor.h

#ifndef MAIN_MOTOR_H_
#define MAIN_MOTOR_H_

#include <stdbool.h>

/**
 * @brief Initializes the motor GPIO configuration.
 *
 * Configures the motor GPIO pin as an output and sets its initial state to OFF.
 */
void motor_init(void);

/**
 * @brief Turns the motor on.
 *
 * Sets the motor GPIO pin to high, updating the motor state to ON.
 */
void motor_on(void);

/**
 * @brief Turns the motor off.
 *
 * Sets the motor GPIO pin to low, updating the motor state to OFF.
 */
void motor_off(void);

/**
 * @brief Retrieves the current state of the motor.
 *
 * @return `true` if the motor is ON, `false` otherwise.
 */
bool get_motor_state();

#endif /* MAIN_MOTOR_H_ */
