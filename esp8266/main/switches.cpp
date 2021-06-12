#include "switches.h"
#include "defines.h"

#include "driver/gpio.h"

void init_switches()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    // bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ULL << DOOR_SW) | (1ULL << HANDLE_SW);
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}

bool is_door_closed()
{
    return gpio_get_level(DOOR_SW);
}

bool is_handle_raised()
{
    return gpio_get_level(HANDLE_SW);
}
