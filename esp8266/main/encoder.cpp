#include "defines.h"
#include "switches.h"

#include <cmath>
#include <stdio.h>

#include "FreeRTOS.h"
#include "freertos/task.h"

Encoder::Encoder(gpio_num_t _pin1, gpio_num_t _pin2, int _lower_bound, int _upper_bound, int initial_pos)
    : pin1(_pin1),
      pin2(_pin2),
      lower_bound(_lower_bound < _upper_bound ? _lower_bound : _upper_bound),
      upper_bound(_lower_bound < _upper_bound ? _upper_bound: _lower_bound)
{
    // Configure GPIO pins
    
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    // bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ULL << _pin1) | (1ULL << _pin2);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    loop();
    resetPosition(initial_pos);
    last_read_ms = 0;
}

void Encoder::resetPosition(int p)
{
    if (p > upper_bound)
        last_position = upper_bound;
    else
        last_position = (lower_bound > p) ? lower_bound : p;

    if (position != last_position)
        position = last_position;

    direction = LEFT;
}

Encoder::Direction Encoder::getDirection() const
{
    return direction;
}

int Encoder::getPosition() const
{
    return position;
}

void Encoder::loop()
{
    const auto pin1_state = !gpio_get_level(pin1);
    const auto pin2_state = !gpio_get_level(pin2);

    int s = state & 3;
    if (pin1_state)
        s |= 4;
    if (pin2_state)
        s |= 8;

    switch (s) {
    case 0: case 5: case 10: case 15:
        break;
    case 1: case 7: case 8: case 14:
        position++; break;
    case 2: case 4: case 11: case 13:
        position--; break;
    case 3: case 12:
        position += 2; break;
    default:
        position -= 2; break;
    }
    state = (s >> 2);

    if (getPosition() >= lower_bound && getPosition() <= upper_bound)
    {
        if (position != last_position)
        {
            if (std::abs(position - last_position) >= 1)
            {
                if (position > last_position)
                    direction = RIGHT;
                else
                    direction = LEFT;
                last_position = position;
            }
        }
    }
    else
        position = last_position;
}

std::atomic<int> encoder_position{0};
std::atomic<bool> reset_encoder{false};

extern "C" void encoder_task(void*)
{
    while (1)
    {
        if (reset_encoder.exchange(false))
            encoder.resetPosition();
        encoder.loop();
        encoder_position = encoder.getPosition();
        led.update();
        switches.update();
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
