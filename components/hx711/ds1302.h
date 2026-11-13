#ifndef DS1302_H
#define DS1302_H

#include <stdint.h>
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


void ds1302_init(
    DS1302_Data* sensor,
    uint8_t dat_pin,
    uint8_t clk_pin,
    uint8_t rst_pin
);

void ds1302_set_datetime(
    DS1302_Data* sensor,
    uint16_t year,
    uint8_t dow,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second
);

void ds1302_get_datetime(DS1302_Data* sensor);

void ds1302_get_timestamp(
    DS1302_Data* sensor,
    char* timestamp
);

bool isHalted(DS1302_Data* sensor);

void ds1302_halt(DS1302_Data* sensor);

#endif
