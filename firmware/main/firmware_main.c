#include <stdio.h>
#include <inttypes.h>
#include <pins.h>
#include <gps.h>
#include <sys/time.h>
#include <time.h>
#include <exercise.h>
#include <ssd1306.h>
#include <haversine.h>
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
int sleeping = 0;
struct tm lastButtonPress;

exercise_t *current_exercise;
i2c_master_bus_handle_t i2c_bus_handle;
i2c_master_dev_handle_t i2c_pcf_handle;

ssd1306_handle_t ssd1306_handle;

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

    lastButtonPress = tm;
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

    ssd1306_config_t ssd1306_config = I2C_SSD1306_128x64_CONFIG_DEFAULT;
    ssd1306_init(i2c_bus_handle, &ssd1306_config, &ssd1306_handle);

    if (ssd1306_handle == NULL) {
        printf("SSD1306 handle init failed");
        assert(ssd1306_handle);
    }

    ssd1306_clear_display(ssd1306_handle, false);
    ssd1306_set_contrast(ssd1306_handle, 0xFF);
    ssd1306_display_text(ssd1306_handle, 0, "Hello World!", false);

    // Start gps task.
    xTaskCreatePinnedToCore(GPSTask, "GPS", 4096, NULL, 5, NULL, 1);

    // Set timezone.
    setenv("TZ", "NZST-12NZDT,M9.5.0/02:00:00,M4.1.0/03:00:00", 1);
    tzset();

    struct timeval now = { .tv_sec = 1782734400 /* Jun 30 2026 in EPOCH */, .tv_usec=0 };
    settimeofday(&now, NULL);

    lastButtonPress = getTime();

    // Loop
    while (1) {
        if (!timeSetup)
            setupTimeGPS();

        struct tm current_time = getTime();

        if (mktime(&current_time) - mktime(&lastButtonPress) > 300 && !sleeping) {
            sleeping = 1;

            ssd1306_disable_display(ssd1306_handle); // Put display to sleep after 5 minutes of no button presses.
        }

        if ((!pcf_digital_read(WAKE_PIN) || !pcf_digital_read(START_PIN)) && sleeping) {
            sleeping = 0;
            lastButtonPress = getTime();

            ssd1306_enable_display(ssd1306_handle);
        }

        if (!pcf_digital_read(START_PIN)) {
            if (!current_exercise)
                current_exercise = (exercise_t*) calloc(1, sizeof(exercise_t));
            else {
                // Handle putting exercise into storage eventually.
                exercise_point_t *p = current_exercise->list;
                exercise_point_t *l = (void*) 0;

                int div = 0;

                while (p) {
                    // Add exercise distance.
                    if (l) {
                        current_exercise->distance += distanceBetweenPoints(l, p);

                        l = (void*) 0;
                    } else
                        l = p;

                    current_exercise->max_alt = p->alt > current_exercise->max_alt ? p->alt : current_exercise->max_alt;
                    current_exercise->min_alt = (p->alt < current_exercise->min_alt || !current_exercise->min_alt) ? p->alt : current_exercise->min_alt;
                    current_exercise->top_speed = p->speed > current_exercise->top_speed ? p->speed : current_exercise->top_speed;

                    current_exercise->alt_diff = current_exercise->max_alt - current_exercise->min_alt;
                    current_exercise->avg_alt += p->alt;
                    current_exercise->avg_speed += p->speed;

                    div++;                    
                    p = p->next;
                }

                current_exercise->avg_alt = current_exercise->avg_alt / div;
                current_exercise->avg_speed = current_exercise->avg_speed / div;

                printf("Distance: %f", current_exercise->distance);

                // Free exercise from memory.
                exercise_point_t *pointah = current_exercise->list;

                while (pointah) {
                    exercise_point_t *next = p->next;

                    free(pointah);
                    pointah = next;
                }

                free(current_exercise);
                current_exercise = (void*) 0; // Set current exercise to null instead of having it reference unallocated memory.
            }
        }

        printf("Satellites: %d\n", getSatellites());

        // Print time.
        char strftime_buf[64];

        strftime(strftime_buf, sizeof(strftime_buf), "%c", &current_time);
        printf("%s\n", strftime_buf);

        if (current_exercise && getLocationValid() && getSatellites()) {
            // Check if things have changed or if there just hasn't been a point yet.
            if (!current_exercise->list || !current_exercise->last || getLat() != current_exercise->last->lat || getLng() != current_exercise->last->lng || getAlititude() != current_exercise->last->alt) {
                // Create pointer for exercise point struct.
                exercise_point_t *point = (exercise_point_t*) malloc(sizeof(exercise_point_t));
                assert(point);

                // Set variables.
                point->lat = getLat();
                point->lng = getLng();
                point->alt = getAlititude();
                point->speed = getSpeed();
                point->time = mktime(&current_time);

                // Add point.
                addExercisePoint(current_exercise, point);
            }
        }
    }

    // Reset ESP32 incase while loop ever exits.
    ssd1306_delete(ssd1306_handle);
    fflush(stdout);
    esp_restart();
}
