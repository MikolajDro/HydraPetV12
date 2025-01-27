// alarms.h

#ifndef ALARMS_H
#define ALARMS_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>

// Maksymalna liczba alarmów
#define MAX_ALARMS 1000

typedef struct {
    struct tm timestamp;      // Czas alarmu
    int32_t target_weight;    // Docelowa waga, domyślnie 200g
} Alarm_t;

/**
 * @brief Inicjalizacja modułu alarmów
 */
void alarms_init(void);

/**
 * @brief Dodanie alarmu
 *
 * @param alarm Alarm do dodania
 * @return true jeśli sukces, false jeśli bufor jest pełny lub błąd
 */
bool add_alarm(const Alarm_t *alarm);

/**
 * @brief Usunięcie alarmu
 *
 * @param timestamp Czas alarmu do usunięcia
 * @return true jeśli sukces, false jeśli nie znaleziono alarmu
 */
bool delete_alarm(const struct tm *timestamp);

/**
 * @brief Pobranie wszystkich alarmów i wysłanie ich przez MQTT
 */
void get_alarms(void);

/**
 * @brief Zadanie monitorujące alarmy
 *
 * @param pvParameters Argument zadania
 */
void alarms_task(void *pvParameters);

#endif // ALARMS_H
