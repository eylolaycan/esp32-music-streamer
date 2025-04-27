#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/i2c.h"
#include <string.h>

#define WIFI_SSID "WINDTRE-14B490"
#define WIFI_PASS "7cx472b8u5u57k8r"
#define OLED_ADDR 0x3C

#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

static const uint8_t font5x7[][5] = {
    // 'A' - 'Z'
    {0x7E,0x11,0x11,0x7E,0x00}, // A
    {0x7F,0x49,0x49,0x36,0x00}, // B
    {0x3E,0x41,0x41,0x22,0x00}, // C
    {0x7F,0x41,0x41,0x3E,0x00}, // D
    {0x7F,0x49,0x49,0x41,0x00}, // E
    {0x7F,0x09,0x09,0x01,0x00}, // F
    {0x3E,0x41,0x51,0x32,0x00}, // G
    {0x7F,0x08,0x08,0x7F,0x00}, // H
    {0x41,0x7F,0x41,0x00,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x00}, // J
    {0x7F,0x08,0x14,0x63,0x00}, // K
    {0x7F,0x40,0x40,0x40,0x00}, // L
    {0x7F,0x02,0x04,0x02,0x7F}, // M
    {0x7F,0x06,0x18,0x7F,0x00}, // N
    {0x3E,0x41,0x41,0x3E,0x00}, // O
    {0x7F,0x09,0x09,0x06,0x00}, // P
    {0x3E,0x41,0x61,0x7E,0x00}, // Q
    {0x7F,0x09,0x19,0x66,0x00}, // R
    {0x46,0x49,0x49,0x31,0x00}, // S
    {0x01,0x7F,0x01,0x01,0x00}, // T
    {0x3F,0x40,0x40,0x3F,0x00}, // U
    {0x1F,0x20,0x40,0x20,0x1F}, // V
    {0x3F,0x40,0x38,0x40,0x3F}, // W
    {0x63,0x14,0x08,0x14,0x63}, // X
    {0x07,0x08,0x70,0x08,0x07}, // Y
    {0x61,0x51,0x49,0x45,0x43}  // Z
};

static const char *TAG = "MAIN";

// Prototypes
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

// OLED functions
void i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    i2c_param_config(I2C_MASTER_NUM, &conf); // &conf is the configuration structure
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0); // 0 is the RX buffer size, 0 is the TX buffer size, 0 is the I2C driver install flags
}

/*
 * CMD Link is a structure that holds the I2C command link. 
 * It is used to create a sequence of I2C commands to be sent to the slave device. 
 * The I2C command link is created using the i2c_cmd_link_create() function.
 * The command link is deleted using the i2c_cmd_link_delete() function.
 */
void oled_send_cmd(uint8_t cmd) {
    // Create a command link to send commands to the OLED display
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    
    // Start the command link
    i2c_master_start(handle);

    /* 
     * (OLED_ADDR << 1) | I2C_MASTER_WRITE is the address of the OLED display in write mode
     * The OLED_ADDR is the 7-bit I2C address of the OLED display.
     * The address is shifted left by 1 bit to make room for the read/write bit by using the bitwise OR operator (|) and << 1
     * I2C_MASTER_WRITE is a macro that defines the write mode for the I2C communication
     * The true parameter indicates that the command was sent successfully
     * The i2c_master_write_byte() function sends a byte to the I2C slave device
     */
    i2c_master_write_byte(handle, (OLED_ADDR << 1) | I2C_MASTER_WRITE, true);
    // The 0x00 command byte is used to indicate that the next byte is a command byte
    i2c_master_write_byte(handle, 0x00, true);
    // cmd is the command byte to be sent to the OLED display
    i2c_master_write_byte(handle, cmd, true);
    i2c_master_stop(handle);

    /* 
     * The i2c_master_cmd_begin() function sends the command link to the I2C bus and waits.
     * The I2C_MASTER_NUM is the I2C port number to be used for the communication
     * The handle is the command link to be sent to the I2C bus
     * The 1000 / portTICK_PERIOD_MS is the timeout for the command link to be sent
     */
    i2c_master_cmd_begin(I2C_MASTER_NUM, handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(handle);
}

/*
 * The oled_send_data() function sends data to the OLED display using the I2C protocol.
 * It creates a command link, starts the command link, sends the address of the OLED display in write mode,
 * sends the data byte, and stops the command link.
 * The function uses the i2c_master_start(), i2c_master_write_byte(), and i2c_master_stop() functions to create the command link.
 * The i2c_master_cmd_begin() function sends the command link to the I2C bus and waits.
 * The i2c_cmd_link_delete() function deletes the command link after it has been sent.
 */
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


//The oled_clear() function clears the OLED display by sending a series of commands to the display.
void oled_clear() {
    /*
     * The idea behind this loop is to clear the OLED display by writing 0x00 to each pixel on the screen.
     * The OLED display is divided into 8 pages, and each page has 128 columns. 
     * We loop through each page (0-7) and set the page address using the 0xB0 command.
     * Then we set the column address using the 0x00 and 0x10 commands.
     * Finally, we write 0x00 to each of the 128 columns in that page.
     */
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
    /*
     * This array works as a font table, where each character is represented by a 5x7 pixel bitmap.
     * We first check if the character is between 'A' and 'Z' (uppercase letters).
     * If it is, we get the corresponding bitmap from the font5x7 array.
     * Then we loop through the 5 bytes of the bitmap and send each byte to the OLED display using the oled_send_data() function.
     * Finally, we send a space (0x00) to create a gap between characters.
     */
    if (c >= 'A' && c <= 'Z') {
        uint8_t *bitmap = (uint8_t *)font5x7[c-'A'];
        for (int i = 0; i < 5; i++) {
            oled_send_data(bitmap[i]);
        }
        oled_send_data(0x00); // Space between characters
    }
}

void oled_write_connected_text() {
    oled_clear();

    oled_send_cmd(0xB2); // page 2.
    oled_send_cmd(0x00); // low column address
    oled_send_cmd(0x10); // high column address

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

// WiFi event handler
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi Connected!");

        oled_write_connected_text(); // 🔥 OLED'e Connected yazısını bas
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi Disconnected. Reconnecting...");
        esp_wifi_connect();
    }
}

void wifi_init_sta() {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);

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
}
