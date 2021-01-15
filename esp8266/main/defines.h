#pragma once

#include "encoder.h"
#include "led.h"
#include "motor.h"

#define VERSION "0.1"

extern Motor* motor;
extern Encoder encoder;
extern Led led;

extern std::atomic<int> encoder_position;
extern std::atomic<int> led_duty_cycle;
