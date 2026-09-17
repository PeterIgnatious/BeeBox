# 🐝 BeeBox — Sistema de Monitoramento Inteligente de Colmeias

Projeto desenvolvido para a **SBESC (Simpósio Brasileiro de Engenharia de Sistemas Computacionais)**, sob orientação do Prof. Victor Medeiros e Profª. Edna Barros.

O sistema BeeBox consiste em uma plataforma embarcada de monitoramento de colmeias de abelhas, utilizando múltiplos sensores ambientais, estruturais e acústicos, além de comunicação de longa distância via LoRa.

---

# 🎯 Objetivo

Desenvolver um sistema embarcado capaz de:

- Monitorar condições internas da colmeia
- Detectar padrões ambientais críticos
- Avaliar vibração e atividade interna das abelhas
- Enviar dados remotamente via comunicação LoRa
- Auxiliar estudos de saúde e produtividade de colmeias

---

# 🧠 Plataforma de Hardware

- BitDogLab v.6.3 (baseada no RP2040 / Raspberry Pi Pico W)

---

# 📡 Sensores utilizados

## 🌡️ Temperatura
- 🔄 DS18B20 (digital, 1-Wire)

## 🌍 Ambiente
- ✅ DHT22 (temperatura e umidade)
- ❌ MQ135 (qualidade do ar / gases)

## ⚖️ Peso da colmeia
- ❌ Célula de carga (com amplificador HX711)

## 🎤 Áudio interno
- ❌ INMP411 (microfone MEMS)

## 🧭 Movimento / vibração
- 🔄 MPU6050 (acelerômetro + giroscópio)

## 📶 Comunicação
- ❌ SX1278 (LoRa — longa distância)

## 🌡️ Tempo Real
- ❌ DS1302 (Real Time Clock Module)

---

# 🛠️ Tecnologias

- C / C++
- Pico SDK
- CMake
- Drivers embarcados customizados
- Comunicação I2C / SPI / 1-Wire / ADC

---

# 🚀 Como baixar e compilar o projeto BeeBox

Este tutorial apresenta os passos necessários para configurar o ambiente, baixar o código-fonte do **BeeBox** e compilar o projeto utilizando o **Visual Studio Code** e a extensão oficial **Raspberry Pi Pico**.

## 1. Pré-requisitos

Antes de começar, instale:

* Visual Studio Code
* Git
* Extensão **Raspberry Pi Pico** para VS Code
* Extensão **Serial Monitor** para VS Code

---

## 2. Clonar o repositório

Abra um terminal no computador e clone o repositório:

```bash
git clone https://github.com/PeterIgnatious/BeeBox.git
```

Após isso abra o projeto no VS Code.

---

## 4. Configurar o projeto para o Raspberry Pi Pico W

Com o projeto aberto, abra a extensão **Raspberry Pi Pico** no menu lateral.

Selecione a opção `Switch Board` e configure para `pico_w`.

A extensão deverá identificar o `CMakeLists.txt` existente no projeto e configurar o ambiente de compilação.

---

## 5. Compilar o projeto

Na extensão **Raspberry Pi Pico**, selecione `Compile Project`:

Se a compilação ocorrer corretamente, será gerado um arquivo `.uf2` dentro da pasta `build`.

Exemplo:

```text
BeeBox/
├── CMakeLists.txt
├── src/
├── include/
├── ...
└── build/
    └── BeeBox.uf2
```

---

## 6. Conectar o Raspberry Pi Pico W

Para gravar o programa:

1. Desconecte o Pico W do computador.
2. Pressione e mantenha pressionado o botão **BOOTSEL**.
3. Conecte o Pico W ao computador através do USB.
4. Solte o botão **BOOTSEL**.

O Pico W aparecerá como uma unidade de armazenamento USB.

---

## 7. Gravar o firmware

Depois de compilar, utilize a opção **Run/Run Project** da extensão Raspberry Pi Pico.

Caso não funcione, o que é provável, copie manualmente o arquivo `.uf2` gerado para a unidade do Pico W.

Após a cópia, o Pico W será reiniciado e começará a executar o firmware.

---

# 📡 Como implementar os sensores na BitDogLab

## 💦 DHT22

### Pinout
<img width="212" height="176" alt="image" src="https://github.com/user-attachments/assets/af404343-b823-47b5-b2ff-dc5314e20805"/>

- É necessário colocar um resistor pull-up entre Vcc e Data para o seu funcionamento;
- Datasheet: [DHT22](src/sensors/dht22/dht11.pdf)


