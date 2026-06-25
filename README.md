# 🐝 BeeBox — Sistema de Monitoramento Inteligente de Colmeias

Projeto desenvolvido para a **SBESC (Simpósio Brasileiro de Engenharia de Sistemas Computacionais)**, sob orientação do Prof. Victor Medeiros.

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
- DS18B20 (digital, 1-Wire)

## 🌍 Ambiente
- BME280 (temperatura, pressão e umidade)
- MQ135 (qualidade do ar / gases)

## ⚖️ Peso da colmeia
- Célula de carga (com amplificador HX711)

## 🎤 Áudio interno
- INMP411 (microfone MEMS)

## 🧭 Movimento / vibração
- MPU6050 (acelerômetro + giroscópio)

## 📶 Comunicação
- SX1278 (LoRa — longa distância)

---

# 🛠️ Tecnologias

- C / C++
- Pico SDK
- CMake
- Drivers embarcados customizados
- Comunicação I2C / SPI / 1-Wire / ADC
