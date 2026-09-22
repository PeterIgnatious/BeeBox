#ifndef DS18B20_H
#define DS18B20_H

#include <stdbool.h>

typedef struct {
    int pin;
    uint8_t temp_lsb;
    uint8_t temp_msb;
    uint8_t tl;
    uint8_t th;
    uint8_t config_reg;
    uint8_t crc;
    float current_temp;
} DS18B20_Data;

bool ds18b20_init(DS18B20_Data *DS_PIN, int gpio);

#endif
