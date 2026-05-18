#include "adc_oneshot.h"
#include "ble_beacon.h"
#include <freertos/semphr.h>
#include <sys/time.h>
#include <esp_system.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include "esp_mac.h"


#ifdef EXAMPLE_SLEEP_TIME
#define SLEEP_TIME EXAMPLE_SLEEP_TIME
#else
#define SLEEP_TIME 6
#endif

#define GPIO_SENSOR 0
#define GPIO_SENSOR2 3
#define POWER_SENSOR 10
#define POWER_SENSOR2 1
#define GPIO_BAT 4
#define GPIO_LED 8

typedef struct {
    int voltage;        // mV
    int soc;
}battery_point_t;

static const char *tag = "main";
RTC_DATA_ATTR static uint8_t boot_count = 0;
SemaphoreHandle_t bleSemaphore = NULL;

// Soil humidity const
const int min = 950;    // min -> fully sumberged
const int max = 2670;    // max -> floating in the air        #TODO have to reduce range
const int min2 = 850;
const int max2 = 2670;


void app_main(void) 
{
    gpio_config_t gpio_conf = {
        .pin_bit_mask = (1ULL<<POWER_SENSOR) | (1ULL<<GPIO_LED) | (1ULL<<POWER_SENSOR2),
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&gpio_conf);

    gpio_set_level(GPIO_LED, 1);
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // from ble_beacon.h
    bleSemaphore = xSemaphoreCreateBinary();
    xQueueReset(bleSemaphore);

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BT);

    printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
           mac[0], mac[1], mac[2],
           mac[3], mac[4], mac[5]);

    esp_reset_reason_t reason = esp_reset_reason();
    
    gpio_set_level(10, 0);
    if (reason == ESP_RST_BROWNOUT)
    {
        gpio_set_level(10, 1);
        vTaskDelay(pdMS_TO_TICKS(5000));
        gpio_set_level(10, 0);
    }


    int chan_num = 3;
    adc_info_t *channels = pvPortMalloc(chan_num * sizeof(adc_info_t));

    channels[0].gpio_num = GPIO_BAT;
    channels[1].gpio_num = GPIO_SENSOR;
    channels[2].gpio_num = GPIO_SENSOR2;

    adc_init(channels, chan_num);
    free(channels);

    //                          BROWNOUT RESET CHECKING

    uint8_t brownout = 0x00;

    if (esp_reset_reason() == ESP_RST_BROWNOUT && boot_count > 0)
    {
        brownout = 0x01;
    }

    //                          BATTERY READING

    float raw, val;
    uint8_t hum, hum2, bat1, bat2;

    raw = read_val(GPIO_BAT);

    val = raw * 2.00511; // bat voltage in mV (*2 => voltage divider)

    uint16_t bat_mv = (uint16_t)val;
    bat1 = (bat_mv >> 8) & 0xFF;
    bat2 = bat_mv & 0xFF;


    printf("Battery level:\n\tRaw val: %.1f \nCalc: %u *255 + %u %%\n\n", val, bat1, bat2);
    
    //                          PLANT 1 - HUMIDITY READINGS


    gpio_set_level(POWER_SENSOR, 1);
    vTaskDelay(pdMS_TO_TICKS(20));

    raw = read_val(GPIO_SENSOR);

    gpio_set_level(POWER_SENSOR, 0);

    if (raw >= min && raw <= max)
    {
        val = 100 - (raw - min) * 100 / (max - min);
        hum = (uint8_t) val;
    }
    else hum = 0xff;  // means error

    printf("Humidity:\n\tRaw val: %.1f \nCalc: %u %%\n\n", read_val(GPIO_SENSOR), hum);

    //                          PLANT 2 - HUMIDITY READINGS


    gpio_set_level(POWER_SENSOR2, 1);
    vTaskDelay(pdMS_TO_TICKS(20));

    raw = read_val(GPIO_SENSOR2);

    gpio_set_level(POWER_SENSOR2, 0);

    if (raw >= min2 && raw <= max2)
    {
        val = 100 - (raw - min2) * 100 / (max2 - min2);
        hum2 = (uint8_t) val;
    }
        else hum2 = 0xff;  // means error

    printf("Humidity:\n\tRaw val: %.1f \nCalc: %u %%\n\n", read_val(GPIO_SENSOR2), hum2);
    
    //                              ADVERTISING

    ble_init(hum, hum2, bat1, bat2, brownout, boot_count);

    //                              SLEEP MODE

    xSemaphoreTake(bleSemaphore, portMAX_DELAY);

    ble_deinit();                   // useless, but let it stay    

    gpio_set_level(GPIO_LED, 0);

    boot_count ++;

    ESP_LOGI(tag, "Entering deep sleep for %d hours", SLEEP_TIME);
    
    esp_deep_sleep(1000000LL * 3600 * SLEEP_TIME);
}
