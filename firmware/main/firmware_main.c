#include <stdio.h>
#include <inttypes.h>
#include <pins.h>
#include <gps.h>
#include <sys/time.h>
#include <time.h>
#include <exercise.h>
#include <display.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/uart.h"
#include "driver/i2c_master.h"

int timeSetup = 0;
int locationValid = 0;
struct tm lastButtonPress;

exercise_t *current_exercise;
static i2c_master_bus_handle_t i2c_bus_handle;
i2c_master_dev_handle_t i2c_pcf_handle;

uint8_t pcf_gpio_state = 0b00000000;

void pcf_digital_write(uint8_t pin, uint8_t value) {
    // Perform bitshift.
    // Either set the bit at position (pin) or clear it.

    if (value)
        pcf_gpio_state |= (1 << pin);
    else
        pcf_gpio_state &= ~(1 << pin);

    ESP_ERROR_CHECK(i2c_master_transmit(i2c_pcf_handle, &pcf_gpio_state, 1, -1));
}

int pcf_digital_read(uint8_t pin) {
    uint8_t state;
    ESP_ERROR_CHECK(i2c_master_receive(i2c_pcf_handle, &state, 1, -1));

    // Perform bitshift.
    // Get if state at position (pin) is 1.
    return (state >> pin) & 1;
}

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
    // Setup
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
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    // Enable serial on UART num 1.
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, TX_GPIO_NUM, RX_GPIO_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, GPS_BUF_SIZE, 0, 0, NULL, 0));

    // Setup I2C for Display and GPIO.
    i2c_master_bus_config_t i2c_bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
    };

    // Set I2C master bus.
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle));

    // Setup PCF expansion board.
    i2c_device_config_t pcf_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PCF_I2C_ADDR,
        .scl_speed_hz = 100000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &pcf_config, &i2c_pcf_handle));

    init_display();

    // Start gps task.
    // xTaskCreatePinnedToCore(gps_task, "GPS", 4096, NULL, 5, NULL, 1);

    // Set timezone.
    setenv("TZ", "NZST-12NZDT,M9.5.0/02:00:00,M4.1.0/03:00:00", 1);
    tzset();

    setupTimeDefault();

    lastButtonPress = getTime();

    // Loop
    while (1) {
        struct tm current_time = getTime();
        if (mktime(&current_time) - mktime(&lastButtonPress) > 30) {
            // Oled driver to handle display darkening.
        }

        doGPS();

        if (!timeSetup)
            setupTimeGPS();

        // This should return garbage unless lib C converts it to a number on it's own. I'm not used to lib C existing so umm yeah.
        // It'll return garbage because we are casting an int to a char and when that happens the int turns into it's utf-8 or ascii letter instead of a number.
        // If there's garbage the solution is to add an offset to an extent that only works upto 10.
        // I could make an ITOA function at some point in time, but idk if lib C does that on it's own.
        printf("Satellites: %s", (char*) getSatellites());

        // Print time.
        struct tm timeinfo = getTime();
        char strftime_buf[64];

        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        printf("%s", strftime_buf);

        if (current_exercise && getLocationValid()) {
            // Check if things have changed or if there just hasn't been a point yet.
            if (!current_exercise->list || !current_exercise->last || getLat() != current_exercise->last->lat || getLng() != current_exercise->last->lng || getAlititude() != current_exercise->last->alt) {
                // Create pointer for exercise point struct.
                exercise_point_t *point = (exercise_point_t*) malloc(sizeof(exercise_point_t));

                // Set variables.
                point->lat = getLat();
                point->lng = getLng();
                point->alt = getAlititude();
                point->speed = getSpeed();
                point->time = getTime();

                // Add point.
                addExercisePoint(current_exercise, point);
            }
        }
    }

    // Reset ESP32 incase while loop ever exits.
    fflush(stdout);
    esp_restart();
}
