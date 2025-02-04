#include "sdcard.h"

#include <esp_camera.h>
#include <esp_netif_sntp.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <driver/sdmmc_default_configs.h>
#include <driver/sdmmc_host.h>
#include <sys/dirent.h>

#define MOUNT_POINT "/sdcard"
#define FORMAT_IF_MOUNT_FAILED false

static const char *TAG = "esp32-cam-sdcard";

static esp_err_t write_file(const char *path, uint8_t *data, size_t size) {
    ESP_LOGI(TAG, "opening file %s", path);

    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "failed to open file for writing");
        return ESP_FAIL;
    }
    fwrite(data, sizeof(uint8_t), size, f);
    fclose(f);

    ESP_LOGI(TAG, "File written");

    return ESP_OK;
}

static void obtain_time() {
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("europe.pool.ntp.org");
    config.start = true; // start SNTP service explicitly (after connecting)
    config.server_from_dhcp = true; // accept NTP offers from DHCP server, if any (need to enable *before* connecting)
    config.renew_servers_after_new_IP = true;
    config.index_of_first_server = 1;
    config.ip_event_to_renew = IP_EVENT_STA_GOT_IP;


    ESP_LOGI(TAG, "Starting SNTP");

    esp_netif_sntp_init(&config);

    // wait for time to be set

    int retry = 0;
    const int retry_count = 15;
    while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    }

    time_t now = 0;
    time(&now);


    struct tm *timeinfo;
    char buffer[80];

    // Convert time_t to struct tm (local time)
    timeinfo = localtime(&now);

    // Format the time as "YYYY-MM-DD HH:MM:SS"
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    ESP_LOGI(TAG, "System time %s", buffer);

    esp_netif_sntp_deinit();
}

esp_err_t init_sdcard() {
    ESP_LOGI(TAG, "Init sd card");

    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = FORMAT_IF_MOUNT_FAILED,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
        .disk_status_check_enable = false,
        .use_one_fat = false,
    };
    sdmmc_card_t *card;

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;

    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdmmc_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem"
                     "If you want the card to be formatted, set the FORMAT_IF_MOUNT_FAILED config option");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s)"
                     "Make sure SD card lines have pull-up resistors in place", esp_err_to_name(ret));
        }
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    obtain_time();

    return ret;
}

esp_err_t start_photo_collect() {
    const char *path = MOUNT_POINT"/image.jpg";


    ESP_LOGI(TAG, "Written to file %s", path);

    camera_fb_t *pic = esp_camera_fb_get();
    if (!pic) {
        esp_camera_fb_return(pic);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "picture taken - size = %zu bytes", pic->len);
    write_file(path, pic->buf, pic->len);

    esp_camera_fb_return(pic);

    return ESP_OK;
}
