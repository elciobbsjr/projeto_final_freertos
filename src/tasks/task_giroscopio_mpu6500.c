#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "mpu6500.h"
#include "hardware/i2c.h"
#include "config_geral.h"
#include "utils_print.h"

#define LIMIAR_QUEDA_G 1.5f
#define TEMPO_IMOBILIDADE_MIN 2
#define AMOSTRAS_IMOBILIDADE (TEMPO_IMOBILIDADE_MIN * 600) // (10 min * 60 seg/min) * (1000ms/100ms)

void task_giroscopio_mpu6500(void *pvParameters) {
    static float giros_z[100];
    static int idx = 0;

    float last_accel_z = 0.0f;
    uint32_t contador_imobilidade = 0;
    uint32_t contador_agitacao = 0;

    while (1) {
        mpu6500_data_t mpu_data;
        mpu6500_read_raw(i2c0, &mpu_data);

        float gyro_z = mpu_data.gyro[2] / 131.0f;
        float accel_x = mpu_data.accel[0] / 16384.0f;
        float accel_y = mpu_data.accel[1] / 16384.0f;
        float accel_z = mpu_data.accel[2] / 16384.0f;

        // 🧠 Regra 1: Monitoramento de queda
        // Se o eixo Z do acelerômetro tiver variação súbita superior a ±1.5g em menos de 500ms → possível queda detectada.
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

            // 🧠 Regra 2: Monitoramento de imobilidade ou inconsciência
            // Se o giroscópio Z ficar próximo de 0°/s por mais de 10 minutos → possível imobilidade ou inconsciência.
            if (media_gyro_z > -0.5f && media_gyro_z < 0.5f) {
                contador_imobilidade += 100; // Cada 100 leituras equivalem a 10 segundos
                if (contador_imobilidade >= AMOSTRAS_IMOBILIDADE) {
                    safe_printf("[MPU6500] ALERTA: Imobilidade ou inconsciência detectada!\n");
                    contador_imobilidade = 0; // Reinicia após alerta
                }
            } else {
                contador_imobilidade = 0; // Reset contador se movimento detectado
            }

            idx = 0;
        }

        // 🧠 Regra 3: Monitoramento de atividade física intensa ou agitação
        // Repetidas acelerações/desacelerações rápidas (em X ou Y) → atividade física intensa ou agitação.
        if (accel_x > 1.2f || accel_x < -1.2f || accel_y > 1.2f || accel_y < -1.2f) {
            contador_agitacao++;
        } else {
            contador_agitacao = 0; // Reset contador se aceleração voltar ao normal
        }

        if (contador_agitacao >= 5) { // 5 leituras consecutivas = 500ms
            safe_printf("[MPU6500] ATENÇÃO: Atividade física intensa ou agitação detectada!\n");
            contador_agitacao = 0; // Reinicia após alerta
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
