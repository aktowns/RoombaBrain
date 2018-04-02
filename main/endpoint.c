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

#include "rpc/request.pb.h"
#include "rpc/response.pb.h"

#include <pb_encode.h>
#include <pb_decode.h>

static const char *TAG = "endpoint";

void handle_rpc_roomba_mode(RPC_ModeRequest *mode) {
  switch (mode->mode) {
    case RPC_ModeRequest_Mode_FULL:
      send_roomba_cmd(OP_FULL, 0);
      break;
    case RPC_ModeRequest_Mode_SAFE:
      send_roomba_cmd(OP_SAFE, 0);
      break;
  }
}

void handle_rpc_roomba_clean(RPC_CleanRequest *clean) {
  switch (clean->mode) {
    case RPC_CleanRequest_CleanMode_CLEAN:
      send_roomba_cmd(OP_CLEAN, 0);
      break;
    case RPC_CleanRequest_CleanMode_DOCK:
      send_roomba_cmd(OP_SEEK_DOCK, 0);
      break;
    case RPC_CleanRequest_CleanMode_MAX:
      send_roomba_cmd(OP_MAX, 0);
      break;
    case RPC_CleanRequest_CleanMode_SPOT:
      send_roomba_cmd(OP_SPOT, 0);
      break;
  }
}

void handle_rpc_roomba_baud(RPC_BaudRequest *baud) {
  switch (baud->rate) {
    case RPC_BaudRequest_BaudRate_BAUD_300:
      send_roomba_cmd(OP_BAUD, 1, BAUD_300);
      break;
    case RPC_BaudRequest_BaudRate_BAUD_600:
      send_roomba_cmd(OP_BAUD, 1, BAUD_600);
      break;
    case RPC_BaudRequest_BaudRate_BAUD_1200:
      send_roomba_cmd(OP_BAUD, 1, BAUD_1200);
      break;
    case RPC_BaudRequest_BaudRate_BAUD_2400:
      send_roomba_cmd(OP_BAUD, 1, BAUD_2400);
      break;
    case RPC_BaudRequest_BaudRate_BAUD_4800:
      send_roomba_cmd(OP_BAUD, 1, BAUD_4800);
      break;
    case RPC_BaudRequest_BaudRate_BAUD_9600:
      send_roomba_cmd(OP_BAUD, 1, BAUD_9600);
      break;
    case RPC_BaudRequest_BaudRate_BAUD_14400:
      send_roomba_cmd(OP_BAUD, 1, BAUD_14400);
      break;
    case RPC_BaudRequest_BaudRate_BAUD_19200:
      send_roomba_cmd(OP_BAUD, 1, BAUD_19200);
      break;
    case RPC_BaudRequest_BaudRate_BAUD_28800:
      send_roomba_cmd(OP_BAUD, 1, BAUD_28800);
      break;
    case RPC_BaudRequest_BaudRate_BAUD_38400:
      send_roomba_cmd(OP_BAUD, 1, BAUD_38400);
      break;
    case RPC_BaudRequest_BaudRate_BAUD_57600:
      send_roomba_cmd(OP_BAUD, 1, BAUD_57600);
      break;
    case RPC_BaudRequest_BaudRate_BAUD_115200:
      send_roomba_cmd(OP_BAUD, 1, BAUD_115200);
      break;
  }
}

void handle_rpc_roomba(RPC_Roomba *roomba) {
  switch (roomba->which_request) {
    case RPC_Roomba_mode_tag:
      handle_rpc_roomba_mode(&roomba->request.mode);
      break;
    case RPC_Roomba_power_tag:
      send_roomba_cmd(OP_POWER, 0);
      break;
    case RPC_Roomba_clean_tag:
      handle_rpc_roomba_clean(&roomba->request.clean);
      break;
    case RPC_Roomba_start_tag:
      send_roomba_cmd(OP_START, 0);
      break;
    case RPC_Roomba_baud_tag:
      handle_rpc_roomba_baud(&roomba->request.baud);
      break;
    default:
      ESP_LOGE(TAG, "Unhandled roomba rpc call: %i", roomba->which_request);
      break;
  }
}

void handle_rpc_actuator(RPC_Actuator *actuator) {
  switch (actuator->which_request) {
    case RPC_Actuator_digital_leds_ascii_tag:
      break;
    case RPC_Actuator_drive_tag:
      break;
    case RPC_Actuator_drive_pwm_tag:
      break;
    case RPC_Actuator_direct_drive_tag:
      break;
    case RPC_Actuator_motors_tag:
      break;
    case RPC_Actuator_motors_pwm_tag:
      break;
    case RPC_Actuator_leds_tag:
      break;
    case RPC_Actuator_digital_leds_tag:
      break;
    case RPC_Actuator_play_tag:
      break;
    case RPC_Actuator_song_tag:
      break;
    default:
      ESP_LOGE(TAG, "Unhandled actuator rpc call: %i", actuator->which_request);
      break;
  }
}

void handle_rpc_request(RPC_Request *rpc_request) {
  switch (rpc_request->which_request) {
    case RPC_Request_roomba_tag:
      handle_rpc_roomba(&rpc_request->request.roomba);
      break;
    case RPC_Request_actuator_tag:
      handle_rpc_actuator(&rpc_request->request.actuator);
      break;
    default:
      ESP_LOGE(TAG, "Unhandled rpc call: %i", rpc_request->which_request);
      break;
  }
}

static void hnd_roomba_cmd_post(coap_context_t *ctx UNUSED,
                                struct coap_resource_t *resource UNUSED,
                                const coap_endpoint_t *local_interface UNUSED,
                                coap_address_t *peer UNUSED,
                                coap_pdu_t *request, str *token UNUSED, coap_pdu_t *response) {
  ESP_LOGI(TAG, "hnd_roomba_cmd_post entered\n");

  size_t size;
  unsigned char *data;

  coap_get_data(request, &size, &data);

  RPC_Request rpc_request = RPC_Request_init_zero;
  pb_istream_t stream = pb_istream_from_buffer(data, strlen((const char *) data));
  pb_decode(&stream, RPC_Request_fields, &rpc_request);
  handle_rpc_request(&rpc_request);

  RPC_Response resp = RPC_Response_init_zero;
  resp.id = rpc_request.id;

  uint8_t resp_buf[120];
  pb_ostream_t ostream = pb_ostream_from_buffer(resp_buf, sizeof(resp_buf));
  pb_encode(&ostream, RPC_Response_fields, &resp);

  unsigned char buf[3];
  coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_OCTET_STREAM), buf);
  coap_add_data(response, ostream.bytes_written, resp_buf);
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
