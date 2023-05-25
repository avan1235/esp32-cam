#ifndef ESP32CAM_CAMERA_SCREEN_H
#define ESP32CAM_CAMERA_SCREEN_H

#include <stdlib.h>
#include <inttypes.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_spiffs.h>
#include <freertos/semphr.h>

#include "st7789.h"
#include "camera.h"

esp_err_t init_camera_screen();

void switch_display();

#endif
