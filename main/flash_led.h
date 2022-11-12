#ifndef ESP32CAM_FLASH_LED_H
#define ESP32CAM_FLASH_LED_H

#include <driver/gpio.h>
#include <esp_log.h>

#include "util.h"

#define FLASH_LED_GPIO GPIO_NUM_4

esp_err_t configure_flash_led();

esp_err_t switch_flash_led();

#endif //ESP32CAM_FLASH_LED_H
