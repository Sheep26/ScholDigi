#include <stdio.h>
#include <inttypes.h>
#include <pins.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/uart.h"
#include <gps.h>
#include <sys/time.h>
#include <time.h>

int timeSetup = 0;

struct tm getTime() {
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    return timeinfo;
}

void setupTimeDefault() {
    struct tm tm;
    tm.tm_year = 2026 - 1990; // Current year minus EPOCH start date.
    tm.tm_mon = 6;
    tm.tm_mday = 30;

    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;

    time_t t = mktime(&tm);

    struct timeval now = { .tv_sec = t, .tv_usec=0 };
    settimeofday(&now, NULL);
}

void setupTimeGPS() {
    // Set date and time.
    if (!getDateValid() || !getTimeValid())
        return;

    struct tm tm;
    tm.tm_year = getYear() - 1990; // Current year minus EPOCH start date.
    tm.tm_mon = getMonth();
    tm.tm_mday = getDay();

    tm.tm_hour = getHour();
    tm.tm_min = getMinute();
    tm.tm_sec = getSecond();

    time_t t = mktime(&tm);

    struct timeval now = { .tv_sec = t, .tv_usec=0 };
    settimeofday(&now, NULL);

    timeSetup = 1;
}

void app_main(void) {
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);

    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ", CONFIG_IDF_TARGET, chip_info.cores, (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "", (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "", (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "", (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);

    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");

        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024), (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    // Initalize GPS serial connection.

    // Set params.
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    // Send params to driver.
    uart_param_config(UART_NUM_1, &uart_config);

    // Set serial pins.
    uart_set_pin(UART_NUM_1, TX_GPIO_NUM, RX_GPIO_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Enable serial on UART num 1.
    uart_driver_install(UART_NUM_1, GPS_BUF_SIZE, 0, 0, NULL, 0);

    // Start gps task.
    xTaskCreatePinnedToCore(gps_task, "GPS", 4096, NULL, 5, NULL, 1);

    // Set timezone.
    setenv("TZ", "NZST-12NZDT,M9.5.0/02:00:00,M4.1.0/03:00:00", 1);
    tzset();

    setupTimeDefault();

    // Loop
    while (1) {
        if (!timeSetup)
            setupTimeGPS();

        // Print time.
        struct tm timeinfo = getTime();
        char strftime_buf[64];

        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        printf("%s", strftime_buf);
    }

    // Reset ESP32 incase while loop ever exits.
    fflush(stdout);
    esp_restart();
}
