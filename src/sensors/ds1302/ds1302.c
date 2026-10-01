#include <stdio.h>
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


void write_byte(DS1302_Data* sensor, uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        
    }
    
}


void read_byte(DS1302_Data* sensor, uint8_t address) {

}

