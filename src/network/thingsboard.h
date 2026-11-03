#ifndef THINGSBOARD_H
#define THINGSBOARD_H

#include <stdbool.h>

// Resolve o endereco IP do servidor ThingsBoard.
bool thingsboard_resolver_dns(void);

// Envia temperatura, umidade e peso para o ThingsBoard.
bool thingsboard_enviar_telemetria(
    float temperatura_dht22,
    float temperatura_ds18b20,
    float umidade,
    float peso
);

#endif