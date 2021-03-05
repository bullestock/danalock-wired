#pragma once

#include "encoder.h"
#include "led.h"
#include "motor.h"

#define VERSION "0.1"

constexpr const int LED_DEFAULT_PERIOD = 10;
constexpr const int LED_DEFAULT_DUTY_CYCLE_NUM = 1;
constexpr const int LED_DEFAULT_DUTY_CYCLE_DEN = 1000;
constexpr const int MAX_ENGAGE_TIME = 2500; // ms

/// Motor power for automatic calibration - reduced to avoid mechanical damage
constexpr const int CALIBRATE_POWER = 250;

/// Number of ticks to back off after hitting limit
constexpr const int BACKOFF_TICKS = 800;

/// Keys for NVS (keep short)
constexpr const char* DEFAULT_POWER_KEY =     "default_pwr";

extern Motor* motor;
extern Encoder encoder;
extern Led led;
extern int motor_power;
extern int locked_position;
extern bool locked_position_set;
extern int unlocked_position;
extern bool unlocked_position_set;

/// Gets updated by encoder_task()
extern std::atomic<int> encoder_position;
