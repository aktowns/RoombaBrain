
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>

#include <coap.h>
#include <esp_log.h>
#include <cJSON.h>

#include "endpoint.h"
#include "wifi.h"

static const char *TAG = "endpoint";

static coap_async_state_t *async = NULL;

static void parse_command(const char *payload) {
  cJSON *json = cJSON_Parse(payload);
  cJSON *id = cJSON_GetObjectItem(json, "id");
  cJSON *method = cJSON_GetObjectItem(json, "method");
  cJSON *arguments = cJSON_GetObjectItem(json, "arguments");

  if (id->type == cJSON_Number) {
    ESP_LOGI(TAG, "id=%i\n", id->valueint);
  }
  if (method->type == cJSON_String && method->valuestring != NULL) {
    ESP_LOGI(TAG, "method=%s\n", method->valuestring);
  }
  if (arguments->type == cJSON_Array) {
    ESP_LOGI(TAG, "arguments.size=%i\n", cJSON_GetArraySize(arguments));
  }
}

static void hnd_roomba_cmd_post(coap_context_t *ctx, struct coap_resource_t *resource,
                                const coap_endpoint_t *local_interface, coap_address_t *peer,
                                coap_pdu_t *request, str *token, coap_pdu_t *response) {
  size_t size;
  unsigned char *data;

  coap_get_data(request, &size, &data);

  parse_command((const char *) data);


}

static void send_async_response(coap_context_t *ctx, const coap_endpoint_t *local_if) {
  coap_pdu_t *response;
  unsigned char buf[3];
  const char *response_data = "Hello World!";
  size_t size = sizeof(coap_hdr_t) + 20;

  response = coap_pdu_init((unsigned char) (async->flags & COAP_MESSAGE_CON),
                           COAP_RESPONSE_CODE(205), 0, size);
  response->hdr->id = coap_new_message_id(ctx);
  if (async->tokenlen) {
    coap_add_token(response, async->tokenlen, async->token);
  }
  coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_JSON), buf);
  coap_add_data(response, strlen(response_data), (unsigned char *) response_data);

  if (coap_send(ctx, local_if, &async->peer, response) == COAP_INVALID_TID) {

  }

  coap_delete_pdu(response);
  coap_async_state_t *tmp;
  coap_remove_async(ctx, async->id, &tmp);
  coap_free_async(async);
  async = NULL;
}

static void async_handler(coap_context_t *ctx, struct coap_resource_t *resource,
                          const coap_endpoint_t *local_interface, coap_address_t *peer,
                          coap_pdu_t *request, str *token, coap_pdu_t *response) {
  async = coap_register_async(ctx, peer, request, COAP_ASYNC_SEPARATE | COAP_ASYNC_CONFIRM,
                              (void *) "no data");
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
      coap_register_handler(resource, COAP_REQUEST_GET, async_handler);
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
            ESP_LOGE(TAG, "select timeout");
          }

          if (async) {
            send_async_response(ctx, ctx->endpoint);
          }
        }
      }
      coap_free_context(ctx);
    }
  }
  vTaskDelete(NULL);
}
