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

// ✅ Flag de emergência
extern volatile bool emergencia_ativa;

// ✅ Mutex para uso exclusivo do barramento I2C1
extern SemaphoreHandle_t i2c1_mutex;

// ✅ Verifica se o sensor está presente lendo o WHO_AM_I
bool mpu6500_check_whoami() {
    uint8_t reg = MPU6500_REG_WHO_AM_I;
    uint8_t id = 0;

    bool success = false;

    if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (i2c_write_blocking(i2c1, MPU6500_ADDR, &reg, 1, true) >= 0 &&
            i2c_read_blocking(i2c1, MPU6500_ADDR, &id, 1, false) >= 0) {
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

    while (1) {
        if (emergencia_ativa) {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        if (!mpu6500_check_whoami()) {
            if (sensor_conectado) {
                safe_printf("[MPU6500] ERRO: Sensor não detectado (falha WHO_AM_I).\n");
                sensor_conectado = false;
            }
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        } else if (!sensor_conectado) {
            safe_printf("[MPU6500] Sensor reconectado com sucesso (WHO_AM_I OK).\n");
            sensor_conectado = true;
        }

        mpu6500_data_t mpu_data;

        // ✅ Protege leitura com mutex
        if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            mpu6500_read_raw(i2c1, &mpu_data);
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
        if (delta_accel_z > LIMIAR_QUEDA_G || delta_accel_z < -LIMIAR_QUEDA_G) {
            safe_printf("[MPU6500] ALERTA: Possível queda detectada! (ΔZ: %.2fg)\n", delta_accel_z);
        }
        last_accel_z = accel_z;

        giros_z[idx++] = gyro_z;

        if (idx >= 100) {
            float soma = 0;
            for (int i = 0; i < 100; i++) soma += giros_z[i];
            float media_gyro_z = soma / 100.0f;

            safe_printf("[MPU6500] Média do Giroscópio Z: %.2f °/s\n", media_gyro_z);

            if (media_gyro_z > -0.5f && media_gyro_z < 0.5f) {
                contador_imobilidade += 100;
                if (contador_imobilidade >= AMOSTRAS_IMOBILIDADE) {
                    safe_printf("[MPU6500] ALERTA: Imobilidade ou inconsciência detectada!\n");
                    contador_imobilidade = 0;
                }
            } else {
                contador_imobilidade = 0;
            }

            idx = 0;
        }

        if (accel_x > 1.2f || accel_x < -1.2f || accel_y > 1.2f || accel_y < -1.2f) {
            contador_agitacao++;
        } else {
            contador_agitacao = 0;
        }

        if (contador_agitacao >= 5) {
            safe_printf("[MPU6500] ATENÇÃO: Atividade física intensa ou agitação detectada!\n");
            contador_agitacao = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // 10 Hz
    }
}
