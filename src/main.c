#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#include "config_geral.h"
#include "task_giroscopio_mpu6500.h"
#include "task_temperatura_aht10.h"
#include "task_distancia_vl53l0x.h"
#include "task_emergencia.h"  // ✅ Inclua a task de emergência
#include "task_wifi.h"  // 👈 Inclua sua nova task Wi-Fi

int main() {
    stdio_init_all();

    config_geral_init();  // Inicializa I2C, GPIOs, sensores, mutex, semáforos

    // ✅ Criação das tasks
    xTaskCreate(task_giroscopio_mpu6500, "MPU6500", 512, NULL, 1, NULL);
    xTaskCreate(task_temperatura_aht10,  "AHT10",   512, NULL, 1, NULL);
    xTaskCreate(task_distancia_vl53l0x,  "VL53L0X", 512, NULL, 1, NULL);
    xTaskCreate(task_emergencia,         "EMERG",   512, NULL, 2, NULL);  // ✅ Adicionada
    xTaskCreate(task_wifi, "WiFi", 1024, NULL, 3, NULL);  // prioridade maior, se quiser

    

    vTaskStartScheduler();

    while (1);  // Nunca deve chegar aqui
}
