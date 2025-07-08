#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "aht10.h"
#include "task_temperatura_aht10.h"
#include "utils_print.h"
#include "config_geral.h"
#include "hardware/gpio.h"

aht10_data_t latest_aht10 = {0};
extern SemaphoreHandle_t i2c1_mutex;

// === Limiar configurável para testes ===
#define TEMP_LIMITE_SUPERIOR    31.0f
#define TEMP_LIMITE_INFERIOR    17.0f
#define UMID_LIMITE_SUPERIOR    75.0f
#define UMID_LIMITE_INFERIOR    29.0f

void ativar_alerta_critico(bool ativar) {
    gpio_put(LED_VERMELHO_PIN, ativar ? 1 : 0);
    gpio_put(LED_VERDE_PIN, ativar ? 1 : 0); // Laranja = Vermelho + Verde
    gpio_put(BUZZER_PIN, ativar ? 1 : 0);
    gpio_put(BUZZER_B_PIN, ativar ? 1 : 0);
}

void task_temperatura_aht10(void *pvParameters) {
    static float temps[12];
    static int temp_idx = 0;
    static int leituras_criticas = 0;
    static int leituras_normais = 0;
    static bool alerta_ativo = false;
    static bool sensor_conectado = true;

    while (1) {
        if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(100))) {
            aht10_data_t data;
            bool leitura_ok = aht10_read_data(I2C1_PORT, &data);

            if (leitura_ok) {
                // Reconexão detectada
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

                if (leituras_criticas >= 3 && !alerta_ativo) {
                    safe_printf("[AHT10] ALERTA: Condições críticas detectadas!\n");
                    ativar_alerta_critico(true);
                    alerta_ativo = true;
                }

                if (leituras_normais >= 3 && alerta_ativo) {
                    safe_printf("[AHT10] ALERTA: Condição normalizada.\n");
                    ativar_alerta_critico(false);
                    alerta_ativo = false;
                }

                if (temp_idx >= 12) {
                    float soma = 0;
                    for (int i = 0; i < 12; i++) soma += temps[i];
                    float media = soma / 12.0f;
                    safe_printf("[AHT10] Média (último 1 min): %.2f °C\n", media);
                    temp_idx = 0;
                }

            } else {
                if (sensor_conectado) {
                    safe_printf("[AHT10] ERRO: Falha na leitura. Sensor possivelmente desconectado.\n");
                    sensor_conectado = false;
                }

                // Tentativa de reinicialização do sensor
                aht10_init(I2C1_PORT);
            }

            xSemaphoreGive(i2c1_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));  // Intervalo entre leituras
    }
}
