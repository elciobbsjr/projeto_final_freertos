#include "task_oximetro_max30102.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <math.h>
#include "utils_print.h"

#define I2C_PORT i2c0
#define MAX30102_ADDR 0x57

// ✅ Flag de emergência
extern volatile bool emergencia_ativa;

static void write_reg(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(I2C_PORT, MAX30102_ADDR, buf, 2, false);
}

static bool read_fifo(uint32_t *ir, uint32_t *red) {
    uint8_t reg = 0x07;
    uint8_t buf[6];
    if (i2c_write_blocking(I2C_PORT, MAX30102_ADDR, &reg, 1, true) < 0) return false;
    if (i2c_read_blocking(I2C_PORT, MAX30102_ADDR, buf, 6, false) < 0) return false;

    *red = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];
    *ir  = ((uint32_t)buf[3] << 16) | ((uint32_t)buf[4] << 8) | buf[5];
    return true;
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
    vTaskDelay(pdMS_TO_TICKS(1000));  // Aguarda sistema/USB
    max30102_init();
    vTaskDelay(pdMS_TO_TICKS(100));
    safe_printf("[MAX30102] Iniciando monitoramento (60s)...\n");

    uint32_t ir = 0, red = 0;
    uint32_t ir_anterior = 0;
    bool pulso_subiu = false;

    int batimentos = 0;
    float spo2_soma = 0;
    int spo2_amostras = 0;

    TickType_t t_inicio = xTaskGetTickCount();
    TickType_t t_inicio_bpm_anormal = 0;
    TickType_t t_ultimo_batimento = xTaskGetTickCount();

    bool em_bpm_anormal = false;
    bool sensor_conectado = true;

    while (1) {
        // ✅ Pausa durante emergência
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
                safe_printf("[MAX30102] Sensor reconectado com sucesso e reinicializado.\n");
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

            if (bpm_medio < 50 || bpm_medio > 100) {
                if (!em_bpm_anormal) {
                    t_inicio_bpm_anormal = agora;
                    em_bpm_anormal = true;
                } else if ((agora - t_inicio_bpm_anormal) >= pdMS_TO_TICKS(15000)) {
                    safe_printf("🚨 [ALERTA] Frequência cardíaca anormal por mais de 15s! BPM: %d\n", bpm_medio);
                }
            } else {
                em_bpm_anormal = false;
            }

            if ((agora - t_ultimo_batimento) >= pdMS_TO_TICKS(10000)) {
                safe_printf("⚠️ [AVISO] Nenhum batimento detectado nos últimos 10s. Verifique o sensor!\n");
            }

            batimentos = 0;
            spo2_soma = 0;
            spo2_amostras = 0;
            t_inicio = agora;
        }

        vTaskDelay(pdMS_TO_TICKS(100));  // 10 Hz
    }
}
