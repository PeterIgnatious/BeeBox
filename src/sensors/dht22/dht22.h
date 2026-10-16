#ifndef DHT22_H
#define DHT22_H

#include <stdbool.h>

typedef struct {
    uint8_t data_pin;
    float humidity;
    float temp_celsius;
} DHT22_Data;

void dht22_init(DHT22_Data *sensor, uint8_t pin);
bool dht22_read(DHT22_Data* sensor);

#endif