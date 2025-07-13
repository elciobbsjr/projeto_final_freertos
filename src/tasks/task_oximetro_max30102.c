#include "task_oximetro_max30102.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <math.h>
#include "utils_print.h"
#include "config_geral.h"
#include "mqtt_lwip.h"

#define I2C_PORT i2c1
#define MAX30102_ADDR 0x57

extern volatile bool emergencia_ativa;
extern SemaphoreHandle_t i2c1_mutex;
extern QueueHandle_t fila_alertas_mqtt;

static void write_reg(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        i2c_write_blocking(I2C_PORT, MAX30102_ADDR, buf, 2, false);
        xSemaphoreGive(i2c1_mutex);
    }
}

static bool read_fifo(uint32_t *ir, uint32_t *red) {
    uint8_t reg = 0x07;
    uint8_t buf[6];
    bool success = false;

    if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (i2c_write_blocking(I2C_PORT, MAX30102_ADDR, &reg, 1, true) >= 0 &&
            i2c_read_blocking(I2C_PORT, MAX30102_ADDR, buf, 6, false) >= 0) {
            *red = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];
            *ir  = ((uint32_t)buf[3] << 16) | ((uint32_t)buf[4] << 8) | buf[5];
            success = true;
        }
        xSemaphoreGive(i2c1_mutex);
    }

    return success;
}

static void max30102_init() {
    write_reg(0x09, 0x00); vTaskDelay(pdMS_TO_TICKS(50));
    write_reg(0x09, 0x03); // SpO2 mode
    write_reg(0x0A, 0x1F); // 100 Hz, 16-bit
    write_reg(0x0C, 0x24); // LED1 RED
    write_reg(0x0D, 0x24); // LED2 IR
    write_reg(0x0F, 0x00); // Slot config
    write_reg(0x08, 0x0F); // FIFO config
}

void task_oximetro_max30102(void *params) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    max30102_init();
    vTaskDelay(pdMS_TO_TICKS(100));
    safe_printf("[MAX30102] Iniciando monitoramento (60s)...\n");

    uint32_t ir = 0, red = 0, ir_anterior = 0;
    bool pulso_subiu = false;

    int batimentos = 0;
    float spo2_soma = 0;
    int spo2_amostras = 0;

    TickType_t t_inicio = xTaskGetTickCount();
    TickType_t t_ultimo_batimento = xTaskGetTickCount();

    TickType_t ultimo_alerta_bpm = 0;
    TickType_t ultimo_alerta_sem_batimento = 0;

    bool em_bpm_anormal = false;
    bool sensor_conectado = true;

    while (1) {
        if (emergencia_ativa) {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        bool leitura_ok = read_fifo(&ir, &red);

        if (!leitura_ok) {
            if (sensor_conectado) {
                safe_printf("[MAX30102] ERRO: Falha na leitura. Sensor possivelmente desconectado.\n");
                sensor_conectado = false;
            }

            vTaskDelay(pdMS_TO_TICKS(2000));
            max30102_init();
            vTaskDelay(pdMS_TO_TICKS(100));

            if (read_fifo(&ir, &red)) {
                safe_printf("[MAX30102] Sensor reconectado com sucesso.\n");
                sensor_conectado = true;
                t_inicio = xTaskGetTickCount();
                t_ultimo_batimento = t_inicio;
                batimentos = 0;
                spo2_soma = 0;
                spo2_amostras = 0;
            }

            continue;
        }

        if (!sensor_conectado) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        if (!pulso_subiu && ir > ir_anterior && (ir - ir_anterior) > 1000) {
            batimentos++;
            pulso_subiu = true;
            t_ultimo_batimento = xTaskGetTickCount();
        } else if (ir < ir_anterior) {
            pulso_subiu = false;
        }
        ir_anterior = ir;

        if (ir != 0) {
            float ratio = red / (float)ir;
            int spo2 = 110 - (int)(25.0f * ratio);
            if (spo2 > 100) spo2 = 100;
            if (spo2 < 70) spo2 = 70;
            spo2_soma += spo2;
            spo2_amostras++;
        }

        TickType_t agora = xTaskGetTickCount();

        if ((agora - t_inicio) >= pdMS_TO_TICKS(60000)) {
            int bpm_medio = batimentos;
            int spo2_medio = spo2_amostras ? (int)(spo2_soma / spo2_amostras) : 0;

            safe_printf("📊 [MÉDIA 60s] BPM: %d | SpO2: %d%%\n", bpm_medio, spo2_medio);

            // 🚨 Frequência cardíaca anormal (imediato, sem 15s)
            if (bpm_medio < 50 || bpm_medio > 100) {
                if (!em_bpm_anormal && (agora - ultimo_alerta_bpm) >= pdMS_TO_TICKS(5000)) {
                    safe_printf("🚨 [ALERTA] BPM anormal detectado: %d\n", bpm_medio);

                    char msg[128];
                    snprintf(msg, sizeof(msg), "🚨 Frequência cardíaca anormal! BPM: %d", bpm_medio);
                    if (fila_alertas_mqtt) xQueueSend(fila_alertas_mqtt, &msg, pdMS_TO_TICKS(100));

                    em_bpm_anormal = true;
                    ultimo_alerta_bpm = agora;
                }
            } else {
                if (em_bpm_anormal) {
                    safe_printf("💓 [INFO] BPM voltou ao normal: %d\n", bpm_medio);

                    char msg[128];
                    snprintf(msg, sizeof(msg), "💓 BPM normalizado: %d batimentos por minuto.", bpm_medio);
                    if (fila_alertas_mqtt) xQueueSend(fila_alertas_mqtt, &msg, pdMS_TO_TICKS(100));
                }
                em_bpm_anormal = false;
            }

            // ⚠️ Sem batimento nos últimos 10s
            if ((agora - t_ultimo_batimento) >= pdMS_TO_TICKS(10000) &&
                (agora - ultimo_alerta_sem_batimento) >= pdMS_TO_TICKS(5000)) {

                safe_printf("⚠️ [AVISO] Nenhum batimento detectado há 10s.\n");

                char msg[128];
                snprintf(msg, sizeof(msg), "⚠️ Nenhum batimento detectado nos últimos 10 segundos. Verifique o sensor!");
                if (fila_alertas_mqtt) xQueueSend(fila_alertas_mqtt, &msg, pdMS_TO_TICKS(100));

                ultimo_alerta_sem_batimento = agora;
            }

            batimentos = 0;
            spo2_soma = 0;
            spo2_amostras = 0;
            t_inicio = agora;
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // 10 Hz
    }
}
