#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

//              #TODO zmienic kanał
#define ADC_CHAN1 ADC_CHANNEL_4
#define ADC_CHAN2 ADC_CHANNEL_5
// dla es32c3, kanał adc == gpio_num (np. channel_3 -> gpio_num_3). kanał 5 to ADC2 -> konflikty z radiem
#define ADC_ATTEN ADC_ATTEN_DB_12

typedef struct {
    adc_cali_handle_t cali_chan_handle;
    adc_channel_t adc_chan;
    int gpio_num;
    bool curve_fitting;
    bool line_fitting;
} adc_info_t;

void adc_init(adc_info_t *_channels, int _chan_num);
void adc_init_num(int *_channels, int _chan_num);
// returns val from range 0-3300 mV
float read_val(int gpio_num);
void adc_deinit();