#include <stdarg.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "semphr.h"

extern SemaphoreHandle_t print_mutex;

void safe_printf(const char *fmt, ...) {
    if (print_mutex != NULL) {
        xSemaphoreTake(print_mutex, portMAX_DELAY);
    }

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    if (print_mutex != NULL) {
        xSemaphoreGive(print_mutex);
    }
}
