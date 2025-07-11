#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "utils_print.h"

#include "config_geral.h"
#include "task_emergencia.h"

// Declaração externa do semáforo e flag
extern SemaphoreHandle_t emergencia_semaforo;
extern volatile bool emergencia_ativa;  // Inicializa como false

// Task de emergência com buzzer em PWM
void task_emergencia(void *pvParameters) {
    const uint pwm_duty_on = 6250;  // 50% duty cycle (metade de 12500)
    const uint pwm_duty_off = 0;

    while (1) {
        if (xSemaphoreTake(emergencia_semaforo, portMAX_DELAY)) {
            printf("[EMERGÊNCIA] Botão de emergência acionado!\n");

            // LED e Buzzer piscando durante emergência
            while (emergencia_ativa) {
                gpio_put(LED_VERMELHO_PIN, 1);
                pwm_set_gpio_level(BUZZER_PIN, pwm_duty_on);  // Liga buzzer (PWM)
                vTaskDelay(pdMS_TO_TICKS(200));

                gpio_put(LED_VERMELHO_PIN, 0);
                pwm_set_gpio_level(BUZZER_PIN, pwm_duty_off); // Desliga buzzer
                vTaskDelay(pdMS_TO_TICKS(200));
            }

            // Garante que desliguem ao sair da emergência
            gpio_put(LED_VERMELHO_PIN, 0);
            pwm_set_gpio_level(BUZZER_PIN, pwm_duty_off);
            printf("[SAÍDA DO MODO DE EMERGÊNCIA]\n");
        }
    }
}

// === Implementação da função de interrupção ===
void gpio_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (gpio == BOTAO_A_PIN && !emergencia_ativa) {
        emergencia_ativa = true;
        xSemaphoreGiveFromISR(emergencia_semaforo, &xHigherPriorityTaskWoken);
    } else if (gpio == BOTAO_B_PIN && emergencia_ativa) {
        emergencia_ativa = false;
        gpio_put(LED_VERMELHO_PIN, 0);
        pwm_set_gpio_level(BUZZER_PIN, 0);  // Desliga buzzer via PWM
        printf("[SAÍDA DO MODO DE EMERGÊNCIA]\n");
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
