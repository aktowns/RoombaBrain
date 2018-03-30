/* 
RoombaBrain
Smallish ESP32 CoAP Server that wraps most roomba 760 opcodes.
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
#include "endpoint.h"

static const char *TAG = "main";

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

  xTaskCreate(&endpoint_task, "coap", 2048, NULL, 5, NULL);
}
