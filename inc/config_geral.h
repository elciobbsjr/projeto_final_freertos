#ifndef CONFIG_GERAL_H
#define CONFIG_GERAL_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "hardware/i2c.h"
#include "vl53l0x.h"

// === I2C (Sensores) ===
#define I2C0_PORT           i2c0
#define I2C0_SDA_PIN        8
#define I2C0_SCL_PIN        9

#define I2C1_PORT           i2c1
#define I2C1_SDA_PIN        2
#define I2C1_SCL_PIN        3

// === Botões ===
#define BOTAO_A_PIN         5
#define BOTAO_B_PIN         6

// === LED RGB ===
#define LED_VERMELHO_PIN    13
#define LED_VERDE_PIN       11
#define LED_AZUL_PIN        12

// === Buzzer ===
#define BUZZER_PIN          21  // Buzzer A via transistor

// === Variáveis globais ===
extern vl53l0x_dev vl53;
extern SemaphoreHandle_t i2c1_mutex;
extern volatile bool emergencia_ativa;

// === Inicialização geral do sistema ===
void config_geral_init(void);

#endif
