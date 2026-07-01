#include <gps.h>
#include <TinyGPSPlus.h>
#include "esp_system.h"
#include "driver/uart.h"

TinyGPSPlus gps;

// This function reads the serial data from the GPS and parses it.
// Returns the length of the data.
int readGPS(uint8_t *buffer) {
    return uart_read_bytes(UART_NUM_1, buffer, GPS_BUF_SIZE, 100 / portTICK_PERIOD_MS);
}

// Task to read gps data.
void gps_task(void *pvParameters) {
    uint8_t *buffer = (uint8_t *) malloc(GPS_BUF_SIZE);

    while (1) {
        int length = readGPS(buffer);

        if (length)
            for (int i = 0; i < length; i++)
                gps.encode(buffer[i]);
    }
}

double getLat() {
    return gps.location.lat();
}

double getLng() {
    return gps.location.lng();
}

double getKMPH() {
    return gps.speed.kmph();
}

double getAlititude() {
    return gps.altitude.meters();
}

int getSatellites() {
    return gps.satellites.value();
}

uint16_t getYear() {
    return gps.date.year();
}

uint8_t getMonth() {
    return gps.date.month();
}

uint8_t getDay() {
    return gps.date.day();
}

uint8_t getHour() {
    return gps.time.hour();
}

uint8_t getMinute() {
    return gps.time.minute();
}

uint8_t getSecond() {
    return gps.time.second();
}

int getLocationValid() {
    return gps.location.isValid();
}