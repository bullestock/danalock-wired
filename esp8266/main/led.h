#pragma once

#include "driver/gpio.h"

class Led
{
public:
    Led(gpio_num_t _pin);

    void update();

    Led& set_period(int ms)
    {
        period = ms;
        return *this;
    }

    // 1-100
    Led& set_duty_cycle(int dc)
    {
        duty_cycle = dc;
        return *this;
    }

private:
    gpio_num_t pin = (gpio_num_t) 0;
    int period = 1;
    int duty_cycle = 1;
    unsigned long last_tick = 0;
    int cycle = 0;
};
