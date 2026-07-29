#ifndef PINS_H
#define PINS_H

#include <config.h>

#define TX_GPIO_NUM 4
#define RX_GPIO_NUM 5

#define I2C_SDA 2
#define I2C_SCL 3

#define PCF_I2C_ADDR 0x20

#define HIGH 0x01
#define LOW 0x00

#ifdef PCF
#define WAKE_PIN 0
#define START_PIN 1
#else
#define WAKE_PIN 0
#define START_PIN 1
#endif

#endif