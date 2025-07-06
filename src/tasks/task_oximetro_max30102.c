#include "task_oximetro_max30102.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <math.h>
#include "utils_print.h"

#define I2C_PORT i2c0
#define MAX30102_ADDR 0x57

static void write_reg(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    int wr = i2c_write_blocking(I2C_PORT, MAX30102_ADDR, buf, 2, false);
    if (wr < 0) {
        printf("[MAX30102] Erro ao escrever reg 0x%02X (wr = %d)\n", reg, wr);
    }
}

static bool read_fifo(uint32_t *ir, uint32_t *red) {
    uint8_t reg = 0x07;
    uint8_t buf[6];
    if (i2c_write_blocking(I2C_PORT, MAX30102_ADDR, &reg, 1, true) < 0) return false;
    if (i2c_read_blocking(I2C_PORT, MAX30102_ADDR, buf, 6, false) < 0) return false;

    *red = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];
    *ir  = ((uint32_t)buf[3] << 16) | ((uint32_t)buf[4] << 8) | buf[5];
    return true;
}

static void max30102_init() {
    write_reg(0x09, 0x00); vTaskDelay(pdMS_TO_TICKS(50));
    write_reg(0x09, 0x03); // SpO2 mode
    write_reg(0x0A, 0x1F); // 100 Hz, 16-bit
    write_reg(0x0C, 0x24); // LED1 RED
    write_reg(0x0D, 0x24); // LED2 IR
    write_reg(0x0F, 0x00); // Slot config
    write_reg(0x08, 0x0F); // FIFO config
}

void task_oximetro_max30102(void *params) {
    vTaskDelay(pdMS_TO_TICKS(1000));  // Aguarda USB / sistema

    max30102_init();
    vTaskDelay(pdMS_TO_TICKS(100));

    printf("Iniciando leitura do MAX30102...\n");

    uint32_t ir = 0, red = 0;

    while (1) {
        if (read_fifo(&ir, &red)) {
            printf("IR: %lu | RED: %lu\n", ir, red);
        } else {
            printf("Erro ao ler FIFO do MAX30102\n");
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
