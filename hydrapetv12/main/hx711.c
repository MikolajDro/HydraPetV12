// hx711.c

#include "hx711.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <time.h>
#include <string.h>
#include "esp_log.h"
#include "mqtt.h"
#include "motor.h"
#include "led.h"

/** @brief GPIO number for the HX711 DATA pin (DOUT) */
#define HX711_DATA_PIN GPIO_NUM_21  // Pin DOUT tensometru HX711

/** @brief GPIO number for the HX711 SCK pin */
#define HX711_SCK_PIN  GPIO_NUM_19  // Pin SCK tensometru HX711

/**
 * @brief Structure holding parameters for filling water task
 */
typedef struct {
    int32_t target_weight; /**< @brief Target weight to reach */
} fill_water_params_t;

/** @brief Circular buffer to store measurements */
static Measurement measurement_buffer[MEASUREMENT_BUFFER_SIZE];

/** @brief Index for writing to the buffer */
static size_t buffer_head = 0; // Save index 

/** @brief Index for reading from the buffer */
static size_t buffer_tail = 0; // Read index

/** @brief Current number of measurements in the buffer */
static size_t buffer_count = 0; 

/** @brief Mutex to protect the measurement buffer */
static SemaphoreHandle_t buffer_mutex = NULL;

/** @brief Mutex to protect HX711 operations */
static SemaphoreHandle_t hx711_mutex = NULL;

/** @brief Tag used for ESP logging */
static const char *TAG = "HX711";

/** @brief Offset value for tare calibration */
static int32_t tare_offset = 0;

/**
 * @brief Initializes the HX711 sensor and related peripherals.
 *
 * Configures the GPIO pins for DATA and SCK, initializes mutexes,
 * and performs tare calibration.
 */
void hx711_init(void)
{
    gpio_config_t io_conf;

    // Configuration DATA (DOUT) 
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << HX711_DATA_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    // Configuration SCK
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << HX711_SCK_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    gpio_set_level(HX711_SCK_PIN, 0);

    // Mutexs initialization
    if (buffer_mutex == NULL)
    {
        buffer_mutex = xSemaphoreCreateMutex();
        if (buffer_mutex == NULL)
        {
            ESP_LOGE(TAG, "buffer_mutex not created");
        }
    }

    if (hx711_mutex == NULL)
    {
        hx711_mutex = xSemaphoreCreateMutex();
        if (hx711_mutex == NULL)
        {
            ESP_LOGE(TAG, "hx711_mutex not created");
        }
    }

    tare();
}

/**
 * @brief Reads raw data from the HX711 sensor.
 *
 * This function waits for the DATA pin to go low, indicating that data is ready.
 * It then reads 24 bits of data, applies the necessary calibration,
 * and returns the raw weight value.
 *
 * @return Raw weight value as a signed 32-bit integer.
 */
static int32_t read_raw(void)
{
    uint32_t count = 0;
    uint8_t i;

    uint32_t timeout = 1000;  // Timeout in ms
    TickType_t start_tick = xTaskGetTickCount();

    while (gpio_get_level(HX711_DATA_PIN))
    {
        vTaskDelay(pdMS_TO_TICKS(1));  // Delay 1 ms
        if ((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS >= timeout)
        {
            // Timeout 
            ESP_LOGE(TAG, "Timeout HX711");
            return 0; 
        }
    }

    // Pobranie mutexu
    if (xSemaphoreTake(hx711_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        for (i = 0; i < 24; i++)
        {
            gpio_set_level(HX711_SCK_PIN, 1);
            esp_rom_delay_us(1); 

            count = count << 1;
            gpio_set_level(HX711_SCK_PIN, 0);
            esp_rom_delay_us(1);

            if (gpio_get_level(HX711_DATA_PIN))
            {
                count++;
            }
        }
        // Gain 128x 
        gpio_set_level(HX711_SCK_PIN, 1);
        esp_rom_delay_us(1);
        gpio_set_level(HX711_SCK_PIN, 0);
        esp_rom_delay_us(1);

        xSemaphoreGive(hx711_mutex);
    }
    else
    {
        ESP_LOGE(TAG, "Can not take hx711_mutex");
        return 0;  
    }

    if (count & 0x800000)
    {
        count |= 0xFF000000;
    }
    else
    {
        count &= 0x00FFFFFF;
    }

    return (int32_t)count;
}

/**
 * @brief Retrieves the current water weight.
 *
 * This function reads the raw weight from the HX711 sensor,
 * applies tare calibration, converts it to grams, and adds the measurement to the buffer.
 *
 * @return Water weight in grams as a signed 32-bit integer.
 */
int32_t get_water_weight(void)
{
    int32_t raw_value = read_raw();
    
    float tmp=0;
    
    int32_t weight = raw_value - tare_offset;
    
    tmp = (float )weight / 1000;
    
    weight = (uint32_t) tmp;


    add_measurement(weight);

    return weight;
}

/**
 * @brief Performs tare calibration to set the current weight as zero.
 *
 * This function reads the raw weight and sets it as the tare offset.
 */
void tare(void)
{
    tare_offset = read_raw();
}

/**
 * @brief Adds a weight measurement to the circular buffer.
 *
 * This function creates a Measurement struct with the current weight and timestamp,
 * then adds it to the buffer in a thread-safe manner.
 *
 * @param weight The weight value to add to the buffer.
 */
void add_measurement(int32_t weight)
{
    time_t now = 0;
    struct tm timeinfo = {0};
    time(&now);
    localtime_r(&now, &timeinfo);

    Measurement measurement;
    measurement.weight = weight;
    measurement.timestamp = timeinfo;

    if (xSemaphoreTake(buffer_mutex, portMAX_DELAY) == pdTRUE)
    {
        measurement_buffer[buffer_head] = measurement;
        buffer_head = (buffer_head + 1) % MEASUREMENT_BUFFER_SIZE;

        if (buffer_count < MEASUREMENT_BUFFER_SIZE)
        {
            buffer_count++;
        }
        else
        {
            buffer_tail = (buffer_tail + 1) % MEASUREMENT_BUFFER_SIZE;
        }

        xSemaphoreGive(buffer_mutex);
    }
}

/**
 * @brief Reads the oldest measurement from the buffer.
 *
 * This function retrieves the oldest measurement from the circular buffer
 * in a thread-safe manner.
 *
 * @param measurement Pointer to a Measurement struct where the data will be stored.
 * @return `1` if a measurement was successfully read, `0` otherwise.
 */
int read_measurement(Measurement *measurement)
{
    if (xSemaphoreTake(buffer_mutex, portMAX_DELAY) == pdTRUE)
    {
        if (buffer_count == 0)
        {
            xSemaphoreGive(buffer_mutex);
            return 0; 
        }

        *measurement = measurement_buffer[buffer_tail];
        buffer_tail = (buffer_tail + 1) % MEASUREMENT_BUFFER_SIZE;
        buffer_count--;

        xSemaphoreGive(buffer_mutex);
        return 1; // Success
    }

    return 0; // Fail
}

/**
 * @brief Retrieves and prints all measurements from the buffer.
 *
 * This function continuously reads measurements from the buffer
 * and prints their weight and timestamp until the buffer is empty.
 */
void get_all_measurements(void)
{
    while (1)
    {
        Measurement m;
        if (read_measurement(&m))
        {
            printf("Waga: %ld, Czas: %04d-%02d-%02dT%02d:%02d:%02d\n",
                   m.weight,
                   m.timestamp.tm_year + 1900, m.timestamp.tm_mon + 1, m.timestamp.tm_mday,
                   m.timestamp.tm_hour, m.timestamp.tm_min, m.timestamp.tm_sec);
        }
        else
        {
            break;
        }
    }
}

/**
 * @brief FreeRTOS task for filling water to a specified weight.
 *
 * This task continuously monitors the water weight and controls the motor
 * to fill water until the target weight is reached. It also handles error
 * conditions such as insufficient weight gain over a period.
 *
 * @param pvParameters Pointer to fill_water_params_t containing the target weight.
 */
static void fill_water_task(void *pvParameters)
{
    fill_water_params_t *params = (fill_water_params_t *)pvParameters;
    int32_t target_weight = params->target_weight;
    free(params); 

    ESP_LOGI(TAG, "Start pouring water to weight: %ld g", target_weight);

    int32_t initial_weight = get_water_weight();
    if (initial_weight >= target_weight)
    {
        ESP_LOGI(TAG, "Measured weight (%ld g) is equal or higher to (%ld g).", initial_weight, target_weight);
        vTaskDelete(NULL); 
        return;
    }

    motor_on();

    TickType_t start_tick = xTaskGetTickCount();
    int32_t last_weight = initial_weight;
    bool error = false;

    while (1)
    {
        int32_t current_weight = get_water_weight();

        if (current_weight >= target_weight)
        {
            ESP_LOGI(TAG, "Target weight achieved: %ld g", current_weight);
            motor_off();
            break;
        }

        TickType_t current_tick = xTaskGetTickCount();
        TickType_t elapsed_ticks = current_tick - start_tick;
        uint32_t elapsed_ms = elapsed_ticks * portTICK_PERIOD_MS;

        if (elapsed_ms >= 3000)
        {
            if ((current_weight - last_weight) < 10)
            {
                ESP_LOGE(TAG, "No significant weight gain within 3 seconds. Motor stops and LED flashes.");
                motor_off();

                led_blink_pair();

                error = true;
                break;
            }
            else
            {
                last_weight = current_weight;
                start_tick = current_tick;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100)); 
    }

    if (!error)
    {
        ESP_LOGI(TAG, "Pouring process success!");
    }

    vTaskDelete(NULL); 
}

/**
 * @brief Initiates the process of filling water to a specified weight.
 *
 * This function allocates memory for the task parameters and creates
 * the FreeRTOS task responsible for monitoring and controlling the filling process.
 *
 * @param target_weight The desired weight to achieve in grams.
 */
void fill_water_to(int32_t target_weight)
{
    fill_water_params_t *params = malloc(sizeof(fill_water_params_t));
    if (params == NULL)
    {
        ESP_LOGE(TAG, "Nie udało się alokować pamięci na parametry zadania dolewania wody.");
        return;
    }

    params->target_weight = target_weight;

    xTaskCreate(fill_water_task, "fill_water_task", 4096, (void *)params, 5, NULL);
}
