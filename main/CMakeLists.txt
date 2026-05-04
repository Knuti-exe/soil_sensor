idf_component_register(SRCS "soil_sensor.c"
                    INCLUDE_DIRS "."
                    REQUIRES adc_oneshot ble_beacon esp_driver_gpio)
