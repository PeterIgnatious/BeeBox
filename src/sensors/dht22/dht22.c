#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "dht22.h"

// Pino 25 LED on board
#ifdef PICO_DEFAULT_LED_PIN
#define LED_PIN PICO_DEFAULT_LED_PIN 
#endif

const uint DHT_PIN = 20;
const uint TIMEOUT_ERROR = 1000;


uint32_t millis() {
    return to_ms_since_boot(get_absolute_time());
}


void start_signal() {
    gpio_init(DHT_PIN);
    gpio_set_dir(DHT_PIN, GPIO_OUT);
    gpio_put(DHT_PIN, 0);
    sleep_ms(18);
    gpio_put(DHT_PIN, 1);
    sleep_us(40);
    gpio_set_dir(DHT_PIN, GPIO_IN);
    gpio_pull_up(DHT_PIN);
}


uint8_t read_data() {
    uint8_t value = 0;
    
    for (int i = 0; i < 8; i++) {
        while (gpio_get(DHT_PIN) == 0);
        sleep_us(30);
        if (gpio_get(DHT_PIN) == 1) {
            value |= (1 << (7 - i));
        }
        while (gpio_get(DHT_PIN) == 1);
    }

    return value;
}


bool dht_read(dht_reading *result) {
    uint8_t data[5] = {0, 0, 0, 0, 0};
    uint last = 1;
    uint j = 0;

    start_signal();
    uint32_t start_time = millis();

    while (gpio_get(DHT_PIN) == 1) {
        if (millis() - start_time > TIMEOUT_ERROR) {
            printf("Não respondeu");
            return false;
        }
    }

    if (gpio_get(DHT_PIN) == 0) {
        sleep_us(80);
        if (gpio_get(DHT_PIN) == 1) {
            sleep_us(80);
            
            for (int j = 0; j < 5; j++) {
                data[j] = read_data();
            }
        }
        if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
            result->humidity = (float) ((data[0] << 8) + data[1]) / 10;
            if (result->humidity > 100) {
                result->humidity = data[0];
            }
            result->temp_celsius = (float) (((data[2] & 0x7F) << 8) + data[3]) / 10;
            if (result->temp_celsius > 125) {
                result->temp_celsius = data[2];
            }
            if (data[2] & 0x80) {
                result->temp_celsius = -result->temp_celsius;
            }
            return true;
        } else {
            return false;
        }
    } 
    return false;

    // for (uint i = 0; i < MAX_TIMINGS; i++) {

        /*
        uint count = 0;
        while (gpio_get(DHT_PIN) == last) {
            count++;
            sleep_us(1);
            if (count == 255) break;
        }
        last = gpio_get(DHT_PIN);

        if (count == 255) break;

        if ((i >= 4) && (i % 2 == 0)) {
            data[j / 8] <<= 1;
            if (count > 16) data[j / 8] |= 1;
            j++;
        }
        */
    // }
}

