#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/i2c.h"
#include <string.h>
#include "esp_http_client.h"
#include "esp_netif.h"

#define WIFI_SSID "WINDTRE-14B490"
#define WIFI_PASS "7cx472b8u5u57k8r"
#define OLED_ADDR 0x3C

#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000

static const uint8_t font5x7[][5] = {
    {0x7E,0x11,0x11,0x7E,0x00}, {0x7F,0x49,0x49,0x36,0x00}, {0x3E,0x41,0x41,0x22,0x00},
    {0x7F,0x41,0x41,0x3E,0x00}, {0x7F,0x49,0x49,0x41,0x00}, {0x7F,0x09,0x09,0x01,0x00},
    {0x3E,0x41,0x51,0x32,0x00}, {0x7F,0x08,0x08,0x7F,0x00}, {0x41,0x7F,0x41,0x00,0x00},
    {0x20,0x40,0x41,0x3F,0x00}, {0x7F,0x08,0x14,0x63,0x00}, {0x7F,0x40,0x40,0x40,0x00},
    {0x7F,0x02,0x04,0x02,0x7F}, {0x7F,0x06,0x18,0x7F,0x00}, {0x3E,0x41,0x41,0x3E,0x00},
    {0x7F,0x09,0x09,0x06,0x00}, {0x3E,0x41,0x61,0x7E,0x00}, {0x7F,0x09,0x19,0x66,0x00},
    {0x46,0x49,0x49,0x31,0x00}, {0x01,0x7F,0x01,0x01,0x00}, {0x3F,0x40,0x40,0x3F,0x00},
    {0x1F,0x20,0x40,0x20,0x1F}, {0x3F,0x40,0x38,0x40,0x3F}, {0x63,0x14,0x08,0x14,0x63},
    {0x07,0x08,0x70,0x08,0x07}, {0x61,0x51,0x49,0x45,0x43}
};

static const char *TAG = "MAIN";

void i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void oled_send_cmd(uint8_t cmd) {
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (OLED_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(handle, 0x00, true);
    i2c_master_write_byte(handle, cmd, true);
    i2c_master_stop(handle);
    i2c_master_cmd_begin(I2C_MASTER_NUM, handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(handle);
}

void oled_send_data(uint8_t data) {
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (OLED_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(handle, 0x40, true);
    i2c_master_write_byte(handle, data, true);
    i2c_master_stop(handle);
    i2c_master_cmd_begin(I2C_MASTER_NUM, handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(handle);
}

void oled_clear() {
    for (uint8_t page = 0; page < 8; page++) {
        oled_send_cmd(0xB0 + page);
        oled_send_cmd(0x00);
        oled_send_cmd(0x10);
        for (uint8_t i = 0; i < 128; i++) {
            oled_send_data(0x00);
        }
    }
}

void oled_write_char(char c) {
    if (c >= 'A' && c <= 'Z') {
        uint8_t *bitmap = (uint8_t *)font5x7[c - 'A'];
        for (int i = 0; i < 5; i++) {
            oled_send_data(bitmap[i]);
        }
        oled_send_data(0x00);
    }
}

void oled_write_connected_text() {
    oled_clear();
    oled_send_cmd(0xB2);
    oled_send_cmd(0x00);
    oled_send_cmd(0x10);
    const char *text = "CONNECTED";
    while (*text) {
        oled_write_char(*text);
        text++;
    }
}

void oled_init() {
    vTaskDelay(100 / portTICK_PERIOD_MS);
    oled_send_cmd(0xAE);
    oled_send_cmd(0x20); oled_send_cmd(0x00);
    oled_send_cmd(0xB0);
    oled_send_cmd(0xC8);
    oled_send_cmd(0x00);
    oled_send_cmd(0x10);
    oled_send_cmd(0x40);
    oled_send_cmd(0x81); oled_send_cmd(0xFF);
    oled_send_cmd(0xA1);
    oled_send_cmd(0xA6);
    oled_send_cmd(0xA8); oled_send_cmd(0x3F);
    oled_send_cmd(0xA4);
    oled_send_cmd(0xD3); oled_send_cmd(0x00);
    oled_send_cmd(0xD5); oled_send_cmd(0xF0);
    oled_send_cmd(0xD9); oled_send_cmd(0x22);
    oled_send_cmd(0xDA); oled_send_cmd(0x12);
    oled_send_cmd(0xDB); oled_send_cmd(0x20);
    oled_send_cmd(0x8D); oled_send_cmd(0x14);
    oled_send_cmd(0xAF);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi Connected!");
        oled_write_connected_text();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi Disconnected. Reconnecting...");
        esp_wifi_connect();
    }
}

void wifi_init_sta(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}

void set_dns_server() {
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_dns_info_t dns;
    dns.ip.u_addr.ip4.addr = ipaddr_addr("8.8.8.8");
    dns.ip.type = IPADDR_TYPE_V4;
    esp_netif_set_dns_info(netif, ESP_NETIF_DNS_MAIN, &dns);
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            printf("%.*s", evt->data_len, (char*)evt->data);
            oled_clear();
            oled_send_cmd(0xB0);
            oled_send_cmd(0x00);
            oled_send_cmd(0x10);
            for (int i = 0; i < evt->data_len; i++) {
                char c = ((char*)evt->data)[i];
                if (c >= 'A' && c <= 'Z') {
                    oled_write_char(c);
                }
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

void http_get_task(void *pvParameters) {
    ESP_LOGI(TAG, "Starting HTTP GET task...");

    esp_http_client_config_t config = {
        .url = "http://192.168.1.7:8080/test.txt",
        .event_handler = _http_event_handler,
        .disable_auto_redirect = true,
        .skip_cert_common_name_check = true,
        .transport_type = HTTP_TRANSPORT_OVER_TCP,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    i2c_master_init();
    oled_init();
    oled_clear();
    wifi_init_sta();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    set_dns_server();
    xTaskCreate(&http_get_task, "http_get_task", 8192, NULL, 5, NULL);
}
