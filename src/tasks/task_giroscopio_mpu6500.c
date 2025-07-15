#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "mpu6500.h"
#include "hardware/i2c.h"
#include "config_geral.h"
#include "utils_print.h"

#define LIMIAR_QUEDA_G 1.5f
#define TEMPO_IMOBILIDADE_MIN 2
#define AMOSTRAS_IMOBILIDADE (TEMPO_IMOBILIDADE_MIN * 600)

#define MPU6500_ADDR         0x68
#define MPU6500_REG_WHO_AM_I 0x75

extern volatile bool emergencia_ativa;
extern SemaphoreHandle_t i2c1_mutex; // Mantenha o nome, mas usaremos para I2C0
extern QueueHandle_t fila_alertas_mqtt;  // ✅ Fila global de alertas

bool mpu6500_check_whoami() {
    uint8_t reg = MPU6500_REG_WHO_AM_I;
    uint8_t id = 0;
    bool success = false;

    // Usamos I2C0_PORT para a comunicação real, mas o mutex ainda é i2c1_mutex
    if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (i2c_write_blocking(I2C0_PORT, MPU6500_ADDR, &reg, 1, true) >= 0 &&
            i2c_read_blocking(I2C0_PORT, MPU6500_ADDR, &id, 1, false) >= 0) {
            success = (id == 0x70 || id == 0x68);
        }
        xSemaphoreGive(i2c1_mutex);
    }

    return success;
}

void task_giroscopio_mpu6500(void *pvParameters) {
    static float giros_z[100];
    static int idx = 0;

    float last_accel_z = 0.0f;
    uint32_t contador_imobilidade = 0;
    uint32_t contador_agitacao = 0;
    bool sensor_conectado = true;

    TickType_t ultimo_queda = 0;
    TickType_t ultimo_imobilidade = 0;
    TickType_t ultimo_agitacao = 0;

    while (1) {
        if (emergencia_ativa) {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        if (!mpu6500_check_whoami()) {
            if (sensor_conectado) {
                safe_printf("[MPU6500] ERRO: Sensor não detectado (falha WHO_AM_I) no I2C0.\n"); // Mensagem atualizada
                sensor_conectado = false;
            }
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        } else if (!sensor_conectado) {
            safe_printf("[MPU6500] Sensor reconectado com sucesso no I2C0.\n"); // Mensagem atualizada
            sensor_conectado = true;
        }

        mpu6500_data_t mpu_data;

        // Usamos I2C0_PORT para a comunicação real, mas o mutex ainda é i2c1_mutex
        if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            mpu6500_read_raw(I2C0_PORT, &mpu_data); // <<< MUDANÇA AQUI: i2c1_mutex agora controla I2C0_PORT
            xSemaphoreGive(i2c1_mutex);
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        float gyro_z = mpu_data.gyro[2] / 131.0f;
        float accel_x = mpu_data.accel[0] / 16384.0f;
        float accel_y = mpu_data.accel[1] / 16384.0f;
        float accel_z = mpu_data.accel[2] / 16384.0f;

        float delta_accel_z = accel_z - last_accel_z;
        TickType_t agora = xTaskGetTickCount();

        // === ⚠️ Queda detectada
        if ((delta_accel_z > LIMIAR_QUEDA_G || delta_accel_z < -LIMIAR_QUEDA_G) &&
            (agora - ultimo_queda) >= pdMS_TO_TICKS(5000)) {

            safe_printf("[MPU6500] ALERTA: Possível queda detectada! (ΔZ: %.2fg)\n", delta_accel_z);

            char msg[128];
            snprintf(msg, sizeof(msg), "⚠️ Possível queda detectada pelo sensor MPU6500 (ΔZ: %.2fg)", delta_accel_z);
            if (fila_alertas_mqtt != NULL) {
                xQueueSend(fila_alertas_mqtt, &msg, pdMS_TO_TICKS(100));
            }

            ultimo_queda = agora;
        }

        last_accel_z = accel_z;

        giros_z[idx++] = gyro_z;

        if (idx >= 100) {
            float soma = 0;
            for (int i = 0; i < 100; i++) soma += giros_z[i];
            float media_gyro_z = soma / 100.0f;

            //safe_printf("[MPU6500] Média do Giroscópio Z: %.2f °/s\n", media_gyro_z);

            // === 🚨 Imobilidade
            if (media_gyro_z > -0.5f && media_gyro_z < 0.5f) {
                contador_imobilidade += 100;
                if (contador_imobilidade >= AMOSTRAS_IMOBILIDADE &&
                    (agora - ultimo_imobilidade) >= pdMS_TO_TICKS(5000)) {

                    safe_printf("[MPU6500] ALERTA: Imobilidade ou inconsciência detectada!\n");

                    char msg[128];
                    snprintf(msg, sizeof(msg), "🚨 Imobilidade detectada! Possível inconsciência.");
                    if (fila_alertas_mqtt != NULL) {
                        xQueueSend(fila_alertas_mqtt, &msg, pdMS_TO_TICKS(100));
                    }

                    ultimo_imobilidade = agora;
                    contador_imobilidade = 0;
                }
            } else {
                contador_imobilidade = 0;
            }

            idx = 0;
        }

        // === 🏃‍♂️ Agitação intensa
        if (accel_x > 1.2f || accel_x < -1.2f || accel_y > 1.2f || accel_y < -1.2f) {
            contador_agitacao++;
        } else {
            contador_agitacao = 0;
        }

        if (contador_agitacao >= 5 && (agora - ultimo_agitacao) >= pdMS_TO_TICKS(5000)) {
            safe_printf("[MPU6500] ATENÇÃO: Atividade física intensa ou agitação detectada!\n");

            char msg[128];
            snprintf(msg, sizeof(msg), "🏃‍♂️ Agitação detectada! Atividade física intensa em andamento.");
            if (fila_alertas_mqtt != NULL) {
                xQueueSend(fila_alertas_mqtt, &msg, pdMS_TO_TICKS(100));
            }

            ultimo_agitacao = agora;
            contador_agitacao = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(100));  // 10 Hz
    }
}