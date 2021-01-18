#pragma once

#include "encoder.h"
#include "led.h"
#include "motor.h"

#define VERSION "0.1"

constexpr const int LED_DEFAULT_PERIOD = 10;
constexpr const int LED_DEFAULT_DUTY_CYCLE_NUM = 1;
constexpr const int LED_DEFAULT_DUTY_CYCLE_DEN = 1000;

/// Motor power for automatic calibration - reduced to avoid mechanical damage
constexpr const int CALIBRATE_POWER = 250;

/// Number of ticks to back off after hitting limit
constexpr const int BACKOFF_TICKS = 500;

/// Keys for NVS
constexpr const char* LOCKED_POSITION_KEY = "lock_pos";
constexpr const char* UNLOCKED_POSITION_KEY = "unlock_pos";

extern Motor* motor;
extern Encoder encoder;
extern Led led;

/// Gets updated by encoder_task()
extern std::atomic<int> encoder_position;
