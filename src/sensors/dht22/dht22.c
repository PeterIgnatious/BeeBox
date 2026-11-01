#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "dht22.h"
#include "utils.h"
#include "hardware/timer.h"

// Tempo máximo de espera para detectar uma resposta do sensor.
const uint TIMEOUT_ERROR = 1000;


// Inicializa o sensor e configura o pino de dados.
void dht22_init(DHT22_Data* sensor, uint8_t pin) {
    sensor->data_pin = pin;

    gpio_init(sensor->data_pin);
    gpio_set_dir(sensor->data_pin, GPIO_OUT);

    // Ativa o pull-up interno e mantém a linha em nível alto.
    gpio_pull_up(sensor->data_pin);
    gpio_put(sensor->data_pin, 1);
}


// Envia o sinal de início para solicitar uma medição ao DHT22.
void start_signal(DHT22_Data* sensor) {

    // O microcontrolador mantém o pino em nível baixo
    // por pelo menos 1 ms; aqui são utilizados 18 ms.
    gpio_set_dir(sensor->data_pin, GPIO_OUT);
    gpio_put(sensor->data_pin, 0);
    sleep_ms(18);

    // Libera a linha para que o sensor possa responder.
    gpio_put(sensor->data_pin, 1);
    sleep_us(40);

    // Configura o pino como entrada para receber a resposta.
    gpio_set_dir(sensor->data_pin, GPIO_IN);
    gpio_pull_up(sensor->data_pin);
}


// Lê um byte (8 bits) enviado pelo sensor.
// Espera um nivel logico, com tempo limite em microssegundos.
static bool esperar_nivel(
    DHT22_Data *sensor,
    bool nivel_esperado,
    uint32_t timeout_us
) {
    uint32_t inicio = time_us_32();

    while ((gpio_get(sensor->data_pin) != 0) != nivel_esperado) {
        if ((uint32_t)(time_us_32() - inicio) >= timeout_us) {
            return false;
        }
    }

    return true;
}


// Le um byte do DHT22 com protecao contra esperas infinitas.
static bool read_data(DHT22_Data *sensor, uint8_t *resultado) {
    uint8_t value = 0;

    for (int i = 0; i < 8; i++) {

        // Aguarda o inicio do pulso alto.
        if (!esperar_nivel(sensor, true, 150)) {
            printf("[DHT22] Timeout esperando nivel alto, bit %d.\n", i);
            return false;
        }

        // Distingue os bits 0 e 1 pela duracao do pulso.
        sleep_us(30);

        if (gpio_get(sensor->data_pin) == 1) {
            value |= (uint8_t)(1u << (7 - i));
        }

        // Aguarda o final do pulso alto.
        if (!esperar_nivel(sensor, false, 150)) {
            printf("[DHT22] Timeout esperando nivel baixo, bit %d.\n", i);
            return false;
        }
    }

    *resultado = value;
    return true;
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
    while (gpio_get(sensor->data_pin) == 1) {
        if (millis() - start_time > TIMEOUT_ERROR) {
            printf("DHT22 ERROR: Timeout\n");
            return false;
        }
    }

    // Verifica a resposta inicial do sensor.
    if (gpio_get(sensor->data_pin) == 0) {
        sleep_us(80);

        if (gpio_get(sensor->data_pin) == 1) {
            sleep_us(80);
            
            for (int j = 0; j < 5; j++){
                    // Recebe os cinco bytes transmitidos pelo DHT22.
                if (!read_data(sensor, &data[j])) {
                    printf("[DHT22] Falha ao ler o byte %d.\n", j);
                    return false;
                }

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
