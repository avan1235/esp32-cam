#ifndef SDCARD_H
#define SDCARD_H

#include <esp_err.h>

esp_err_t init_sdcard();

esp_err_t start_photo_collect();

#endif //SDCARD_H
