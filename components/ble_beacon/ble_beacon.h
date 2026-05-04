#pragma once

#include <string.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"

extern SemaphoreHandle_t bleSemaphore;

void ble_init(uint8_t _hum, uint8_t _bat);