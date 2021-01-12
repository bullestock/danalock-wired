#pragma once

#include "driver/gpio.h"

class Motor
{
public:
    Motor(gpio_num_t In1pin, gpio_num_t In2pin, gpio_num_t PWMpin, gpio_num_t STBYpin);      

    // Drive in direction given by sign, at speed given by magnitude of the parameter (0-1000).
    void drive(int speed);  
    
    // Stop motor by setting both input pins high
    void brake(); 
    
    // Set the chip to standby mode.
    void standby(); 
    
private:
    gpio_num_t In1 = (gpio_num_t) 0;
    gpio_num_t In2 = (gpio_num_t) 0;
    gpio_num_t PWM = (gpio_num_t) 0;
    gpio_num_t Standby = (gpio_num_t) 0;
    
    void fwd(int speed);
    void rev(int speed);
};
