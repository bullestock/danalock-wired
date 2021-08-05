#include "motor.h"
#include "defines.h"

#include "driver/ledc.h"

#include <cstdlib>

Motor::Motor(gpio_num_t In1pin, gpio_num_t In2pin, gpio_num_t PWMpin, gpio_num_t STBYpin)
    : In1(In1pin),
      In2(In2pin),
      PWM(PWMpin),
      Standby(STBYpin)
{
    // Configure GPIO pins
    
    gpio_config_t io_conf;
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ULL << In1) | (1ULL << In2) |  (1ULL << PWM) |  (1ULL << Standby);
    // disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    // configure GPIO with the given settings
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_10_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 1000,  // Set output frequency at 1 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = PWM,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

int Motor::get_max_engage_time_ms(int pwr) const
{
    int ms = 2500; // heuristically determined to be sufficient at power = 500

    int abs_pwr = abs(pwr);
    if (abs_pwr < 400)
        ms = 3000;

    return ms;
}

int Motor::get_backoff_time_ms(int pwr)
{
    int ms = BACKOFF_MS; // heuristically determined to be suitable at power = 300

    int abs_pwr = abs(pwr);
    if (abs_pwr > 400)
        ms = BACKOFF_MS*2;

    return ms;
}

int Motor::get_rotation_timeout_ms(int pwr) const
{
    int ms = 275;

    int abs_pwr = abs(pwr);
    if (abs_pwr < 400)
        ms = 600;

    return ms;
}

void Motor::drive(int speed)
{
    ESP_ERROR_CHECK(gpio_set_level(Standby, 1));
    if (speed >= 0)
        fwd(speed);
    else
        rev(-speed);
}

void Motor::fwd(int speed)
{
    ESP_ERROR_CHECK(gpio_set_level(In1, 1));
    ESP_ERROR_CHECK(gpio_set_level(In2, 0));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, speed));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}

void Motor::rev(int speed)
{
    ESP_ERROR_CHECK(gpio_set_level(In1, 0));
    ESP_ERROR_CHECK(gpio_set_level(In2, 1));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, speed));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}

void Motor::brake()
{
    ESP_ERROR_CHECK(gpio_set_level(In1, 1));
    ESP_ERROR_CHECK(gpio_set_level(In2, 1));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}

void Motor::standby()
{
    ESP_ERROR_CHECK(gpio_set_level(Standby, 0));
}
