#ifndef HX711_H
#define HX711_H

#include <stdbool.h>

typedef struct {
    uint8_t dt_pin;
    uint8_t sck_pin;
    uint32_t tara_count;
    uint32_t scale_factor;
    uint32_t current_count;
} HX711_Data;

void hx711_init(HX711_Data* sensor, uint8_t dt_pin, uint8_t sck_pin, uint32_t scale_factor, uint32_t tara_count);
void hx711_get_tara(HX711_Data* sensor);
void hx711_get_weight(HX711_Data* sensor, float scale_factor);
void hx711_read_count(HX711_Data* sensor);

#endif
