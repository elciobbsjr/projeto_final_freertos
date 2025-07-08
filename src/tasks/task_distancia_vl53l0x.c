#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "task_distancia_vl53l0x.h"
#include "config_geral.h"
#include "utils_print.h"
#include <stdlib.h>

// 🔁 IMPORTANTE: adicionar a flag da emergência
extern volatile bool emergencia_ativa;
extern SemaphoreHandle_t i2c1_mutex;
extern vl53l0x_dev vl53;

void task_distancia_vl53l0x(void *pvParameters) {
    static uint16_t ultima_distancia = 0;
    static bool alerta_anterior = false;
    static bool sensor_conectado = true;

    TickType_t tempo_inicio_proximidade = 0;
    bool em_proximidade = false;

    while (1) {
        // 🔒 Verifica modo de emergência ativo e pausa se necessário
        if (emergencia_ativa) {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(100))) {
            uint16_t distancia = vl53l0x_read_range_continuous_millimeters(&vl53);

            bool erro_leitura = (distancia == 65535);
            bool fora_do_alcance = (distancia > 2000);
            bool leitura_valida = (!erro_leitura && !fora_do_alcance);

            if (erro_leitura) {
                if (sensor_conectado) {
                    safe_printf("[VL53L0X] ERRO: Sensor não respondeu ou desconectado (timeout ou valor inválido).\n");
                    sensor_conectado = false;
                }
                xSemaphoreGive(i2c1_mutex);
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }

            if (!sensor_conectado && leitura_valida) {
                safe_printf("[VL53L0X] Sensor reconectado com sucesso.\n");
                sensor_conectado = true;
            }

            if (fora_do_alcance) {
                safe_printf("[VL53L0X] Fora de alcance (>30 cm).\n");
                xSemaphoreGive(i2c1_mutex);
                vTaskDelay(pdMS_TO_TICKS(500));
                continue;
            }

            if (abs(distancia - ultima_distancia) > 10) {
                safe_printf("[VL53L0X] Distância: %d mm\n", distancia);
            }

            if (distancia < 200) {
                if (!em_proximidade) {
                    tempo_inicio_proximidade = xTaskGetTickCount();
                    em_proximidade = true;
                } else {
                    if ((xTaskGetTickCount() - tempo_inicio_proximidade) >= pdMS_TO_TICKS(5000)) {
                        safe_printf("[ALERTA] Objeto muito próximo por mais de 5s! Risco de esbarrar ou cair.\n");
                    }
                }
            } else {
                em_proximidade = false;
            }

            if (distancia > 300 && ultima_distancia < 100) {
                safe_printf("[ALERTA] Movimento brusco detectado (afastamento rápido ou queda).\n");
            }

            ultima_distancia = distancia;

            if (distancia < 200 && !alerta_anterior) {
                safe_printf("[INFO] Objeto próximo detectado.\n");
                alerta_anterior = true;
            } else if (distancia >= 200 && alerta_anterior) {
                safe_printf("[INFO] Objeto afastado.\n");
                alerta_anterior = false;
            }

            xSemaphoreGive(i2c1_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
