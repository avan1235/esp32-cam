#ifndef MAIN_ST7789_H_
#define MAIN_ST7789_H_

#include <string.h>
#include <inttypes.h>
#include <math.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>

#include <esp_log.h>

#define BLACK            0x0000

#define DIRECTION0        0
#define DIRECTION90        1
#define DIRECTION180        2
#define DIRECTION270        3

typedef struct {
    uint16_t _width;
    uint16_t _height;
    uint16_t _offsetx;
    uint16_t _offsety;
    int16_t _dc;
    int16_t _bl;
    spi_device_handle_t _handle;
} TFT_t;

void
spi_master_init(
        TFT_t *dev,
        int16_t GPIO_MOSI,
        int16_t GPIO_SCLK,
        int16_t GPIO_CS,
        int16_t GPIO_DC,
        int16_t GPIO_RESET,
        int16_t GPIO_BL
);

void lcd_init(TFT_t *dev, int width, int height, int offsetx, int offsety);

void lcd_draw_multi_pixels(TFT_t *dev, uint16_t x, uint16_t y, uint16_t size, uint16_t *colors);

void lcd_draw_fill_rect(TFT_t *dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

void lcd_display_off(TFT_t *dev);

void lcd_display_on(TFT_t *dev);

void lcd_fill_screen(TFT_t *dev, uint16_t color);

#endif

