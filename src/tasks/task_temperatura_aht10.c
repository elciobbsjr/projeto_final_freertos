#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "aht10.h"
#include "task_temperatura_aht10.h"
#include "utils_print.h"

aht10_data_t latest_aht10 = {0};
extern SemaphoreHandle_t i2c1_mutex;

void task_temperatura_aht10(void *pvParameters) {
    static float temps[12];  // 12 leituras de 5s = 60s
    static int temp_idx = 0;

    while (1) {
        if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(100))) {
            aht10_data_t data;
            if (aht10_read_data(i2c1, &data)) {
                latest_aht10 = data;

                safe_printf("[AHT10] Temperatura: %.2f °C | Umidade: %.2f %%\n",
                            data.temperature, data.humidity);

                if (data.temperature > -40.0f && data.temperature < 85.0f) {
                    temps[temp_idx++] = data.temperature;
                }

                if (temp_idx >= 12) {  // A cada 1 min (12x5s)
                    float soma = 0;
                    for (int i = 0; i < 12; i++) soma += temps[i];
                    float media = soma / 12.0f;
                    safe_printf("[AHT10] Média (último 1 min): %.2f °C\n", media);
                    temp_idx = 0;
                }

            } else {
                safe_printf("[AHT10] Falha na leitura.\n");
            }

            xSemaphoreGive(i2c1_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));  // Leitura a cada 5s
    }
}
