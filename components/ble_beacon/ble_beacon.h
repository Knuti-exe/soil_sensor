#pragma once

#include <string.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"

extern SemaphoreHandle_t bleSemaphore;

// hum, hum2, bat1, bat2, brownout, boot_count
void ble_init(uint8_t _hum, uint8_t _hum2, uint8_t _bat1, uint8_t _bat2, uint8_t _brownout, uint8_t _boot_count);
void ble_deinit();