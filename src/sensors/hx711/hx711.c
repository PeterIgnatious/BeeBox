#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hx711.h"


const int LOADCELL_TARA_DEFAULT = 8405871;
const int LOADCELL_SCALE_FACTOR_DEFAULT = (8476597 - LOADCELL_TARA_DEFAULT) / 189;

void hx711_init(HX711_Data* sensor, uint8_t dt_pin, uint8_t sck_pin, uint32_t scale_factor, uint32_t tara_count) {
    sensor->dt_pin = dt_pin;
    sensor->sck_pin = sck_pin;
    
    sensor->scale_factor = (scale_factor == 0) ? LOADCELL_SCALE_FACTOR_DEFAULT : scale_factor;
    sensor->tara_count = (tara_count == 0) ? LOADCELL_TARA_DEFAULT : tara_count;

    gpio_init(sensor->dt_pin);
    gpio_set_dir(sensor->dt_pin, GPIO_IN);

    gpio_init(sensor->sck_pin);
    gpio_set_dir(sensor->sck_pin, GPIO_OUT);
    gpio_put(sensor->sck_pin, 0);
    sleep_us(20);
}


void hx711_read_count(HX711_Data* sensor) {
    uint32_t count = 0;
    while (gpio_get(sensor->dt_pin));

    for (int i = 0; i < 24; i++) {
        gpio_put(sensor->sck_pin, 1);
        sleep_us(20);
        count = count << 1;
        gpio_put(sensor->sck_pin, 0);
        sleep_us(20);
        if (gpio_get(sensor->dt_pin)) count++;
    }

    gpio_put(sensor->sck_pin, 1);
    sleep_us(20);
    count = count ^ 0x800000;
    gpio_put(sensor->sck_pin, 0);
    sleep_us(20);

    sensor->current_count = count;
}


void hx711_get_tara(HX711_Data* sensor) {
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        hx711_read_count(sensor);
        sum += sensor->current_count;
    }

    sensor->tara_count = sum / 20;
    printf("Tara: %d\n", sensor->current_count);
}


void hx711_get_scale_factor(HX711_Data* sensor, int weight) {
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        hx711_read_count(sensor);
        sum += sensor->current_count - sensor->tara_count;
    }

    sensor->scale_factor = (sum / 20) / weight;
}


void hx711_get_weight(HX711_Data* sensor) {
    hx711_read_count(sensor);
    float weight = (sensor->current_count - sensor->tara_count) / sensor->scale_factor;
    printf("Peso: %.2f g\n", weight);
}
