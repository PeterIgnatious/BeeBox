#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/dns.h"
#include "lwip/err.h"

#include "thingsboard.h"
#include "thingsboard_tls.h"
#include "thingsboard_config.h"
#include "globalsign_root_r46.h"


// Variaveis internas para controlar o DNS.
static volatile bool dns_finalizado = false;
static volatile bool dns_sucesso = false;

static ip_addr_t endereco_tb;


// Callback chamado quando a consulta DNS termina.
static void resposta_dns(
    const char *nome,
    const ip_addr_t *ip,
    void *arg
) {
    (void)nome;
    (void)arg;

    if (ip != NULL) {
        endereco_tb = *ip;
        dns_sucesso = true;
    } else {
        dns_sucesso = false;
    }

    dns_finalizado = true;
}


// Resolve o endereco IP do servidor ThingsBoard.
bool thingsboard_resolver_dns(void) {

    printf("[ThingsBoard] Procurando servidor...\n");

    dns_finalizado = false;
    dns_sucesso = false;

    cyw43_arch_lwip_begin();

    err_t resultado = dns_gethostbyname(
        THINGSBOARD_HOST,
        &endereco_tb,
        resposta_dns,
        NULL
    );

    cyw43_arch_lwip_end();

    // Verifica se o endereco ja estava no cache DNS.
    if (resultado == ERR_OK) {

        dns_sucesso = true;
        dns_finalizado = true;

    } else if (resultado != ERR_INPROGRESS) {

        printf("[ThingsBoard] Erro DNS: %d\n", resultado);
        return false;
    }

    // Aguarda a resposta DNS por ate 10 segundos.
    absolute_time_t limite = make_timeout_time_ms(10000);

    while (!dns_finalizado && !time_reached(limite)) {
        sleep_ms(100);
    }

    if (dns_finalizado && dns_sucesso) {

        printf("[ThingsBoard] DNS resolvido com sucesso!\n");
        printf(
            "[ThingsBoard] IP encontrado: %s\n",
            ipaddr_ntoa(&endereco_tb)
        );

        return true;

    } else {

        printf("[ThingsBoard] Falha ao resolver servidor.\n");
        return false;
    }
}


// Monta e envia a telemetria para o ThingsBoard.
bool thingsboard_enviar_telemetria(
    float temperatura_dht22,
    float temperatura_ds18b20,
    float umidade,
    float peso
) {

    // Verifica se o DNS ja foi resolvido.
    if (!dns_sucesso) {
        printf("[ThingsBoard] DNS ainda nao esta disponivel.\n");
        return false;
    }

    char corpo[128];

    int tamanho_corpo = snprintf(
        corpo,
        sizeof(corpo),
        "{\"temperature\":%.1f,"
        "\"temperature_ds18b20\":%.1f,"
        "\"humidity\":%.1f,"
        "\"weight\":%.2f}",
        (double)temperatura_dht22,
        (double)temperatura_ds18b20,
        (double)umidade,
        (double)peso
    );

    if (tamanho_corpo < 0 ||
        tamanho_corpo >= (int)sizeof(corpo)) {

        printf("[ThingsBoard] Erro ao montar JSON.\n");
        return false;
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
        return false;
    }

    printf(
        "[ThingsBoard] Enviando: DHT22=%.1f C, "
        "DS18B20=%.1f C, umidade=%.1f %%, peso=%.2f\n",
        (double)temperatura_dht22,
        (double)temperatura_ds18b20,
        (double)umidade,
        (double)peso
    );

    // Realiza a comunicacao HTTPS.
    bool resultado = run_tls_client_test(
        THINGSBOARD_ROOT_CA,
        sizeof(THINGSBOARD_ROOT_CA),
        &endereco_tb,
        THINGSBOARD_HOST,
        requisicao,
        15
    );

    if (resultado) {
        printf("[ThingsBoard] Envio finalizado.\n");
    } else {
        printf("[ThingsBoard] Falha na conexao TLS/HTTPS.\n");
    }

    return resultado;
}