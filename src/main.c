#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "FreeRTOS.h"
#include "task.h"

// Inclusões dos headers das tasks e configurações
#include "config_geral.h"
#include "task_giroscopio_mpu6500.h"
#include "task_temperatura_aht10.h"
#include "task_distancia_vl53l0x.h"
#include "task_emergencia.h"
#include "task_wifi.h"
#include "task_oximetro_max30102.h"
#include "task_mqtt.h"             // ✅ Adiciona a task do MQTT
#include "utils_print.h"
#include "task_telegram.h"

// Função para inicializar PWM no buzzer
void buzzer_pwm_init(uint gpio_pin) {
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio_pin);

    pwm_set_wrap(slice_num, 12500);      // Define frequência (~1 kHz)
    pwm_set_gpio_level(gpio_pin, 0);     // Começa desligado
    pwm_set_enabled(slice_num, true);    // Ativa PWM
}

int main() {
    // Inicializa a comunicação padrão (USB serial)
    stdio_init_all();

    // Inicializa periféricos, GPIOs, I2C, semáforos etc.
    config_geral_init();

    // Inicializa PWM do buzzer
    buzzer_pwm_init(BUZZER_PIN);

    // Criação das tarefas do sistema
    xTaskCreate(task_giroscopio_mpu6500, "MPU6500",   512,  NULL, 1, NULL);
    xTaskCreate(task_temperatura_aht10,  "AHT10",     512,  NULL, 1, NULL);
    xTaskCreate(task_distancia_vl53l0x,  "VL53L0X",   512,  NULL, 1, NULL);
    xTaskCreate(task_emergencia,         "EMERG",     512,  NULL, 2, NULL);
    xTaskCreate(task_wifi,               "WiFi",     1024,  NULL, 3, NULL);
    xTaskCreate(task_oximetro_max30102,  "MAX30102", 2048,  NULL, 1, NULL);
    xTaskCreate(tarefa_mqtt,             "MQTT",     2048,  NULL, 2, NULL);  // ✅ Task MQTT adicionada
    xTaskCreate(tarefa_telegram_teste, "Telegram", 2048, NULL, 1, NULL);


    // Inicia o escalonador do FreeRTOS
    vTaskStartScheduler();

    // Nunca deve chegar aqui
    while (1);
}
