idf_component_register(
    SRCS "main.c" 
         "buttons.c" 
         "mqtt.c" 
         "led.c" 
         "hx711.c"
         "wifi.c"
         "motor.c"
         "alarms.c"
         "water_level_sensor.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_event nvs_flash driver freertos esp_wifi mqtt
)
