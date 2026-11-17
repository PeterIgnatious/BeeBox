#ifndef SDCARD_H
#define SDCARD_H

#include <stdio.h>
#include "esp_err.h"

// Inicialização do cartão SD
esp_err_t sdcard_init(void);

// Salva os dados dos sensores
esp_err_t sdcard_save_data(
    float temperature_dht22,
    float humidity_dht22,
    float temperature_ds18b20,
    float weight,
    char *timestamp,
    int is_send_to_thingsboard
);

// Desmonta o cartão SD
void sdcard_deinit(void);

#endif