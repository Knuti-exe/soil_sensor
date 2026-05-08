#include "adc_oneshot.h"
#include "ble_beacon.h"
#include <freertos/semphr.h>
#include <sys/time.h>
#include <esp_system.h>
#include <esp_sleep.h>
#include <driver/gpio.h>


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
RTC_DATA_ATTR static int boot_count = 0;      // #TODO send via BLE
SemaphoreHandle_t bleSemaphore = NULL;

// Soil humidity const
const int min = 950;    // min -> fully sumberged
const int max = 2770;    // max -> floating in the air    
const int min2 = 850;
const int max2 = 2770;
// Li-ion battery const
const battery_point_t battery_table[] = {
    {4350, 100},
    {4300, 95},
    {4250, 90},
    {4200, 80},
    {4100, 70},
    {4000, 60},
    {3920, 50},
    {3850, 40},
    {3800, 30},
    {3750, 20},
    {3700, 15},
    {3650, 10},
    {3550, 5},
    {3400, 0}
};

#define BATTERY_TABLE_SIZE (sizeof(battery_table) / sizeof(battery_table[0]))

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

    int chan_num = 3;
    adc_info_t *channels = pvPortMalloc(chan_num * sizeof(adc_info_t));

    channels[0].gpio_num = GPIO_BAT;
    channels[0].power_gpio = -1;
    channels[1].gpio_num = GPIO_SENSOR;
    channels[1].power_gpio = -1;  // #TODO      adc_oneshot.c might open declared power_pin
    channels[2].gpio_num = GPIO_SENSOR2;
    channels[2].power_gpio = -1;  // #TODO

    adc_init(channels, chan_num);
    free(channels);

    //                          BATTERY STATE READING

    int raw, val;
    uint8_t hum, bat, hum2;


    raw = read_val(GPIO_BAT);

    val = raw * 2; // bat voltage in mV (*2 => voltage divider)

    int i=0;
    while (i < BATTERY_TABLE_SIZE - 1)
    {
        if (battery_table[i++].voltage < val) break;
    }

    bat = (uint8_t) battery_table[i].soc;


    printf("Battery level:\n\tRaw val: %.1f \nCalc: %d %%\n\n", read_val(GPIO_BAT), battery_table[i].soc);
    
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

    printf("Humidity:\n\tRaw val: %.1f \nCalc: %d %%\n\n", read_val(GPIO_SENSOR), hum == 0xff ? 255 : raw);

    //                          PLANT 2 - HUMIDITY READINGS


    gpio_set_level(POWER_SENSOR2, 1);
    vTaskDelay(pdMS_TO_TICKS(20));

    raw = read_val(GPIO_SENSOR2);

    gpio_set_level(POWER_SENSOR2, 0);

    if (raw >= min && raw <= max)
    {
        val = 100 - (raw - min2) * 100 / (max2 - min2);
        hum2 = (uint8_t) val;
    }
        else hum2 = 0xff;  // means error

    printf("Humidity:\n\tRaw val: %.1f \nCalc: %d %%\n\n", read_val(GPIO_SENSOR2), hum2 == 0xff ? 255 : raw);
    
    vTaskDelay(pdMS_TO_TICKS(3000));
    //                              ADVERTISING

    ble_init(hum, hum2, bat);

    //                              SLEEP MODE

    xSemaphoreTake(bleSemaphore, portMAX_DELAY);

    ble_deinit();                   // useless, but let it stay    

    gpio_set_level(GPIO_LED, 0);

    ESP_LOGI(tag, "Entering deep sleep for %d hours", SLEEP_TIME);
    
    esp_deep_sleep(1000000LL * 3600 * SLEEP_TIME);
}