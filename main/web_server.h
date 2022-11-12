#ifndef ESP32CAM_WEB_SERVER_H
#define ESP32CAM_WEB_SERVER_H

#include <esp_log.h>
#include <esp_netif.h>
#include <esp_http_server.h>

#include "util.h"
#include "camera.h"
#include "flash_led.h"

#define WS_CMD_TYPE HTTPD_WS_TYPE_TEXT
#define WS_CMD_SHOOT "s"
#define WS_CMD_INCREASE_RESOLUTION "i"
#define WS_CMD_DECREASE_RESOLUTION "d"
#define WS_CMD_CHANGE_FLASH_LED "l"

esp_err_t start_webserver();

#endif //ESP32CAM_WEB_SERVER_H
