#include "defines.h"
#include "encoder.h"
#include "led.h"
#include "motor.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

const auto AIN1 = (gpio_num_t) 5; // D1
const auto AIN2 = (gpio_num_t) 4; // D2
const auto PWMA = (gpio_num_t) 0; // D3
const auto STBY = (gpio_num_t) 2; // D4
const auto ENC_A = (gpio_num_t) 13; // D7
const auto ENC_B = (gpio_num_t) 12; // D6 - on Wemos D1 mini, D8 has a physical pulldown
const auto LED = (gpio_num_t) 14; // D5

extern "C" void console_task(void*);
extern "C" void encoder_task(void*);

Encoder encoder(ENC_A, ENC_B);
Led led(LED);
Motor* motor = nullptr;

int motor_power = 500;
int locked_position = 0;
bool locked_position_set = false;
int unlocked_position = 0;
bool unlocked_position_set = false;

extern "C" void app_main()
{
    // We don't need wifi
    ESP_ERROR_CHECK(esp_wifi_deinit());

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    int32_t val = 0;
    err = nvs_get_i32(my_handle, LOCKED_POSITION_KEY, &val);
    switch (err)
    {
    case ESP_OK:
        locked_position = val;
        locked_position_set = true;
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        break;
    default:
        printf("%s: NVS error %d\n", LOCKED_POSITION_KEY, err);
        break;
    }
    err = nvs_get_i32(my_handle, UNLOCKED_POSITION_KEY, &val);
    switch (err)
    {
    case ESP_OK:
        unlocked_position = val;
        unlocked_position_set = true;
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        break;
    default:
        printf("%s: NVS error %d\n", UNLOCKED_POSITION_KEY, err);
        break;
    }
    bool default_power_set = false;
    err = nvs_get_i32(my_handle, DEFAULT_POWER_KEY, &val);
    switch (err)
    {
    case ESP_OK:
        motor_power = val;
        default_power_set = true;
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        break;
    default:
        printf("%s: NVS error %d\n", DEFAULT_POWER_KEY, err);
        break;
    }
    nvs_close(my_handle);
    
    motor = new Motor(AIN1, AIN2, PWMA, STBY);
    
    led.set_params(LED_DEFAULT_DUTY_CYCLE_NUM,
                   LED_DEFAULT_DUTY_CYCLE_DEN,
                   LED_DEFAULT_PERIOD);

    printf("Danalock " VERSION " ready");
    if (locked_position_set)
        printf(" locked: %d", locked_position);
    if (unlocked_position_set)
        printf(" unlocked: %d", unlocked_position);
    if (default_power_set)
        printf(" power: %d", motor_power);
    printf("\n");
    
    xTaskCreate(console_task, "console_task", 4*1024, NULL, 5, NULL);
    xTaskCreate(encoder_task, "encoder_task", 4*1024, NULL, 5, NULL);
}
