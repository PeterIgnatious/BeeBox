#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "ds18b20.h"
#include "utils.h"


const int PRESENCE_MAX_TIME = 240;
const uint8_t READ_ROM = 0x33;
const uint8_t SKIP_ROM = 0xCC;
const uint8_t CONVERT_T = 0x44;
const uint8_t WRITE_SCRATCHPAD = 0x4E;
const uint8_t READ_SCRATCHPAD = 0xBE;
const uint8_t TEMP_MIN = 0x00; // Definir melhor
const uint8_t TEMP_MAX = 0xFF; // Definir melhor
const uint8_t RESOLUTION = 0x7F;
const int CONVERTION_TIME = 750;


void send_byte(DS18B20_Data* sensor, uint8_t command);
void send_bit(DS18B20_Data* sensor, bool bit_state);
uint8_t read_byte(DS18B20_Data* sensor);
bool read_bit(DS18B20_Data* sensor);
void read_scratchpad(DS18B20_Data* sensor);


void init_procedure(DS18B20_Data* sensor) {
    gpio_set_dir(sensor->pin, GPIO_OUT);
    gpio_put(sensor->pin, 0);
    sleep_us(490);
    gpio_put(sensor->pin, 1);
    sleep_us(60);
    gpio_put(sensor->pin, 0);
    sleep_us(10);
    gpio_set_dir(sensor->pin, GPIO_IN);
    sleep_us(20);

    while (gpio_get(sensor->pin));
    sleep_us(490);
}


bool ds18b20_init(DS18B20_Data* sensor, int ds_pin) {
    sensor->pin = ds_pin;
    gpio_init(sensor->pin);
    init_procedure(sensor);
    send_byte(sensor, SKIP_ROM);
    send_byte(sensor, WRITE_SCRATCHPAD);
    send_byte(sensor, TEMP_MAX);
    send_byte(sensor, TEMP_MIN);
    send_byte(sensor, RESOLUTION);
    read_scratchpad(sensor);

    return true;
}


void read_scratchpad(DS18B20_Data* sensor) {
    init_procedure(sensor);
    send_byte(sensor, SKIP_ROM);
    send_byte(sensor, CONVERT_T);
    sleep_ms(CONVERTION_TIME);

    init_procedure(sensor);
    send_byte(sensor, SKIP_ROM);
    send_byte(sensor, READ_SCRATCHPAD);

    sensor->temp_lsb = read_byte(sensor);
    sensor->temp_msb = read_byte(sensor);
    sensor->th= read_byte(sensor);
    sensor->tl = read_byte(sensor);
    sensor->config_reg = read_byte(sensor);
    sensor->reserved1 = read_byte(sensor);
    sensor->reserved2 = read_byte(sensor);
    sensor->reserved3 = read_byte(sensor);
    sensor->crc = read_byte(sensor);
}


void ds18b20_get_temperature(DS18B20_Data* sensor) {
    read_scratchpad(sensor);
    uint16_t value = sensor->temp_lsb | (sensor->temp_msb << 8);
    float temperature = value * 0.0625;
    sensor->current_temp = temperature;
}


void send_byte(DS18B20_Data* sensor, uint8_t command) {
    for (int i = 0; i < 8; i++) {
        send_bit(sensor, command & 0x01);
        command >>= 1;
    }

    sleep_us(5);
}


void send_bit(DS18B20_Data* sensor, bool bit_state) {
    gpio_set_dir(sensor->pin, GPIO_OUT);
    gpio_put(sensor->pin, 0);
    (bit_state) ? sleep_us(5) : sleep_us(90);
    gpio_put(sensor->pin, 1);
    (bit_state) ? sleep_us(45) : sleep_us(10);
}


uint8_t read_byte(DS18B20_Data* sensor) {
    uint8_t byte_val = 0x00;
    for (int i = 0; i < 8; i++) {
        byte_val |= (read_bit(sensor) << i);
    }

    return byte_val;
}

bool read_bit(DS18B20_Data* sensor) {
    bool bit_val = 0;
    gpio_set_dir(sensor->pin, GPIO_OUT);
    gpio_put(sensor->pin, 0);
    sleep_us(2);
    gpio_put(sensor->pin, 0);
    gpio_set_dir(sensor->pin, GPIO_IN);
    sleep_us(2);
    bit_val = gpio_get(sensor->pin);
    sleep_us(80);

    return bit_val;
}
