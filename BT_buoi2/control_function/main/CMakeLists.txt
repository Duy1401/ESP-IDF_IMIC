idf_component_register(SRCS "control_function.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_uart esp_driver_ledc esp_driver_gpio freertos)
