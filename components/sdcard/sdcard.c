
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"

#include "driver/spi_common.h"
#include "driver/sdspi_host.h"

#include "sdmmc_cmd.h"

#include "sdcard.h"

// ==========================================
// CONFIGURAÇÕES
// ==========================================

static const char *TAG = "SDCARD";

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5

#define MOUNT_POINT "/sdcard"

#define FILE_PATH MOUNT_POINT "/data_log.txt"

// ==========================================
// VARIÁVEIS GLOBAIS
// ==========================================

static sdmmc_card_t *card = NULL;

static bool sdcard_mounted = false;

// ==========================================
// INICIALIZAÇÃO DO SD CARD
// ==========================================

esp_err_t sdcard_init(void)
{
    esp_err_t ret;

    // Configuração para montagem do sistema de arquivos
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    // Configuração do host SPI
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    host.max_freq_khz = 400;

    // Configuração dos pinos SPI
    spi_bus_config_t bus_config = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000
    };

    // Inicializa o barramento SPI
    ret = spi_bus_initialize(
        host.slot,
        &bus_config,
        SDSPI_DEFAULT_DMA
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI initialization failed: %s",
                 esp_err_to_name(ret));
        return ret;
    }

    // Configuração do dispositivo SD via SPI
    sdspi_device_config_t slot_config =
        SDSPI_DEVICE_CONFIG_DEFAULT();

    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    // Monta o cartão SD
    ret = esp_vfs_fat_sdspi_mount(
        MOUNT_POINT,
        &host,
        &slot_config,
        &mount_config,
        &card
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SD card mount failed: %s",
                 esp_err_to_name(ret));

        spi_bus_free(host.slot);

        return ret;
    }

    sdcard_mounted = true;

    ESP_LOGI(TAG, "SD card mounted successfully!");

    // Exibe informações do cartão
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}

// ==========================================
// GRAVAÇÃO DOS DADOS
// ==========================================

esp_err_t sdcard_save_data(
    float temperature_dht22,
    float humidity_dht22,
    float temperature_ds18b20,
    float weight,
    char *timestamp,
    int is_send_to_thingsboard
)
{
    if (!sdcard_mounted) {
        ESP_LOGE(TAG, "SD card is not mounted!");
        return ESP_ERR_INVALID_STATE;
    }

    // Abre o arquivo no modo append
    // Se não existir, será criado
    FILE *file = fopen(FILE_PATH, "a");

    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open data_log.txt");
        return ESP_FAIL;
    }

    // Escreve os dados no arquivo
    fprintf(
        file,
        "%s,%.2f,%.2f,%.2f,%.2f,%d\n",
        timestamp,
        temperature_dht22,
        humidity_dht22,
        temperature_ds18b20,
        weight,
        is_send_to_thingsboard
    );

    // Garante que os dados sejam enviados ao cartão
    fflush(file);

    // Verifica possíveis erros de escrita
    bool write_error = ferror(file);

    fclose(file);

    if (write_error) {
        ESP_LOGE(TAG, "Error writing data to SD card");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Data saved successfully!");

    return ESP_OK;
}

// ==========================================
// DESMONTAGEM DO SD CARD
// ==========================================

void sdcard_deinit(void)
{
    if (!sdcard_mounted) {
        return;
    }

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);

    spi_bus_free(host.slot);

    card = NULL;
    sdcard_mounted = false;

    ESP_LOGI(TAG, "SD card unmounted!");
}
