#pragma once

#include "encoder.h"
#include "led.h"
#include "motor.h"

#define VERSION "0.3"

constexpr const int LED_DEFAULT_PERIOD = 10;
constexpr const int LED_DEFAULT_DUTY_CYCLE_NUM = 1;
constexpr const int LED_DEFAULT_DUTY_CYCLE_DEN = 1000;

/// Motor power for automatic calibration - reduced to avoid mechanical damage
constexpr const int MOTOR_CALIBRATE_POWER = 250;

/// Motor power for normal operation
constexpr const int MOTOR_DEFAULT_POWER = 300; // reduced for testing

/// Number of ms to back off after hitting limit
constexpr const int BACKOFF_MS = 1200;

/// Keys for NVS (keep short)
constexpr const char* DEFAULT_POWER_KEY =     "default_pwr";

extern Motor* motor;
extern Encoder encoder;
extern Led led;
extern int default_motor_power;

/// Gets updated by encoder_task()
extern std::atomic<int> encoder_position;

/// Set this to make current position zero
extern std::atomic<bool> reset_encoder;
