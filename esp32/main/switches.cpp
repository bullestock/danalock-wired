#include "switches.h"
#include "defines.h"

#include "driver/gpio.h"

constexpr int NOF_READS = 10;

Switches::Switches()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    // bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ULL << DOOR_SW) | (1ULL << HANDLE_SW);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}

bool Switches::is_door_closed() const
{
    for (int i = 0; i < NOF_READS; ++i)
    {
        if (gpio_get_level(DOOR_SW))
            return false;
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    return true;
}

bool Switches::is_handle_raised() const
{
    for (int i = 0; i < NOF_READS; ++i)
    {
        if (gpio_get_level(HANDLE_SW))
            return false;
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    return true;
}

void Switches::set_door_locked()
{
    m_door_locked.store(true);
}

bool Switches::was_door_open() const
{
    return !m_door_locked.load();
}

void Switches::update()
{
    if (!is_door_closed())
    {
        // Remember that the door was opened
        m_door_locked.store(false);
        return;
    }
}

extern "C" void switch_task(void*)
{
    while (1)
    {
        led.update();
        switches.update();
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
