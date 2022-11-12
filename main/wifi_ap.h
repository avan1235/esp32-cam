#ifndef ESP32CAM_WIFI_AP_H
#define ESP32CAM_WIFI_AP_H

#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_mac.h>
#include <string.h>

#define ESP_WIFI_CHANNEL CONFIG_ESP_WIFI_CHANNEL
#define ESP_MAX_STA_CONN CONFIG_ESP_MAX_STA_CONN

#define ESP_WIFI_PASS CONFIG_ESP_WIFI_PASS
#define ESP_WIFI_SSID_FORMAT "CAR-%02X%02X%02X%02X%02X%02X"

void wifi_init_ap();

#endif //ESP32CAM_WIFI_AP_H
