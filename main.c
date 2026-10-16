#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "hardware/uart.h"

#include "dht22.h"
#include "ds18b20.h"
#include "hx711.h"
#include "ds1302.h"

// Definição dos pinos GPIO utilizados pelos sensores
const uint DHT22_DATA_PIN = 21;  // Pino de dados do DHT22
const uint DS18B20_DATA_PIN = 20; // Pino de dados do DS18B20
const uint HX711_DT_PIN = 18;    // Pino DT: saída de dados do HX711
const uint HX711_SCK_PIN = 19;   // Pino SCK: sinal de clock do HX711
const uint HX711_TARA = 0;
const uint HX711_SCALE_FACTOR = 0;

int main()
{
    // Inicializa as entradas e saídas padrão, permitindo a comunicação serial com o computador.
    stdio_init_all();

    // Aguarda 1 segundo para permitir a inicialização
    // da comunicação serial antes de começar as leituras.
    sleep_ms(1000);

    // Declara as estruturas que armazenam os dados
    // e as configurações de cada sensor.
    DHT22_Data dht22;
    DS18B20_Data ds18b20;
    HX711_Data hx711;

    // Inicializa o sensor DHT22, informando seu pino de dados.
    dht22_init(&dht22, DHT22_DATA_PIN);

    // Inicializa o DS18B20, informando seu pino de dados.
    ds18b20_init(&ds18b20, DS18B20_DATA_PIN);

    // Inicializa o HX711, informando os pinos DT e SCK.
    // Os dois últimos argumentos (0, 0) dependem da
    // implementação da função no módulo hx711.
    hx711_init(&hx711, HX711_DT_PIN, HX711_SCK_PIN, HX711_TARA, HX711_SCALE_FACTOR);

    while (true) {

        // Realiza a leitura de temperatura e umidade do DHT22.
        dht22_read(&dht22);

        // Obtém a temperatura medida pelo DS18B20.
        ds18b20_get_temperature(&ds18b20);

        // Obtém o peso medido pela célula de carga
        // através do conversor HX711.
        hx711_get_weight(&hx711);

        printf("\n===============================\n");
        printf("Relatório de Sensores:\n");
        printf("DHT22 - Temperatura: %.1f °C, Umidade: %.1f %%\n",
               dht22.temp_celsius, dht22.humidity);
        printf("DS18B20 - Temperatura: %.1f °C\n",
               ds18b20.current_temp);
        printf("HX711 - Peso: %.2f g\n", hx711.weight);
        
        sleep_ms(1000);
    }
}
