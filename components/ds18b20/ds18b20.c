
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ds18b20.h"

// Tempo máximo utilizado na verificação de presença.
const int PRESENCE_MAX_TIME = 240;

// Comandos do protocolo OneWire.
const uint8_t READ_ROM = 0x33;
const uint8_t SKIP_ROM = 0xCC;
const uint8_t CONVERT_T = 0x44;
const uint8_t WRITE_SCRATCHPAD = 0x4E;
const uint8_t READ_SCRATCHPAD = 0xBE;

// Limites de temperatura.
const uint8_t TEMP_MIN = 0x00;
const uint8_t TEMP_MAX = 0xFF;

// Configuração de resolução.
const uint8_t RESOLUTION = 0x7F;

// Tempo de conversão para 12 bits.
const int CONVERTION_TIME = 750;

// Define valor de TAG
static const char *TAG = "DS18B20";

// Declaração das funções auxiliares.
void send_byte(DS18B20_Data* sensor, uint8_t command);
void send_bit(DS18B20_Data* sensor, bool bit_state);
uint8_t read_byte(DS18B20_Data* sensor);
bool read_bit(DS18B20_Data* sensor);
void read_scratchpad(DS18B20_Data* sensor);


// Reset do barramento OneWire.
void init_procedure(DS18B20_Data* sensor) {

    gpio_set_direction(sensor->pin, GPIO_MODE_OUTPUT);
    gpio_set_level(sensor->pin, 0);
    esp_rom_delay_us(490);

    gpio_set_level(sensor->pin, 1);
    esp_rom_delay_us(60);

    gpio_set_level(sensor->pin, 0);
    esp_rom_delay_us(10);

    gpio_set_direction(sensor->pin, GPIO_MODE_INPUT);
    esp_rom_delay_us(20);

    int64_t start = esp_timer_get_time();

    while (gpio_get_level(sensor->pin)) {
        if ((esp_timer_get_time() - start) > 1000) {
            break;
        }
    }

    esp_rom_delay_us(490);
}


// Inicialização do DS18B20.
bool ds18b20_init(DS18B20_Data* sensor, int ds_pin) {
    ESP_LOGI(TAG, "Initializing DS18B20...");

    sensor->pin = ds_pin;

    gpio_reset_pin(sensor->pin);

    gpio_set_direction(sensor->pin, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level(sensor->pin, 1);

    ESP_LOGI(TAG, "DS18B20 initialized with data pin: %d", sensor->pin);
    ESP_LOGI(TAG, "Setting up DS18B20 configuration...");

    init_procedure(sensor);

    send_byte(sensor, SKIP_ROM);

    send_byte(sensor, WRITE_SCRATCHPAD);

    send_byte(sensor, TEMP_MAX);
    send_byte(sensor, TEMP_MIN);

    send_byte(sensor, RESOLUTION);

    ESP_LOGI(TAG, "DS18B20 configuration set up!");
    ESP_LOGI(TAG, "Reading initial temperature...");
    
    read_scratchpad(sensor);

    ESP_LOGI(TAG, "DS18B20 initial temperature read: %.2f °C", sensor->current_temp);

    return true;
}


// Inicia conversão e lê o scratchpad.
void read_scratchpad(DS18B20_Data* sensor) {

    init_procedure(sensor);

    send_byte(sensor, SKIP_ROM);

    send_byte(sensor, CONVERT_T);

    vTaskDelay(pdMS_TO_TICKS(CONVERTION_TIME));

    init_procedure(sensor);

    send_byte(sensor, SKIP_ROM);
    send_byte(sensor, READ_SCRATCHPAD);

    sensor->temp_lsb = read_byte(sensor);
    sensor->temp_msb = read_byte(sensor);

    sensor->th = read_byte(sensor);
    sensor->tl = read_byte(sensor);

    sensor->config_reg = read_byte(sensor);
    sensor->reserved1 = read_byte(sensor);
    sensor->reserved2 = read_byte(sensor);
    sensor->reserved3 = read_byte(sensor);

    sensor->crc = read_byte(sensor);
}


// Obtém a temperatura.
void ds18b20_get_temperature(DS18B20_Data* sensor) {

    read_scratchpad(sensor);

    uint16_t value = sensor->temp_lsb |
                     (sensor->temp_msb << 8);

    float temperature = (int16_t)value * 0.0625f;

    sensor->current_temp = temperature;
}


// Envia um byte pelo barramento OneWire.
void send_byte(DS18B20_Data* sensor, uint8_t command) {

    for (int i = 0; i < 8; i++) {

        send_bit(sensor, command & 0x01);

        command >>= 1;
    }

    esp_rom_delay_us(5);
}


// Envia um bit.
void send_bit(DS18B20_Data* sensor, bool bit_state) {

    gpio_set_direction(sensor->pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(sensor->pin, 0);

    bit_state ? esp_rom_delay_us(5) :
                esp_rom_delay_us(90);

    // Libera o barramento.
    gpio_set_level(sensor->pin, 1);

    bit_state ? esp_rom_delay_us(45) :
                esp_rom_delay_us(10);
}


// Lê um byte.
uint8_t read_byte(DS18B20_Data* sensor) {

    uint8_t byte_val = 0x00;

    for (int i = 0; i < 8; i++) {

        byte_val |= (read_bit(sensor) << i);
    }

    return byte_val;
}


// Lê um bit.
bool read_bit(DS18B20_Data* sensor) {

    bool bit_val = 0;

    gpio_set_direction(sensor->pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(sensor->pin, 0);

    esp_rom_delay_us(2);

    // Libera a linha para leitura.
    gpio_set_level(sensor->pin, 1);
    gpio_set_direction(sensor->pin, GPIO_MODE_INPUT);

    esp_rom_delay_us(2);

    bit_val = gpio_get_level(sensor->pin);

    esp_rom_delay_us(80);

    return bit_val;
}
