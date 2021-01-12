#include "led.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

Led::Led(gpio_num_t _pin)
    : pin(_pin)
{
    gpio_config_t io_conf;
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ULL << pin);
    // disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    // configure GPIO with the given settings
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}

void Led::update()
{
    const auto now = xTaskGetTickCount()*portTICK_PERIOD_MS;
    const auto elapsed = now - last_tick;
    if (elapsed < period)
        return;
    last_tick = now;
    if (cycle >= 100)
        cycle = 0;
    const bool is_on = (cycle <= duty_cycle);
    ++cycle;
    ESP_ERROR_CHECK(gpio_set_level(pin, is_on));
}
