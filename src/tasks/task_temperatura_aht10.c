#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "aht10.h"
#include "task_temperatura_aht10.h"
#include "utils_print.h"
#include "config_geral.h"
#include "hardware/gpio.h"
#include "mqtt_lwip.h"

extern volatile bool emergencia_ativa;
aht10_data_t latest_aht10 = {0};
extern SemaphoreHandle_t i2c1_mutex;
extern QueueHandle_t fila_alertas_mqtt;

#define TEMP_LIMITE_SUPERIOR    35.0f
#define TEMP_LIMITE_INFERIOR    17.0f
#define UMID_LIMITE_SUPERIOR    80.0f
#define UMID_LIMITE_INFERIOR    29.0f

static bool alerta_ativo = false;
static TickType_t ultima_piscada = 0;
static TickType_t ultimo_alerta_mqtt = 0;

void piscar_alerta_critico() {
    TickType_t agora = xTaskGetTickCount();
    if ((agora - ultima_piscada) >= pdMS_TO_TICKS(10000)) {
        for (int i = 0; i < 2; i++) {
            gpio_put(LED_VERMELHO_PIN, 1);
            gpio_put(LED_VERDE_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            gpio_put(LED_VERMELHO_PIN, 0);
            gpio_put(LED_VERDE_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        ultima_piscada = agora;
    }
}

void task_temperatura_aht10(void *pvParameters) {
    static float temps[12];
    static int temp_idx = 0;
    static int leituras_criticas = 0;
    static int leituras_normais = 0;
    static bool sensor_conectado = true;

    while (1) {
        if (emergencia_ativa) {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(100))) {
            aht10_data_t data;
            bool leitura_ok = aht10_read_data(I2C1_PORT, &data);

            if (leitura_ok) {
                if (!sensor_conectado) {
                    safe_printf("[AHT10] Sensor reconectado com sucesso.\n");
                    sensor_conectado = true;
                }

                latest_aht10 = data;
                safe_printf("[AHT10] Temperatura: %.2f °C | Umidade: %.2f %%\n",
                            data.temperature, data.humidity);

                if (data.temperature > -40.0f && data.temperature < 85.0f) {
                    temps[temp_idx++] = data.temperature;
                }

                bool temp_critica = (data.temperature > TEMP_LIMITE_SUPERIOR ||
                                     data.temperature < TEMP_LIMITE_INFERIOR);
                bool umidade_critica = (data.humidity > UMID_LIMITE_SUPERIOR ||
                                        data.humidity < UMID_LIMITE_INFERIOR);
                bool cond_critica = temp_critica || umidade_critica;

                if (cond_critica) {
                    leituras_criticas++;
                    leituras_normais = 0;
                } else {
                    leituras_normais++;
                    leituras_criticas = 0;
                }

                TickType_t agora = xTaskGetTickCount();
                if (leituras_criticas >= 3 && !alerta_ativo) {
                    safe_printf("[AHT10] ALERTA: Condições críticas detectadas!\n");
                    alerta_ativo = true;
                    ultima_piscada = agora;

                    if ((agora - ultimo_alerta_mqtt) >= pdMS_TO_TICKS(5000)) {
                        char msg[128];
                        snprintf(msg, sizeof(msg), "🌡️ ALERTA: Temperatura ou umidade fora dos limites! Temp: %.1f °C | Umid: %.1f%%",
                                 data.temperature, data.humidity);
                        if (fila_alertas_mqtt) xQueueSend(fila_alertas_mqtt, &msg, pdMS_TO_TICKS(100));
                        ultimo_alerta_mqtt = agora;
                    }
                }

                if (leituras_normais >= 3 && alerta_ativo) {
                    safe_printf("[AHT10] ALERTA: Condição normalizada.\n");
                    alerta_ativo = false;
                    gpio_put(LED_VERMELHO_PIN, 0);
                    gpio_put(LED_VERDE_PIN, 0);

                    char msg[128];
                    snprintf(msg, sizeof(msg), "✅ TEMP/UMIDADE normalizadas. Temp: %.1f °C | Umid: %.1f%%",
                             data.temperature, data.humidity);
                    if (fila_alertas_mqtt) xQueueSend(fila_alertas_mqtt, &msg, pdMS_TO_TICKS(100));
                }

                if (temp_idx >= 12) {
                    float soma = 0;
                    for (int i = 0; i < 12; i++) soma += temps[i];
                    float media = soma / 12.0f;
                    safe_printf("[AHT10] Média (1 min): %.2f °C\n", media);
                    temp_idx = 0;
                }

            } else {
                if (sensor_conectado) {
                    safe_printf("[AHT10] ERRO: Falha na leitura. Sensor possivelmente desconectado.\n");
                    sensor_conectado = false;
                }
                aht10_init(I2C1_PORT);
            }
            xSemaphoreGive(i2c1_mutex);
        }

        if (alerta_ativo) {
            piscar_alerta_critico();
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
