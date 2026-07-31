idf_component_register(SRCS "firmware_main.c" "gps.cpp" "exercise.c" "haversine.c" "gpio.c"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS "include"
                       REQUIRES esp_driver_uart esp_driver_i2c esp_lcd)
