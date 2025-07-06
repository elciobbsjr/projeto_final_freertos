#ifndef TASK_EMERGENCIA_H
#define TASK_EMERGENCIA_H

#include <stdbool.h>

extern volatile bool emergencia_ativa;

void gpio_callback(uint gpio, uint32_t events);
void task_emergencia(void *pvParameters);

#endif
