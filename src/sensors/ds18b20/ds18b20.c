#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "ds18b20.h"
#include "utils.h"


const int PRESENCE_MAX_TIME = 240;


bool ds18b20_init(DS18B20_Data* sensor, int ds_pin) {
    sensor->pin = ds_pin;
    gpio_init(sensor->pin);
    gpio_set_dir(sensor->pin, GPIO_OUT);
    gpio_put(sensor->pin, 0);
    sleep_us(490);
    gpio_put(sensor->pin, 1);
    sleep_us(60);
    gpio_put(sensor->pin, 0);
    sleep_us(10);
    gpio_set_dir(sensor->pin, GPIO_IN);
    sleep_us(20);

    uint32_t start_time = millis();
    while (millis() - start_time < PRESENCE_MAX_TIME) {
        if (gpio_get(sensor->pin) == 0) {
            sensor->status = 1;
            return true;
        }
    }

    sensor->status = 0;
    return false;
}


void send_byte(DS18B20_Data* sensor, uint8_t command) {
    for (int i = 0; i < 8; i++) {
        send_bit(sensor, command & 0x01);
        command >>= 1;
    }
}


void send_bit(DS18B20_Data* sensor, bool bit_state) {
    gpio_set_dir(sensor->pin, GPIO_OUT);
    gpio_put(sensor->pin, 0);
    (bit_state) ? sleep_us(5) : sleep_us(90);
    gpio_put(sensor->pin, 1);
    (bit_state) ? sleep_us(45) : sleep_us(10);
}


void read_byte(DS18B20_Data* sensor, uint8_t data) {
    uint8_t data[5] = {0};
    for (int j = 0; j < 5; j++) {
        for (int i = 0; i < 8; i++) {
            data[j] |= (read_bit() << i);
        }
    }
}

/*
char read_byte()
{
  char byte_val = 0x00;                        //stores byte value
  char i;                                      //iteration variable

  for(i=0; i<8; i++)                           //sweep from 0 to 7 bits
    byte_val |= (read_bit() << i);             //read current bit and generate byte

  return byte_val;                             //return byte value

} //end read_byte