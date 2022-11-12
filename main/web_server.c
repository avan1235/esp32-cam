#include "web_server.h"

#define free_ptr(ptr) do {         \
    void **temp = (void **) (ptr); \
    free(*temp);                   \
    *temp = NULL;                  \
    } while(false)

#define IS_WS_CMD(ws_pkt, cmd) (ws_pkt.type == WS_CMD_TYPE && strcmp((char *) ws_pkt.payload, cmd) == 0)

static const char *TAG = "esp32-cam-web-server";

static const char *INDEX_HTML =
        "<!DOCTYPE html>"
        "<html lang=\"EN\">"
        "<head>"
        "    <title>Camera</title>"
        "    <meta charset=\"utf-8\">"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, maximum-scale=1\">"
        "    <style>"
        "        body {"
        "            margin: 0;"
        "            width: 100%;"
        "            height: 100%;"
        "        }"
        ""
        "        img {"
        "            text-align: center;"
        "            display: block;"
        "            margin: auto;"
        "            max-width: 100%;"
        "            max-height: 100%;"
        "            width: auto;"
        "            height: auto;"
        "        }"
        ""
        "        #btn-container {"
        "            text-align: center;"
        "        }"
        "    </style>"
        "</head>"
        "<body>"
        "<img id=\"video-frame\" alt=\"loading video\" src=\"\"/>"
        "<div id=\"btn-container\">"
        "    <button id=\"decrease-btn\">-</button>"
        "    <button id=\"led-btn\">LED</button>"
        "    <button id=\"increase-btn\">+</button>"
        "</div>"
        "<script>"
        "    (function () {"
        "        const VIDEO_DELAY = 125;"
        "        const camera = new WebSocket(\"ws://192.168.4.1:80/video\");"
        "        const control = new WebSocket(\"ws://192.168.4.1:80/control\");"
        "        const img = document.getElementById(\"video-frame\");"
        ""
        "        function receiveVideoFrameLoop() {"
        "            camera.send(\"s\");"
        "            setTimeout(receiveVideoFrameLoop, VIDEO_DELAY);"
        "        }"
        ""
        "        function setupControlBtn(id, cmd) {"
        "            const element = document.getElementById(id);"
        "            element.addEventListener(\"click\", () => control.send(cmd));"
        "        }"
        ""
        "        camera.onerror = (event) => img.alt = \"camera websocket error\";"
        "        control.onerror = (event) => img.alt = \"control websocket error\";"
        ""
        "        camera.binaryType = \"arraybuffer\";"
        "        camera.onopen = (event) => receiveVideoFrameLoop();"
        "        camera.onmessage = (event) => {"
        "            const uint_data = new Uint8Array(event.data);"
        "            const result = btoa([].reduce.call(uint_data, (p, c) => p + String.fromCharCode(c), \"\"));"
        "            img.src = \"data:image/jpeg;base64,\" + result;"
        "        };"
        "        setupControlBtn(\"decrease-btn\", \"d\");"
        "        setupControlBtn(\"increase-btn\", \"i\");"
        "        setupControlBtn(\"led-btn\", \"l\");"
        "    })();"
        "</script>"
        "</body>"
        "</html>";

static esp_err_t receive_frame_len(
        httpd_req_t *req,
        httpd_ws_frame_t *ws_pkt
) {
    esp_err_t ret = ESP_OK;
    ESP_ERROR_CHECK_RETURN_MSG(httpd_ws_recv_frame(req, ws_pkt, /* max_len = */ 0), "httpd_ws_recv_frame failed");
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
        ws_pkt->payload = NULL;
        free_ptr(buf);
        return ret;
    }
    ESP_LOGI(TAG, "got packet payload - packet type: %d", ws_pkt->type);
    return ESP_OK;
}

static esp_err_t send_picture(
        httpd_req_t *req
) {
    ESP_LOGI(TAG, "taking picture");
    camera_fb_t *pic = esp_camera_fb_get();
    ESP_LOGI(TAG, "picture taken - size = %zu bytes", pic->len);

    httpd_ws_frame_t ws_send_pkt;
    memset(&ws_send_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_send_pkt.payload = pic->buf;
    ws_send_pkt.len = pic->len;
    ws_send_pkt.type = HTTPD_WS_TYPE_BINARY;

    esp_err_t ret = httpd_ws_send_frame(req, &ws_send_pkt);
    esp_camera_fb_return(pic);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame failed sending frame");
    }
    return ret;
}


static esp_err_t receive_ws_pkt(
        httpd_req_t *req,
        httpd_ws_frame_t *ws_recv_pkt,
        uint8_t **buf
) {
    memset(ws_recv_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_recv_pkt->type = HTTPD_WS_TYPE_TEXT;

    esp_err_t ret = ESP_OK;
    ESP_ERROR_CHECK_RETURN(receive_frame_len(req, ws_recv_pkt));
    ESP_ERROR_CHECK_RETURN(receive_frame_payload(req, ws_recv_pkt, buf));
    return ESP_OK;
}

static esp_err_t video_handler(
        httpd_req_t *req
) {
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "video_handler - handshake done after new connection was opened");
        return ESP_OK;
    }
    ESP_LOGI(TAG, "running video_handler with no handshake");

    esp_err_t ret = ESP_OK;
    uint8_t *buf = NULL;
    httpd_ws_frame_t ws_recv_pkt;

    ESP_ERROR_CHECK_RETURN(receive_ws_pkt(req, &ws_recv_pkt, &buf));
    if (IS_WS_CMD(ws_recv_pkt, WS_CMD_SHOOT)) {
        ret = send_picture(req);
    } else {
        ESP_LOGE(TAG, "received unknown command on video socket");
    }
    free_ptr(&buf);
    return ret;
}

static const httpd_uri_t VIDEO_WS = {
        .uri        = "/video",
        .method     = HTTP_GET,
        .handler    = video_handler,
        .user_ctx   = NULL,
        .is_websocket = true
};

static esp_err_t control_handler(
        httpd_req_t *req
) {
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "control_handler - handshake done after new connection was opened");
        return ESP_OK;
    }
    ESP_LOGI(TAG, "running control_handler with no handshake");

    esp_err_t ret = ESP_OK;
    uint8_t *buf = NULL;
    httpd_ws_frame_t ws_recv_pkt;

    ESP_ERROR_CHECK_RETURN(receive_ws_pkt(req, &ws_recv_pkt, &buf));
    if (IS_WS_CMD(ws_recv_pkt, WS_CMD_INCREASE_RESOLUTION)) {
        ret = change_camera_resolution_by(+1);
    } else if (IS_WS_CMD(ws_recv_pkt, WS_CMD_DECREASE_RESOLUTION)) {
        ret = change_camera_resolution_by(-1);
    } else if (IS_WS_CMD(ws_recv_pkt, WS_CMD_CHANGE_FLASH_LED)) {
        ret = switch_flash_led();
    } else {
        ESP_LOGE(TAG, "received unknown command on control socket");
    }
    free_ptr(&buf);
    return ret;
}

static const httpd_uri_t CONTROL_WS = {
        .uri        = "/control",
        .method     = HTTP_GET,
        .handler    = control_handler,
        .user_ctx   = NULL,
        .is_websocket = true
};

static esp_err_t index_handler(
        httpd_req_t *req
) {
    return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

static const httpd_uri_t INDEX = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL
};

esp_err_t start_webserver() {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "starting server on port %d", config.server_port);

    esp_err_t ret = ESP_OK;
    ESP_ERROR_CHECK_RETURN_MSG(httpd_start(&server, &config), "httpd_start failed starting server");

    ESP_LOGI(TAG, "registering URI handlers");

    ESP_ERROR_CHECK_RETURN_MSG(httpd_register_uri_handler(server, &VIDEO_WS),
                               "VIDEO_WS httpd_register_uri_handler failed");
    ESP_LOGI(TAG, "VIDEO_WS registered");

    ESP_ERROR_CHECK_RETURN_MSG(httpd_register_uri_handler(server, &CONTROL_WS),
                               "CONTROL_WS httpd_register_uri_handler failed");
    ESP_LOGI(TAG, "CONTROL_WS registered");

    ESP_ERROR_CHECK_RETURN_MSG(httpd_register_uri_handler(server, &INDEX),
                               "INDEX httpd_register_uri_handler failed");
    ESP_LOGI(TAG, "INDEX registered");

    return ret;
}