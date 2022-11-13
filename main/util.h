#ifndef ESP32CAM_UTIL_H
#define ESP32CAM_UTIL_H

#define ESP_ERROR_CHECK_RETURN_MSG(x, msg) do { \
    ret = (x);                                  \
    if (ret != ESP_OK) {                        \
        ESP_LOGE(TAG, msg);                     \
        return ret;                             \
    }                                           \
} while(false)

#define ESP_ERROR_CHECK_RETURN(x) do { \
    ret = (x);                         \
    if (ret != ESP_OK) {               \
        return ret;                    \
    }                                  \
} while(false)

#define STRINGIFY(x) #x

#endif //ESP32CAM_UTIL_H
