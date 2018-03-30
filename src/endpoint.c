#include <stdbool.h>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>

#include <coap.h>
#include <esp_log.h>
#include <cJSON.h>

#include "endpoint.h"
#include "wifi.h"
#include "roomba.h"

static const char *TAG = "endpoint";

bool command_map(const char* cmd) {
  if (strcmp(cmd, "start") == 0) {
    send_roomba_cmd(OP_START);
  } else {
    ESP_LOGI(TAG, "unhandled command: %s\n", cmd);
    return false;
  }

  return true;
}

static void hnd_roomba_cmd_post(coap_context_t *ctx, struct coap_resource_t *resource,
                                const coap_endpoint_t *local_interface, coap_address_t *peer,
                                coap_pdu_t *request, str *token, coap_pdu_t *response) {
  ESP_LOGI(TAG, "hnd_roomba_cmd_post entered\n");

  size_t size;
  unsigned char *data;

  coap_get_data(request, &size, &data);

  cJSON *json = cJSON_Parse((const char *) data);
  cJSON *id = cJSON_GetObjectItem(json, "id");
  cJSON *method = cJSON_GetObjectItem(json, "message");

  cJSON *obj = cJSON_CreateObject();

  if (id->type == cJSON_Number) {
    ESP_LOGI(TAG, "id=%i\n", id->valueint);

    cJSON_AddNumberToObject(obj, "id", id->valueint);
  }

  if (method->type == cJSON_String && method->valuestring != NULL) {
    ESP_LOGI(TAG, "method=%s\n", method->valuestring);
    if (command_map(method->valuestring)) {
      cJSON_AddStringToObject(obj, "message", "ok");
      cJSON_AddBoolToObject(obj, "successful", true);
    } else {
      cJSON_AddStringToObject(obj, "message", "command not found");
    }
  }

  char *response_data = cJSON_Print(obj);

  cJSON_Delete(json);

  unsigned char buf[3];
  coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_JSON), buf);
  coap_add_data(response, strlen(response_data), (const unsigned char *) response_data);
  coap_send(ctx, local_interface, peer, response);

  free(response_data);
  cJSON_Delete(obj);
}

static void hnd_roomba_cmd_get(coap_context_t *ctx, struct coap_resource_t *resource,
                               const coap_endpoint_t *local_interface, coap_address_t *peer,
                               coap_pdu_t *request, str *token, coap_pdu_t *response) {
  ESP_LOGI(TAG, "hnd_roomba_cmd_get entered\n");

  unsigned char buf[3];
  cJSON *obj = cJSON_CreateObject();
  cJSON_AddStringToObject(obj, "message", "use post to send commands");
  char *response_data = cJSON_Print(obj);

  coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_JSON), buf);
  coap_add_data(response, strlen(response_data), (const unsigned char *) response_data);
  coap_send(ctx, local_interface, peer, response);

  free(response_data);
  cJSON_Delete(obj);
}

void endpoint_task(void *pvParameter) {
  coap_context_t *ctx = NULL;
  coap_address_t serv_addr;
  fd_set readfds;
  struct timeval tv;
  int flags = 0;

  while (1) {
    wifi_wait_active();
    ESP_LOGI(TAG, "coap thread woken");

    coap_address_init(&serv_addr);
    serv_addr.addr.sin.sin_family = AF_INET;
    serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
    serv_addr.addr.sin.sin_port = htons(COAP_DEFAULT_PORT);
    ctx = coap_new_context(&serv_addr);

    if (ctx) {
      flags = fcntl(ctx->sockfd, F_GETFL, 0);
      fcntl(ctx->sockfd, F_SETFL, flags | O_NONBLOCK);

      tv.tv_usec = 0;
      tv.tv_sec = 5;

      coap_resource_t *resource = coap_resource_init((unsigned char *) "roomba/cmd", 10, 0);
      coap_register_handler(resource, COAP_REQUEST_GET, hnd_roomba_cmd_get);
      coap_register_handler(resource, COAP_REQUEST_POST, hnd_roomba_cmd_post);
      coap_add_resource(ctx, resource);

      if (resource) {
        for (;;) {
          FD_ZERO(&readfds);
          FD_CLR(ctx->sockfd, &readfds);
          FD_SET(ctx->sockfd, &readfds);

          int result = select(ctx->sockfd + 1, &readfds, 0, 0, &tv);
          if (result > 0) {
            if (FD_ISSET(ctx->sockfd, &readfds))
              coap_read(ctx);
          } else if (result < 0) {
            break;
          } else {
          }
        }
      }
      coap_free_context(ctx);
    }
  }
  vTaskDelete(NULL);
}
