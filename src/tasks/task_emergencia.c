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
#include "mqtt_lwip.h"  // ✅ Para envio direto via MQTT

// Declaração externa do semáforo e flag
extern SemaphoreHandle_t emergencia_semaforo;
extern volatile bool emergencia_ativa;

void task_emergencia(void *pvParameters) {
    const uint pwm_duty_on = 6250;  // 50% duty cycle
    const uint pwm_duty_off = 0;

    while (1) {
        if (xSemaphoreTake(emergencia_semaforo, portMAX_DELAY)) {
            printf("[EMERGÊNCIA] Botão de emergência acionado!\n");

            // ✅ Alerta imediato de entrada em emergência
            if (cliente_mqtt_esta_conectado()) {
                const char *mensagem = "🛑 EMERGÊNCIA: Botão A acionado! Sistema em estado crítico.";
                publicar_mensagem_mqtt("/notificacoes/telegram", mensagem);
                safe_printf("[MQTT] Alerta de emergência enviado.\n");
            } else {
                safe_printf("[MQTT] Alerta de emergência não enviado (sem conexão).\n");
            }

            // LED e buzzer piscando
            while (emergencia_ativa) {
                gpio_put(LED_VERMELHO_PIN, 1);
                pwm_set_gpio_level(BUZZER_PIN, pwm_duty_on);
                vTaskDelay(pdMS_TO_TICKS(200));

                gpio_put(LED_VERMELHO_PIN, 0);
                pwm_set_gpio_level(BUZZER_PIN, pwm_duty_off);
                vTaskDelay(pdMS_TO_TICKS(200));
            }

            // Finaliza emergência
            gpio_put(LED_VERMELHO_PIN, 0);
            pwm_set_gpio_level(BUZZER_PIN, pwm_duty_off);
            printf("[SAÍDA DO MODO DE EMERGÊNCIA]\n");

            // ✅ Alerta imediato de saída da emergência
            if (cliente_mqtt_esta_conectado()) {
                const char *mensagem_saida = "✅ Modo de emergência cancelado. Sistema voltou ao normal.";
                publicar_mensagem_mqtt("/notificacoes/telegram", mensagem_saida);
                safe_printf("[MQTT] Aviso de saída da emergência enviado.\n");
            } else {
                safe_printf("[MQTT] Saída da emergência não enviada (sem conexão).\n");
            }
        }
    }
}

// === Interrupção dos botões ===
void gpio_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (gpio == BOTAO_A_PIN && !emergencia_ativa) {
        emergencia_ativa = true;
        xSemaphoreGiveFromISR(emergencia_semaforo, &xHigherPriorityTaskWoken);
    } else if (gpio == BOTAO_B_PIN && emergencia_ativa) {
        emergencia_ativa = false;

        gpio_put(LED_VERMELHO_PIN, 0);
        pwm_set_gpio_level(BUZZER_PIN, 0);
        printf("[SAÍDA DO MODO DE EMERGÊNCIA]\n");
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
