
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"

#include "hx711.h"


// Valor padrão da leitura do HX711 com a balança sem carga adicional (tara).
const int LOADCELL_TARA_DEFAULT = 8691130;

// Fator de escala padrão, obtido durante a calibração da célula de carga.
const int LOADCELL_SCALE_FACTOR_DEFAULT = 461;


// Inicializa o HX711, configurando os pinos e os parâmetros de calibração.
void hx711_init(
    HX711_Data* sensor,
    uint8_t dt_pin,
    uint8_t sck_pin,
    float scale_factor,
    uint32_t tara_count
) {

    // Armazena os pinos utilizados pelo sensor.
    sensor->dt_pin = dt_pin;
    sensor->sck_pin = sck_pin;

    // Utiliza os valores fornecidos ou os valores padrão.
    sensor->scale_factor = (scale_factor == 0)
        ? LOADCELL_SCALE_FACTOR_DEFAULT
        : scale_factor;

    sensor->tara_count = (tara_count == 0)
        ? LOADCELL_TARA_DEFAULT
        : tara_count;

    // Configura o pino DT como entrada.
    gpio_reset_pin(sensor->dt_pin);
    gpio_set_direction(sensor->dt_pin, GPIO_MODE_INPUT);
    gpio_pullup_en(sensor->dt_pin);

    sensor->current_count = 0;
    sensor->weight = 0.0f;

    // Configura o pino SCK como saída.
    gpio_reset_pin(sensor->sck_pin);
    gpio_set_direction(sensor->sck_pin, GPIO_MODE_OUTPUT);

    // Mantém o clock em nível baixo.
    gpio_set_level(sensor->sck_pin, 0);

    esp_rom_delay_us(20);
}


// Realiza a leitura dos 24 bits disponibilizados pelo HX711.
bool hx711_read_count(HX711_Data* sensor) {

    uint32_t count = 0;
    uint32_t inicio = (uint32_t)esp_timer_get_time();

    // Aguarda o HX711 disponibilizar uma nova conversão.
    while (gpio_get_level(sensor->dt_pin) != 0) {

        if ((uint32_t)((uint32_t)esp_timer_get_time() - inicio) >= 150000u) {

            printf("[HX711] Timeout: DT nao ficou baixo.\n");

            return false;
        }

        // Equivalente ao tight_loop_contents() do Pico SDK.
    }

    // Lê os 24 bits enviados pelo conversor.
    for (int i = 0; i < 24; i++) {

        gpio_set_level(sensor->sck_pin, 1);

        esp_rom_delay_us(20);

        count <<= 1;

        gpio_set_level(sensor->sck_pin, 0);

        esp_rom_delay_us(20);

        if (gpio_get_level(sensor->dt_pin)) {
            count++;
        }
    }

    // Pulso adicional para finalizar a leitura.
    gpio_set_level(sensor->sck_pin, 1);

    esp_rom_delay_us(20);

    count = count ^ 0x800000;

    gpio_set_level(sensor->sck_pin, 0);

    esp_rom_delay_us(20);

    sensor->current_count = count;

    return true;
}


bool hx711_get_average_count(HX711_Data *sensor, int samples)
{
    if (samples <= 0) {
        return false;
    }

    int64_t sum = 0;

    for (int i = 0; i < samples; i++) {

        if (!hx711_read_count(sensor)) {
            return false;
        }

        sum += sensor->current_count;
    }

    sensor->current_count = (uint32_t)(sum / samples);

    return true;
}


// Obtém o peso a partir da leitura média do HX711.
bool hx711_get_weight(HX711_Data* sensor, int samples) {

    if (!hx711_get_average_count(sensor, samples)) {
        sensor->weight = 0.0f;
        return false;
    }

    float weight = 0.0f;

    if (sensor->current_count > sensor->tara_count) {

        weight = (sensor->current_count - sensor->tara_count)
                 / sensor->scale_factor;

        weight = round(weight / 10.0f) * 10.0f;
    }

    sensor->weight = weight;

    return true;
}
