#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_sleep.h"

#include "dht22.h"
#include "ds18b20.h"
#include "hx711.h"
#include "ds1302.h"
// #include "microsd.h"

// ==================================================
// CONFIGURACAO DOS PINOS - ESP32 DEVKIT
// ==================================================

// Sensores
#define DHT22_DATA_PIN       16
#define DS18B20_DATA_PIN     4

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
// CONFIGURACAO DOS PARÂMETROS
// ==================================================

#define SENSOR_INTERVAL_MS   60000

#define SENSOR_TASK_STACK    4096
#define LOGGER_TASK_STACK    4096

#define SENSOR_TASK_PRIORITY 5
#define LOGGER_TASK_PRIORITY 4

#define QUEUE_LENGTH         10

#define SAMPLES_HX711        10

#define TOTAL_ACQUISITIONS   5

#define READING_INTERVAL     60000000 // 1 minuto


static const char *TAG = "BEEBOX";
static QueueHandle_t sensor_queue;

// ==================================================
// ESTRUTURAS DE DADOS
// ==================================================

typedef struct {
    float temperature_ds18b20_sum;
    float temperature_dht22_sum;
    float humidity_sum;
    float weight_sum;

    int ds18b20_total;
    int dht22_total;
    int hx711_total;

    bool data_send_to_thingsboard;
} BeeBox_Data;

RTC_DATA_ATTR static BeeBox_Data data;
RTC_DATA_ATTR static int counter = 0;

typedef struct {
    float temperature_dht22;
    float temperature_ds18b20;
    float humidity;
    float weight;
} SensorAverage;

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
    ESP_LOGI(TAG, "Initializing sensors...");
    // Inicializa o DHT22, informando o pino de dados.
    dht22_init(&dht22, DHT22_DATA_PIN);

    // Inicializa o DS18B20, informando o pino de dados.
    ds18b20_init(&ds18b20, DS18B20_DATA_PIN);

    // Inicializa o HX711, informando os pinos DT e SCK.
    hx711_init(&hx711, HX711_DT_PIN, HX711_SCK_PIN, HX711_SCALE_FACTOR, HX711_TARA);

    // Inicializa o DS1302, informando os pinos CLK, DATA e RST.
    ds1302_init(&ds1302, DS1302_CLK_PIN, DS1302_DATA_PIN, DS1302_RST_PIN);

    if (counter == 0) {
        data.temperature_dht22_sum = 0;
        data.temperature_ds18b20_sum = 0;
        data.humidity_sum = 0;
        data.weight_sum = 0;
        data.dht22_total = 0;
        data.ds18b20_total = 0;
        data.hx711_total = 0;
    }

    ESP_LOGI(TAG, "Sensors initialized!");
}

// ==================================================
// TASK DA AQUISIÇÃO DOS DADOS
// ==================================================

void sensors_read(void *pvParameters) {
    ESP_LOGI(TAG, "Acquiring data from sensors...");

    // dht22_read(&dht22);
    // ds18b20_get_temperature(&ds18b20);
    hx711_get_weight(&hx711, SAMPLES_HX711);
    ESP_LOGI(TAG, "HX711: %.2f", hx711.weight);

    counter++;

    data.temperature_dht22_sum += dht22.temp_celsius;
    data.dht22_total++;
    data.temperature_ds18b20_sum += ds18b20.current_temp;
    data.ds18b20_total++;
    data.weight_sum += hx711.weight;
    data.hx711_total++;

    if (counter < TOTAL_ACQUISITIONS) {
        esp_sleep_enable_timer_wakeup(READING_INTERVAL);
        esp_deep_sleep_start();
    } else {
        SensorAverage average = {0};

        average.temperature_ds18b20 
            = data.temperature_ds18b20_sum / data.ds18b20_total;
        average.temperature_dht22 
            = data.temperature_dht22_sum / data.dht22_total;
        average.weight 
            = data.weight_sum / data.hx711_total;
        
        xQueueSend(sensor_queue, &average, portMAX_DELAY);
    
        vTaskDelete(NULL);
    }
}
    

void app_main(void)
{
    sensor_queue = xQueueCreate(1, sizeof(SensorAverage));

    sensors_init();
    
    xTaskCreate(
        sensors_read,
        "sensor_task",
        4096,
        NULL,
        5,
        NULL
    );


}
