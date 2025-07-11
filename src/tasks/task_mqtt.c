#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "mqtt_lwip.h"
#include "config_geral.h"
#include "pico/cyw43_arch.h"  // Necessário para verificar status do Wi-Fi

void tarefa_mqtt(void *pvParameters) {
    printf("[MQTT] Aguardando Wi-Fi se conectar...\n");

    // Aguarda até o Wi-Fi estar conectado (bloqueante)
    while (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_UP) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    printf("[MQTT] Wi-Fi conectado, inicializando cliente MQTT...\n");
    iniciar_mqtt_cliente();

    while (true) {
        if (cliente_mqtt_esta_conectado()) {
            printf("[MQTT] ✅ Conectado ao broker MQTT em %s:%d\n", MQTT_BROKER_IP, MQTT_BROKER_PORT);
        } else {
            printf("[MQTT] ❌ Desconectado do broker. Tentando manter conexão...\n");
        }

        vTaskDelay(pdMS_TO_TICKS(5000));  // Verifica a cada 5 segundos
    }
}
