#include "FreeRTOS.h"
#include "task.h"
#include "mqtt_lwip.h"
#include <stdio.h>

void tarefa_telegram_teste(void *pvParameters) {
    // Aguarda Wi-Fi e MQTT estarem prontos
    printf("[TELEGRAM] Aguardando Wi-Fi e MQTT...\n");

    // Espera até o MQTT estar conectado
    while (!cliente_mqtt_esta_conectado()) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    printf("[TELEGRAM] Conectado ao MQTT. Enviando mensagem de teste...\n");

    const char *topico = "/notificacoes/telegram";
    const char *mensagem = "✅ Teste: Pico W publicou via MQTT para o bot do Telegram!";

    publicar_mensagem_mqtt(topico, mensagem);

    // Aguarda um pouco para garantir envio antes de deletar
    vTaskDelay(pdMS_TO_TICKS(1000));

    printf("[TELEGRAM] Mensagem enviada. Finalizando task.\n");

    vTaskDelete(NULL);  // Task termina após enviar a mensagem
}
