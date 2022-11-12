#include "flash_led.h"

static const char *TAG = "esp32-cam-flash-led";

static bool LED_STATE = 0;

esp_err_t configure_flash_led() {
    esp_err_t ret = gpio_reset_pin(FLASH_LED_GPIO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "flash led pin cannot be reset");
        return ret;
    }
    ret = gpio_set_direction(FLASH_LED_GPIO, GPIO_MODE_OUTPUT);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "flash led direction cannot be set");
    }
    return ret;
}

esp_err_t switch_flash_led() {
    bool led_state = !LED_STATE;
    ESP_LOGI(TAG, "switching led state to %d", led_state);
    LED_STATE = led_state;
    return gpio_set_level(FLASH_LED_GPIO, led_state);
}