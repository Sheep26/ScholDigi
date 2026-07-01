#ifndef GPS_H
#define GPS_H

#define GPS_BUF_SIZE 1024

#include <inttypes.h>
#include "esp_system.h"

#ifdef __cplusplus
extern "C" {
#endif

int readGPS(uint8_t *buffer);
void gps_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif