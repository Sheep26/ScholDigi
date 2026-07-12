idf_component_register(SRCS "firmware_main.c" "gps.cpp" "exercise.c"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS "include"
                       REQUIRES esp_driver_uart)
