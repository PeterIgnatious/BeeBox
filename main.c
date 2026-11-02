#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "wifi_config.h"
#include "hardware/uart.h"

#include "dht22.h"
#include "ds18b20.h"
#include "hx711.h"
#include "ds1302.h"
#include "microsd.h"

// Includes para a conexão ao thingsboard
#include <stdbool.h>
#include "lwip/dns.h"
#include "thingsboard_tls.h"
#include "thingsboard_config.h"
#include "globalsign_root_r46.h"

// Definição dos pinos GPIO utilizados pelos sensores
const uint DHT22_DATA_PIN = 20;  // Pino de dados do DHT22
const uint DS18B20_DATA_PIN = 17; // Pino de dados do DS18B20
const uint HX711_DT_PIN = 18;    // Pino DT: saída de dados do HX711
const uint HX711_SCK_PIN = 19;   // Pino SCK: sinal de clock do HX711
const uint DS1302_CLK_PIN = 16;   // Pino CLK: sinal de clock do DS1302
const uint DS1302_DATA_PIN = 15;
const uint DS1302_RST_PIN = 14;   // Pino RST: sinal de reset do DS1302
const uint SDCARD_CS_PIN = 13;   // Pino CS: Chip Select do cartão SD
const uint HX711_TARA = 8690395;
const float HX711_SCALE_FACTOR = 424;


static volatile bool dns_finalizado = false;
static volatile bool dns_sucesso = false;
static ip_addr_t endereco_tb;

static void resposta_dns(const char *nome,
                         const ip_addr_t *ip,
                         void *arg) {
    if (ip != NULL) {
        endereco_tb = *ip;
        dns_sucesso = true;
    }

    dns_finalizado = true;
}

void testar_conexao_thingsboard() {
    printf("[ThingsBoard] Procurando servidor...\n");

    dns_finalizado = false;
    dns_sucesso = false;

    cyw43_arch_lwip_begin();

    err_t resultado = dns_gethostbyname(
        "tb.cin.ufpe.br",
        &endereco_tb,
        resposta_dns,
        NULL
    );

    cyw43_arch_lwip_end();

    if (resultado == ERR_OK) {
        dns_sucesso = true;
        dns_finalizado = true;
    } else if (resultado != ERR_INPROGRESS) {
        printf("[ThingsBoard] Erro DNS: %d\n", resultado);
        return;
    }

    absolute_time_t limite = make_timeout_time_ms(10000);

    while (!dns_finalizado && !time_reached(limite)) {
        sleep_ms(100);
    }

    if (dns_finalizado && dns_sucesso) {
        printf("[ThingsBoard] DNS resolvido com sucesso!\n");
    } else {
        printf("[ThingsBoard] Falha ao resolver servidor.\n");
    }
}

static void enviar_telemetria_teste(
    float temperatura,
    float umidade,
    float peso
)
{
    char corpo[128];

    int tamanho_corpo = snprintf(
        corpo,
        sizeof(corpo),
        "{\"temperature\":%.1f,\"humidity\":%.1f,\"weight\":%.2f}",
        (double)temperatura,
        (double)umidade,
        (double)peso
    );

    if (tamanho_corpo < 0 ||
        tamanho_corpo >= (int)sizeof(corpo)) {
        printf("[ThingsBoard] Erro ao montar JSON.\n");
        return;
    }

    char requisicao[512];

    int tamanho_requisicao = snprintf(
        requisicao,
        sizeof(requisicao),
        "POST /api/v1/%s/telemetry HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        THINGSBOARD_TOKEN,
        THINGSBOARD_HOST,
        tamanho_corpo,
        corpo
    );

    if (tamanho_requisicao < 0 ||
        tamanho_requisicao >= (int)sizeof(requisicao)) {
        printf("[ThingsBoard] Erro ao montar requisicao.\n");
        return;
    }

    printf(
        "[ThingsBoard] Enviando: temperatura=%.1f, "
        "umidade=%.1f, peso=%.2f\n",
        (double)temperatura,
        (double)umidade,
        (double)peso
    );

    bool resultado = run_tls_client_test(
        THINGSBOARD_ROOT_CA,
        sizeof(THINGSBOARD_ROOT_CA),
        &endereco_tb,
        THINGSBOARD_HOST,
        requisicao,
        15
        );

    if (!resultado) {
        printf("[ThingsBoard] Falha na conexao TLS/HTTPS.\n");
    }
}

int main()
{
    // Inicializa as entradas e saídas padrão, permitindo a comunicação serial com o computador.
    stdio_init_all();

    // Aguarda 1 segundo para permitir a inicialização
    // da comunicação serial antes de começar as leituras.
    sleep_ms(5000);

    // ETAPA 1: conectar a placa ao Wi-Fi (sem enviar telemetria ainda).
    // Esta configuracao atende a redes WPA2-Personal, com SSID e senha.
    // Redes WPA2-Enterprise/802.1X (login institucional) exigem
    // outra implementacao e nao funcionam com esta chamada.
    printf("[Wi-Fi] Inicializando controlador...\n");
    int wifi_init_result = cyw43_arch_init();
    if (wifi_init_result != 0) {
        printf("[Wi-Fi] Falha ao inicializar: %d\n", wifi_init_result);
    } else {
        cyw43_arch_enable_sta_mode();

        printf("[Wi-Fi] Conectando a %s...\n", WIFI_SSID);

        int wifi_result = cyw43_arch_wifi_connect_timeout_ms(
            WIFI_SSID,
            WIFI_PASSWORD,
            CYW43_AUTH_WPA2_AES_PSK,
            30000
        );

        // Mostra o resultado da tentativa de conexao
        printf("[Wi-Fi] Resultado da conexao: %d\n", wifi_result);

        if (wifi_result == 0) {
            printf("[Wi-Fi] Conectado com sucesso!\n");

            // Testa a resolucao DNS do ThingsBoard

        } else {
            printf("[Wi-Fi] Falha ao conectar!\n");
            printf("[Wi-Fi] Codigo do erro: %d\n", wifi_result);
        }

    }

    // Declara as estruturas que armazenam os dados
    // e as configurações de cada sensor.
    DHT22_Data dht22 = {0};
    DS18B20_Data ds18b20;
    HX711_Data hx711;
    DS1302_Data ds1302;

    // Inicializa o sensor DHT22, informando seu pino de dados.
    dht22_init(&dht22, DHT22_DATA_PIN);

    // Inicializa o DS18B20, informando seu pino de dados.
    ds18b20_init(&ds18b20, DS18B20_DATA_PIN);

    // Inicializa o HX711, informando os pinos DT e SCK.
    // Os dois últimos argumentos (0, 0) dependem da
    // implementação da função no módulo hx711.
    hx711_init(&hx711, HX711_DT_PIN, HX711_SCK_PIN, HX711_SCALE_FACTOR, HX711_TARA);

    // Inicializa o DS1302, informando os pinos CLK, DATA e RST.
    ds1302_init(&ds1302, DS1302_DATA_PIN, DS1302_CLK_PIN, DS1302_RST_PIN);

    if (sd_card_init()) {
        printf("[SD Card] Inicializacao bem-sucedida.\n");
    } else {
        printf("[SD Card] Falha na inicializacao.\n");
        while (1);
    }

    bool dns_teste_executado = false;

    while (true) {

        bool dht22_ok = dht22_read(&dht22);
        float temperatura_ds18b20 = 0.0f;
        float temperatura_dht22 = 0.0f;
        float umidade_envio = 0.0f;
        float peso_envio = 0.0f;
        char timestamp[20];
        char data_buffer[128];

        if (dht22_ok) {
            temperatura_dht22 = dht22.temp_celsius;
            umidade_envio = dht22.humidity;
        }

        // Obtém a temperatura medida pelo DS18B20.
        ds18b20_get_temperature(&ds18b20);

        // Obtém o peso medido pela célula de carga
        // através do conversor HX711.
        hx711_get_weight(&hx711, 50);

        ds1302_get_timestamp(&ds1302, timestamp);

        snprintf(
            data_buffer,
            sizeof(data_buffer),
            "%s,%.1f,%.1f,%.1f,%.2f\n",
            timestamp,
            (double)ds18b20.current_temp,
            (double)dht22.temp_celsius,
            (double)dht22.humidity,
            (double)hx711.weight
        );

        sd_card_write(data_buffer);
        
        printf("===============================\n");
        printf("Relatório de Sensores:\n");
        if (dht22_ok)
        {
            printf("DHT22 - Temperatura: %.1f °C\n",(double)temperatura_dht22);
            printf("DHT22 - Umidade: %.1f %%\n",(double)umidade_envio);
        } else {
            printf("[DHT22] Falha na leitura.\n");
            printf("Enviando zeros temporariamente.\n");
        }
        
        if (wifi_init_result == 0) {
            int status = cyw43_tcpip_link_status(
            &cyw43_state,
            CYW43_ITF_STA
            );

            if (status == CYW43_LINK_UP) {
                printf("[Wi-Fi] Conectado com IP!\n");

                if (!dns_teste_executado) {
                printf("[ThingsBoard] Iniciando teste DNS...\n");

                testar_conexao_thingsboard();

                    if (dns_sucesso) {
                        dns_teste_executado = true;
                        printf("[ThingsBoard] DNS confirmado.\n");
                    } else {
                    printf("[ThingsBoard] DNS falhou; tentaremos novamente.\n");
                    }
                }

                if (dns_teste_executado) {
                    enviar_telemetria_teste(
                    temperatura_dht22,
                    umidade_envio,
                    0.0f
                );
                }

            } else {
                printf("[Wi-Fi] Status: %d\n", status);
                printf("[Wi-Fi] Tentando reconectar...\n");

                int reconexao = cyw43_arch_wifi_connect_timeout_ms(
                    WIFI_SSID,
                    WIFI_PASSWORD,
                    CYW43_AUTH_WPA2_AES_PSK,
                    10000
                );

                printf("[Wi-Fi] Resultado da reconexao: %d\n", reconexao);

                if (reconexao == 0) {
                    printf("[Wi-Fi] Reconectado com sucesso!\n");

                    dns_teste_executado = false;
                    dns_sucesso = false;
                } else {
                    printf("[Wi-Fi] Reconexao falhou; tentaremos novamente.\n");
                }
            }

        } else {
            printf("[Wi-Fi] Falha na inicializacao!\n");
        }

        printf("===============================\n");

        printf("[DEBUG] Fim do ciclo; aguardando 5 segundos.\n");
        sleep_ms(5000);
        printf("[DEBUG] Fim da espera; iniciando proximo ciclo.\n");
    }
}
