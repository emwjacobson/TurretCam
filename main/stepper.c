#include "stepper.h"

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "util.h"

static const char* TAG_STEPPER = "Servo";

esp_err_t stepper_step();

typedef struct {
    int8_t amount;
} movement_event_t;

QueueHandle_t movement_queue;

static uint8_t _delay;

void stepper_task(void * pvParameters) {
    movement_event_t event;
    while(1) {
        // This will block until an item is in the queue
        xQueueReceive(movement_queue, &event, portMAX_DELAY);

        stepper_set_enabled(1); // Enable stepper
        
        // TODO: Do something with event.amount
        ESP_LOGI(TAG_STEPPER, "Movement processing %i", event.amount);

        // The sign of `event.amount` tells the direction, the magnitude is the number of steps
        if (event.amount < 0) {
            stepper_set_direction(STEPPER_DIR_CCW);
            event.amount *= -1;
        } else if (event.amount > 0) {
            stepper_set_direction(STEPPER_DIR_CW);
        }

        for(int i=0; i<event.amount; i++) {
            stepper_step();
            vTaskDelay(_delay / portTICK_PERIOD_MS);
        }

        stepper_set_enabled(0); // Disable stepper at end
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
    // err = gpio_set_level(STEPPER_PIN_ENABLE, 0); // Disables stepper
    stepper_set_enabled(0); // Disable stepper

    movement_queue = xQueueCreate(10, sizeof(movement_event_t));

    BaseType_t ret = xTaskCreate(stepper_task, "Stepper Task", 2048, NULL, tskIDLE_PRIORITY, NULL);
    if (ret != pdPASS) {
        ESP_LOGW(TAG_STEPPER, "Error starting stepper task");
    }

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
    
    // Because ENABLE is active low, need to invert
    err = gpio_set_level(STEPPER_PIN_ENABLE, (~enable & 0x01));
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
    ESP_LOGI(TAG_STEPPER, "Adding movement to queue %i", amount);
    movement_event_t event = {
        .amount = amount
    };
    xQueueSend(movement_queue, &event, 0);

    return ESP_OK;
}

/**
 * @brief Sets the speed that the stepper should run at.
 * 
 * @param speed Valid values are from 20-100
 * @return esp_err_t 
 */
esp_err_t stepper_set_speed(int8_t speed) {
    if (speed < 20) speed = 20;
    if (speed > 100) speed = 100;
    ESP_LOGI(TAG_STEPPER, "Setting stepper speed to %i", speed);

    // Maps from range 20-100 (speed values) to 100-10(delay values)
    _delay = map(speed, 20, 100, STEPPER_DELAY_MAX, STEPPER_DELAY_MIN);
    ESP_LOGI(TAG_STEPPER, "Speed mapped to delay %i", _delay);

    return ESP_OK;
}

esp_err_t stepper_set_center() {
    return ESP_OK;
}
