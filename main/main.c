#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

#define ESP_WIFI_PASS      "awesomeMIM"
#define ESP_WIFI_SSID_FORMAT "CAR-%02X%02X%02X%02X%02X%02X"

static const char *TAG = "esp32-cam";

static void wifi_event_handler(
        void *arg,
        esp_event_base_t event_base,
        int32_t event_id,
        void *event_data
) {
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    wifi_config_t wifi_config = {
            .ap = {
                    .channel = ESP_WIFI_CHANNEL,
                    .password = ESP_WIFI_PASS,
                    .max_connection = MAX_STA_CONN,
                    .authmode = WIFI_AUTH_WPA2_PSK,
                    .pmf_cfg = {
                            .required = false,
                    },
            },
    };

    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_read_mac((uint8_t *) mac, ESP_MAC_WIFI_SOFTAP));
    uint8_t ssid[sizeof(wifi_config.ap.ssid)] = {0};
    snprintf((char *) ssid, sizeof(ssid), ESP_WIFI_SSID_FORMAT, MAC2STR(mac));
    memcpy(wifi_config.ap.ssid, ssid, sizeof(ssid));

    if (strlen(ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID: %s password: %s channel: %d",
             ssid,
             ESP_WIFI_PASS,
             ESP_WIFI_CHANNEL
    );
}

void init_nvs() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "nvs initialized");
}

void app_main() {
    init_nvs();
    wifi_init_softap();
}
