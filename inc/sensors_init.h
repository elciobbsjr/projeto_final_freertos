#ifndef SENSORS_INIT_H
#define SENSORS_INIT_H

#include "FreeRTOS.h"
#include "semphr.h"

void sensors_init(SemaphoreHandle_t *mutex);

#endif
