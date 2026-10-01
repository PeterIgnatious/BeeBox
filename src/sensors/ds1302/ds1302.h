#ifndef DS1302_H
#define DS1302_H

#include <stdbool.h>

typedef struct {
    uint8_t dat_pin;
    uint8_t clk_pin;
    uint8_t rst_pin;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t day_of_week;
    uint8_t year;
} DS1302_Data;

void ds1302_init(DS1302_Data* sensor, uint8_t dat_pin, uint8_t clk_pin, uint8_t rst_pin);

#endif
