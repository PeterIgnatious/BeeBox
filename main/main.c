#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_err.h"

#include "dht22.h"
#include "ds18b20.h"
#include "hx711.h"
#include "ds1302.h"
// #include "microsd.h"

// ==================================================
// CONFIGURACAO DOS PINOS - ESP32 DEVKIT
// ==================================================

// Sensores
#define DHT22_DATA_PIN       4
#define DS18B20_DATA_PIN     16

#define HX711_DT_PIN         32
#define HX711_SCK_PIN        33

// DS1302
#define DS1302_CLK_PIN       25
#define DS1302_DATA_PIN      26
#define DS1302_RST_PIN       27

// Cartao SD - SPI
#define SD_MISO_PIN          19
#define SD_MOSI_PIN          23
#define SD_SCK_PIN           18
#define SD_CS_PIN             5

// ==================================================
// CALIBRACAO HX711
// ==================================================

#define HX711_TARA           8693267
#define HX711_SCALE_FACTOR   424.0

// ==================================================
// CONFIGURACAO DAS TASKS
// ==================================================

#define SENSOR_INTERVAL_MS   60000

#define SENSOR_TASK_STACK    4096
#define LOGGER_TASK_STACK    4096

#define SENSOR_TASK_PRIORITY 5
#define LOGGER_TASK_PRIORITY 4

#define QUEUE_LENGTH         10


static const char *TAG = "BEEBOX";

// ==================================================
// ESTRUTURAS DE DADOS
// ==================================================

typedef struct {
    float temperature_ds18b20;
    float temperature_dht22;
    float humidity;
    float weight;
    char timestamp[30];

    bool dht22_ok;
    bool ds18b20_ok;
    bool hx711_ok;
    bool data_send_to_thingsboard;

    bool wifi_connected;
} BeeBox_Data;

// ==================================================
// INSTANCIAS DOS SENSORES
// ==================================================

static DHT22_Data dht22 = {0};
static DS18B20_Data ds18b20 = {0};
static HX711_Data hx711 = {0};
static DS1302_Data ds1302 = {0};

// ==================================================
// INICIALIZACAO DOS SENSORES
// ==================================================

void sensors_init(void) {
    // Inicializa o DHT22, informando o pino de dados.
    dht22_init(&dht22, DHT22_DATA_PIN);

    // Inicializa o DS18B20, informando o pino de dados.
    ds18b20_init(&ds18b20, DS18B20_DATA_PIN);

    // Inicializa o HX711, informando os pinos DT e SCK.
    hx711_init(&hx711, HX711_DT_PIN, HX711_SCK_PIN, HX711_SCALE_FACTOR, HX711_TARA);

    // Inicializa o DS1302, informando os pinos CLK, DATA e RST.
    ds1302_init(&ds1302, DS1302_CLK_PIN, DS1302_DATA_PIN, DS1302_RST_PIN);
}

void app_main(void)
{
    sensors_init();
    while (true) {
        printf("Hello from app_main!\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
