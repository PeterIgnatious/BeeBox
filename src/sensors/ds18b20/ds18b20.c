#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "ds18b20.h"
#include "utils.h"


const int PRESENCE_MAX_TIME = 240;


void send_byte(DS18B20_Data* sensor, uint8_t command);
void send_bit(DS18B20_Data* sensor, bool bit_state);
uint8_t read_byte(DS18B20_Data* sensor);
bool read_bit(DS18B20_Data* sensor);


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
    send_byte(sensor, 0xCC);
    send_byte(sensor, 0x44);
    sleep_us(750000);

    init_procedure(sensor);
    send_byte(sensor, 0xCC);
    send_byte(sensor, 0xBE);

    sensor->temp_lsb = read_byte(sensor);
    sensor->temp_msb = read_byte(sensor);
    sensor->th= read_byte(sensor);
    sensor->tl = read_byte(sensor);
    sensor->config_reg = read_byte(sensor);

    printf("temp_lsb: ");

    for (int i = 7; i >= 0; i--) {
        printf("%d", (sensor->temp_lsb >> i) & 1);
    }

    printf("\n");
    printf("temp_msb: ");

    for (int i = 7; i >= 0; i--) {
        printf("%d", (sensor->temp_msb >> i) & 1);
    }

    printf("\n");
    return true;

    

    /*
    uint32_t start_time = millis();
    while (millis() - start_time < PRESENCE_MAX_TIME) {
        if (gpio_get(sensor->pin) == 0) {
            init_procedure(sensor);
            send_byte(sensor, 0xCC);
            send_byte(sensor, 0x44);
            sleep_us(750000);

            init_procedure(sensor);
            send_byte(sensor, 0xCC);
            send_byte(sensor, 0xBE);

            sensor->temp_lsb = read_byte(sensor);
            sensor->temp_msb = read_byte(sensor);
            sensor->th= read_byte(sensor);
            sensor->tl = read_byte(sensor);
            sensor->config_reg = read_byte(sensor);

            printf("temp_lsb: 0x%02X\n", sensor->temp_lsb);
            printf("temp_msb: 0x%02X\n", sensor->temp_msb);
            printf("temp_th: 0x%02X\n", sensor->th);
            return true;
        }
    }
    return false;
    */
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
