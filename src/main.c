#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

// Inclusões dos headers das tasks
#include "config_geral.h"
#include "task_giroscopio_mpu6500.h"
#include "task_temperatura_aht10.h"
#include "task_distancia_vl53l0x.h"
#include "task_emergencia.h"
#include "task_wifi.h"
#include "task_oximetro_max30102.h"  // ✅ Nova task do oxímetro
#include "utils_print.h"

int main() {
    // Inicializa entrada/saída padrão (ex: USB serial)
    stdio_init_all();

    // Inicializa periféricos, barramentos, mutex, semáforos etc.
    config_geral_init();

    // Criação das tarefas do sistema
    xTaskCreate(task_giroscopio_mpu6500,   "MPU6500",   512,  NULL, 1, NULL);
    xTaskCreate(task_temperatura_aht10,    "AHT10",     512,  NULL, 1, NULL);
    xTaskCreate(task_distancia_vl53l0x,    "VL53L0X",   512,  NULL, 1, NULL);
    xTaskCreate(task_emergencia,           "EMERG",     512,  NULL, 2, NULL);
    xTaskCreate(task_wifi,                 "WiFi",     1024,  NULL, 3, NULL);
    xTaskCreate(task_oximetro_max30102, "MAX30102", 2048, NULL, 1, NULL);



    // Inicia o agendador do FreeRTOS
    vTaskStartScheduler();

    // Nunca deve chegar aqui
    while (1);
}