#include "motor.h"
#include "defines.h"

#include "driver/pwm.h"

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

    // Configure PWM pin
    
    const uint32_t pwm_pin_num[1] = {
        PWM
    };
    uint32_t pwm_duty[1] = {
        500
    };
    float pwm_phase[1] = {
        0
    };

    ESP_ERROR_CHECK(pwm_init(1000, pwm_duty, 1, pwm_pin_num));
    ESP_ERROR_CHECK(pwm_set_phases(pwm_phase));
    ESP_ERROR_CHECK(pwm_start());
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
        ms = BACKOFF_MS/2;

    return ms;
}

int Motor::get_rotation_timeout_ms(int pwr) const
{
    int ms = 275;

    int abs_pwr = abs(pwr);
    if (abs_pwr < 400)
        ms = 400;

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
    ESP_ERROR_CHECK(gpio_set_level(In1, 0));
    ESP_ERROR_CHECK(gpio_set_level(In2, 1));
    ESP_ERROR_CHECK(pwm_set_duty(0, speed));
    ESP_ERROR_CHECK(pwm_start());
}

void Motor::rev(int speed)
{
    ESP_ERROR_CHECK(gpio_set_level(In1, 1));
    ESP_ERROR_CHECK(gpio_set_level(In2, 0));
    ESP_ERROR_CHECK(pwm_set_duty(0, speed));
    ESP_ERROR_CHECK(pwm_start());
}

void Motor::brake()
{
    ESP_ERROR_CHECK(gpio_set_level(In1, 1));
    ESP_ERROR_CHECK(gpio_set_level(In2, 1));
    ESP_ERROR_CHECK(pwm_set_duty(0, 0));
    ESP_ERROR_CHECK(pwm_start());
}

void Motor::standby()
{
    ESP_ERROR_CHECK(gpio_set_level(Standby, 0));
}
