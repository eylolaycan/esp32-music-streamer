#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h" // to see events
#include "esp_log.h" 
#include "nvs_flash.h" // non volatile storage
#include "driver/i2c.h" // I2C driver
 
#define WIFI_SSID "WINDTRE-14B490"
#define WIFI_PASS "7cx472b8u5u57k8r"
#define OLED_ADDR 0x3C // I2C address of the OLED display

#define I2C_MASTER_SCL_IO 21   // GPIO pin for I2C SDA
#define I2C_MASTER_SDA_IO 22   // GPIO pin for I2C SCL
#define I2C_MASTER_NUM I2C_NUM_0 // I2C port number for master dev
#define I2C_MASTER_FREQ_HZ 100000 // I2C frequency
#define I2C_MASTER_TX_BUF_DISABLE 0 // I2C master doesn't need buffer
#define I2C_MASTER_RX_BUF_DISABLE 0 // I2C master doesn't need buffer



// The default Wi-Fi configuration
static const char *TAG = "WIFI";

void wifi_init_sta(void) {
    esp_netif_init();                // Initialize the network interface

    esp_event_loop_create_default(); // Create the default event loop
                                     // This is where we will handle events

    esp_netif_create_default_wifi_sta(); // Create the default Wi-Fi station interface
                                         // This is where we will connect to the Wi-Fi network

    // Initialize the Wi-Fi driver
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // information about the Wi-Fi configuration
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    // Set the Wi-Fi configuration as station (client) not AP (access point)
    esp_wifi_set_mode(WIFI_MODE_STA);
    // Set the Wi-Fi configuration
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    // Start the Wi-Fi driver
    esp_wifi_start();
    // writes to log
    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

// i2c_master_init function initializes the I2C master
// It sets the I2C mode, SDA and SCL pins, pull-up resistors, and clock speed
// It also installs the I2C driver
// The I2C driver is used to communicate with the OLED display
void i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER, // I2C master mode
        .sda_io_num = I2C_MASTER_SDA_IO, // SDA pin
        .scl_io_num = I2C_MASTER_SCL_IO, // SCL pin
        .sda_pullup_en = GPIO_PULLUP_ENABLE, // Enable pull-up resistor for SDA pin
        .scl_pullup_en = GPIO_PULLUP_ENABLE, // Enable pull-up resistor for SCL pin
        .master.clk_speed = I2C_MASTER_FREQ_HZ, // I2C clock speed
    };
    // Install the I2C driver
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode,
                       I2C_MASTER_RX_BUF_DISABLE,
                       I2C_MASTER_TX_BUF_DISABLE, 0);
}

void oled_send_command(uint8_t command) {
    i2c_cmd_handle_t handle = i2c_cmd_link_create(); // Create a new I2C command link
    i2c_master_start(handle); // Start the I2C command link
    i2c_master_write_byte(handle, OLED_ADDR << 1 | I2C_MASTER_WRITE, true); // Write the OLED address and command
    i2c_master_write_byte(handle, 0x00, true); // Send the command byte
    i2c_master_write_byte(handle, command, true); // Send the command
    i2c_master_stop(handle); // Stop the I2C command link
    i2c_master_cmd_begin(I2C_MASTER_NUM, handle, 1000 / portTICK_PERIOD_MS); // Execute the command
    i2c_cmd_link_delete(handle); // Delete the command link
}  

void oled_init() {
    oled_send_command(0xAE); // Display OFF (sleep mode)
    oled_send_command(0x20); // Set Memory Addressing Mode
    oled_send_command(0x00); // Horizontal addressing mode
    oled_send_command(0xB0); // Set Page Start Address for Page Addressing Mode
    oled_send_command(0xC8); // COM Output Scan Direction
    oled_send_command(0x00); // --set low column address
    oled_send_command(0x10); // --set high column address
    oled_send_command(0x40); // --set start line address
    oled_send_command(0x81); oled_send_command(0xFF); // Set contrast control
    oled_send_command(0xA1); // Set segment re-map 0 to 127
    oled_send_command(0xA6); // Normal display (not inverted)
    oled_send_command(0xA8); oled_send_command(0x3F); // Set multiplex ratio (1/64 duty)
    oled_send_command(0xA4); // Entire Display ON
    oled_send_command(0xD3); oled_send_command(0x00); // Set display offset
    oled_send_command(0xD5); oled_send_command(0xF0); // Set display clock divide ratio
    oled_send_command(0xD9); oled_send_command(0x22); // Set pre-charge period
    oled_send_command(0xDA); oled_send_command(0x12); // Set COM pins hardware configuration 
    oled_send_command(0xDB); oled_send_command(0x20); // Set VCOMH Deselect Level which is 0.77*Vcc
    oled_send_command(0x8D); oled_send_command(0x14); // Enable charge pump
    oled_send_command(0xAF); // Display ON (sleep mode off)
}

void app_main(void) {
    /* // Initialize the NVS flash storage
    // This is used to store the Wi-Fi credentials and other data
    // that need to be retained across reboots
    // This is a one-time operation, so we do it here in the app_main function
    esp_err_t ret = nvs_flash_init();
    // this is used to check if the NVS flash storage is already initialized
    // If it is not initialized, we need to erase it and initialize it again
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Initialize the Wi-Fi driver and connect to the Wi-Fi network
    // This is where we will handle the Wi-Fi events
    // such as connection, disconnection, etc.
    wifi_init_sta();
    // FreeRTOS frees the memory used by the task every 1000 ms
    // This is used to prevent the task from consuming too much CPU time
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    } 
    */
    i2c_master_init(); // Initialize the I2C master
    oled_init(); // Initialize the OLED display

}
