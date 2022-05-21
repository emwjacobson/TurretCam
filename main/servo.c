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
esp_err_t servo_set_rotation(uint8_t percent) {
    if (percent > 100) percent = 100;

    // Convert percent (0 to 100) to PWM range (1000 to 2000)
    uint32_t duty = map(percent, 100, 0, PWM_MIN, PWM_MAX);

    ESP_LOGI(TAG_SERVO, "Setting servo. Percent %i = %i duty", percent, duty);

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
