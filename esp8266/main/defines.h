#pragma once

#include "encoder.h"
#include "led.h"
#include "motor.h"

#define VERSION "0.3"

constexpr const auto AIN1 = (gpio_num_t) 5; // D1
constexpr const auto AIN2 = (gpio_num_t) 4; // D2
constexpr const auto PWMA = (gpio_num_t) 0; // D3 - 12K pullup
constexpr const auto STBY = (gpio_num_t) 2; // D4 - 12K pullup + LED
constexpr const auto ENC_A = (gpio_num_t) 13; // D7
constexpr const auto ENC_B = (gpio_num_t) 12; // D6
constexpr const auto LED = (gpio_num_t) 14; // D5
constexpr const auto DOOR_SW = (gpio_num_t) 16; // D0
constexpr const auto HANDLE_SW = (gpio_num_t) 15; // D8 - 12K pulldown

constexpr const int LED_DEFAULT_PERIOD = 10;
constexpr const int LED_DEFAULT_DUTY_CYCLE_NUM = 1;
constexpr const int LED_DEFAULT_DUTY_CYCLE_DEN = 100;

/// Motor power for automatic calibration - reduced to avoid mechanical damage
constexpr const int MOTOR_CALIBRATE_POWER = 250;

/// Motor power for normal operation
constexpr const int MOTOR_DEFAULT_POWER = 300; // reduced for testing

constexpr const int DEFAULT_BACKOFF_PULSES = 10;

/// Number of ms to back off after hitting limit
constexpr const int BACKOFF_MS = 1200;

/// Keys for NVS (keep short)
constexpr const char* DEFAULT_POWER_KEY =     "default_pwr";
constexpr const char* BACKOFF_PULSES_KEY =    "backoff_ps";

extern Motor* motor;
extern Encoder encoder;
extern Led led;
extern int default_motor_power;
extern int backoff_pulses;

/// Gets updated by encoder_task()
extern std::atomic<int> encoder_position;

/// Set this to make current position zero
extern std::atomic<bool> reset_encoder;
