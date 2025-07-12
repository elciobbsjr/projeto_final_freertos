#ifndef CONFIG_GERAL_H
#define CONFIG_GERAL_H
/**
 * @file config_geral.h
 * @brief Centraliza pinos de hardware, mutex globais, Wi-Fi e MQTT.
 *
 *  ⚠️  NÃO versionar credenciais reais em repositórios públicos!
 */

/* ======== INCLUDES ========= */
#include "FreeRTOS.h"
#include "semphr.h"
#include "hardware/i2c.h"
#include "vl53l0x.h"

/* ======== HARDWARE PINOUT ======== */
/* I2C0 – Sensores internos (ex.: MPU6500) */
#define I2C0_PORT           i2c0
#define I2C0_SDA_PIN        0
#define I2C0_SCL_PIN        1
/* I2C1 – Sensores externos (ex.: VL53L0X) */
#define I2C1_PORT           i2c1
#define I2C1_SDA_PIN        2
#define I2C1_SCL_PIN        3

/* Botões */
#define BOTAO_A_PIN         5
#define BOTAO_B_PIN         6

/* LED RGB discreto */
#define LED_VERMELHO_PIN    13
#define LED_VERDE_PIN       11
#define LED_AZUL_PIN        12

/* Buzzer */
#define BUZZER_PIN          21      /* PWM via transistor */
#define BUZZER_B_PIN        10      /* Saída direta */
#define BUZZER_PWM_SLICE    pwm_gpio_to_slice_num(BUZZER_PIN)

/* ======== REDE WI-FI ======== */
#define WIFI_SSID           "ELCIO J"
#define WIFI_PASSWORD       "elc10jun10r"

/* ======== MQTT ======== */
#define MQTT_BROKER_IP      "192.168.1.101"
#define MQTT_BROKER_PORT    1883
#define DEVICE_ID           "LumiConnect"

/* — Tópicos de publicação (adicione novos sensores conforme necessidade) — */
#define TOPICO_LUZ          "dados/luminosidade"
// #define TOPICO_TEMPERATURA   "dados/temperatura"
// #define TOPICO_GAS          "dados/gas"

/* ======== VARIÁVEIS GLOBAIS (extern) ======== */
extern SemaphoreHandle_t print_mutex;
extern SemaphoreHandle_t i2c1_mutex;
extern vl53l0x_dev       vl53;
extern volatile bool     emergencia_ativa;

/* ======== PROTÓTIPOS ======== */
void config_geral_init(void);

#endif /* CONFIG_GERAL_H */
