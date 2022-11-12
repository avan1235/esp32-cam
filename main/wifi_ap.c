#include "wifi_ap.h"

static const char *TAG = "esp32-cam-wifi-ap";

static void wifi_event_handler(
        void *arg,
        esp_event_base_t event_base,
        int32_t event_id,
        void *event_data
) {
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        ESP_LOGI(TAG, "station "MACSTR" joined, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        ESP_LOGI(TAG, "station "MACSTR" left, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_ap() {
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
                    .max_connection = ESP_MAX_STA_CONN,
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

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_ap finished. SSID: %s password: %s channel: %d",
             ssid,
             ESP_WIFI_PASS,
             ESP_WIFI_CHANNEL
    );
}