#ifndef DISPLAY_H
#define DISPLAY_H

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ssd1306.h"

typedef struct {
    esp_lcd_panel_handle_t panel;
    esp_lcd_panel_io_handle_t io;
    uint8_t *framebuffer;
} display_t;

void init_display();
void fill_screen(uint8_t colour);
void flush_buffer();

#endif