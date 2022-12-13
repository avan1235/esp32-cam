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

#define HTTP_PART_BOUNDARY "123456789000000000000987654321"
#define MAX_NUMBER_DIGITS 11

esp_err_t start_webserver();

#endif //ESP32CAM_WEB_SERVER_H
