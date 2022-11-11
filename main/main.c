#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "sys/param.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

#include "esp_camera.h"


#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET (-1) // software reset
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

#define ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

#define ESP_WIFI_PASS      "awesomeMIM"
#define ESP_WIFI_SSID_FORMAT "CAR-%02X%02X%02X%02X%02X%02X"

static const char *TAG = "esp32-cam";

#define free_ptr(ptr) do {         \
    void **temp = (void **) (ptr); \
    free(*temp);                   \
    *temp = NULL;                  \
    } while(false)

static camera_config_t camera_config = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = FRAMESIZE_HD,    // QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

        .jpeg_quality = 12, //0-63, for OV series camera sensors, lower number means higher quality
        .fb_count = 1,       //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

static esp_err_t init_camera() {
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }
    return ESP_OK;
}


//////////////////////////////// CAMERA


//////////////////////////////// WEBSOCKET

static esp_err_t receive_frame_len(
        httpd_req_t *req,
        httpd_ws_frame_t *ws_pkt
) {
    esp_err_t ret = httpd_ws_recv_frame(req, ws_pkt, /* max_len = */ 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "frame len is %d", ws_pkt->len);
    return ESP_OK;
}

static esp_err_t receive_frame_payload(
        httpd_req_t *req,
        httpd_ws_frame_t *ws_pkt,
        uint8_t **buf
) {
    if (ws_pkt->len == 0) {
        ESP_LOGI(TAG, "received frame with empty payload");
        return ESP_OK;
    }
    *buf = calloc(1, ws_pkt->len + 1);
    if (*buf == NULL) {
        ESP_LOGE(TAG, "failed to calloc memory for buf");
        return ESP_ERR_NO_MEM;
    }
    ws_pkt->payload = *buf;
    esp_err_t ret = httpd_ws_recv_frame(req, ws_pkt, /* max_len = */ ws_pkt->len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "got packet payload");
    ESP_LOGI(TAG, "packet type: %d", ws_pkt->type);
    return ESP_OK;
}

static esp_err_t camera_handler(
        httpd_req_t *req
) {
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "camera_handler - handshake done after new connection was opened");
        return ESP_OK;
    }
    ESP_LOGI(TAG, "running camera_handler with no handshake");

    esp_err_t ret = ESP_OK;
    uint8_t *buf = NULL;
    httpd_ws_frame_t ws_pkt;

    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    ret = receive_frame_len(req, &ws_pkt);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = receive_frame_payload(req, &ws_pkt, &buf);
    if (ret != ESP_OK) {
        free_ptr(&buf);
        return ret;
    }

    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp((char *) ws_pkt.payload, "s") == 0) {
        ESP_LOGI(TAG, "taking picture...");
        camera_fb_t *pic = esp_camera_fb_get();
        ESP_LOGI(TAG, "picture taken - size = %zu bytes", pic->len);

        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
        ws_pkt.payload = pic->buf;
        ws_pkt.len = pic->len;
        ws_pkt.type = HTTPD_WS_TYPE_BINARY;

        ret = httpd_ws_send_frame(req, &ws_pkt);
        esp_camera_fb_return(pic);

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_send_frame failed sending frame");
        }
    }
    free_ptr(&buf);
    return ret;
}

static const httpd_uri_t ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = camera_handler,
        .user_ctx   = NULL,
        .is_websocket = true
};


static httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Error starting server!");
        return NULL;
    }

    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &ws);
    return server;
}

//////////////////////////////// WEBSOCKET

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

esp_err_t init_nvs() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

void app_main() {
    esp_log_level_set("*", ESP_LOG_NONE);
    ESP_ERROR_CHECK(init_nvs());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(init_camera());
    wifi_init_softap();
    start_webserver();
}
