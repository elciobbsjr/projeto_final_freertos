#ifndef TASK_DISTANCIA_VL53L0X_H
#define TASK_DISTANCIA_VL53L0X_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "vl53l0x.h"

extern vl53l0x_dev vl53;
extern SemaphoreHandle_t i2c1_mutex;

void task_distancia_vl53l0x(void *pvParameters);

#endif
