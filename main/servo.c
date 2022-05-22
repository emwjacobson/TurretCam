#include "servo.h"
#include "driver/pwm.h"
#include "esp_log.h"
#include "util.h"

static const char* TAG_SERVO = "Servo";

uint32_t pwm_duties = {
    PWM_CENTER
};

uint32_t pwm_pins = {
    5
};

/**
 * @brief Initializes the servo, sets to center position.
 * 
 * @return esp_err_t 
 */
esp_err_t init_servo() {
    ESP_LOGI(TAG_SERVO, "Initializing Servo");
    esp_err_t err;
    err = pwm_init(PWM_PERIOD, &pwm_duties, 1, &pwm_pins);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_SERVO, "Error initializing PWM.");
        return err;
    }

    err = pwm_set_phase(0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_SERVO, "Error setting PWM phase.");
        return err;
    }

    err = pwm_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG_SERVO, "Error starting PWM.");
        return err;
    }

    return ESP_OK;
}

/**
 * @brief Sets the servo rotation
 * 
 * @param percent Range from 0 to 100. 0 being full down, 100 being full up
 * @return esp_err_t 
 */
esp_err_t servo_set_rotation_absolute(uint8_t percent) {
    if (percent > 100) percent = 100;

    // Convert percent (0 to 100) to PWM range (1000 to 2000)
    uint32_t duty = map(percent, 100, 0, PWM_MIN, PWM_MAX);

    ESP_LOGI(TAG_SERVO, "Setting servo. Percent %i = %i duty", percent, duty);

    if (duty > PWM_PERIOD) {
        duty = PWM_MAX;
    } else if (duty < PWM_MIN) {
        duty = PWM_MIN;
    }
    
    esp_err_t err;
    err = pwm_set_duty(0, duty);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_SERVO, "Error setting PWM duty.");
        return err;
    }

    err = pwm_start();
    if (err != ESP_OK) {
        ESP_LOGW(TAG_SERVO, "Error starting PWM.");
        return err;
    }

    return ESP_OK;
}

/**
 * @brief Rotates the servo by a set amount, in percentage.
 * 
 * @param amount Percent to be rotated
 * @return esp_err_t 
 */
esp_err_t servo_set_rotation_relative(int8_t amount) {
    uint32_t duty;
    esp_err_t err;

    err = pwm_get_duty(0, &duty);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_SERVO, "Error getting current PWM duty.");
        return err;
    }

    // At this point duty is the PWM duty, needs to be converted from 0 to 100
    uint8_t percent = map(duty, PWM_MIN, PWM_MAX, 100, 0);
    percent += amount;
    duty = map(percent, 100, 0, PWM_MIN, PWM_MAX);

    if (duty > PWM_MAX)
        duty = PWM_MAX;
    else if (duty < PWM_MIN)
        duty = PWM_MIN;

    err = pwm_set_duty(0, duty);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_SERVO, "Error setting relative PWM duty.");
        return err;
    }
    
    err = pwm_start();
    if (err != ESP_OK) {
        ESP_LOGW(TAG_SERVO, "Error starting PWM.");
        return err;
    }

    return ESP_OK;
}
