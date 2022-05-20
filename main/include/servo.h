#ifndef SERVO_H
#define SERVO_H

#include "stdint.h"
#include "esp_err.h"

// Servo Information
// 1ms pulse => Full Up
// 1.5ms pulse => Center
// 2ms pulse => Full Down
#define PWM_PERIOD 20000 // 20000 us => 50 Hz => Servo PWM period
#define PWM_MIN 1000
#define PWM_CENTER 1500
#define PWM_MAX 2000
#define PWM_PIN 99 // TODO: Set this pin number

static const char* TAG = "Main";

static const uint32_t duties = {
    PWM_CENTER / 2
};

esp_err_t init_servo();
esp_err_t set_duty(uint32_t duty);

#endif