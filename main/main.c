#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_sleep.h"

#include "dht22.h"
#include "ds18b20.h"
#include "hx711.h"
#include "ds1302.h"
#include "sdcard.h"

#include "wifi_manager.h"
#include "thingsboard.h"

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

// ==================================================
// CALIBRACAO HX711
// ==================================================

#define HX711_TARA           8689723
#define HX711_SCALE_FACTOR   416

// ==================================================
// CONFIGURACAO DOS PARÂMETROS
// ==================================================

#define SENSOR_INTERVAL_MS   60000

#define SENSOR_TASK_STACK    4096
#define LOGGER_TASK_STACK    4096

#define SENSOR_TASK_PRIORITY 5
#define LOGGER_TASK_PRIORITY 4

#define QUEUE_LENGTH         10

#define SAMPLES_HX711       10

#define TOTAL_ACQUISITIONS   5

#define READING_INTERVAL     60000000 // 1 minuto

#define LED_PIN GPIO_NUM_2


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
    ds1302_init(&ds1302, DS1302_DATA_PIN, DS1302_CLK_PIN, DS1302_RST_PIN);

    if (counter == 0) {
        data.temperature_dht22_sum = 0;
        data.temperature_ds18b20_sum = 0;
        data.humidity_sum = 0;
        data.weight_sum = 0;
        data.dht22_total = 0;
        data.ds18b20_total = 0;
        data.hx711_total = 0;
    }

    ESP_LOGI(TAG, "ALL SENSORS INITIALIZED!");
}

// ==================================================
// INICIALIZAÇÃO DO LED BUILTIN - 2
// ==================================================

void led_init(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);
}

// ==================================================
// ENTRANDO NO DEEPSLEEP
// ==================================================

void enter_deep_sleep(void) {
    gpio_set_level(HX711_DT_PIN, 0);
    gpio_set_level(DS18B20_DATA_PIN, 0);
    gpio_set_level(DHT22_DATA_PIN, 0);
    gpio_set_level(LED_PIN, 0);

    ESP_LOGI(TAG, "Entering sleep mode...");
    esp_sleep_enable_timer_wakeup(READING_INTERVAL);
    esp_deep_sleep_start();
}

// ==================================================
// TASK DA AQUISIÇÃO DOS DADOS
// ==================================================

void sensors_task(void *pvParameters) {
    ESP_LOGI(TAG, "Acquiring data from sensors...");
    bool status = dht22_read(&dht22);
    if (status) {
        ESP_LOGI(TAG, "DHT22: %.1f - %.1f", dht22.temp_celsius, dht22.humidity);
        data.temperature_dht22_sum += dht22.temp_celsius;
        data.dht22_total++;
        data.humidity_sum += dht22.humidity;

    }
    if (ds18b20_get_temperature(&ds18b20)) {
        ESP_LOGI(TAG, "DS18B20: %.1f", ds18b20.current_temp);
        data.temperature_ds18b20_sum += ds18b20.current_temp;
        data.ds18b20_total++;
    }
    if (hx711_get_weight(&hx711, SAMPLES_HX711)) {
        ESP_LOGI(TAG, "HX711: %.2f", hx711.weight);
        data.weight_sum += hx711.weight;
        data.hx711_total++;
    }
    
    counter++;

    if (counter < TOTAL_ACQUISITIONS) {
        enter_deep_sleep();
    } else {
        SensorAverage average = {0};

        average.temperature_ds18b20 
            = data.temperature_ds18b20_sum / data.ds18b20_total;
        average.temperature_dht22 
            = data.temperature_dht22_sum / data.dht22_total;
        average.humidity
            = data.humidity_sum / data.dht22_total;
        average.weight 
            = data.weight_sum / data.hx711_total;
        
        xQueueSend(sensor_queue, &average, portMAX_DELAY);
    
        vTaskDelete(NULL);
    }
}

// ==================================================
// TASK DO ARMAZENAMENTO E ENVIO
// ==================================================

void output_task (void *pvParameters) {
    SensorAverage average = {0};

    if (xQueueReceive(sensor_queue, &average, portMAX_DELAY) == true) {

        if (sdcard_init() != ESP_OK) {
            ESP_LOGE(TAG, "SD initialization failed!");
        } else {
            char timestamp[30];
            int is_send_to_thingsboard = 1;
            ds1302_get_timestamp(&ds1302, timestamp);

            if (sdcard_save_data(
                average.temperature_dht22,
                average.humidity,
                average.temperature_ds18b20,
                average.weight,
                timestamp,
                is_send_to_thingsboard
            ) != ESP_OK) {
                ESP_LOGE(TAG, "SAVE DATA OPERATION FAILED!");
            }
            sdcard_deinit();
        }

        // =========================================
        // 2. INICIALIZACAO DO WIFI
        // =========================================

        ESP_LOGI(TAG, "Initializing Wi-Fi...");

        wifi_manager_init();

        // Aguarda ate 20 segundos pela conexao
        if (wifi_manager_wait_connected(20000))
        {
            ESP_LOGI(TAG, "Wi-Fi connected!");

            // =====================================
            // 3. ENVIO AO THINGSBOARD
            // =====================================

            esp_err_t result = thingsboard_send_telemetry(
                average.temperature_dht22,
                average.humidity,
                average.weight,
                average.temperature_ds18b20
            );

            if (result == ESP_OK)
            {
                ESP_LOGI(TAG, "Data sent to ThingsBoard!");
            }
            else
            {
                ESP_LOGE(TAG, "ThingsBoard transmission failed!");
            }
        }
        else
        {
            ESP_LOGW(TAG, "Wi-Fi connection timeout!");
        }


    }
    
    counter = 0;

    enter_deep_sleep();
}
    

void app_main(void)
{
    led_init();
    gpio_set_level(LED_PIN, 1);
    sensor_queue = xQueueCreate(1, sizeof(SensorAverage));

    sensors_init();
    
    xTaskCreatePinnedToCore(
        sensors_task,
        "sensor_task",
        4096,
        NULL,
        5,
        NULL,
        1
    );

    xTaskCreatePinnedToCore(
        output_task,
        "output_task",
        8192,
        NULL,
        4,
        NULL,
        0
    );
}
