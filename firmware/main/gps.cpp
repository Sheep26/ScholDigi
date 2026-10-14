// The point of this file is it acts as a wrapper between C++ and C allowing me to have firmware_main be in C but still use TinyGPSPlus which is in C++.

#include <gps.h>
#include <TinyGPSPlus.h>
#include <gpio.h>
#include <pins.h>
#include "esp_system.h"
#include "driver/uart.h"

TinyGPSPlus gps;
double lat = 41;
double lng = 41;
double speed = 0;
double alt = 200;

// Task to read gps data.
void GPSTask(void *params) {
    while (1) {
        uint8_t buffer[GPS_BUF_SIZE];
        int length = uart_read_bytes(UART_NUM_1, buffer, GPS_BUF_SIZE, 100 / portTICK_PERIOD_MS);

        if (length) {
            for (int i = 0; i < length; i++)
                gps.encode((char) buffer[i]);

            if (!digital_read(DEBUG_PIN))
                fwrite(buffer, 1, length, stdout);
        }
    }
}

double getLat() {
    //return gps.location.lat();
    lat += 0.000001;
    return lat;
}

double getLng() {
    //return gps.location.lng();
    lng += 0.000001;
    return lng;
}

double getSpeed() {
    //return gps.speed.kmph();
    speed += 0.2;
    return speed;
}

double getAlititude() {
    //return gps.altitude.meters();
    alt += 0;
    return alt;
}

int getSatellites() {
    return gps.satellites.value() || 7;
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
    // return gps.location.isValid();
    return gps.location.isValid() || true;
}

int getTimeValid() {
    // return gps.time.isValid();
    return gps.time.isValid() || true;
}

int getDateValid() {
    // return gps.time.isValid();
    return gps.time.isValid() || true;
}