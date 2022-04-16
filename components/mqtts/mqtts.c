#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "mqtts.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "cJSON_Utils.h"

static const char *TAG = "MQTTS_EXAMPLE";

extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_crt_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_crt_end");

const char *pubtopic_create_certificate = "$aws/certificates/create/json";
const char *subtopic_create_certificate = "$aws/certificates/create/json/accepted";
const char *pubtopic_provision_template = "$aws/provisioning-templates/Fleet_Template/provision/json";
const char *subtopic_provision_template = "$aws/provisioning-templates/Fleet_Template/provision/json/accepted";
const char *subtopic_provision_template_fail = "$aws/provisioning-templates/Fleet_Template/provision/json/rejected";

static char json_certificate_str[JSON_CERTIFICATE_LEN];
static char client_cert[CLIENT_CERT_MAX_LEN];
static uint16_t client_cert_len;
static char client_key[CLIENT_KEY_MAX_LEN];
static uint16_t client_key_len;
static cJSON *certificateOwnershipToken;
static cJSON *client_cert_json;
static cJSON *client_key_json;
static cJSON *json_certificate;
static cJSON *json_register_thing;
static cJSON *parameter;
nvs_handle_t certificate_nvs_handler;
static uint8_t certificate_status;
uint8_t count = 0;

static uint8_t check_certificate()
{
    esp_err_t err;
    err = nvs_open("storage", NVS_READWRITE, &certificate_nvs_handler);
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(__FUNCTION__, "nvs_open done\n");
        err = nvs_get_str(certificate_nvs_handler, "client_cert", client_cert, &client_cert_len);
        switch (err)
        {
        case ESP_OK:
            /* code */
            ESP_LOGI(__FUNCTION__, "Flash have Client Cert");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(__FUNCTION__, "Flash don't have Client Cert");
            return 1;
        default:
            break;
        }

        err = nvs_get_str(certificate_nvs_handler, "client_key", client_key, &client_key_len);
        switch (err)
        {
        case ESP_OK:
            /* code */
            ESP_LOGI(__FUNCTION__, "Flash have Client Cert");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(__FUNCTION__, "Flash don't have Client Cert");
            return 1;
        default:
            break;
        }
        err = nvs_commit(certificate_nvs_handler);
        if (err != ESP_OK)
        {
            ESP_LOGE(__FUNCTION__, "Commit change fail\n");
        }
        nvs_close(certificate_nvs_handler);
    }
    return 0;
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        count = 0;
        if (certificate_status == 0)
        {
            msg_id = esp_mqtt_client_unsubscribe(client, subtopic_create_certificate);
            ESP_LOGI(TAG, "sent unsubscribe successful, topic=%s", subtopic_create_certificate);
            msg_id = esp_mqtt_client_unsubscribe(client, subtopic_provision_template);
            ESP_LOGI(TAG, "sent unsubscribe successful, topic=%s", subtopic_provision_template);
            msg_id = esp_mqtt_client_unsubscribe(client, subtopic_provision_template_fail);
            ESP_LOGI(TAG, "sent unsubscribe successful, topic=%s", subtopic_provision_template_fail);
            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        }
        else
        {
            msg_id = esp_mqtt_client_subscribe(client, subtopic_create_certificate, 1);
            ESP_LOGI(TAG, "sent subscribe successful, topic=%s", subtopic_create_certificate);
            msg_id = esp_mqtt_client_subscribe(client, subtopic_provision_template, 1);
            ESP_LOGI(TAG, "sent subscribe successful, topic=%s", subtopic_provision_template);
            msg_id = esp_mqtt_client_subscribe(client, subtopic_provision_template_fail, 1);
            ESP_LOGI(TAG, "sent subscribe successful, topic=%s", subtopic_provision_template_fail);
        }
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED\n");
        count++;
        if (certificate_status == 1)
        {
            if (count == 2)
            {
                esp_mqtt_client_publish(client, pubtopic_create_certificate, "", 0, 1, 0);
                ESP_LOGI(TAG, "Publish message, topic=%s", pubtopic_create_certificate);
            }
        }

        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        static bool getting_certificate = false;
        if (certificate_status == 1)
        {
            if (event->topic != NULL)
            {
                if (strstr(event->topic, subtopic_create_certificate))
                {
                    memcpy(json_certificate_str + event->current_data_offset, event->data, event->data_len);
                    if (event->total_data_len == event->current_data_offset + event->data_len)
                    {
                        getting_certificate = false;
                    }
                    else
                    {
                        getting_certificate = true;
                    }
                }
                else if (strstr(event->topic, subtopic_provision_template))
                {
                    ESP_LOGI(TAG, "Register Thing Done");
                    ESP_LOGI(TAG, "%s", event->data);
                }
                else if (strstr(event->topic, subtopic_provision_template_fail))
                {
                    ESP_LOGI(TAG, "Register Thing Fail");
                    ESP_LOGI(TAG, "%s", event->data);
                }
            }
            else if ((event->topic == NULL && getting_certificate))
            {

                memcpy(json_certificate_str + event->current_data_offset, event->data, event->data_len);
                if (event->total_data_len == event->current_data_offset + event->data_len)
                {
                    ESP_LOGI(TAG, "Data: %s", json_certificate_str);
                    json_certificate = cJSON_ParseWithLength(json_certificate_str, strlen(json_certificate_str));
                    certificateOwnershipToken = cJSON_GetObjectItemCaseSensitive(json_certificate, "certificateOwnershipToken");
                    ESP_LOGI(TAG, "certificateOwnershipToken: %s", certificateOwnershipToken->valuestring);
                    client_cert_json = cJSON_GetObjectItemCaseSensitive(json_certificate, "certificatePem");
                    ESP_LOGI(TAG, "client_cert_json: %s", client_cert_json->valuestring);
                    client_key_json = cJSON_GetObjectItemCaseSensitive(json_certificate, "privateKey");
                    ESP_LOGI(TAG, "client_key_json: %s", client_key_json->valuestring);
                    memcpy(client_cert, client_cert_json->valuestring, strlen(client_cert_json->valuestring));
                    memcpy(client_key, client_key_json->valuestring, strlen(client_key_json->valuestring));
                    json_register_thing = cJSON_CreateObject();
                    parameter = cJSON_CreateObject();
                    cJSON_AddStringToObject(json_register_thing, "certificateOwnershipToken", certificateOwnershipToken->valuestring);
                    cJSON_AddStringToObject(parameter, "ThingName", "GreenGarden");
                    cJSON_AddStringToObject(parameter, "SerialNumber", "123");
                    cJSON_AddItemReferenceToObject(json_register_thing, "parameters", parameter);
                    char *registries_thing_payload = cJSON_Print(json_register_thing);
                    ESP_LOGI(TAG, "registries_thing_payload: %s", registries_thing_payload);
                    esp_mqtt_client_publish(client, pubtopic_provision_template, registries_thing_payload, strlen(registries_thing_payload), 1, 0);
                    getting_certificate = false;
                }
                else
                {
                    getting_certificate = true;
                }
            }
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{
    certificate_status = check_certificate();

    esp_mqtt_client_config_t mqtt_cfg = {
        .host = "a2c52hjyaxit18-ats.iot.ap-southeast-1.amazonaws.com",
        .port = 8883,
        .client_cert_pem = (const char *)client_cert_pem_start,
        .client_key_pem = (const char *)client_key_pem_start,
        .cert_pem = (const char *)server_cert_pem_start,
        .transport = MQTT_TRANSPORT_OVER_SSL};

    // mqtt_cfg.client_cert_pem = certificate_status ? claim_client_cert : client_cert;
    // mqtt_cfg.client_key_pem = certificate_status ? claim_client_key : client_key;
    // mqtt_cfg.cert_pem = claim_ca_cert;

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}