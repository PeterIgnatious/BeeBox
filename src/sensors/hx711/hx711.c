#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hx711.h"
#include <math.h>

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

    // Configura o pino SCK como saída para gerar os pulsos de clock.
    gpio_init(sensor->sck_pin);
    gpio_set_dir(sensor->sck_pin, GPIO_OUT);

    // Mantém o clock em nível baixo enquanto não está realizando uma leitura.
    gpio_put(sensor->sck_pin, 0);
    sleep_us(20);
}


// Realiza a leitura dos 24 bits disponibilizados pelo HX711.
void hx711_read_count(HX711_Data* sensor) {

    uint32_t count = 0;

    // Aguarda o pino DT ficar em nível baixo, indicando
    // que uma nova conversão está disponível.
    while (gpio_get(sensor->dt_pin));

    // Lê os 24 bits enviados pelo conversor.
    for (int i = 0; i < 24; i++) {

        // Gera um pulso de clock para avançar a leitura.
        gpio_put(sensor->sck_pin, 1);
        sleep_us(20);

        // Desloca os bits anteriores uma posição para a esquerda.
        count = count << 1;

        // Finaliza o pulso de clock.
        gpio_put(sensor->sck_pin, 0);
        sleep_us(20);

        // Lê o bit atual e adiciona seu valor à leitura.
        if (gpio_get(sensor->dt_pin)) count++;
    }

    // Gera um pulso adicional para finalizar a leitura
    // e selecionar o próximo ciclo de conversão.
    gpio_put(sensor->sck_pin, 1);
    sleep_us(20);

    // Ajusta a representação da leitura de 24 bits.
    count = count ^ 0x800000;

    gpio_put(sensor->sck_pin, 0);
    sleep_us(20);

    // Armazena o valor lido na estrutura do sensor.
    sensor->current_count = count;
}


void hx711_get_average_count(HX711_Data *sensor, int samples)
{
    if (samples <= 0) return;

    int64_t sum = 0;

    for (int i = 0; i < samples; i++) {
        hx711_read_count(sensor);
        sum += sensor->current_count;
    }

    sensor->current_count = (int32_t)(sum / samples);
}


// Obtém o peso a partir da leitura média do HX711.
void hx711_get_weight(HX711_Data* sensor, int samples) {

    // Realiza 10 leituras e utiliza a média como valor atual.
    hx711_get_average_count(sensor, samples);

    float weight = 0;

    // Calcula o peso quando a leitura é maior que a tara.
    if (sensor->current_count > sensor->tara_count) {

        // Subtrai a tara e divide pelo fator de escala
        // para converter a leitura em peso.
        weight = (sensor->current_count - sensor->tara_count) / sensor->scale_factor;
        weight = round(weight / 10) * 10;
    }

    // Armazena o peso calculado na estrutura do sensor.
    sensor->weight = weight;
}
