#include "ble_beacon.h"


int rc;
const char *tag = "BLE_ADV";
static uint8_t ble_own_addr;
static uint8_t bat, hum;

static int ble_gap_callback(struct ble_gap_event *event, void *arg);

static void ble_advertise()
{
    struct ble_hs_adv_fields fields;
    struct ble_gap_adv_params adv_params;

    memset(&fields, 0, sizeof(fields));
    memset(&adv_params, 0, sizeof(adv_params));

    char *name = "Plant_sensor";
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    uint8_t data[4] = {0xff, 0xff, bat, hum};

    fields.mfg_data = data;
    fields.mfg_data_len = sizeof(data);

    rc = ble_gap_adv_set_fields(&fields);

    assert(rc == 0);

    adv_params.conn_mode = BLE_GAP_CONN_MODE_NON;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    adv_params.itvl_min = 160;
    adv_params.itvl_max = 240;

    rc = ble_gap_adv_start(
                            ble_own_addr,       // addr
                                NULL,               // direct advertising
                            800,                // advertise for 800 ms
                            &adv_params,        // low level params
                            ble_gap_callback,   // event callback
                            NULL                // event loop args
    );

    printf("Error code: %d\n", rc);
    assert(rc == 0);
}

static void on_sync()
{
    ble_hs_id_infer_auto(0, &ble_own_addr);

    ble_advertise();
}

static void ble_host_task(void *param) {
    nimble_port_run(); // inf loop
    
    nimble_port_freertos_deinit();
}

void ble_init(uint8_t _hum, uint8_t _bat)
{
    hum = _hum;
    bat = _bat;

    ESP_ERROR_CHECK(nimble_port_init());

    ble_hs_cfg.sync_cb = on_sync;

    nimble_port_freertos_init(ble_host_task);
}

static int ble_gap_callback(struct ble_gap_event *event, void *arg) 
{
    if (event->type == BLE_GAP_EVENT_ADV_COMPLETE) 
    {
        // deinit
        xSemaphoreGive(bleSemaphore);
    }
    return 0;
}