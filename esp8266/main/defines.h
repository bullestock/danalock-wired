#pragma once

#include "encoder.h"
#include "led.h"
#include "motor.h"

#define VERSION "0.1"

constexpr const int LED_DEFAULT_PERIOD = 10;
constexpr const int LED_DEFAULT_DUTY_CYCLE = 1;

/// Motor power for automatic calibration - reduced to avoid mechanical damage
constexpr const int CALIBRATE_POWER = 300;

extern Motor* motor;
extern Encoder encoder;
extern Led led;

extern std::atomic<int> encoder_position;
extern std::atomic<int> led_duty_cycle;
extern std::atomic<int> led_period;
