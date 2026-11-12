# 🐝 BeeBox — Sistema de Monitoramento de Colmeias

Projeto desenvolvido para a **SBESC (Simpósio Brasileiro de Engenharia de Sistemas Computacionais)**, sob orientação do Prof. Victor Medeiros e da Profª. Edna Barros.

O BeeBox consiste em uma plataforma embarcada de monitoramento de colmeias de abelhas, utilizando sensores para acompanhar condições ambientais e características físicas da colmeia.

---

# 🎯 Objetivo

Desenvolver um sistema embarcado capaz de:

- Monitorar as condições internas da colmeia;
- Detectar padrões ambientais críticos;
- Acompanhar variações de peso da colmeia;
- Auxiliar estudos relacionados à saúde e produtividade das colmeias;
- Transmitir os dados coletados para uma plataforma de monitoramento remoto.

---

# 🧠 Plataforma de Hardware

- **ESP32 DevKit** (ESP32);
- Alimentação por bateria com suporte a recarga por painel solar.

---

# 📡 Sensores e módulos utilizados

## 🌡️ Temperatura

- ✅ **DS18B20** — Sensor digital de temperatura (1-Wire).

## 🌍 Ambiente

- ✅ **DHT22** — Sensor de temperatura e umidade.

## ⚖️ Peso da colmeia

- ✅ **Célula de carga** — Medição da variação de peso da colmeia.
- ✅ **HX711** — Conversor e amplificador para leitura da célula de carga.

## 🌡️ Tempo Real

- ✅ **DS1302** — Módulo RTC para registro de data e horário das medições.

## 📶 Comunicação

- ✅ **Wi-Fi integrado ao ESP32** — Transmissão dos dados coletados para a plataforma de monitoramento.

---

# 🛠️ Tecnologias

- C / C++;
- ESP-IDF (Espressif IoT Development Framework);
- FreeRTOS;
- CMake;
- Drivers embarcados customizados;
- Comunicação I2C / SPI / 1-Wire;
- Comunicação Wi-Fi;
- ThingsBoard para visualização e monitoramento remoto.

---

# 🚀 Como baixar e compilar o projeto BeeBox

Este tutorial apresenta os passos necessários para configurar o ambiente, baixar o código-fonte do **BeeBox** e compilar o firmware utilizando o **Visual Studio Code** e o ambiente oficial **ESP-IDF**.

## 1. Pré-requisitos

Antes de começar, instale:

- Visual Studio Code;
- Git;
- Extensão **ESP-IDF** para VS Code (Espressif IDF);
- Driver USB da placa ESP32, caso necessário.

Também é necessário possuir uma placa ESP32 DevKit e um cabo USB compatível com transferência de dados.

---

## 2. Configurar o ambiente ESP-IDF

Abra o Visual Studio Code e acesse a extensão **ESP-IDF**.

Caso ainda não tenha configurado o ambiente:

1. Abra a extensão ESP-IDF no menu lateral;
2. Selecione a opção de configuração do ambiente;
3. Utilize a opção de instalação expressa (*Express Installation*);
4. Aguarde a instalação do ESP-IDF, ferramentas de compilação e demais dependências.

Após a instalação, abra o terminal ESP-IDF pelo próprio VS Code.

---

## 3. Clonar o repositório

Abra o terminal e execute:

```bash
git clone https://github.com/PeterIgnatious/BeeBox.git
```

Entre na pasta do projeto:

```bash
cd BeeBox
```

Em seguida, abra o projeto no Visual Studio Code.

---

## 4. Configurar o projeto

Com o projeto aberto no VS Code:

1. Abra a extensão ESP-IDF;
2. Selecione a opção **Set Espressif Device Target**;
3. Escolha o alvo correspondente ao ESP32 utilizado:

```bash
idf.py set-target esp32
```

4. Configure os parâmetros do projeto, caso necessário:

```bash
idf.py menuconfig
```

Nessa etapa, podem ser configuradas opções relacionadas ao sistema, comunicação e parâmetros de compilação.

**Observação:** As credenciais de Wi-Fi e os parâmetros de conexão com o ThingsBoard devem ser configurados nos arquivos correspondentes do projeto.

---

## 5. Compilar o projeto

No terminal ESP-IDF, execute:

```bash
idf.py build
```

Se a compilação ocorrer corretamente, os arquivos do firmware serão gerados dentro da pasta `build`.

A estrutura será semelhante a:

```text
BeeBox/
├── CMakeLists.txt
├── main/
├── components/
├── build/
│   ├── BeeBox.bin
│   ├── bootloader/
│   └── partition_table/
└── ...
```

---

## 6. Conectar o ESP32

Conecte a placa ESP32 DevKit ao computador utilizando um cabo USB.

Verifique qual porta serial foi identificada pelo sistema operacional.

No Windows, geralmente será uma porta COM, como `COM3` ou `COM4`.

No Linux, geralmente será `/dev/ttyUSB0` ou `/dev/ttyACM0`.

---

## 7. Gravar o firmware

Para gravar o firmware no ESP32, execute:

```bash
idf.py -p COM3 flash
```

Substitua `COM3` pela porta serial correspondente à sua placa.

Caso a gravação não seja iniciada automaticamente, mantenha pressionado o botão **BOOT** da placa durante o início do processo.

Após a gravação, o ESP32 poderá ser reiniciado automaticamente.

Para visualizar as mensagens de execução pelo monitor serial:

```bash
idf.py -p COM3 monitor
```

Também é possível realizar a compilação, gravação e abertura do monitor serial em uma única execução:

```bash
idf.py -p COM3 flash monitor
```

Para sair do monitor serial, utilize `Ctrl + ]`.

---

# 📊 Funcionamento do sistema

O BeeBox realiza a aquisição periódica dos dados dos sensores, processa as medições e calcula os valores médios de cada intervalo de coleta.

Os dados são organizados e transmitidos via Wi-Fi para o ThingsBoard, permitindo o acompanhamento remoto das condições da colmeia.

O sistema também utiliza um módulo RTC para associar as medições aos respectivos registros de data e horário.