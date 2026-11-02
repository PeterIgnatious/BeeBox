#ifndef MICROSD_H
#define MICROSD_H

#include <stdbool.h>

int sd_card_init();
int sd_card_write(char *data);
int sd_card_read();
void sd_card_unmount();

#endif