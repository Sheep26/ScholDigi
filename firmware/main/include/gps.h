#ifndef GPS_H
#define GPS_H

#define GPS_BUF_SIZE 1024

#include <inttypes.h>
#include "esp_system.h"

#ifdef __cplusplus
extern "C" {
#endif

// GPS task.
void gps_task(void *pvParameters);

// GPS data.
double getLat();
double getLng();

double getKMPH();

double getAlititude();

int getSatellites();

// Date and time.
uint16_t getYear();
uint8_t getMonth();
uint8_t getDay();

uint8_t getHour();
uint8_t getMinute();
uint8_t getSecond();

#ifdef __cplusplus
}
#endif

#endif