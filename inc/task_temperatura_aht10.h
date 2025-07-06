#ifndef TASK_TEMPERATURA_AHT10_H
#define TASK_TEMPERATURA_AHT10_H

#include "FreeRTOS.h"
#include "semphr.h"

extern SemaphoreHandle_t i2c1_mutex;

void task_temperatura_aht10(void *pvParameters);

#endif
