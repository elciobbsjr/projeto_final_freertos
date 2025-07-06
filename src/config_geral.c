#include <stdio.h>
#include "config_geral.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "mpu6500.h"
#include "aht10.h"
#include "vl53l0x.h"
#include "task_emergencia.h"

// === Definições reais das variáveis globais ===
vl53l0x_dev vl53;
SemaphoreHandle_t i2c1_mutex;
SemaphoreHandle_t emergencia_semaforo;
volatile bool emergencia_ativa = false;



void config_geral_init(void) {
    // === Inicializa I2C0 (MPU6500) ===
    i2c_init(I2C0_PORT, 400 * 1000);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);

    print_mutex = xSemaphoreCreateMutex();

    
    // === Inicializa I2C1 (AHT10 + VL53L0X) ===
    i2c_init(I2C1_PORT, 100 * 1000);
    gpio_set_function(I2C1_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SDA_PIN);
    gpio_pull_up(I2C1_SCL_PIN);

    // === Inicialização dos botões ===
    gpio_init(BOTAO_A_PIN);
    gpio_set_dir(BOTAO_A_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_A_PIN);

    gpio_init(BOTAO_B_PIN);
    gpio_set_dir(BOTAO_B_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_B_PIN);

    // === Inicialização do LED vermelho ===
    gpio_init(LED_VERMELHO_PIN);
    gpio_set_dir(LED_VERMELHO_PIN, GPIO_OUT);
    gpio_put(LED_VERMELHO_PIN, 0);

    // === Inicialização do buzzer A ===
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);

    // === Semáforos ===
    i2c1_mutex = xSemaphoreCreateMutex();
    emergencia_semaforo = xSemaphoreCreateBinary();

    // === IRQs ===
    gpio_set_irq_enabled_with_callback(BOTAO_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BOTAO_B_PIN, GPIO_IRQ_EDGE_FALL, true);

    // === Inicialização dos sensores ===
    mpu6500_init(I2C0_PORT);

    if (!aht10_init(I2C1_PORT)) {
        printf("[ERRO] Falha na inicialização do AHT10!\n");
    } else {
        printf("[OK] AHT10 inicializado.\n");
    }

    if (!vl53l0x_init(&vl53, I2C1_PORT)) {
        printf("[ERRO] VL53L0X falhou na inicialização.\n");
    } else {
        printf("[OK] VL53L0X inicializado.\n");
        vl53l0x_start_continuous(&vl53, 0);
    }
}

