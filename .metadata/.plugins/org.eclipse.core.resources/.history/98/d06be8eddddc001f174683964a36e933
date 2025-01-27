// hx711.h

#ifndef HX711_H
#define HX711_H

#include <stdint.h>
#include <time.h>

// Definicja struktury pomiaru
typedef struct {
    int32_t weight;          // Waga w jednostkach surowych
    struct tm timestamp;     // Znacznik czasu pomiaru
} Measurement;

// Definicja rozmiaru bufora kołowego
#define MEASUREMENT_BUFFER_SIZE 1000

// Deklaracje funkcji
void hx711_init(void);
int32_t get_water_weight(void);
void tare(void);
void add_measurement(int32_t weight);
int read_measurement(Measurement *measurement); // Nowa funkcja do odczytu
void get_all_measurements(void); // Opcjonalnie, funkcja do odczytu wszystkich pomiarów
void hx711_task(void *pvParameters);

#endif // HX711_H
