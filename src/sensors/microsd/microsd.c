#include <stdio.h>
#include "pico/stdlib.h"
#include "sd_card.h"
#include "ff.h"


FRESULT fr;
FATFS fs;
FIL fil;
int ret;
char buffer[100];
char filename[100] = "datalog.txt";


int sd_card_init() {
    if (!sd_init_driver()) {
        printf("ERROR: Inicialização do driver do cartão SD falhou.\n");
        return -1;
    }

    fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        printf("ERROR: Falha ao montar o sistema de arquivos. Código de erro: %d\n", fr);
        return -1;
    }

    return 1;
}


int sd_card_write(char *data) {
    gpio_set_dir(13, GPIO_OUT);
    gpio_put(13, 1);
    fr = f_open(&fil, filename, FA_WRITE | FA_OPEN_APPEND);
    if (fr != FR_OK) {
        printf("ERROR: Falha ao abrir o arquivo para escrita. Código de erro: %d\n", fr);
        return -1;
    }

    ret = f_printf(&fil, "%s\n", data);
    if (ret < 0) {
        printf("ERROR: Falha ao escrever no arquivo. Código de erro: %d\n", ret);
        f_close(&fil);
        return -1;
    }

    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("ERROR: Falha ao fechar o arquivo. Código de erro: %d\n", fr);
        return -1;
    }

    gpio_put(13, 0);
    return 1;
}

int sd_card_read() {
    fr = f_open(&fil, filename, FA_READ);
    if (fr != FR_OK) {
        printf("ERROR: Falha ao abrir o arquivo para leitura. Código de erro: %d\n", fr);
        return -1;
    }

    while (f_gets(buffer, sizeof(buffer), &fil)) {
        printf("%s", buffer);
    }

    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("ERROR: Falha ao fechar o arquivo. Código de erro: %d\n", fr);
        return -1;
    }

    return 1;
}

void sd_card_unmount() {
    f_unmount("0:");
}