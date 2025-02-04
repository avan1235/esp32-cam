#include <esp_netif.h>
#include <esp_event.h>

#include "sdcard.h"
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

void app_main() {
    ESP_ERROR_CHECK(init_nvs());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(init_camera());
    ESP_ERROR_CHECK(init_flash_led());
    ESP_ERROR_CHECK(init_wifi());
    ESP_ERROR_CHECK(init_sdcard());
    ESP_ERROR_CHECK(start_webserver());
    ESP_ERROR_CHECK(start_photo_collect());
}
