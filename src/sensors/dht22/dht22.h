#ifndef DHT22_H
#define DHT22_H

#include <stdbool.h>

typedef struct
{
    float humidity;
    float temp_celsius;
} dht_reading;

bool dht_read(dht_reading *result);

#endif