#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>
#include "config_geral.h"
#include "mqtt_lwip.h"
#include "utils_print.h"

void task_alertas_mqtt(void *pvParameters) {
    char alerta[128];

    while (1) {
        // Espera por mensagens na fila
        if (xQueueReceive(fila_alertas_mqtt, &alerta, portMAX_DELAY) == pdTRUE) {
            if (cliente_mqtt_esta_conectado()) {
                publicar_mensagem_mqtt("/notificacoes/telegram", alerta);
                safe_printf("[MQTT] Alerta enviado: %s\n", alerta);
            } else {
                safe_printf("[MQTT] MQTT desconectado. Alerta descartado: %s\n", alerta);
            }

            vTaskDelay(pdMS_TO_TICKS(500));  // Delay para evitar congestão
        }
    }
}
