#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "aht10.h"
#include "task_temperatura_aht10.h"

aht10_data_t latest_aht10 = {0};
extern SemaphoreHandle_t i2c1_mutex;

void task_temperatura_aht10(void *pvParameters) {
    static float temps[100];
    static int temp_idx = 0;

    while (1) {
        if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(100))) {
            aht10_data_t data;
            if (aht10_read_data(i2c1, &data)) {
                latest_aht10 = data;

                printf("[DEBUG AHT10] Temp: %.2f C | Umid: %.2f%%\n", data.temperature, data.humidity);

                if (data.temperature > -40.0f && data.temperature < 85.0f) {
                    temps[temp_idx++] = data.temperature;
                }

                if (temp_idx >= 100) {
                    float soma = 0;
                    for (int i = 0; i < 100; i++) soma += temps[i];
                    float media = soma / 100.0f;
                    printf("[AHT10] Média da Temperatura: %.2f °C\n", media);
                    temp_idx = 0;
                }

            } else {
                printf("[ERRO] Falha na leitura do AHT10!\n");
            }

            xSemaphoreGive(i2c1_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
