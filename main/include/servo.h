#ifndef SERVO__H
#define SERVO__H

#include "stdint.h"
#include "esp_err.h"
#include "driver/gpio.h"

// Servo Information. The numbers are mainly reference. For my stepper 0.7ms is full down.
// 1ms pulse => Full Up
// 1.5ms pulse => Center
// 2ms pulse => Full Down
#define PWM_PERIOD 20000 // 20000 us => 50 Hz => Servo PWM period
#define PWM_MIN 700
#define PWM_CENTER 1500
#define PWM_MAX 2300

esp_err_t init_servo();
esp_err_t servo_set_rotation_absolute(uint8_t percent);
esp_err_t servo_set_rotation_relative(int8_t amount);

#endif