# 🧓📡 Sistema Embarcado Multissensorial para Monitoramento de Idosos com Comunicação MQTT e Alertas via Telegram

Este projeto consiste em um sistema embarcado desenvolvido com **Raspberry Pi Pico W** e **FreeRTOS**, capaz de monitorar variáveis vitais e ambientais de idosos em tempo real. O sistema detecta situações críticas como quedas, imobilidade, variações de temperatura, agitação extrema e ausência de batimentos cardíacos, enviando alertas automaticamente via **MQTT** para um bot do **Telegram**.

---

## 🔧 Funcionalidades

- **Leitura de sensores**:
  - Temperatura e umidade com AHT10
  - Frequência cardíaca e oxigenação com MAX30102
  - Distância para detecção de obstáculos com VL53L0X
  - Queda, imobilidade e agitação com MPU6500
- **Modo de emergência manual** com botão físico (ativa buzzer e LED vermelho piscando)
- **Envio de alertas críticos** via MQTT (tópico `/notificacoes/telegram`)
- **Visualização dos dados** por meio de um bot Telegram vinculado
- **Reconexão automática** de sensores desconectados
- **Sistema multitarefa** baseado no FreeRTOS

---

## 🧱 Arquitetura do Projeto

```
Raspberry Pi Pico W + FreeRTOS
│
├── Sensor de Temperatura (AHT10)
├── Sensor de Batimento/SpO2 (MAX30102)
├── Sensor de Distância (VL53L0X)
├── Acelerômetro/Giroscópio (MPU6500)
│
├── Comunicação Wi-Fi (cyw43_arch)
├── Cliente MQTT (publicação em tópicos)
│
├── Botão de Emergência (GPIO)
│
└── Alertas via Telegram através do Broker MQTT
```

---

## 📁 Organização dos Arquivos

| Arquivo                        | Descrição |
|-------------------------------|-----------|
| `main.c`                      | Inicializa o sistema, Wi-Fi, semáforos, sensores e tarefas |
| `task_wifi.c`                 | Conecta à rede Wi-Fi |
| `task_mqtt.c`                 | Mantém a conexão com o broker MQTT |
| `task_alertas_mqtt.c`         | Publica alertas nas filas MQTT |
| `task_telegram.c`             | Envia uma mensagem de teste ao bot do Telegram |
| `task_emergencia.c`           | Trata o botão de emergência e envia alertas |
| `task_distancia_vl53l0x.c`    | Detecta obstáculos e riscos de colisão |
| `task_temperatura_aht10.c`    | Monitora temperatura/umidade e envia alertas ambientais |
| `task_oximetro_max30102.c`    | Mede batimentos e SpO2 e envia alertas cardíacos |
| `task_giroscopio_mpu6500.c`   | Detecta quedas, imobilidade e agitação intensa |
| `config_geral.h`              | Configurações de pinos, limites críticos e mutexes globais |
| `utils_print.h/.c`            | Impressão segura em multitarefa |

---

## 📡 Tópicos MQTT Utilizados

- `/notificacoes/telegram`: alertas enviados para o bot

---

## 🧪 Alertas Gerados

- **Queda detectada**
- **Objeto muito próximo por mais de 5s**
- **Imobilidade ou inconsciência**
- **Agitação intensa**
- **Temperatura ou umidade fora do intervalo**
- **Frequência cardíaca anormal**
- **Sensor desconectado**
- **Modo de emergência ativado/cancelado**

---

## 🚀 Como Executar

### 1. Clone o repositório com submódulos

Este projeto utiliza submódulos Git para bibliotecas externas. Use o seguinte comando para clonar corretamente:

```bash
git clone --recurse-submodules https://github.com/elciobbsjr/projeto_final_freertos.git
```

> Se já clonou sem submódulos:
>
> ```bash
> git submodule update --init --recursive
> ```

### 2. Configure o ambiente

- Instale o SDK do Raspberry Pi Pico (C/C++) com suporte ao **FreeRTOS**
- Configure seu projeto para compilar com CMake
- Certifique-se de que as bibliotecas dos sensores estejam acessíveis no include path

### 3. Atualize credenciais

No arquivo `config_geral.h`, edite:

```c
#define WIFI_SSID     "SEU_SSID"
#define WIFI_PASSWORD "SUA_SENHA"
#define MQTT_BROKER_IP "192.168.x.x"
```

### 4. Compile o projeto

```bash
mkdir build
cd build
cmake ..
make
```

### 5. Grave na placa Pico W

Use o modo BOOTSEL ou ferramenta como o `picotool`:

```bash
picotool load firmware.uf2
```

### 6. Execute o bot do Telegram

O bot deve estar ativo e ouvindo o tópico `/notificacoes/telegram` no broker MQTT.

---

## 🛠️ Requisitos

- Raspberry Pi Pico W
- FreeRTOS para Pico
- Sensores: AHT10, MAX30102, MPU6500, VL53L0X
- Broker MQTT (ex: Mosquitto local ou em nuvem)
- Bot Telegram vinculado ao broker MQTT
- IDE compatível com C/C++ (ex: VS Code + SDK do Pico)

---

## 📌 Observações

- Todos os alertas são enviados apenas quando há conexão Wi-Fi e com o broker MQTT.
- O sistema entra em **modo de emergência** quando o botão A é pressionado e só sai com o botão B.
- Cada sensor possui verificação de funcionamento e reconexão em caso de falhas.

---

## 👨‍💻 Autor

**Luiz Carlos**  
Curso: Ciência e Tecnologia - UFMA  
Matrícula: 12.023.04.1590  
Orientador: Prof. Thales
