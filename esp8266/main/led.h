#pragma once

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "semphr.h"

class Led
{
public:
    Led(gpio_num_t _pin);

    void update();

    void set_params(int num, int den, int period_ms);

private:
    gpio_num_t pin = (gpio_num_t) 0;
    int period = 1;
    int duty_cycle_num = 1;
    int duty_cycle_den = 100;
    unsigned long last_tick = 0;
    int cycle = 0;
    SemaphoreHandle_t mutex_handle = (SemaphoreHandle_t) 0;
};
