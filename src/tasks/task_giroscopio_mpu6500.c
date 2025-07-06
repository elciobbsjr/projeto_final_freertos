#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "mpu6500.h"
#include "hardware/i2c.h"
#include "config_geral.h"
#include "utils_print.h"  // safe_printf

void task_giroscopio_mpu6500(void *pvParameters) {
    static float giros_z[100];
    static int idx = 0;

    while (1) {
        mpu6500_data_t mpu_data;
        mpu6500_read_raw(i2c0, &mpu_data);

        float gyro_z = mpu_data.gyro[2] / 131.0f;
        giros_z[idx++] = gyro_z;

        if (idx >= 100) {
            float soma = 0;
            for (int i = 0; i < 100; i++) soma += giros_z[i];
            float media = soma / 100.0f;

            // ✅ Usa printf protegido por mutex
            safe_printf("[MPU6500] Média do Giroscópio Z: %.2f °/s\n", media);

            idx = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
