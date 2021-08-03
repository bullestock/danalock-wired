#include "switches.h"
#include "defines.h"

#include "driver/gpio.h"

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
    return !gpio_get_level(DOOR_SW);
}

bool Switches::is_handle_raised() const
{
    return m_handle_raised.load();
}

// The handle state changes to 'raised' if
// we have observed that the handle switch was active AND the door switch was active.
// If the door switch is inactive at any point after this, the handle state changes to 'not raised'.
void Switches::update()
{
    if (!is_door_closed())
    {
        // Door is open, handle cannot be raised
        m_handle_raised.store(false);
        return;
    }
    if (!gpio_get_level(HANDLE_SW))
    {
        // Door is closed, and handle switch is active. Handle is now raised.
        m_handle_raised.store(true);
    }
}
