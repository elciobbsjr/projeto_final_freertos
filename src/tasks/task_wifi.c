#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pico/cyw43_arch.h"
#include "utils_print.h"
#include "config_geral.h"  // ✅ Agora importa as configurações centralizadas

void task_wifi(void *pvParameters) {
    if (cyw43_arch_init()) {
        printf("❌ Falha ao inicializar Wi-Fi\n");
        vTaskDelete(NULL);
        return;
    }

    cyw43_arch_enable_sta_mode();

    printf("🌐 Conectando ao Wi-Fi SSID: %s\n", WIFI_SSID);
    int result = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID, WIFI_PASSWORD,
        CYW43_AUTH_WPA2_AES_PSK,
        10000
    );

    if (result == 0) {
        printf("✅ Conectado ao Wi-Fi!\n");
    } else {
        printf("❌ Erro ao conectar. Código: %d\n", result);
    }

    while (true) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
