#ifndef STEPPER_H
#define STEPPER_H

#include <stdint.h>
#include "esp_err.h"

#define STEPPER_PIN_STEP 12
#define STEPPER_PIN_DIR 13
#define STEPPER_PIN_ENABLE 4
#define STEPPER_DIR_CW 0
#define STEPPER_DIR_CCW 1
#define STEPPER_DELAY_MIN 100
#define STEPPER_DELAY_MAX 10

esp_err_t init_stepper();
esp_err_t stepper_set_direction(uint8_t dir);
esp_err_t stepper_set_enabled(uint8_t enable);
esp_err_t stepper_make_move(int8_t amount);
esp_err_t stepper_set_speed(int8_t speed);

#endif