#ifndef THINGSBOARD_TLS_H
#define THINGSBOARD_TLS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "lwip/ip_addr.h"

bool run_tls_client_test(
    const uint8_t *cert,
    size_t cert_len,
    const ip_addr_t *server_ip,
    const char *server,
    const char *request,
    int timeout
);

#endif