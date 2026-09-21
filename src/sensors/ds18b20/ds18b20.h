#ifndef DS18B20_H
#define DS18B20_H

#include <stdbool.h>

typedef struct {
    int pin;
    int status;
    float tl;
    float th;
    float current_temp;
} DS18B20_Data;

bool ds18b20_init(DS18B20_Data* DS_PIN, int gpio);
void ds18b20_resolution(int resolution);

#endif