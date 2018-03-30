/* 
RoombaBrain
Smallish ESP32 CoAP Server that wraps most roomba 760 opcodes.

This is split into two seperate parts, an rpc endpoint that takes commands without return values
and rest-like sensors objects.

endpoints: 

POST roomba/cmd 
{ 
    "id": 1,
    "method": "mode/safe"
}
methods:
    "mode/safe"
    "mode/full"
    "clean/clean"
    "clean/max"
    "clean/spot"
    "clean/dock"
    "clean/schedule" [days] [sun hour] [sun min] [mon hour] [mon min] [tue hour] [tue min] [wed hour] [wed minute] ..
    "clean/clock" [day] [hour] [minute]
    "clean/power"
    "actuator/drive" [velocity] [radius]
    "actuator/drive_direct" [right velocity] [left velocity]
    "actuator/drive_pwn" [right pwm] [left pwm]
    "actuator/motors" [off/on] [direction] 
    "actuator/motors_pwm" [main brush pwm] [side brush pwm] [vacuum pwm]
    "actuator/leds" [led] [color] [intensity]
    "input/sensors" ..

GET roomba/cmd?cmd=start
GET roomba/cmd/start

roomba/stop
roomba/pause

/version
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
//#include <stddef.h>
#include <limits.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <lwip/err.h>
#include <lwip/sys.h>

#include <coap.h>

#include <driver/gpio.h>
#include <driver/uart.h>

#include "sdkconfig.h"
#include "wifi.h"
#include "roomba.h"

static const char *TAG = "main";

static coap_async_state_t *async = NULL;

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

void coap_thread(void *pvParameter) {
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

      coap_resource_t *resource = coap_resource_init((unsigned char *) "roomba/rpc", 10, 0);

      if (resource) {
        coap_register_handler(resource, COAP_REQUEST_GET, async_handler);
        //coap_register_handler(resource, COAP_REQUEST_POST, )
        coap_add_resource(ctx, resource);

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

void app_main() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_LOGI(TAG, "booting esp");

  wifi_init_sta();
  roomba_init();

  xTaskCreate(&coap_thread, "coap", 2048, NULL, 5, NULL);
}
