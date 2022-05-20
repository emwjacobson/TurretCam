#include "servo.h"
#include "pwm.h"
#include "esp_log.h"
#include "util.h"

esp_err_t init_servo() {
    esp_err_t err;
    err = pwm_init(PWM_PERIOD, duties, 1, PWM_PIN);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error initializing PWM.");
        return err;
    }

    err = pwm_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error starting PWM.");
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
esp_err_t servo_set_rotation(uint8_t percent) {
    if (percent > 100) percent = 100;

    // Convert percent (0 to 100) to PWM range (1000 to 2000)
    uint32_t duty = map(percent, 0, 100, PWM_MIN, PWM_MAX);

    // TODO: This check is probably redundent
    if (duty > PWM_PERIOD) {
        duty = PWM_MAX;
    } else if (duty < PWM_MIN) {
        duty = PWM_MIN;
    }
    
    esp_err_t err;

    // TODO: Test if channel_num is PWM_PIN, or if it is a (1 or 0)
    //       indexed of number of channel from pwm_init
    err = pwm_set_duty(0, duty);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Error setting PWM duty.");
        return err;
    }

    err = pwm_start();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Error starting PWM.");
        return err;
    }

    return ESP_OK;
}
