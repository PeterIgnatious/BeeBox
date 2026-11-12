#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

#include "dht22.h"


// Tempo máximo de espera para detectar uma resposta do sensor.
const uint32_t TIMEOUT_ERROR = 1000;


// Equivalente à função millis() do Raspberry Pi Pico.
static uint32_t millis(void) {
    return (uint32_t)(esp_timer_get_time() / 1000);
}


// Inicializa o sensor e configura o pino de dados.
void dht22_init(DHT22_Data* sensor, uint8_t pin) {
    sensor->data_pin = pin;

    gpio_reset_pin((gpio_num_t)sensor->data_pin);
    gpio_set_direction((gpio_num_t)sensor->data_pin, GPIO_MODE_OUTPUT);

    // Ativa o pull-up interno e mantém a linha em nível alto.
    gpio_pullup_en((gpio_num_t)sensor->data_pin);
    gpio_set_level((gpio_num_t)sensor->data_pin, 1);
}


// Envia o sinal de início para solicitar uma medição ao DHT22.
void start_signal(DHT22_Data* sensor) {

    // O microcontrolador mantém o pino em nível baixo
    // por pelo menos 1 ms; aqui são utilizados 18 ms.
    gpio_set_direction((gpio_num_t)sensor->data_pin, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)sensor->data_pin, 0);
    esp_rom_delay_us(18000);

    // Libera a linha para que o sensor possa responder.
    gpio_set_level((gpio_num_t)sensor->data_pin, 1);
    esp_rom_delay_us(40);

    // Configura o pino como entrada para receber a resposta.
    gpio_set_direction((gpio_num_t)sensor->data_pin, GPIO_MODE_INPUT);
    gpio_pullup_en((gpio_num_t)sensor->data_pin);
}


// Lê um byte (8 bits) enviado pelo sensor.
int read_data(DHT22_Data* sensor) {
    int value = 0;

    for (int i = 0; i < 8; i++) {

        // Aguarda o início do pulso alto correspondente ao bit.
        while (gpio_get_level((gpio_num_t)sensor->data_pin) == 0);

        // Aguarda parte do pulso para distinguir 0 de 1.
        esp_rom_delay_us(30);

        // Se a linha continuar alta após 30 us,
        // considera o bit recebido como 1.
        if (gpio_get_level((gpio_num_t)sensor->data_pin) == 1) {
            value |= (1 << (7 - i));
        }

        // Aguarda o fim do pulso alto antes do próximo bit.
        uint32_t start = millis();

        while (gpio_get_level((gpio_num_t)sensor->data_pin) == 1) {
            if (millis() - start > TIMEOUT_ERROR) {
                value = -1;
                return value;
            }
        }
    }

    return value;
}


// Solicita e interpreta uma leitura completa do sensor.
bool dht22_read(DHT22_Data* sensor) {

    // O DHT22 transmite 5 bytes:
    // 2 de umidade, 2 de temperatura e 1 de checksum.
    uint8_t data[5] = {0, 0, 0, 0, 0};

    start_signal(sensor);

    // Marca o instante inicial para controlar o timeout.
    uint32_t start_time = millis();

    // Aguarda o sensor começar a responder, levando
    // a linha de dados para nível baixo.
    while (gpio_get_level((gpio_num_t)sensor->data_pin) == 1) {
        if (millis() - start_time > TIMEOUT_ERROR) {
            printf("DHT22 ERROR: Timeout\n");
            return false;
        }
    }

    // Verifica a resposta inicial do sensor.
    if (gpio_get_level((gpio_num_t)sensor->data_pin) == 0) {
        esp_rom_delay_us(80);

        if (gpio_get_level((gpio_num_t)sensor->data_pin) == 1) {
            esp_rom_delay_us(80);

            // Recebe os cinco bytes transmitidos pelo DHT22.
            for (int j = 0; j < 5; j++) {
                data[j] = read_data(sensor);
            }
        }

        // Confere se a soma dos quatro primeiros bytes
        // corresponde ao checksum enviado no quinto byte.
        if (data[4] ==
            ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {

            // Converte os dois primeiros bytes em umidade (%).
            sensor->humidity =
                (float)((data[0] << 8) + data[1]) / 10.0f;

            // Trata uma leitura de umidade fora da faixa esperada.
            if (sensor->humidity > 100) {
                sensor->humidity = data[0];
            }

            // Converte os dois bytes seguintes em temperatura (°C).
            sensor->temp_celsius =
                (float)(((data[2] & 0x7F) << 8) + data[3]) / 10.0f;

            // Trata uma leitura acima da faixa do sensor.
            if (sensor->temp_celsius > 125) {
                sensor->temp_celsius = data[2];
            }

            // O bit mais significativo do byte da temperatura
            // indica se o valor é negativo.
            if (data[2] & 0x80) {
                sensor->temp_celsius = -sensor->temp_celsius;
            }

            return true; // Leitura válida.
        } else {
            return false; // Falha na verificação do checksum.
        }
    }

    return false; // Resposta inicial inválida.
}
