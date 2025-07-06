#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "task_distancia_vl53l0x.h"
#include "config_geral.h"
#include "utils_print.h"

extern SemaphoreHandle_t i2c1_mutex;
extern vl53l0x_dev vl53;  // DECLARAÇÃO (externa)


void task_distancia_vl53l0x(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(100))) {
            uint16_t distance = vl53l0x_read_range_continuous_millimeters(&vl53);

            if (distance == 65535) {
                printf("[VL53L0X] Erro na leitura ou timeout.\n");
            } else if (distance > 300) {
                printf("[VL53L0X] Fora de alcance (>30 cm).\n");
            } else {
                printf("[VL53L0X] Distância: %d mm\n", distance);
                if (distance < 200) {
                    printf("[ALERTA] Objeto próximo detectado!\n");
                }
            }

            xSemaphoreGive(i2c1_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
