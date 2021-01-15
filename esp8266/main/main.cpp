#include "encoder.h"
#include "led.h"
#include "motor.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"

const auto AIN1 = (gpio_num_t) 5; // D1
const auto AIN2 = (gpio_num_t) 4; // D2
const auto PWMA = (gpio_num_t) 0; // D3
const auto STBY = (gpio_num_t) 2; // D4
const auto ENC_A = (gpio_num_t) 13; // D7
const auto ENC_B = (gpio_num_t) 12; // D6 - on Wemos D1 mini, D8 has a physical pulldown
const auto LED = (gpio_num_t) 14; // D5

extern "C" void console_task(void*);
extern "C" void encoder_task(void*);

Motor motor(AIN1, AIN2, PWMA, STBY);
Encoder encoder(ENC_A, ENC_B);

extern "C" void app_main()
{
    // We don't need wifi
    ESP_ERROR_CHECK(esp_wifi_deinit());

    xTaskCreate(console_task, "console_task", 4*1024, NULL, 5, NULL);
    xTaskCreate(encoder_task, "encoder_task", 4*1024, NULL, 5, NULL);
    
#if 0
    int speed = 100;
    int period = 3000;
    for (int i = 10; i >= 0; i--)
    {
        printf("Forwards %d\n", speed);
        motor1.drive(speed);
        vTaskDelay(period / portTICK_PERIOD_MS);
        printf("Brake\n");
        motor1.brake();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Backwards %d\n", speed);
        motor1.drive(-speed);
        vTaskDelay(period / portTICK_PERIOD_MS);
        printf("Brake\n");
        motor1.brake();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        speed += 100;
    }
#endif

    Led led(LED);
    led.set_period(10);
    led.set_duty_cycle(1);
    
    int n = 0;
    int cycle = 1;
    while (1)
    {
        encoder.loop();
        led.update();
        if (++n > 100000)
        {
            n = 0;
            //printf("pos %d\n", encoder.getPosition());
            vTaskDelay(10 / portTICK_PERIOD_MS);
            ++cycle;
            led.set_duty_cycle(cycle);
        }
    }
}
