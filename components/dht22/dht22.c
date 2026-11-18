
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_log.h"

#include "dht22.h"

#define DHT_TIMEOUT_US 120

static const char *TAG = "DHT22";

// Aguarda determinado nível lógico com timeout.
static bool wait_level(gpio_num_t pin,
                       int expected,
                       uint32_t timeout_us)
{
    while (gpio_get_level(pin) != expected) {
        if (timeout_us == 0) {
            return false;
        }

        esp_rom_delay_us(1);
        timeout_us--;
    }

    return true;
}

// Inicialização.
void dht22_init(DHT22_Data *sensor, uint8_t pin)
{
    sensor->data_pin = pin;

    gpio_reset_pin((gpio_num_t)pin);

    gpio_set_direction(
        (gpio_num_t)pin,
        GPIO_MODE_INPUT_OUTPUT_OD
    );

    gpio_set_pull_mode(
        (gpio_num_t)pin,
        GPIO_PULLUP_ONLY
    );

    gpio_set_level((gpio_num_t)pin, 1);

    ESP_LOGI(TAG, "DHT22 initialized on GPIO %d", pin);
}

// Envia o sinal de início.
static void start_signal(DHT22_Data *sensor)
{
    gpio_num_t pin = (gpio_num_t)sensor->data_pin;

    gpio_set_level(pin, 0);
    esp_rom_delay_us(18000);

    // Libera a linha open-drain.
    gpio_set_level(pin, 1);

    esp_rom_delay_us(30);
}

// Lê um bit.
static bool read_bit(gpio_num_t pin, uint8_t *bit)
{
    // Espera o início do pulso HIGH.
    if (!wait_level(pin, 1, DHT_TIMEOUT_US)) {
        return false;
    }

    // Ponto de amostragem.
    esp_rom_delay_us(40);

    *bit = gpio_get_level(pin) ? 1 : 0;

    // Espera o término do pulso HIGH.
    if (!wait_level(pin, 0, DHT_TIMEOUT_US)) {
        return false;
    }

    return true;
}

// Lê os 40 bits.
static bool read_bytes(gpio_num_t pin, uint8_t data[5])
{
    for (int i = 0; i < 5; i++) {
        data[i] = 0;

        for (int j = 0; j < 8; j++) {
            uint8_t bit;

            if (!read_bit(pin, &bit)) {
                return false;
            }

            data[i] = (data[i] << 1) | bit;
        }
    }

    return true;
}

// Leitura completa.
bool dht22_read(DHT22_Data *sensor)
{
    uint8_t data[5] = {0};

    gpio_num_t pin = (gpio_num_t)sensor->data_pin;

    start_signal(sensor);

    // Resposta inicial: LOW.
    if (!wait_level(pin, 0, DHT_TIMEOUT_US)) {
        ESP_LOGE(TAG, "Initial LOW response timeout");
        return false;
    }

    // Resposta inicial: HIGH.
    if (!wait_level(pin, 1, DHT_TIMEOUT_US)) {
        ESP_LOGE(TAG, "Initial HIGH response timeout");
        return false;
    }

    // Início da transmissão dos bits.
    if (!wait_level(pin, 0, DHT_TIMEOUT_US)) {
        ESP_LOGE(TAG, "Data transmission not detected");
        return false;
    }

    if (!read_bytes(pin, data)) {
        ESP_LOGE(TAG, "Failed to read DHT22 bits");
        return false;
    }

    // Validação do checksum.
    uint8_t checksum =
        (data[0] + data[1] + data[2] + data[3]) & 0xFF;

    if (checksum != data[4]) {
        ESP_LOGE(TAG, "Invalid checksum");
        return false;
    }

    // Umidade.
    sensor->humidity =
        ((data[0] << 8) | data[1]) / 10.0f;

    // Temperatura.
    sensor->temp_celsius =
        (((data[2] & 0x7F) << 8) | data[3]) / 10.0f;

    if (data[2] & 0x80) {
        sensor->temp_celsius *= -1.0f;
    }

    ESP_LOGI(TAG,
             "Temperature: %.1f C | Humidity: %.1f%%",
             sensor->temp_celsius,
             sensor->humidity);

    return true;
}
