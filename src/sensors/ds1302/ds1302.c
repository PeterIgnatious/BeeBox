#include <stdio.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "ds1302.h"


#define REG_SECONDS  0x80 
#define REG_MINUTES  0x82 
#define REG_HOUR     0x84 
#define REG_DATE     0x86 
#define REG_MONTH    0x88 
#define REG_DAY      0x8A 
#define REG_YEAR     0x8C
#define REG_WP       0x8E 
#define REG_BURST    0xBE 
#define REG_RAM00     0xC0
#define REG_RAM01     0xC2
#define REG_RAM29     0xFA
#define REG_RAM30     0xFC
#define REG_RAM_BURST 0xFE


void prepare_write(DS1302_Data *sensor, uint8_t reg);
void prepare_read(DS1302_Data *sensor, uint8_t reg);
void write_byte(DS1302_Data *sensor, uint8_t byte);
uint8_t ds1302_read_byte(DS1302_Data *sensor);
void next_bit(DS1302_Data *sensor);
uint8_t dec2bcd(uint8_t dec);
uint8_t bcd2dec(uint8_t bcd);


void ds1302_init(DS1302_Data* sensor, uint8_t dat_pin, uint8_t clk_pin, uint8_t rst_pin) {
    sensor->dat_pin = dat_pin;
    sensor->clk_pin = clk_pin;
    sensor->rst_pin = rst_pin;

    gpio_init(sensor->dat_pin);
    gpio_set_dir(sensor->dat_pin, GPIO_OUT);
    gpio_put(sensor->dat_pin, 0);
    
    gpio_init(sensor->clk_pin);
    gpio_set_dir(sensor->dat_pin, GPIO_OUT);
    gpio_put(sensor->clk_pin, 0);
    
    gpio_init(sensor->rst_pin);
    gpio_set_dir(sensor->dat_pin, GPIO_OUT);
    gpio_put(sensor->clk_pin, 0);
}


void ds1302_set_datetime(
    DS1302_Data* sensor,
    uint16_t year,
    uint8_t dow,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second
) {
    prepare_write(sensor, REG_WP);
    write_byte(sensor, 0b00000000);

    prepare_write(sensor, REG_BURST);
    write_byte(sensor, dec2bcd(second));
    write_byte(sensor, dec2bcd(minute));
    write_byte(sensor, dec2bcd(hour));
    write_byte(sensor, dec2bcd(day));
    write_byte(sensor, dec2bcd(month));
    write_byte(sensor, dec2bcd(dow));
    write_byte(sensor, dec2bcd(year));
    write_byte(sensor, 0b00000000);
}


void ds1302_get_datetime(DS1302_Data* sensor) {
    prepare_read(sensor, REG_BURST);
    sensor->seconds     = bcd2dec(ds1302_read_byte(sensor) & 0b01111111);
    sensor->minutes     = bcd2dec(ds1302_read_byte(sensor) & 0b01111111);
    sensor->hour        = bcd2dec(ds1302_read_byte(sensor) & 0b00111111);
    sensor->day         = bcd2dec(ds1302_read_byte(sensor) & 0b00111111);
    sensor->month       = bcd2dec(ds1302_read_byte(sensor) & 0b00011111);
    sensor->day_of_week = bcd2dec(ds1302_read_byte(sensor) & 0b00000111);
    sensor->year        = bcd2dec(ds1302_read_byte(sensor) & 0b01111111);
}


void write_byte(DS1302_Data* sensor, uint8_t byte) {
    gpio_set_dir(sensor->dat_pin, GPIO_OUT);
    for (int i = 0; i < 8; i++) {
        gpio_put(sensor->dat_pin, byte & 0x01);
        byte >>= 1;
        next_bit(sensor);
    }
    gpio_put(sensor->rst_pin, 0);
}


uint8_t ds1302_read_byte(DS1302_Data* sensor) {
    uint8_t byte = 0;
    gpio_set_dir(sensor->dat_pin, GPIO_IN);
    for (int i = 0; i < 8; i++) {
        if (gpio_get(sensor->dat_pin)) byte |= 0x01 << i;
        next_bit(sensor);
    }
    gpio_put(sensor->rst_pin, 0);
    return byte;
}


void prepare_write(DS1302_Data* sensor, uint8_t reg) {
    gpio_put(sensor->rst_pin, 1);
    uint8_t command = 0b10000000 | reg;
    write_byte(sensor, command);
}


void prepare_read(DS1302_Data* sensor, uint8_t reg) {
    gpio_put(sensor->rst_pin, 1);
    uint8_t command = 0b10000001 | reg;
    write_byte(sensor, command);
}


void next_bit(DS1302_Data* sensor) {
    gpio_put(sensor->clk_pin, 1);
    sleep_us(2);
    gpio_put(sensor->clk_pin, 0);
    sleep_us(2);
}


uint8_t dec2bcd(uint8_t dec) {
  return ((dec / 10 * 16) + (dec % 10));
}


uint8_t bcd2dec(uint8_t bcd) {
  return ((bcd / 16 * 10) + (bcd % 16));
}


bool isHalted(DS1302_Data* sensor) {
    prepare_read(sensor, REG_SECONDS);
    return (ds1302_read_byte(sensor) & 0b10000000);
}


void ds1302_halt(DS1302_Data* sensor) {
    prepare_write(sensor, REG_SECONDS);
    write_byte(sensor, 0b10000000);
}
