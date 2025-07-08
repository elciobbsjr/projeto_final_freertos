#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "utils_print.h"

#include "config_geral.h"
#include "task_emergencia.h"

// Declaração externa do semáforo e flag
extern SemaphoreHandle_t emergencia_semaforo;
extern volatile bool emergencia_ativa;  // Inicializa como false

// Callback de interrupção para botões
void gpio_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (gpio == BOTAO_A_PIN && !emergencia_ativa) {
        emergencia_ativa = true;
        xSemaphoreGiveFromISR(emergencia_semaforo, &xHigherPriorityTaskWoken);
    } else if (gpio == BOTAO_B_PIN && emergencia_ativa) {
        emergencia_ativa = false;
        gpio_put(LED_VERMELHO_PIN, 0);
        gpio_put(BUZZER_PIN, 0);
        printf("[SAÍDA DO MODO DE EMERGÊNCIA]\n");
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Task de emergência
void task_emergencia(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(emergencia_semaforo, portMAX_DELAY)) {
            printf("[EMERGÊNCIA] Botão de emergência acionado!\n");

            // LED e Buzzer piscando durante emergência
            while (emergencia_ativa) {
                gpio_put(LED_VERMELHO_PIN, 1);
                gpio_put(BUZZER_PIN, 1);
                vTaskDelay(pdMS_TO_TICKS(200));
                gpio_put(LED_VERMELHO_PIN, 0);
                gpio_put(BUZZER_PIN, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }

            // Garante que desliguem ao sair da emergência
            gpio_put(LED_VERMELHO_PIN, 0);
            gpio_put(BUZZER_PIN, 0);
        }
    }
}
