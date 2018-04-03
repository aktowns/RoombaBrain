#include <stdbool.h>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>

#include <coap.h>
#include <esp_log.h>

#include <lwip/sys.h>
#include <lwip/err.h>

#include "endpoint.h"
#include "wifi.h"
#include "roomba.h"
#include "utils.h"
#include "rpc.h"

#include "rpc/response.pb.h"

#include <pb_encode.h>
static const char *TAG = "endpoint";

static void hnd_roomba_cmd_post(coap_context_t *ctx UNUSED,
                                struct coap_resource_t *resource UNUSED,
                                const coap_endpoint_t *local_interface UNUSED,
                                coap_address_t *peer UNUSED,
                                coap_pdu_t *request, str *token UNUSED, coap_pdu_t *response) {
  ESP_LOGI(TAG, "hnd_roomba_cmd_post entered\n");

  size_t size;
  unsigned char *data;

  coap_get_data(request, &size, &data);

  size_t out_size;
  uint8_t *resp_buf = perform_rpc_request(data, &out_size);

  unsigned char buf[3];
  coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_OCTET_STREAM), buf);
  coap_add_data(response, out_size, resp_buf);

  free(resp_buf);
}

static void hnd_roomba_cmd_get(coap_context_t *ctx UNUSED, struct coap_resource_t *resource UNUSED,
                               const coap_endpoint_t *local_interface UNUSED,
                               coap_address_t *peer UNUSED,
                               coap_pdu_t *request UNUSED, str *token UNUSED,
                               coap_pdu_t *response) {
  ESP_LOGI(TAG, "hnd_roomba_cmd_get entered\n");

  RPC_Response cmd_resp = RPC_Response_init_zero;
  cmd_resp.id = 0;

  uint8_t resp_buf[120];
  pb_ostream_t ostream = pb_ostream_from_buffer(resp_buf, sizeof(resp_buf));
  pb_encode(&ostream, RPC_Response_fields, &cmd_resp);

  unsigned char buf[3];
  coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_JSON), buf);
  coap_add_data(response, ostream.bytes_written, resp_buf);
}

void endpoint_task(void *pvParameter UNUSED) {
  coap_context_t *ctx = NULL;
  coap_address_t serv_addr;
  fd_set readfds;
  struct timeval tv;
  int flags = 0;

  forever {
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
