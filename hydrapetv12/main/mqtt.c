// mqtt.c

#include "mqtt.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>
#include "hx711.h"
#include "motor.h"
#include "led.h"
#include "buttons.h"
#include <sys/time.h>
#include <errno.h>
#include "alarms.h"


static const char *TAG = "MQTT";

static esp_mqtt_client_handle_t mqtt_client = NULL;

// Callback globalny do obsługi wiadomości
static mqtt_callback_t global_mqtt_callback = NULL;

/**
 * @brief Handler eventów MQTT
 *
 * @param handler_args Argument handlera
 * @param base Podstawa eventu
 * @param event_id ID eventu
 * @param event_data Dane eventu
 */
static void mqtt_event_handler_cb(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            // Subskrybujemy tematy po połączeniu
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/set/time", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/get/time", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/set/water", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/get/water", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/get/status", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/set/alarm", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/get/alarms", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/del/alarm", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/put/pourwater", 0);
            esp_mqtt_client_subscribe(mqtt_client, "hydrapet0001/update/set/tare", 0);
            ESP_LOGI(TAG, "Subskrybowanie tematów MQTT zakończone");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_DATA:
            // Obsługa odebranych danych
            if (global_mqtt_callback != NULL) {
                // Kopiowanie tematu i wiadomości do bufora
                char topic[128];
                char message[256];
                memset(topic, 0, sizeof(topic));
                memset(message, 0, sizeof(message));

                // Kopiujemy temat
                strncpy(topic, event->topic, event->topic_len);
                topic[event->topic_len] = '\0';

                // Kopiujemy wiadomość
                strncpy(message, event->data, event->data_len);
                message[event->data_len] = '\0';

                // Wywołujemy callbacka
                global_mqtt_callback(topic, message);
            }
            break;
        // Obsługa innych eventów w razie potrzeby
        default:
            break;
    }
}

void mqtt_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = "mqtt://test.mosquitto.org:1883", // Zmień na własny broker
            },
        },
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return;
    }

    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler_cb, NULL);
    esp_mqtt_client_start(mqtt_client);
    ESP_LOGI(TAG, "MQTT initialized.");
}

void mqtt_publish(const char *topic, const char *payload)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return;
    }

    esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);
    ESP_LOGI(TAG, "MQTT publish: %s -> %s", topic, payload);
}

void mqtt_publish_all(int32_t weight, struct tm timestamp, bool button_state, bool led_state, bool pin15_state)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return;
    }

    char topic[50];
    char payload[300]; // Zwiększamy rozmiar, aby pomieścić wszystkie dane

    // Topic: hydrapet/get/all
    snprintf(topic, sizeof(topic), "hydrapet/get/all");

    // Tworzymy payload z danymi w formacie JSON
    snprintf(payload, sizeof(payload),
             "{\"weight\": %ld, \"timestamp\": \"%04d-%02d-%02dT%02d:%02d:%02d\", \"button_state\": \"%s\", \"led_state\": \"%s\", \"motor state\": \"%s\"}",
             weight,
             timestamp.tm_year + 1900, timestamp.tm_mon + 1, timestamp.tm_mday,
             timestamp.tm_hour, timestamp.tm_min, timestamp.tm_sec,
             button_state ? "PRESSED" : "RELEASED",
             led_state ? "ON" : "OFF",
             pin15_state ? "ON" : "OFF");

    esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);
    ESP_LOGI(TAG, "MQTT publish: %s -> %s", topic, payload);
}

void mqtt_publish_water_state(int water_state)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return;
    }

    char topic[50];
    char payload[50];

    // Topic: hydrapet0001/hydrapetinfo/water
    snprintf(topic, sizeof(topic), "hydrapet0001/hydrapetinfo/water");

    // Tworzymy payload z danymi w formacie JSON
    snprintf(payload, sizeof(payload), "{\"water_state\": %d}", water_state);

    esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);
    ESP_LOGI(TAG, "MQTT publish: %s -> %s", topic, payload);
}

void mqtt_publish_current_time(struct tm timestamp)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return;
    }

    char topic[50];
    char payload[100];

    // Topic: hydrapet0001/hydrapetinfo/time
    snprintf(topic, sizeof(topic), "hydrapet0001/hydrapetinfo/time");

    // Tworzymy payload z danymi w formacie JSON
    snprintf(payload, sizeof(payload), "{\"current_time\": \"%04d-%02d-%02dT%02d:%02d:%02d\"}",
             timestamp.tm_year + 1900, timestamp.tm_mon + 1, timestamp.tm_mday,
             timestamp.tm_hour, timestamp.tm_min, timestamp.tm_sec);

    esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);
    ESP_LOGI(TAG, "MQTT publish: %s -> %s", topic, payload);
}

void mqtt_set_message_callback(void (*callback)(const char *, const char *))
{
    global_mqtt_callback = callback;
    ESP_LOGI(TAG, "MQTT message callback set.");
}


bool get_pin15_state(void) {
    // Używamy funkcji z motor.h
    return get_motor_state();
}

static esp_err_t set_system_time(const char *time_str) {
    struct tm tm_time;
    memset(&tm_time, 0, sizeof(struct tm));

    // Parsowanie ciągu znaków do struktury tm
    // Zakładamy format "YYYY-MM-DDTHH:MM:SS"
    if (sscanf(time_str, "%d-%d-%dT%d:%d:%d",
               &tm_time.tm_year,
               &tm_time.tm_mon,
               &tm_time.tm_mday,
               &tm_time.tm_hour,
               &tm_time.tm_min,
               &tm_time.tm_sec) != 6) {
        ESP_LOGE(TAG, "Niepoprawny format czasu: %s", time_str);
        return ESP_ERR_INVALID_ARG;
    }

    // Korekta wartości
    tm_time.tm_year -= 1900; // tm_year: lata od 1900
    tm_time.tm_mon -= 1;     // tm_mon: miesiące od stycznia [0-11]

    // Konwersja struktury tm do czasu w sekundach od epoki
    time_t epoch_time = mktime(&tm_time);
    if (epoch_time == -1) {
        ESP_LOGE(TAG, "Błąd konwersji czasu");
        return ESP_ERR_INVALID_ARG;
    }

    // Ustawienie czasu systemowego
    struct timeval tv;
    tv.tv_sec = epoch_time;
    tv.tv_usec = 0;
    if (settimeofday(&tv, NULL) != 0) {
        ESP_LOGE(TAG, "Nie udało się ustawić czasu: %s", strerror(errno));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Ustawiono czas na: %s", time_str);
    return ESP_OK;
}

/**
 * @brief Funkcja obsługująca nalewanie wody na podstawie otrzymanej wiadomości MQTT
 *
 * @param message Otrzymana wiadomość MQTT
 */
static void handle_pourwater(const char *message) {
    ESP_LOGI(TAG, "Obsługa pourwater z wiadomością: %s", message);

    int target_weight = 0;

    // Sprawdzenie, czy wiadomość zaczyna się od '{', co sugeruje format JSON
    if (message[0] == '{') {
        ESP_LOGI(TAG, "Parsowanie wiadomości jako JSON.");

        // Poszukiwanie klucza "target_weight"
        const char *key = "\"target_weight\"";
        char *key_ptr = strstr(message, key);
        if (key_ptr != NULL) {
            // Poszukiwanie dwukropka po kluczu
            char *colon_ptr = strstr(key_ptr, ":");
            if (colon_ptr != NULL) {
                // Parsowanie wartości po dwukropku
                // Zakładamy, że wartość jest liczbą całkowitą
                if (sscanf(colon_ptr, ":%d", &target_weight) == 1) {
                    ESP_LOGI(TAG, "Parsed target_weight from JSON: %d g", target_weight);
                } else {
                    ESP_LOGE(TAG, "Nie udało się sparsować wartości target_weight z JSON.");
                    return;
                }
            } else {
                ESP_LOGE(TAG, "Nie znaleziono dwukropka po kluczu target_weight.");
                return;
            }
        } else {
            ESP_LOGE(TAG, "Nie znaleziono klucza \"target_weight\" w wiadomości JSON.");
            return;
        }
    } else {
        ESP_LOGI(TAG, "Parsowanie wiadomości jako int.");
        // Parsowanie wiadomości jako prostą liczbę całkowitą
        target_weight = atoi(message);
        if (target_weight <= 0) {
            ESP_LOGE(TAG, "Nieprawidłowa wartość target_weight: %s", message);
            return;
        }
        ESP_LOGI(TAG, "Parsed target_weight as int: %d g", target_weight);
    }

    // Sprawdzenie, czy target_weight jest poprawny
    if (target_weight <= 0) {
        ESP_LOGE(TAG, "Nieprawidłowa wartość target_weight po parsowaniu: %d", target_weight);
        return;
    }

    // Wywołanie funkcji dolewania wody
    ESP_LOGI(TAG, "Rozpoczynanie dolewania wody do wagi: %d g", target_weight);
    fill_water_to(target_weight);
}

static void handle_tare(const char *message) {
    ESP_LOGI(TAG, "Obsługa tare z wiadomością: %s", message);
    
    led_blink_once();
    
    tare();

    ESP_LOGI(TAG, "Funkcja tare() została wywołana.");
}

/**
 * @brief Callback do obsługi odebranych wiadomości MQTT
 *
 * @param topic Otrzymany temat
 * @param message Otrzymana wiadomość
 */
void mqtt_message_handler(const char *topic, const char *message)
{
    ESP_LOGI(TAG, "Odebrano wiadomość na temat: %s -> %s", topic, message);

    // Obsługa różnych tematów
    if (strcmp(topic, "hydrapet0001/update/set/time") == 0) {
        ESP_LOGI(TAG, "Ustawianie czasu na podstawie wiadomości: %s", message);

        esp_err_t err = set_system_time(message);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Czas został pomyślnie ustawiony.");
        } else {
            ESP_LOGE(TAG, "Nie udało się ustawić czasu.");
        }
    }
    else if (strcmp(topic, "hydrapet0001/update/get/time") == 0) {
        // Przykład: wysyłanie aktualnego czasu
        ESP_LOGI(TAG, "Wysyłanie aktualnego czasu na żądanie");
        // Pobierz aktualny czas i wyślij go
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        mqtt_publish_current_time(timeinfo);
    }
    else if (strcmp(topic, "hydrapet0001/update/set/water") == 0) {
        // Przykład: ustawianie stanu wody
        ESP_LOGI(TAG, "Ustawianie stanu wody na podstawie wiadomości: %s", message);
        // Możesz tutaj zaktualizować stan wody, np. poprzez sterowanie zaworem
        // Konwersja wiadomości na int
        int target_weight = (int )atoi(message);
        
		if (target_weight > 0) {
            fill_water_to(target_weight);
        } else {
            ESP_LOGE(TAG, "Nieprawidłowa wartość docelowej wagi: %s", message);
        }
    }
    else if (strcmp(topic, "hydrapet0001/update/get/water") == 0) {
        // Przykład: wysyłanie aktualnego stanu wody
        ESP_LOGI(TAG, "Wysyłanie aktualnego stanu wody na żądanie");
        // Pobierz aktualny stan wody i wyślij
        // Zakładam, że stan wody to waga, ale możesz to dostosować
        int water_state = get_water_weight(); // Dostosuj do rzeczywistej funkcji
        mqtt_publish_water_state(water_state);
    }
    else if (strcmp(topic, "hydrapet0001/update/get/status") == 0) {
        // Przykład: wysyłanie statusu urządzenia
        ESP_LOGI(TAG, "Wysyłanie statusu urządzenia na żądanie");
        // Zbierz wszystkie informacje o stanie urządzenia i wyślij
        int32_t weight = get_water_weight();
        time_t now;
        struct tm current_time;
        time(&now);
        localtime_r(&now, &current_time);
        bool button_state = pair_button_state();
        bool led_state = led_state; // Zakładam LED na GPIO 2
        bool pin15_state = led_state;

        mqtt_publish_all(weight, current_time, button_state, led_state, pin15_state);
    }
    else if (strcmp(topic, "hydrapet0001/update/set/alarm") == 0) {
        // Ustawianie alarmu
        ESP_LOGI(TAG, "Ustawianie alarmu na podstawie wiadomości: %s", message);
        
        // Parsowanie JSON
        char timestamp_str[25];
        int target_weight = 200; // Domyślna wartość

        // Zakładamy, że wiadomość jest w formacie JSON:
        // {"timestamp": "YYYY-MM-DDTHH:MM:SS", "target_weight": 200}
        sscanf(message, "{\"timestamp\": \"%24[^\"]\", \"target_weight\": %d}", timestamp_str, &target_weight);

        // Parsowanie timestamp_str do struct tm
        struct tm alarm_time;
        memset(&alarm_time, 0, sizeof(struct tm));
        if (sscanf(timestamp_str, "%d-%d-%dT%d:%d:%d",
                   &alarm_time.tm_year,
                   &alarm_time.tm_mon,
                   &alarm_time.tm_mday,
                   &alarm_time.tm_hour,
                   &alarm_time.tm_min,
                   &alarm_time.tm_sec) != 6) {
            ESP_LOGE(TAG, "Niepoprawny format czasu alarmu: %s", timestamp_str);
            return;
        }

        // Korekta wartości
        alarm_time.tm_year -= 1900; // tm_year: lata od 1900
        alarm_time.tm_mon -= 1;     // tm_mon: miesiące od stycznia [0-11]

        // Dodanie alarmu
        Alarm_t new_alarm;
        new_alarm.timestamp = alarm_time;
        new_alarm.target_weight = target_weight;

        if (add_alarm(&new_alarm)) {
            ESP_LOGI(TAG, "Alarm został dodany.");
        } else {
            ESP_LOGE(TAG, "Nie udało się dodać alarmu.");
        }
    }
    else if (strcmp(topic, "hydrapet0001/update/get/alarms") == 0) {
        // Pobieranie listy alarmów
        ESP_LOGI(TAG, "Pobieranie listy alarmów na żądanie.");
        get_alarms();
    }
    else if (strcmp(topic, "hydrapet0001/update/del/alarm") == 0) {
        // Usuwanie alarmu
        ESP_LOGI(TAG, "Usuwanie alarmu na podstawie wiadomości: %s", message);
        
        // Parsowanie JSON
        char timestamp_str[25];
        sscanf(message, "{\"timestamp\": \"%24[^\"]\"}", timestamp_str);

        // Parsowanie timestamp_str do struct tm
        struct tm del_alarm_time;
        memset(&del_alarm_time, 0, sizeof(struct tm));
        if (sscanf(timestamp_str, "%d-%d-%dT%d:%d:%d",
                   &del_alarm_time.tm_year,
                   &del_alarm_time.tm_mon,
                   &del_alarm_time.tm_mday,
                   &del_alarm_time.tm_hour,
                   &del_alarm_time.tm_min,
                   &del_alarm_time.tm_sec) != 6) {
            ESP_LOGE(TAG, "Niepoprawny format czasu alarmu do usunięcia: %s", timestamp_str);
            return;
    	}
        // Korekta wartości
        del_alarm_time.tm_year -= 1900; // tm_year: lata od 1900
        del_alarm_time.tm_mon -= 1;     // tm_mon: miesiące od stycznia [0-11]

        // Usunięcie alarmu
        if (delete_alarm(&del_alarm_time)) {
            ESP_LOGI(TAG, "Alarm został usunięty.");
        } else {
            ESP_LOGE(TAG, "Nie znaleziono alarmu do usunięcia.");
        }
    }
	else if (strcmp(topic, "hydrapet0001/update/put/pourwater") == 0) {
        // Obsługa nalewania wody na podstawie otrzymanej wagi
        handle_pourwater(message);
    }
    else if (strcmp(topic, "hydrapet0001/update/set/tare") == 0) {
        // Obsługa tare
        handle_tare(message);
    }
}