#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pico/cyw43_arch.h"

void task_wifi(void *pvParameters) {
    if (cyw43_arch_init()) {
        printf("❌ Falha ao inicializar Wi-Fi\n");
        vTaskDelete(NULL);
        return;
    }

    cyw43_arch_enable_sta_mode();

    printf("🌐 Conectando ao Wi-Fi...\n");
    int result = cyw43_arch_wifi_connect_timeout_ms("ELCIO J", "elc10jun10r", CYW43_AUTH_WPA2_AES_PSK, 10000);

    if (result == 0) {
        printf("✅ Conectado ao Wi-Fi!\n");
    } else {
        printf("❌ Erro ao conectar. Código: %d\n", result);
    }

    // Wi-Fi conectado, pode agora manter a task viva ou deletá-la
    while (true) {
        // Exemplo: piscar LED indicando status
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
