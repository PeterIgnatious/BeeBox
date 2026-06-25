- Conjunto oficial de bibliotecas e ferramentas para desenvolver programas em C/C++ em microcontroladores Raspberry Pi Pico;
- Oferece bibliotecas para controle de GPIO, UART, I²C, PWM, etc;
- CMakeLists.txt informa ao compilador quais bibliotecas do SDK serão utilizadas;

```
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
  
  
int main()
{
    stdio_init_all();
    
    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }
  
    // Example to turn on the Pico W LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
```

![[Pasted image 20260624182416.png]]

![[Pasted image 20260624183416.png]]


![[Pasted image 20260624183118.png]]


## Pinagem usada

- LED RGB: 
	- Vermelho: GPIO13; 
	- Verde: GPIO11; 
	- Azul: GPIO12;
- Matriz 5x5 NeoPixel: GPIO7
- Display OLED: 
	- SDA: GPIO2; 
	- SCL: GPIO3;
- Botões: 
	- A: GPIO5; 
	- B: GPIO6; 
	- C: GPIO10; 
	- Joystick: GPIO22;
- Buzzer: GPIO21;
- Joystick: 
	- VRx: GPIO27; 
	- VRy: GPIO26;
- Microfone: GPIO28.

## GPIO's Livres

- **GPIO0**
- **GPIO1**
- **GPIO4**
- **GPIO8**
- **GPIO9**
- **GPIO14**
- **GPIO15**
- **GPIO16**
- **GPIO17**
- **GPIO18**
- **GPIO19**
- **GPIO20**

## Funções Importantes

- *stdio_init_all()*: Inicializa todos os dispositivos de entrada e saída (USB CDC, UART, etc);
- gpio_init(15): Inicializa um pino GPIO;
- gpio_set_dir(15, GPIO_OUT/GPIO_OUT): Define a direção do pino;
- gpio_put(15, 0): Define nível lógico em uma saída;
- gpio_get(15): Lê o estado de um pino;
- gpio_pull_up(15): Ativa o resistor de pull_up interno;
- gpio_pull_down(15): Ativa o resistor de pull_down interno;
- gpio_disable_pulls(15): Desativa resistores internos;
- gpio_set_function(4, GPIO_FUNC_I2C): Configura a função alternativa do pino;
	- GPIO_FUNC_I2C;
	- GPIO_FUNC_UART;
	- GPIO_FUNC_PWM;
	- GPIO_FUNC_PIO0;
	- GPIO_FUNC_PIO1;
- sleep_ms(): Espera tempo em milissegundos;
- sleep_us(): Espera em microssegundos;
- get_absolute_time(): Obtém tempo atual;
- reset_usb_boot(0, 0): Coloca o pico em BOOTSEL 


## Links Importantes

1. GitHub BitDogLab V7: https://github.com/BitDogLab/BitDogLab/tree/main/basic-examples
2. GitHub BitDogLab V6.3 (Utilizado): https://github.com/ThiagoSousa81/Datasheet-BitDogLab/blob/main/images/Datasheet_BitDogLab_v6.3.jpg
3. Abordagem Prática do BDL: https://books-sol.sbc.org.br/index.php/sbc/catalog/download/187/848/1769?inline=1
4. Libraries Raspberry PI (SDK): https://pip-assets.raspberrypi.com/categories/609-microcontroller-boards/documents/RP-009085-KB-1-raspberry-pi-pico-c-sdk.pdf