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
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"
#include "servo.h"
#include "stepper.h"

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
static bool wifi_connected = false;

/**
 * @brief Handler for MQTT events
 * 
 * @param event_handler_arg Arbiturary data argument. Is set to NULL in my implementation.
 * @param event_base Unused
 * @param event_id Unused
 * @param event_data The data of the event
 */
void mqtt_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            
            msg_id = esp_mqtt_client_subscribe(client, "turretcam/move", 2);
            ESP_LOGI(TAG, "Subscribed to turretcam/move, msg_id=%d", msg_id);

            // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            if (strncmp(event->topic, 
                        "turretcam/move",
                        event->topic_len < sizeof("turretcam/move") ? event->topic_len : sizeof("turretcam/move")
                        ) == 0) {
                ESP_LOGI(TAG, "Got move event: %.*s", event->data_len, event->data);
                /*
                    Rotation is absolute, ranging from 0 to 100 degrees
                    Height is absolute, ranging from 0 to 100 degrees

                    {
                        "rotation": int,
                        "height": int
                    }
                */

                cJSON* json = cJSON_Parse(event->data);
                if (json == NULL) {
                    const char* error_ptr = cJSON_GetErrorPtr();
                    if (error_ptr != NULL) {
                        ESP_LOGW(TAG, "Error before: %.*s", 10, error_ptr);
                    }
                    ESP_LOGW(TAG, "Error parsing move event JSON");
                    cJSON_Delete(json);
                    break;
                }

                int rotation = INT_MAX;
                int height = INT_MAX;
                int height_mode = INT_MAX;
                int speed = INT_MAX;

                cJSON* rotation_cj = cJSON_GetObjectItem(json, "rotation");
                if (cJSON_IsNumber(rotation_cj)) {
                    rotation = rotation_cj->valueint;
                }

                cJSON* height_cj = cJSON_GetObjectItem(json, "height");
                if (cJSON_IsNumber(height_cj)) {
                    height = height_cj->valueint;
                }

                cJSON* height_mode_cj = cJSON_GetObjectItem(json, "height_mode");
                if (cJSON_IsNumber(height_mode_cj)) {
                    height_mode = height_mode_cj->valueint;
                }

                cJSON* speed_cj = cJSON_GetObjectItem(json, "speed");
                if (cJSON_IsNumber(speed_cj)) {
                    speed = speed_cj->valueint;
                }

                cJSON_Delete(json);
                ESP_LOGI(TAG, "Got rotation: %i height: %i speed: %i", rotation, height, speed);

                if (speed != INT_MAX) {
                    // TODO: Set speed
                }

                if (rotation != INT_MAX) {
                    // TODO: Set stepper rotation
                }

                // If height != INT_MAX, then we should set the height
                if (height != INT_MAX) {
                    if (height_mode == 1 || height_mode == INT_MAX)
                        servo_set_rotation_absolute(height);
                    else if (height_mode == 2)
                        servo_set_rotation_relative(height);
                }
            }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}


/**
 * @brief Handler for wifi events. Used to toggle the wifi_connected flag to true
 * 
 * @param arg 
 * @param event_base 
 * @param event_id 
 * @param event_data 
 */
void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        ESP_LOGI(TAG, "WiFi got IP: %s", ip4addr_ntoa(&event->ip_info.ip));
        wifi_connected = true;
    }
}

/**
 * @brief Initialize MQTT and connect to broker
 * 
 * @return esp_err_t 
 */
esp_err_t init_mqtt() {
    esp_err_t err;

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
        .username = CONFIG_BROKER_USER,
        .password = CONFIG_BROKER_PASSWORD
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == NULL) {
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "MQTT Client Initialized");

    err = esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, &mqtt_event_handler, NULL);
    if (err != ESP_OK) {
        return err;
    }
    ESP_LOGI(TAG, "MQTT Events Registered");

    return esp_mqtt_client_start(client);
}

/**
 * @brief Initialize wifi and connect to access point
 * 
 * @return esp_err_t 
 */
esp_err_t init_wifi() {
    esp_err_t err;

    esp_netif_init();
    err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        return err;
    }

    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

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

    err = esp_wifi_connect();
    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

/**
 * @brief Main initializer function. Called the sub-init functions
 * 
 */
void init() {
    esp_err_t err;

    err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS. Error: %s", esp_err_to_name(err));
        exit(1);
    }

    err = init_wifi();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize wifi. Error: %s", esp_err_to_name(err));
        exit(1);
    }

    err = init_servo();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize servo. Error: %s", esp_err_to_name(err));
        exit(1);
    }

    err = init_stepper();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize stepper. Error: %s", esp_err_to_name(err));
        exit(1);
    }

    // Wait until WiFi is connected before trying to connect to mqtt broker
    uint8_t counter = 0;
    while (!wifi_connected) {
        if (counter >= 10) {
            esp_restart();
        }
        ESP_LOGI(TAG, "Waiting for WiFi to connect...");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        counter++;
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

    while (true) {
        // TODO: Setup state machines!
    }

}