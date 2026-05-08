#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define ADC_ATTEN ADC_ATTEN_DB_12

typedef struct {
    adc_cali_handle_t cali_chan_handle;
    adc_channel_t adc_chan;
    int gpio_num;
    bool curve_fitting;
    int power_gpio;
} adc_info_t;

void adc_init(adc_info_t *_channels, int _chan_num);
void adc_init_num(int *_channels, int _chan_num);
// returns val from range 0-3300 mV
float read_val(int gpio_num);
void adc_deinit();