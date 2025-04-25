#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#define OLED_ADDR 0x3C

#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM    I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000

void i2c_master_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

// Komut gönder (control byte = 0x00)
void oled_send_command(uint8_t command)
{
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (OLED_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(handle, 0x00, true);  // COMMAND
    i2c_master_write_byte(handle, command, true);
    i2c_master_stop(handle);
    i2c_master_cmd_begin(I2C_MASTER_NUM, handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(handle);
}

// Veri gönder (control byte = 0x40)
void oled_send_data(uint8_t data)
{
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (OLED_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(handle, 0x40, true);  // DATA
    i2c_master_write_byte(handle, data, true);
    i2c_master_stop(handle);
    i2c_master_cmd_begin(I2C_MASTER_NUM, handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(handle);
}

// OLED ekranı başlat
void oled_init()
{
    oled_send_command(0xAE);
    oled_send_command(0x20); oled_send_command(0x00);
    oled_send_command(0xB0);
    oled_send_command(0xC8);
    oled_send_command(0x00);
    oled_send_command(0x10);
    oled_send_command(0x40);
    oled_send_command(0x81); oled_send_command(0xFF);
    oled_send_command(0xA1);
    oled_send_command(0xA6);
    oled_send_command(0xA8); oled_send_command(0x3F);
    oled_send_command(0xA4);
    oled_send_command(0xD3); oled_send_command(0x00);
    oled_send_command(0xD5); oled_send_command(0xF0);
    oled_send_command(0xD9); oled_send_command(0x22);
    oled_send_command(0xDA); oled_send_command(0x12);
    oled_send_command(0xDB); oled_send_command(0x20);
    oled_send_command(0x8D); oled_send_command(0x14);
    oled_send_command(0xAF);
}

// OLED ekranı temizle
void oled_clear()
{
    for (uint8_t page = 0; page < 8; page++) {
        oled_send_command(0xB0 + page);
        oled_send_command(0x00);
        oled_send_command(0x10);
        for (uint8_t col = 0; col < 128; col++) {
            oled_send_data(0x00);
        }
    }
}

// OLED'e basit bir şey çiz
void oled_draw_simple()
{
    oled_send_command(0xB0); // 0. page
    oled_send_command(0x00); // Low column
    oled_send_command(0x10); // High column

    oled_send_data(0xFF);
    oled_send_data(0x81);
    oled_send_data(0x81);
    oled_send_data(0xFF);
}

void app_main(void)
{
    i2c_master_init();
    oled_init();
    oled_clear();
    oled_draw_simple();
}
