#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hx711.h"
#include <math.h>
#include "hardware/timer.h"

// Valor padrão da leitura do HX711 com a balança sem carga adicional (tara).
const int LOADCELL_TARA_DEFAULT = 8691130;

// Fator de escala padrão, obtido durante a calibração da célula de carga.
// É utilizado para converter a diferença entre a leitura atual e a tara em peso.
const int LOADCELL_SCALE_FACTOR_DEFAULT = 461;


// Inicializa o HX711, configurando os pinos e os parâmetros de calibração.
void hx711_init(HX711_Data* sensor, uint8_t dt_pin, uint8_t sck_pin, float scale_factor, uint32_t tara_count) {

    // Armazena os pinos utilizados pelo sensor.
    sensor->dt_pin = dt_pin;
    sensor->sck_pin = sck_pin;

    // Utiliza os valores fornecidos ou, se forem zero,
    // utiliza os valores padrão de calibração.
    sensor->scale_factor = (scale_factor == 0) ? LOADCELL_SCALE_FACTOR_DEFAULT : scale_factor;
    sensor->tara_count = (tara_count == 0) ? LOADCELL_TARA_DEFAULT : tara_count;

    // Configura o pino DT como entrada para receber os dados do HX711.
    gpio_init(sensor->dt_pin);
    gpio_set_dir(sensor->dt_pin, GPIO_IN);
    gpio_pull_up(sensor->dt_pin);

    sensor->current_count = 0;
    sensor->weight = 0.0f;

    // Configura o pino SCK como saída para gerar os pulsos de clock.
    gpio_init(sensor->sck_pin);
    gpio_set_dir(sensor->sck_pin, GPIO_OUT);

    // Mantém o clock em nível baixo enquanto não está realizando uma leitura.
    gpio_put(sensor->sck_pin, 0);
    sleep_us(20);
}


// Realiza a leitura dos 24 bits disponibilizados pelo HX711.
bool hx711_read_count(HX711_Data* sensor) {

    uint32_t count = 0;
    uint32_t inicio = time_us_32();

    // Aguarda o HX711 disponibilizar uma nova conversão.
    while (gpio_get(sensor->dt_pin) != 0) {

        if ((uint32_t)(time_us_32() - inicio) >= 150000u) {
            printf("[HX711] Timeout: DT nao ficou baixo.\n");
            return false;
        }

        tight_loop_contents();
    }

    // Lê os 24 bits enviados pelo conversor.
    for (int i = 0; i < 24; i++) {

        gpio_put(sensor->sck_pin, 1);
        sleep_us(20);

        count <<= 1;

        gpio_put(sensor->sck_pin, 0);
        sleep_us(20);

        if (gpio_get(sensor->dt_pin)) {
            count++;
        }
    }

    // Pulso adicional para finalizar a leitura.
    gpio_put(sensor->sck_pin, 1);
    sleep_us(20);

    count = count ^ 0x800000;

    gpio_put(sensor->sck_pin, 0);
    sleep_us(20);

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
