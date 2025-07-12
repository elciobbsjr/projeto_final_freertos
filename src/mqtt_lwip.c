#include "config_geral.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>

// Ponteiro para o cliente MQTT
static mqtt_client_t *client;
static bool publicacao_em_andamento = false;

// === CALLBACK: Conexão MQTT ===
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    switch (status) {
        case MQTT_CONNECT_ACCEPTED:
            printf("[MQTT] ✅ Conectado ao broker! (Modo Publicador)\n");
            break;
        case MQTT_CONNECT_REFUSED_PROTOCOL_VERSION:
            printf("[MQTT] ❌ Erro: Versão de protocolo MQTT não suportada\n");
            break;
        case MQTT_CONNECT_REFUSED_IDENTIFIER:
            printf("[MQTT] ❌ Erro: ID de cliente rejeitado\n");
            break;
        case MQTT_CONNECT_REFUSED_SERVER:
            printf("[MQTT] ❌ Erro: Servidor indisponível\n");
            break;
        case MQTT_CONNECT_REFUSED_USERNAME_PASS:
            printf("[MQTT] ❌ Erro: Falta usuário/senha (não suportado no firmware atual)\n");
            break;
        case MQTT_CONNECT_REFUSED_NOT_AUTHORIZED_:
            printf("[MQTT] ❌ Erro: Conexão não autorizada\n");
            break;
        default:
            printf("[MQTT] ❌ Erro desconhecido. Código: %d\n", status);
            break;
    }
}

// === CALLBACK: Publicação ===
static void mqtt_pub_request_cb(void *arg, err_t err) {
    publicacao_em_andamento = false;
}

// === Publicador MQTT ===
void publicar_mensagem_mqtt(const char *topico, const char *mensagem) {
    if (!client || !mqtt_client_is_connected(client) || publicacao_em_andamento) {
        return;
    }

    err_t err = mqtt_publish(client, topico, mensagem, strlen(mensagem), 1, 0, mqtt_pub_request_cb, NULL);

    if (err == ERR_OK) {
        publicacao_em_andamento = true;
    } else {
        printf("[MQTT] ❌ Erro ao publicar: %d\n", err);
    }
}

// === Inicializador MQTT ===
void iniciar_mqtt_cliente(void) {
    ip_addr_t broker_ip;
    ip4addr_aton(MQTT_BROKER_IP, &broker_ip);

    printf("[MQTT] Inicializando cliente...\n");
    printf("[MQTT] IP do broker: %s\n", MQTT_BROKER_IP);

    client = mqtt_client_new();
    if (!client) {
        printf("[MQTT] ❌ Erro ao alocar mqtt_client_t\n");
        return;
    }

    char client_id[32];
    snprintf(client_id, sizeof(client_id), "%s_client", DEVICE_ID);

    struct mqtt_connect_client_info_t ci = { .client_id = client_id };

    mqtt_client_connect(client, &broker_ip, MQTT_BROKER_PORT, mqtt_connection_cb, 0, &ci);
}

// === Verificador de conexão ===
bool cliente_mqtt_esta_conectado(void) {
    return client && mqtt_client_is_connected(client);
}
