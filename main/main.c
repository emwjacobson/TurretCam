#include <stdio.h>
#include <stddef.h>
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mqtt_client.h"
#include "esp_wifi.h"

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

void mqtt_event_handler() {
    // TODO
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

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        return err;
    }
    ESP_LOGI(TAG, "Wifi Initialized");

    wifi_config_t cfg = {
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

    err = esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg);
    if (err != ESP_OK) {
        return err;
    }
    ESP_LOGI(TAG, "WiFi config set");

    err = esp_wifi_start();
    if (err != ESP_OK) {
        return err;
    }
    ESP_LOGI(TAG, "WiFi started");

    return ESP_OK;
}

void init() {
    esp_err_t err;
    err = init_wifi();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize wifi. Error: %s", esp_err_to_name(err));
    }

    // Initialize MQTT
    err = init_mqtt();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client. Error: %s", esp_err_to_name(err));
        exit(1);
    }
}

void app_main() {
    init();

}
