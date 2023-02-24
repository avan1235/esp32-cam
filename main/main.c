#include <esp_netif.h>
#include <esp_event.h>
#include "camera.h"
#include "flash_led.h"
#include "web_server.h"
#include "memory.h"

#if CONFIG_ESP_WIFI_MODE_ACCESS_POINT

#include "wifi_ap.h"

#define init_wifi wifi_init_ap
#elif CONFIG_ESP_WIFI_MODE_STATION
#include "wifi_sta.h"
#define init_wifi wifi_init_sta
#endif

//void app_main() {
//    ESP_ERROR_CHECK(init_nvs());
//    ESP_ERROR_CHECK(esp_netif_init());
//    ESP_ERROR_CHECK(esp_event_loop_create_default());
//    ESP_ERROR_CHECK(init_camera());
//    ESP_ERROR_CHECK(init_flash_led());
//    ESP_ERROR_CHECK(init_wifi());
//    ESP_ERROR_CHECK(start_webserver());
//}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"

#include "st7789.h"

#define INTERVAL 1
#define WAIT vTaskDelay(INTERVAL)

esp_err_t camera_raw(TFT_t *dev, uint8_t *out) {

    camera_fb_t *pic = esp_camera_fb_get();
    if (!pic) {
        esp_camera_fb_return(pic);
        return ESP_FAIL;
    }
    jpg2rgb565(pic->buf, pic->len, out, JPG_SCALE_NONE);
    esp_camera_fb_return(pic);

    int imageWidth = 240;
    int imageHeight = 240;
    uint16_t *colors = (uint16_t *) malloc(sizeof(uint16_t) * imageWidth);

    for (int y = 0; y < imageHeight; y++) {
        for (int x = 0; x < imageWidth; x++) {
            colors[x] = ((uint16_t *) out)[y * imageWidth + x];
        }
        lcdDrawMultiPixels(dev, 0, y, imageWidth, colors);
    }

    free(colors);

    return ESP_OK;
}

void ST7789(void *pvParameters) {

    TFT_t dev;
    spi_master_init(&dev,
                    CONFIG_MOSI_GPIO,
                    CONFIG_SCLK_GPIO,
                    CONFIG_CS_GPIO,
                    CONFIG_DC_GPIO,
                    CONFIG_RESET_GPIO,
                    CONFIG_BL_GPIO);
    lcdInit(&dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);

    lcdSetFontDirection(&dev, 0);
    lcdFillScreen(&dev, BLACK);

    uint8_t *out = malloc(sizeof(uint8_t) * 240 * 240 * 2);
    while (true) {
        esp_err_t err = camera_raw(&dev, out);
        if (err != ESP_OK) {
            free(out);
            return;
        }
        WAIT;
    }
}


void app_main(void) {

    ESP_ERROR_CHECK(init_nvs());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(init_camera());
    ESP_ERROR_CHECK(init_flash_led());
    ESP_ERROR_CHECK(init_wifi());
    ESP_ERROR_CHECK(start_webserver());

    xTaskCreate(ST7789, "ST7789", 1024 * 6, NULL, 2, NULL);
}
