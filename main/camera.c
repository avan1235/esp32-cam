#include "camera.h"

static const char *TAG = "esp32-cam-camera";

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

        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_SVGA,
        .jpeg_quality = 10,
        .fb_count = 4,
        .fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_LATEST,
};

esp_err_t init_camera() {
    esp_err_t ret = ESP_OK;
    ESP_ERROR_CHECK_RETURN_MSG(esp_camera_init(&camera_config), "esp_camera_init failed");
    return ret;
}

int change_camera_resolution_by(int8_t change) {
    ESP_LOGI(TAG, "change camera resolution");
    sensor_t *camera = esp_camera_sensor_get();
    framesize_t framesize = camera->status.framesize;
    ESP_LOGI(TAG, "old frame size: %d", framesize);
    int64_t changed_framesize = framesize + change;
    if (changed_framesize < 0 || changed_framesize > MAX_FRAME_SIZE) {
        ESP_LOGI(TAG, "changed_framesize %lld is out of bounds - no change", changed_framesize);
        return 0;
    }
    int ret = camera->set_framesize(camera, changed_framesize);
    if (!ret) {
        ESP_LOGE(TAG, "error changing camera resolution: %d", ret);
    }
    return ret;
}