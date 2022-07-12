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
    const auto d1 = !gpio_get_level(DOOR_SW);
    vTaskDelay(30 / portTICK_PERIOD_MS);
    const auto d2 = !gpio_get_level(DOOR_SW);
    vTaskDelay(30 / portTICK_PERIOD_MS);
    const auto d3 = !gpio_get_level(DOOR_SW);
    return d1 && d2 && d3;
}

bool Switches::is_handle_raised() const
{
    const auto h1 = !gpio_get_level(HANDLE_SW);
    vTaskDelay(30 / portTICK_PERIOD_MS);
    const auto h2 = !gpio_get_level(HANDLE_SW);
    vTaskDelay(30 / portTICK_PERIOD_MS);
    const auto h3 = !gpio_get_level(HANDLE_SW);
    return h1 && h2 && h3;
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
        vTaskDelay(30 / portTICK_PERIOD_MS);
        if (is_door_closed())
            return; // Must be noise
        vTaskDelay(30 / portTICK_PERIOD_MS);
        if (is_door_closed())
            return; // Must be noise
        vTaskDelay(30 / portTICK_PERIOD_MS);
        if (is_door_closed())
            return; // Must be noise
        // Remember that the door was opened
        m_door_locked.store(false);
        return;
    }
}
