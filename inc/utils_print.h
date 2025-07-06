#ifndef UTILS_PRINT_H
#define UTILS_PRINT_H
#include "FreeRTOS.h"
#include "semphr.h"


#include <stdarg.h>

// Mutex externo
extern SemaphoreHandle_t print_mutex;

// Função de print segura
void safe_printf(const char *fmt, ...);

// Redefine printf para a versão segura
#define printf(...) safe_printf(__VA_ARGS__)

#endif
