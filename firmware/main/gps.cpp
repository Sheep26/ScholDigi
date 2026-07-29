// The point of this file is it acts as a wrapper between C++ and C allowing me to have firmware_main be in C but still use TinyGPSPlus which is in C++.

#include <gps.h>
#include <TinyGPSPlus.h>
#include "esp_system.h"
#include "driver/uart.h"

TinyGPSPlus gps;

// Task to read gps data.
void GPSTask(void *params) {
    while (1) {
        uint8_t *buffer = (uint8_t*) malloc(GPS_BUF_SIZE);
        int length = uart_read_bytes(UART_NUM_1, buffer, GPS_BUF_SIZE, 100 / portTICK_PERIOD_MS);

        if (length) {
            for (int i = 0; i < length; i++)
                gps.encode(buffer[i]);

            fwrite(buffer, 1, length, stdout);
            //printf("Lat: %f, Lng: %f, Satellites: %d\n", getLat(), getLng(), getSatellites());
        }

        free(buffer);
    }
}

double getLat() {
    return gps.location.lat();
}

double getLng() {
    return gps.location.lng();
}

double getSpeed() {
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

int getTimeValid() {
    return gps.time.isValid();
}

int getDateValid() {
    return gps.time.isValid();
}