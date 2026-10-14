#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "ble_uart.h"

void bluetooth_init();
void ble_send_data(void *data);

#endif