#include "adc_oneshot.h"

static adc_oneshot_unit_handle_t adc_handle;
static adc_info_t *adc_channels = NULL;

static int chan_num = 0;
const static char *adc_tag = "adc";

static void adc_calibration_init(adc_info_t *adc_channel);

void adc_init(adc_info_t *_channels, int _chan_num)
{
    esp_err_t ret;


    adc_channels = pvPortMalloc(_chan_num * sizeof(adc_info_t));
    
    memset(adc_channels, 0, _chan_num * sizeof(adc_info_t));
    memcpy(adc_channels, _channels, _chan_num * sizeof(adc_info_t));

    chan_num = _chan_num;

    adc_unit_t t;

    for (int i=0; i<chan_num; i++)
    {
        if (adc_channels[i].adc_chan) ESP_ERROR_CHECK(adc_oneshot_io_to_channel(adc_channels[i].gpio_num, &t, &(adc_channels[i].adc_chan)));
    }

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    for (int i=0; i<chan_num; i++)
    {
        ret = adc_oneshot_config_channel(adc_handle, adc_channels[i].adc_chan, &config);
        if (ret == ESP_OK) ESP_LOGI(adc_tag, "Adc channel_%d configured succeed!", i);
        else ESP_LOGE(adc_tag, "Adc channel_%d configuration failed!", i);
        
        adc_calibration_init(&adc_channels[i]);
        
    }
}


void adc_init_num(int *_channels, int _chan_num)
{
    if (chan_num != 0 && adc_channels) return;
    esp_err_t ret;

    chan_num = _chan_num;

    adc_channels = pvPortMalloc(chan_num * sizeof(adc_info_t));

    for (int i=0; i<chan_num; i++)
    {
        if (adc_channels[i].adc_chan)
        {
            ESP_ERROR_CHECK(adc_oneshot_io_to_channel(_channels[i], NULL, &adc_channels[i].adc_chan));
            adc_channels[i].gpio_num = _channels[i];
        }
    }

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    for (int i=0; i<chan_num; i++)
    {
        if (adc_channels[i].adc_chan) 
        {
            ret = adc_oneshot_config_channel(adc_handle, adc_channels[i].adc_chan, &config);
            if (ret == ESP_OK) ESP_LOGI(adc_tag, "Adc channel_%d configured succeed!", i);
            else ESP_LOGE(adc_tag, "Adc channel_%d configuration failed!", i);

            adc_calibration_init(&adc_channels[i]);
        }
    }
}

float read_val(int gpio_num)
{
    int avg = 0;
    int temp;
    int id = -1;

    for (int i=0; i<chan_num; i++)
    {
        if (gpio_num == adc_channels[i].gpio_num) id = i;
    }

    if (id == -1)
    {
        ESP_LOGE(adc_tag, "ADC channel not initialized");
        ESP_ERROR_CHECK(ESP_ERR_NOT_FOUND);
    } 

    for (int i=0; i<16; i++) 
    {
        ESP_ERROR_CHECK(adc_oneshot_get_calibrated_result(adc_handle, adc_channels[id].cali_chan_handle, 
                                                        adc_channels[id].adc_chan, &temp));
        avg += temp;   
    }

    return (float) avg / 16.0;    
}

void adc_deinit()
{
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc_handle));
    for (int i=0; i<chan_num; i++)
    {
        if (adc_channels[i].curve_fitting) 
        {
            ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(adc_channels[i].cali_chan_handle));
        }
    }
}

static void adc_calibration_init(adc_info_t *adc_channel)
{
    adc_channel->cali_chan_handle = NULL;
    esp_err_t ret = ESP_FAIL;
    adc_channel->curve_fitting = false;

    ESP_LOGI(adc_tag, "Calibrating with Curve Fitting...");
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &(adc_channel->cali_chan_handle));
    if (ret == ESP_OK) {
        adc_channel->curve_fitting = true;
    }
    
    if (ret == ESP_OK) {
        ESP_LOGI(adc_tag, "Curve fitting Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !adc_channel->curve_fitting) {
        ESP_LOGW(adc_tag, "eFuse not burnt, skip software calibration (curve)");
    } else {
        ESP_LOGE(adc_tag, "Invalid arg or no memory");
    }
}