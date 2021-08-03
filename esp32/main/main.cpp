#include "defines.h"
#include "encoder.h"
#include "led.h"
#include "motor.h"
#include "switches.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

extern "C" void console_task(void*);
extern "C" void encoder_task(void*);

Encoder encoder(ENC_A, ENC_B);
Led led(LED);
Motor* motor = nullptr;
Switches switches;

int default_motor_power = MOTOR_DEFAULT_POWER;
int backoff_pulses = DEFAULT_BACKOFF_PULSES;

extern "C" void app_main()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    default_motor_power = MOTOR_DEFAULT_POWER;
    backoff_pulses = DEFAULT_BACKOFF_PULSES;
    int32_t val = 0;
    err = nvs_get_i32(my_handle, DEFAULT_POWER_KEY, &val);
    switch (err)
    {
    case ESP_OK:
        default_motor_power = val;
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        break;
    default:
        printf("%s: NVS error %d\n", DEFAULT_POWER_KEY, err);
        break;
    }
    err = nvs_get_i32(my_handle, BACKOFF_PULSES_KEY, &val);
    switch (err)
    {
    case ESP_OK:
        backoff_pulses = val;
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        break;
    default:
        printf("%s: NVS error %d\n", DEFAULT_POWER_KEY, err);
        break;
    }
    nvs_close(my_handle);
    
    motor = new Motor(AIN1, AIN2, PWMA, STBY);

    // Not calibrated yet
    led.set_params(80, 100, 10);

    printf("Danalock " VERSION " ready, default power: %d, backoff: %d\n",
           default_motor_power, backoff_pulses);
    
    xTaskCreate(console_task, "console_task", 4*1024, NULL, 5, NULL);
    xTaskCreate(encoder_task, "encoder_task", 4*1024, NULL, 5, NULL);
}
