#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "task_distancia_vl53l0x.h"
#include "config_geral.h"
#include "utils_print.h"
#include <stdlib.h>


extern SemaphoreHandle_t i2c1_mutex;
extern vl53l0x_dev vl53;

void task_distancia_vl53l0x(void *pvParameters) {
    static uint16_t ultima_distancia = 0;
    static bool erro_anterior = false;
    static bool alerta_anterior = false;

    while (1) {
        if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(100))) {
            uint16_t distancia = vl53l0x_read_range_continuous_millimeters(&vl53);

            if (distancia == 65535) {
                if (!erro_anterior) {
                    safe_printf("[VL53L0X] Erro na leitura ou timeout.\n");
                    erro_anterior = true;
                }
            } else {
                erro_anterior = false;

                if (distancia > 300) {
                    safe_printf("[VL53L0X] Fora de alcance (>30 cm).\n");
                } else if (abs(distancia - ultima_distancia) > 10) {
                    safe_printf("[VL53L0X] Distância: %d mm\n", distancia);
                    ultima_distancia = distancia;
                }

                if (distancia < 200 && !alerta_anterior) {
                    safe_printf("[ALERTA] Objeto próximo detectado!\n");
                    alerta_anterior = true;
                } else if (distancia >= 200 && alerta_anterior) {
                    safe_printf("[INFO] Objeto afastado.\n");
                    alerta_anterior = false;
                }
            }

            xSemaphoreGive(i2c1_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
