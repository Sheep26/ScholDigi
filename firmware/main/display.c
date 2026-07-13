#include <display.h>
#include <string.h>
#include <pins.h>

display_t *display;

void init_display() {
    display = (display_t*) calloc(1, sizeof(display_t));
    assert(display);

    display->framebuffer = (uint8_t*) calloc(1, DISPLAY_WIDTH * DISPLAY_HEIGHT/8);
    assert(display->framebuffer);

    ESP_ERROR_CHECK(i2c_master_get_bus_handle(0, &display->bus));

    esp_lcd_panel_io_i2c_config_t display_config = {
        .dev_addr = SSD1306_I2C_ADDR,
        .scl_speed_hz = 400000,

        .control_phase_bytes = 1,
        .dc_bit_offset = 6,

        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(display->bus, &display_config, &display->io));

    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = -1,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(display->io, &panel_config, &display->panel));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(display->panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(display->panel));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(display->panel, true));

    fill_screen(0xFF); // Set all pixels to on.
    flush_buffer();
}

void fill_screen(uint8_t on) {
    memset(display->framebuffer, on, DISPLAY_WIDTH * DISPLAY_HEIGHT/8);
}

void flush_buffer() {
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(display->panel, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, display->framebuffer));
}