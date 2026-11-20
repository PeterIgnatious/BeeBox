#ifndef THINGSBOARD_H
#define THINGSBOARD_H

#include "esp_err.h"

esp_err_t thingsboard_send_telemetry(
    float temperature_dht22,
    float humidity,
    float weight,
    float temperature_ds18b20
);

#endif
