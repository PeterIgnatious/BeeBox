
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "ds18b20.h"
#include "utils.h"

// Código funcional para somente 1 sensor DS18B20.
// Não suporta múltiplos sensores no mesmo barramento OneWire.

// Tempo máximo utilizado na verificação de presença do sensor.
const int PRESENCE_MAX_TIME = 240;

// Comandos do protocolo OneWire.
const uint8_t READ_ROM = 0x33;          // Lê o código único do sensor (ROM).
const uint8_t SKIP_ROM = 0xCC;          // Ignora o endereço ROM (um único sensor).
const uint8_t CONVERT_T = 0x44;         // Inicia a conversão de temperatura.
const uint8_t WRITE_SCRATCHPAD = 0x4E;  // Escreve nos registradores de configuração.
const uint8_t READ_SCRATCHPAD = 0xBE;   // Lê os dados armazenados no scratchpad.

// Valores utilizados na configuração dos limites de temperatura.
const uint8_t TEMP_MIN = 0x00;
const uint8_t TEMP_MAX = 0xFF;

// Configuração de resolução do sensor.
const uint8_t RESOLUTION = 0x7F;

// Tempo máximo de conversão para a resolução de 12 bits: 750 ms.
const int CONVERTION_TIME = 750;


// Declaração das funções auxiliares utilizadas pelo driver.
void send_byte(DS18B20_Data* sensor, uint8_t command);
void send_bit(DS18B20_Data* sensor, bool bit_state);
uint8_t read_byte(DS18B20_Data* sensor);
bool read_bit(DS18B20_Data* sensor);
void read_scratchpad(DS18B20_Data* sensor);


// Executa o reset do barramento OneWire e verifica a presença do sensor.
void init_procedure(DS18B20_Data* sensor) {

    // Coloca o barramento em nível baixo para iniciar o reset.
    gpio_set_dir(sensor->pin, GPIO_OUT);
    gpio_put(sensor->pin, 0);
    sleep_us(490);

    // Libera a linha e aguarda a resposta de presença do sensor.
    gpio_put(sensor->pin, 1);
    sleep_us(60);

    // Volta a colocar a linha em nível baixo durante o procedimento.
    gpio_put(sensor->pin, 0);
    sleep_us(10);

    // Libera o pino para que o DS18B20 possa responder.
    gpio_set_dir(sensor->pin, GPIO_IN);
    sleep_us(20);

    // Aguarda a linha voltar ao nível alto após a resposta.
    while (gpio_get(sensor->pin));

    // Aguarda o término do intervalo de reset.
    sleep_us(490);
}


// Inicializa o DS18B20 e configura seus registradores.
bool ds18b20_init(DS18B20_Data* sensor, int ds_pin) {

    // Armazena o GPIO utilizado pelo sensor.
    sensor->pin = ds_pin;

    // Inicializa o pino GPIO.
    gpio_init(sensor->pin);

    // Reinicia o barramento e inicia a comunicação OneWire.
    init_procedure(sensor);

    // Seleciona o sensor. SKIP_ROM é adequado quando há
    // somente um DS18B20 conectado ao barramento.
    send_byte(sensor, SKIP_ROM);

    // Envia o comando para configurar o scratchpad.
    send_byte(sensor, WRITE_SCRATCHPAD);

    // Define os limites de temperatura configurados.
    send_byte(sensor, TEMP_MAX);
    send_byte(sensor, TEMP_MIN);

    // Define a resolução do sensor.
    send_byte(sensor, RESOLUTION);

    // Lê os registradores para atualizar os dados armazenados.
    read_scratchpad(sensor);

    return true;
}


// Inicia a conversão e lê os dados do scratchpad.
void read_scratchpad(DS18B20_Data* sensor) {

    // Reinicia a comunicação com o sensor.
    init_procedure(sensor);

    // Seleciona o único sensor conectado.
    send_byte(sensor, SKIP_ROM);

    // Solicita uma nova conversão de temperatura.
    send_byte(sensor, CONVERT_T);

    // Aguarda a conclusão da conversão.
    sleep_ms(CONVERTION_TIME);

    // Reinicia o barramento para iniciar a leitura dos dados.
    init_procedure(sensor);

    // Seleciona o sensor e solicita a leitura do scratchpad.
    send_byte(sensor, SKIP_ROM);
    send_byte(sensor, READ_SCRATCHPAD);

    // Lê os 9 bytes do scratchpad.
    // Os dois primeiros contêm a temperatura.
    sensor->temp_lsb = read_byte(sensor);
    sensor->temp_msb = read_byte(sensor);

    // Registradores de limite de temperatura.
    sensor->th = read_byte(sensor);
    sensor->tl = read_byte(sensor);

    // Registrador de configuração e bytes reservados.
    sensor->config_reg = read_byte(sensor);
    sensor->reserved1 = read_byte(sensor);
    sensor->reserved2 = read_byte(sensor);
    sensor->reserved3 = read_byte(sensor);

    // Byte utilizado para verificar a integridade dos dados (CRC).
    sensor->crc = read_byte(sensor);
}


// Obtém a temperatura a partir dos dois bytes de leitura.
void ds18b20_get_temperature(DS18B20_Data* sensor) {

    // Atualiza os registradores com uma nova leitura.
    read_scratchpad(sensor);

    // Combina o byte mais significativo (MSB) e o menos
    // significativo (LSB) em um valor de 16 bits.
    uint16_t value = sensor->temp_lsb | (sensor->temp_msb << 8);

    // Na resolução de 12 bits, cada unidade representa 0,0625 °C.
    float temperature = value * 0.0625;

    // Armazena a temperatura calculada na estrutura do sensor.
    sensor->current_temp = temperature;
}


// Envia um byte pelo barramento OneWire, bit a bit.
// O protocolo transmite primeiro o bit menos significativo.
void send_byte(DS18B20_Data* sensor, uint8_t command) {

    for (int i = 0; i < 8; i++) {

        // Envia o bit menos significativo do comando.
        send_bit(sensor, command & 0x01);

        // Desloca o byte para preparar o próximo bit.
        command >>= 1;
    }

    // Pequeno intervalo após o envio do byte.
    sleep_us(5);
}


// Envia um único bit por meio de pulsos temporizados.
void send_bit(DS18B20_Data* sensor, bool bit_state) {

    // Inicia o intervalo de escrita levando a linha ao nível baixo.
    gpio_set_dir(sensor->pin, GPIO_OUT);
    gpio_put(sensor->pin, 0);

    // A duração do nível baixo diferencia os bits:
    // pulso curto para 1 e pulso longo para 0.
    bit_state ? sleep_us(5) : sleep_us(90);

    // Libera a linha, colocando-a em nível alto.
    gpio_put(sensor->pin, 1);

    // Aguarda o intervalo restante do slot de escrita.
    bit_state ? sleep_us(45) : sleep_us(10);
}


// Lê um byte recebido pelo barramento OneWire.
uint8_t read_byte(DS18B20_Data* sensor) {

    uint8_t byte_val = 0x00;

    for (int i = 0; i < 8; i++) {

        // Lê um bit e o coloca na posição correspondente.
        byte_val |= (read_bit(sensor) << i);
    }

    return byte_val;
}


// Lê um único bit utilizando um slot de leitura OneWire.
bool read_bit(DS18B20_Data* sensor) {

    bool bit_val = 0;

    // Inicia o slot de leitura colocando a linha em nível baixo.
    gpio_set_dir(sensor->pin, GPIO_OUT);
    gpio_put(sensor->pin, 0);
    sleep_us(2);

    // Libera a linha para que o sensor transmita o bit.
    gpio_put(sensor->pin, 0);
    gpio_set_dir(sensor->pin, GPIO_IN);

    // Aguarda o momento de amostragem e lê o nível lógico.
    sleep_us(2);
    bit_val = gpio_get(sensor->pin);

    // Aguarda o restante do intervalo de leitura.
    sleep_us(80);

    return bit_val;
}
