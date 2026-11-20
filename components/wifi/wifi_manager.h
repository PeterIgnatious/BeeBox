#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

void wifi_manager_init(void);

bool wifi_manager_is_connected(void);

bool wifi_manager_wait_connected(uint32_t timeout_ms);

#endif
