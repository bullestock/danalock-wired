#pragma once

#include <atomic>
#include <limits>

#include "driver/gpio.h"

class Encoder
{
public:
    Encoder(gpio_num_t pin1, gpio_num_t pin2,
            int lower_bound = std::numeric_limits<int>::min(),
            int upper_bound = std::numeric_limits<int>::max(),
            int initial_pos = 0);

    // 50 steps per revolution
    int getPosition() const;
    void resetPosition(int p = 0);

    enum Direction
    {
        LEFT,
        RIGHT
    };
    
    Direction getDirection() const;

    void loop();

protected:
    gpio_num_t pin1 = (gpio_num_t) 0;
    gpio_num_t pin2 = (gpio_num_t) 0;
    int position = 0;
    int last_position = 0;
    int lower_bound = 0;
    int upper_bound = 0;
    unsigned long last_read_ms = 0;
    Direction direction = LEFT;
    int state = 0;
};
