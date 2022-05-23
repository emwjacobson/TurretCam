#include "stepper.h"

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char* TAG_STEPPER = "Servo";

void stepper_task(void * pvParameters) {
    while(1) {
        // TODO: Check for item in queue
        //       If item in queue, process that # of steps
    }
}

esp_err_t init_stepper() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1<<STEPPER_PIN_STEP) | (1<<STEPPER_PIN_DIR) | (1<<STEPPER_PIN_ENABLE),
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };

    esp_err_t err;

    err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_STEPPER, "Error setting gpio config.");
        return err;
    }

    err = gpio_set_level(STEPPER_PIN_STEP, 0);
    err = gpio_set_level(STEPPER_PIN_DIR, STEPPER_DIR_CW);
    err = gpio_set_level(STEPPER_PIN_ENABLE, 0); // 0 enables stepper

    BaseType_t ret = xTaskCreate(stepper_task, "Stepper Task", 1024, NULL, tskIDLE_PRIORITY, NULL);
    if (ret != pdPASS) {
        ESP_LOGW(TAG_STEPPER, "Error starting stepper task");
    }

    // TODO: Create queue

    return ESP_OK;
}

/**
 * @brief Sets rotation direction of servo.
 * 
 * @param dir 0 = counter clockwise, 1 = clockwise
 * @return esp_err_t 
 */
esp_err_t stepper_set_direction(uint8_t dir) {
    esp_err_t err;

    err = gpio_set_level(STEPPER_PIN_DIR, (dir & 0x01));
    if (err != ESP_OK) {
        ESP_LOGW(TAG_STEPPER, "Error setting stepper direction");
        return err;
    }

    return ESP_OK;
}

/**
 * @brief Enable or disable the stepper to save power.
 * 
 * @param enable 0 = disabled, 1 = enabled
 * @return esp_err_t 
 */
esp_err_t stepper_set_enabled(uint8_t enable) {
    esp_err_t err;

    // Because stepper driver is ~ENABLE, need to invert
    err = gpio_set_level(STEPPER_PIN_ENABLE, ~(enable & 0x01));
    if (err != ESP_OK) {
        ESP_LOGW(TAG_STEPPER, "Error setting stepper enable/disable");
        return err;
    }

    return ESP_OK;
}

/**
 * @brief Steps the stepper 1 step.
 * 10ms between steps is fastest speed
 * 
 * @return esp_err_t 
 */
esp_err_t stepper_step() {
    esp_err_t err;

    err = gpio_set_level(STEPPER_PIN_STEP, 1);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_STEPPER, "Error stepper to 1");
        return err;
    }
    err = gpio_set_level(STEPPER_PIN_STEP, 0);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_STEPPER, "Error stepper to 0");
        return err;
    }

    return ESP_OK;
}

esp_err_t stepper_make_move(int8_t amount) {
    // TODO: Add item to queue
}
