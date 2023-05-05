#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <driver/pcnt.h>

class Encoder
{
public:
    // 50 steps per revolution
    static constexpr int STEPS_PER_REVOLUTION = 50;
    
    Encoder(pcnt_unit_t unit,
            int gpio1, int gpio2);

    int64_t poll();

    void set_zero();
    
private:
    struct pcnt_evt_t
    {
        Encoder* enc = 0;
        uint32_t status = 0;
    };

    static void IRAM_ATTR quad_enc_isr(void*);

    pcnt_unit_t unit = (pcnt_unit_t) 0;
    
    // A queue to handle pulse counter events
    static QueueHandle_t pcnt_evt_queue;

    int64_t accumulated = 0;
};

extern Encoder encoder;
