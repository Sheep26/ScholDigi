#include <gpio.h>
#include <config.h>
#include <pins.h>
#include "driver/i2c_master.h"

#ifdef PCF
i2c_master_bus_handle_t i2c_bus;
i2c_master_dev_handle_t i2c_pcf;

uint8_t pcf_gpio_state = 0b00000000;
#endif

void setup_gpio() {
#ifdef PCF
    ESP_ERROR_CHECK(i2c_master_get_bus_handle(0, &i2c_bus));

    // Setup PCF expansion board.
    i2c_device_config_t pcf_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PCF_I2C_ADDR,
        .scl_speed_hz = 100000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &pcf_config, &i2c_pcf));

    uint8_t init_state = 0xFF;
    esp_err_t err = i2c_master_transmit(i2c_pcf, &init_state, 1, -1);
    printf("transmit: %s\n", esp_err_to_name(err));
#endif
}

void pcf_digital_write(uint8_t pin, uint8_t value) {
    // Perform bitshift.
    // Either set the bit at position (pin) or clear it.

    if (value)
        pcf_gpio_state |= (1 << pin);
    else
        pcf_gpio_state &= ~(1 << pin);

    ESP_ERROR_CHECK(i2c_master_transmit(i2c_pcf, &pcf_gpio_state, 1, -1));
}

int pcf_digital_read(uint8_t pin) {
    uint8_t state;
    ESP_ERROR_CHECK(i2c_master_receive(i2c_pcf, &state, 1, -1));

    // Perform bitshift.
    // Get if state at position (pin) is 1.
    return (state >> pin) & 1;
}

void digital_write(uint8_t pin, uint8_t value) {
#ifdef PCF
    pcf_digital_write(pin, value);
#else
    // TODO
#endif
}

int digital_read(uint8_t pin) {
#ifdef PCF
    return pcf_digital_read(pin);
#else
    // TODO
    return 0;
#endif
}