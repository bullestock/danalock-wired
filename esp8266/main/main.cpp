#include "motor.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

const auto AIN1 = (gpio_num_t) 5; // D1
const auto AIN2 = (gpio_num_t) 4; // D2
const auto PWMA = (gpio_num_t) 0; // D3
const auto STBY = (gpio_num_t) 2; // D4

extern "C" void app_main()
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP8266 chip with %d CPU cores, WiFi, ",
            chip_info.cores);

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    Motor motor1(AIN1, AIN2, PWMA, STBY);

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
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
