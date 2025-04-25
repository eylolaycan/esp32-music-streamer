#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h" // to see events
#include "esp_log.h" 
#include "nvs_flash.h" // non volatile storage
 
#define WIFI_SSID "WINDTRE-14B490"
#define WIFI_PASS "7cx472b8u5u57k8r"

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

void app_main(void) {
    // Initialize the NVS flash storage
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
}
