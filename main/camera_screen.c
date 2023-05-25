#include "camera_screen.h"

#define INTERVAL 1
#define WAIT vTaskDelay(INTERVAL)

static const char *TAG = "esp32-cam-camera-screen";

static TFT_t DISPLAY_DEV;
static SemaphoreHandle_t DISPLAY_DEV_MUTEX;

static esp_err_t draw_camera_picture(uint8_t *out, uint16_t *colors) {
    camera_fb_t *pic = esp_camera_fb_get();
    if (!pic) {
        esp_camera_fb_return(pic);
        return ESP_FAIL;
    }
    if (pic->width != CONFIG_SCREEN_WIDTH) {
        esp_camera_fb_return(pic);
        return ESP_OK;
    }
    if (pic->height != CONFIG_SCREEN_HEIGHT) {
        esp_camera_fb_return(pic);
        return ESP_OK;
    }
    jpg2rgb565(pic->buf, pic->len, out, JPG_SCALE_NONE);
    esp_camera_fb_return(pic);

    xSemaphoreTake(DISPLAY_DEV_MUTEX, portMAX_DELAY);
    for (size_t y = 0; y < CONFIG_SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < CONFIG_SCREEN_WIDTH; x++) {
            colors[x] = ((uint16_t *) out)[y * CONFIG_SCREEN_WIDTH + x];
        }
        lcd_draw_multi_pixels(&DISPLAY_DEV, 0, y, CONFIG_SCREEN_WIDTH, colors);
    }
    xSemaphoreGive(DISPLAY_DEV_MUTEX);

    return ESP_OK;
}

static void st7789_camera_task(void *params) {
    xSemaphoreTake(DISPLAY_DEV_MUTEX, portMAX_DELAY);
    spi_master_init(&DISPLAY_DEV,
                    CONFIG_SCREEN_MOSI_GPIO,
                    CONFIG_SCREEN_SCLK_GPIO,
                    CONFIG_SCREEN_CS_GPIO,
                    CONFIG_SCREEN_DC_GPIO,
                    CONFIG_SCREEN_RESET_GPIO,
                    CONFIG_SCREEN_BL_GPIO);
    lcd_init(&DISPLAY_DEV, CONFIG_SCREEN_WIDTH, CONFIG_SCREEN_HEIGHT, CONFIG_SCREEN_OFFSETX, CONFIG_SCREEN_OFFSETY);
    lcd_fill_screen(&DISPLAY_DEV, BLACK);
    xSemaphoreGive(DISPLAY_DEV_MUTEX);

    uint8_t *out = malloc(sizeof(uint8_t) * CONFIG_SCREEN_HEIGHT * CONFIG_SCREEN_WIDTH);
    uint16_t *colors = malloc(sizeof(uint16_t) * CONFIG_SCREEN_WIDTH);
    while (true) {
        esp_err_t err = draw_camera_picture(out, colors);
        if (err != ESP_OK) {
            free(colors);
            free(out);
            return;
        }
        WAIT;
    }
}

esp_err_t init_camera_screen() {
    DISPLAY_DEV_MUTEX = xSemaphoreCreateMutex();
    BaseType_t result = xTaskCreate(st7789_camera_task, "st7789_camera_task", 1024 * 6, NULL, 2, NULL);
    if (pdPASS != result) return ESP_FAIL;
    return ESP_OK;
}

void switch_display() {
    static bool CAMERA_DEV_STATE = true;
    bool dev_state = !CAMERA_DEV_STATE;
    ESP_LOGI(TAG, "switching dev state to %d", dev_state);
    CAMERA_DEV_STATE = dev_state;
    xSemaphoreTake(DISPLAY_DEV_MUTEX, portMAX_DELAY);
    if (CAMERA_DEV_STATE) {
        lcd_display_on(&DISPLAY_DEV);
    } else {
        lcd_display_off(&DISPLAY_DEV);
    }
    xSemaphoreGive(DISPLAY_DEV_MUTEX);
}

