#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

void setup_gpio();

void pcf_digital_write(uint8_t pin, uint8_t value);
int pcf_digital_read(uint8_t pin);

void digital_write(uint8_t pin, uint8_t value);
int digital_read(uint8_t pin);

#endif