#include "st7789.h"

#if CONFIG_SPI2_HOST
#define HOST_ID SPI2_HOST
#elif CONFIG_SPI3_HOST
#define HOST_ID SPI3_HOST
#endif

#define SPI_COMMAND_MODE 0
#define SPI_DATA_MODE 1
#define SPI_FREQUENCY SPI_MASTER_FREQ_20M

static const char *TAG = "esp32-cam-st7789";

static void delay_ms(TickType_t ms) {
    TickType_t xTicksToDelay = (ms + (portTICK_PERIOD_MS - 1)) / portTICK_PERIOD_MS;
    vTaskDelay(xTicksToDelay);
}

static void spi_master_write_byte(spi_device_handle_t handle, const uint8_t *data, size_t length) {
    spi_transaction_t transaction;
    esp_err_t ret = ESP_OK;

    if (length > 0) {
        memset(&transaction, 0, sizeof(spi_transaction_t));
        transaction.length = length * 8;
        transaction.tx_buffer = data;
        ret = spi_device_transmit(handle, &transaction);
    }
    assert(ret == ESP_OK);
}

static void spi_master_write_command(TFT_t *dev, uint8_t cmd) {
    static uint8_t static_cmd = 0;
    static_cmd = cmd;
    gpio_set_level(dev->_dc, SPI_COMMAND_MODE);
    spi_master_write_byte(dev->_handle, &static_cmd, 1);
}

static void spi_master_write_data_byte(TFT_t *dev, uint8_t data) {
    static uint8_t static_data = 0;
    static_data = data;
    gpio_set_level(dev->_dc, SPI_DATA_MODE);
    spi_master_write_byte(dev->_handle, &static_data, 1);
}

static void spi_master_write_addr(TFT_t *dev, uint16_t addr1, uint16_t addr2) {
    static uint8_t static_addr[4];
    static_addr[0] = (addr1 >> 8) & 0xFF;
    static_addr[1] = addr1 & 0xFF;
    static_addr[2] = (addr2 >> 8) & 0xFF;
    static_addr[3] = addr2 & 0xFF;
    gpio_set_level(dev->_dc, SPI_DATA_MODE);
    spi_master_write_byte(dev->_handle, static_addr, 4);
}

static void spi_master_write_color(TFT_t *dev, uint16_t color, uint16_t size) {
    static uint8_t static_color[1024];
    int index = 0;
    for (int i = 0; i < size; i++) {
        static_color[index++] = (color >> 8) & 0xFF;
        static_color[index++] = color & 0xFF;
    }
    gpio_set_level(dev->_dc, SPI_DATA_MODE);
    spi_master_write_byte(dev->_handle, static_color, size * 2);
}

static void spi_master_write_colors(TFT_t *dev, const uint16_t *colors, uint16_t size) {
    static uint8_t static_colors[1024];
    int index = 0;
    for (int i = 0; i < size; i++) {
        static_colors[index++] = (colors[i] >> 8) & 0xFF;
        static_colors[index++] = colors[i] & 0xFF;
    }
    gpio_set_level(dev->_dc, SPI_DATA_MODE);
    spi_master_write_byte(dev->_handle, static_colors, size * 2);
}

void
spi_master_init(
        TFT_t *dev,
        int16_t GPIO_MOSI,
        int16_t GPIO_SCLK,
        int16_t GPIO_CS,
        int16_t GPIO_DC,
        int16_t GPIO_RESET,
        int16_t GPIO_BL
) {
    esp_err_t ret;

    ESP_LOGI(TAG, "GPIO_CS=%d", GPIO_CS);
    if (GPIO_CS >= 0) {
        gpio_reset_pin(GPIO_CS);
        gpio_set_direction(GPIO_CS, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_CS, 0);
    }

    ESP_LOGI(TAG, "GPIO_DC=%d", GPIO_DC);
    gpio_reset_pin(GPIO_DC);
    gpio_set_direction(GPIO_DC, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_DC, 0);

    ESP_LOGI(TAG, "GPIO_RESET=%d", GPIO_RESET);
    if (GPIO_RESET >= 0) {
        gpio_reset_pin(GPIO_RESET);
        gpio_set_direction(GPIO_RESET, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_RESET, 1);
        delay_ms(100);
        gpio_set_level(GPIO_RESET, 0);
        delay_ms(100);
        gpio_set_level(GPIO_RESET, 1);
        delay_ms(100);
    }

    ESP_LOGI(TAG, "GPIO_BL=%d", GPIO_BL);
    if (GPIO_BL >= 0) {
        gpio_reset_pin(GPIO_BL);
        gpio_set_direction(GPIO_BL, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_BL, 0);
    }

    ESP_LOGI(TAG, "GPIO_MOSI=%d", GPIO_MOSI);
    ESP_LOGI(TAG, "GPIO_SCLK=%d", GPIO_SCLK);
    spi_bus_config_t buscfg = {
            .mosi_io_num = GPIO_MOSI,
            .miso_io_num = -1,
            .sclk_io_num = GPIO_SCLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 0,
            .flags = 0
    };

    ret = spi_bus_initialize(HOST_ID, &buscfg, SPI_DMA_CH_AUTO);
    ESP_LOGD(TAG, "spi_bus_initialize=%d", ret);
    assert(ret == ESP_OK);

    spi_device_interface_config_t devcfg;
    memset(&devcfg, 0, sizeof(devcfg));
    devcfg.clock_speed_hz = SPI_FREQUENCY;
    devcfg.queue_size = 7;
    devcfg.mode = 2;
    devcfg.flags = SPI_DEVICE_NO_DUMMY;

    if (GPIO_CS >= 0) {
        devcfg.spics_io_num = GPIO_CS;
    } else {
        devcfg.spics_io_num = -1;
    }

    spi_device_handle_t handle;
    ret = spi_bus_add_device(HOST_ID, &devcfg, &handle);
    ESP_LOGD(TAG, "spi_bus_add_device=%d", ret);
    ESP_ERROR_CHECK(ret);
    dev->_dc = GPIO_DC;
    dev->_bl = GPIO_BL;
    dev->_handle = handle;
}

void lcd_init(TFT_t *dev, int width, int height, int offsetx, int offsety) {
    dev->_width = width;
    dev->_height = height;
    dev->_offsetx = offsetx;
    dev->_offsety = offsety;

    spi_master_write_command(dev, 0x01);
    delay_ms(150);

    spi_master_write_command(dev, 0x11);
    delay_ms(255);

    spi_master_write_command(dev, 0x3A);
    spi_master_write_data_byte(dev, 0x55);
    delay_ms(10);

    spi_master_write_command(dev, 0x36);
    spi_master_write_data_byte(dev, 0x00);

    spi_master_write_command(dev, 0x2A);
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0xF0);

    spi_master_write_command(dev, 0x2B);
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0xF0);

    spi_master_write_command(dev, 0x21);
    delay_ms(10);

    spi_master_write_command(dev, 0x13);
    delay_ms(10);

    spi_master_write_command(dev, 0x29);
    delay_ms(255);

    if (dev->_bl >= 0) {
        gpio_set_level(dev->_bl, 1);
    }
}


void lcd_draw_multi_pixels(TFT_t *dev, uint16_t x, uint16_t y, uint16_t size, uint16_t *colors) {
    if (x + size > dev->_width) return;
    if (y >= dev->_height) return;

    uint16_t _x1 = x + dev->_offsetx;
    uint16_t _x2 = _x1 + (size - 1);
    uint16_t _y1 = y + dev->_offsety;
    uint16_t _y2 = _y1;

    spi_master_write_command(dev, 0x2A);
    spi_master_write_addr(dev, _x1, _x2);
    spi_master_write_command(dev, 0x2B);
    spi_master_write_addr(dev, _y1, _y2);
    spi_master_write_command(dev, 0x2C);
    spi_master_write_colors(dev, colors, size);
}

void lcd_draw_fill_rect(TFT_t *dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    if (x1 >= dev->_width) return;
    if (x2 >= dev->_width) x2 = dev->_width - 1;
    if (y1 >= dev->_height) return;
    if (y2 >= dev->_height) y2 = dev->_height - 1;

    ESP_LOGD(TAG, "offset(x)=%d offset(y)=%d", dev->_offsetx, dev->_offsety);
    uint16_t _x1 = x1 + dev->_offsetx;
    uint16_t _x2 = x2 + dev->_offsetx;
    uint16_t _y1 = y1 + dev->_offsety;
    uint16_t _y2 = y2 + dev->_offsety;

    spi_master_write_command(dev, 0x2A);
    spi_master_write_addr(dev, _x1, _x2);
    spi_master_write_command(dev, 0x2B);
    spi_master_write_addr(dev, _y1, _y2);
    spi_master_write_command(dev, 0x2C);
    for (int i = _x1; i <= _x2; i++) {
        uint16_t size = _y2 - _y1 + 1;
        spi_master_write_color(dev, color, size);
    }
}

void lcd_display_off(TFT_t *dev) {
    spi_master_write_command(dev, 0x28);
}

void lcd_display_on(TFT_t *dev) {
    spi_master_write_command(dev, 0x29);
}

void lcd_fill_screen(TFT_t *dev, uint16_t color) {
    lcd_draw_fill_rect(dev, 0, 0, dev->_width - 1, dev->_height - 1, color);
}
