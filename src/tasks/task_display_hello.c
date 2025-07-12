#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "config_geral.h"

void task_display_hello(void *pvParameters) {
    sleep_ms(500);

    while (1) {
        memset(oled.ram_buffer + 1, 0, oled.bufsize - 1);
        ssd1306_draw_string(oled.ram_buffer + 1, 20, 30, "Ola, mundo!");

        ssd1306_command(&oled, ssd1306_set_column_address);
        ssd1306_command(&oled, 0);
        ssd1306_command(&oled, OLED_WIDTH - 1);
        ssd1306_command(&oled, ssd1306_set_page_address);
        ssd1306_command(&oled, 0);
        ssd1306_command(&oled, (OLED_HEIGHT / 8) - 1);

        ssd1306_send_data(&oled);

        printf("[Display] Tela atualizada com 'Olá, mundo!'\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
