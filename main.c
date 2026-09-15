#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "hardware/uart.h"
#include "dht22.h"


// SPI Defines
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// I2C defines
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9


// ============================================================
// MPU6050
// ============================================================

#define MPU6050_ADDR       0x68
#define MPU6050_WHO_AM_I   0x75
#define MPU6050_PWR_MGMT_1 0x6B


// ============================================================
// UART defines
// ============================================================

#define UART_ID uart1
#define BAUD_RATE 115200

#define UART_TX_PIN 4
#define UART_RX_PIN 5


// ============================================================
// FUNÇÕES MPU6050
// ============================================================

// Escreve um valor em um registrador do MPU6050
void mpu6050_write(uint8_t reg, uint8_t data)
{
    uint8_t buffer[2];

    buffer[0] = reg;
    buffer[1] = data;

    i2c_write_blocking(
        I2C_PORT,
        MPU6050_ADDR,
        buffer,
        2,
        false
    );
}


// Lê um registrador do MPU6050
uint8_t mpu6050_read(uint8_t reg)
{
    uint8_t data;

    i2c_write_blocking(
        I2C_PORT,
        MPU6050_ADDR,
        &reg,
        1,
        true
    );

    i2c_read_blocking(
        I2C_PORT,
        MPU6050_ADDR,
        &data,
        1,
        false
    );

    return data;
}


// Inicializa e testa o MPU6050
bool mpu6050_init(void)
{
    uint8_t who_am_i;

    // Lê o registrador WHO_AM_I
    who_am_i = mpu6050_read(MPU6050_WHO_AM_I);

    printf("MPU6050 WHO_AM_I: 0x%02X\n", who_am_i);

    if (who_am_i != 0x68)
    {
        printf("MPU6050 nao encontrado!\n");
        return false;
    }

    printf("MPU6050 encontrado!\n");

    // Acorda o MPU6050
    mpu6050_write(MPU6050_PWR_MGMT_1, 0x00);

    printf("MPU6050 inicializado!\n");

    return true;
}


int main()
{
    stdio_init_all();


    // ========================================================
    // I2C - ADICIONADO
    // ========================================================

    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);


    // ========================================================
    // TESTE MPU6050 - ADICIONADO
    // ========================================================

    sleep_ms(1000);

    mpu6050_init();


    /*
    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Example to turn on the Pico W LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    */
    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    uart_puts(UART_ID, " Hello, UART!\n");



    dht_reading reading;

    while (true) {
        if (dht_read(&reading)) {
            printf("Temperatura: %.1f °C\n", reading.temp_celsius);
            printf("Umidade: %.1f %%\n", reading.humidity);
        } else {
            printf("Erro :(\n");
        }

        sleep_ms(500);
    }
}
