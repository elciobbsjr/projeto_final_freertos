#ifndef TASK_ALERTAS_MQTT_H
#define TASK_ALERTAS_MQTT_H

/**
 * @file task_alertas_mqtt.h
 * @brief Responsável por enviar alertas MQTT a partir de uma fila.
 */

#ifdef __cplusplus
extern "C" {
#endif

// Protótipo da função da task
void task_alertas_mqtt(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif // TASK_ALERTAS_MQTT_H
