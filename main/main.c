#include "camera.h"
#include "flash_led.h"
#include "wifi_ap.h"
#include "web_server.h"
#include "memory.h"

void app_main() {
    ESP_ERROR_CHECK(init_nvs());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(init_camera());
    ESP_ERROR_CHECK(configure_flash_led());
    wifi_init_ap();
    ESP_ERROR_CHECK(start_webserver());
}
