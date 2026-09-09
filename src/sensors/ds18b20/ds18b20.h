#ifndef DHT22_H
#define DHT22_H

#include <stdbool.h>

typedef struct {
    float temperature;
    float humidity;
} DHT22_Data;

void dht22_init(uint gpio);
void dht22_read(DHT22_Data *data);

#endif