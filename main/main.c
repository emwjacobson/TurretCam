#include <stdio.h>
#include <stddef.h>
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mqtt_client.h"

// Setup configs
// These should /technically/ be set using `make menuconfig`
#ifndef CONFIG_WIFI_SSID
#define CONFIG_WIFI_SSID "Your_SSID_Here" 
#endif

#ifndef CONFIG_WIFI_PASSWORD
#define CONFIG_WIFI_PASSWORD "Your_Password_Here" 
#endif

#ifndef CONFIG_BROKER_URL
#define CONFIG_BROKER_URL "mqtt://mqtt.eclipseprojects.io" 
#endif

#ifndef CONFIG_BROKER_USER
#define CONFIG_BROKER_USER "User123" 
#endif

#ifndef CONFIG_BROKER_PASSWORD
#define CONFIG_BROKER_PASSWORD "Password123" 
#endif

static const char* TAG = "Main";

esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
    return ESP_OK;
}

esp_err_t init_mqtt() {
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
        .username = CONFIG_BROKER_USER,
        .password = CONFIG_BROKER_PASSWORD,
        .event_handle = mqtt_event_handler
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    return esp_mqtt_client_start(client);
}

esp_err_t init_wifi() {
    esp_err_t err;

    esp_netif_init();
    err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        return err;
    }

    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();

    err = esp_wifi_init(&init_cfg);
    if (err != ESP_OK) {
        return err;
    }
    ESP_LOGI(TAG, "Wifi Initialized");

    wifi_config_t wifi_cfg = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .threshold = {
                .authmode = WIFI_AUTH_WPA2_PSK
            }
        }
    };

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) {
        return err;
    }
    ESP_LOGI(TAG, "WiFi mode set to station");

    err = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg);
    if (err != ESP_OK) {
        return err;
    }
    ESP_LOGI(TAG, "WiFi config set");

    err = esp_wifi_start();
    if (err != ESP_OK) {
        return err;
    }
    ESP_LOGI(TAG, "WiFi started, connecting to " CONFIG_WIFI_SSID);

    return ESP_OK;
}

void init() {
    esp_err_t err;

    err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS. Error: %s", esp_err_to_name(err));
    }

    err = init_wifi();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize wifi. Error: %s", esp_err_to_name(err));
    }

    // // Initialize MQTT
    // err = init_mqtt();
    // if (err != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to initialize MQTT client. Error: %s", esp_err_to_name(err));
    //     exit(1);
    // }
}

void app_main() {
    
    init();

}
