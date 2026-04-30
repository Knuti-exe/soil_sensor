#include "adc_oneshot.h"

// #TODO zmienic piny, gdy konieczne

#define GPIO_SENSOR 4
#define GPIO_BAT 3

static const char *tag = "main";

void app_main(void) 
{
    // samemu trzeba pilnowac ktory element tablicy, to ktory ADC kanał...
    int chan_num = 2;
    adc_info_t *channels = pvPortMalloc(chan_num * sizeof(adc_info_t));
    
    channels[0].gpio_num = GPIO_BAT;
    channels[1].gpio_num = GPIO_SENSOR;
    
    adc_init(channels, chan_num);
    free(channels);

    float min, max; // #TODO
    float raw_val = read_val(GPIO_SENSOR);
    float val = val - min / (max - min); // ???

    ESP_LOGI(tag, "Read val: %.1f", raw_val);
    ESP_LOGI(tag, "Hum: %.1f", );


}

// Instead of a capacitor, I will use multisampling


/*
    Sensor measuring soil humidity at most 3x during the day (at night it stays hibernated).
    After successful measurement, it sends data via BLE to other smart device:
        - to light controller or window roller,
        - or broadcasting to the ether (any microcontroller can pick data).
    No matter who will get data, it should send it to MQTT broker.
    User will have notification via MQTT app.
    
    TODO
    - [ ] BLE

*/