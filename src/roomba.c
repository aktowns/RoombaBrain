#include <stdint.h>
#include <stdarg.h>

#include <freertos/FreeRTOS.h>
#include <driver/uart.h>
#include <esp_log.h>

#include "roomba.h"

#define ROOMBA_UART UART_NUM_1
#define BUF_SIZE 1024

static const char *TAG = "roomba";

QueueHandle_t roomba_queue;

void roomba_uart_task(void *pvParameters) {
  uart_event_t event;
  size_t buffered_size;
  uint8_t* dtmp = (uint8_t*)malloc(BUF_SIZE);

  for (;;) {
    if (xQueueReceive(roomba_queue, (void*)&event, (portTickType)portMAX_DELAY)) {
      ESP_LOGI(TAG, "roomba uart event:");
      switch (event.type) {
        case UART_DATA:
          uart_get_buffered_data_len(ROOMBA_UART, &buffered_size);
          ESP_LOGI(TAG, "data, len: %d; buffered len: %d", event.size, buffered_size);
          break;
        case UART_FIFO_OVF:
          ESP_LOGI(TAG, "hw fifo overflow\n");
          uart_flush(ROOMBA_UART);
          break;
        case UART_BUFFER_FULL:
          ESP_LOGI(TAG, "ring buffer full\n");
          uart_flush(ROOMBA_UART);
          break;
        case UART_BREAK:
          ESP_LOGI(TAG, "uart rx break\n");
          break;
        case UART_PARITY_ERR:
          ESP_LOGI(TAG, "uart parity error\n");
          break;
        case UART_FRAME_ERR:
          ESP_LOGI(TAG, "uart frame error\n");
          break;
        case UART_PATTERN_DET:
          ESP_LOGI(TAG, "uart pattern detected\n");
          break;
        default:
          ESP_LOGI(TAG, "uart event type: %d\n", event.type);
          break;
      }
    }
  }
  free(dtmp);
  dtmp = NULL;
  vTaskDelete(NULL);
}

void roomba_init() {
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity    = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122
  };

  uart_param_config(ROOMBA_UART, &uart_config);
  uart_driver_install(ROOMBA_UART, BUF_SIZE * 2, BUF_SIZE * 2, 10, &roomba_queue, 0);
  xTaskCreate(roomba_uart_task, "roomba_uart_task", 2048, (void*)ROOMBA_UART, 12, NULL);
}

void send_roomba_cmd(roomba_opcode_t op, ...) {
  va_list ap;

  uart_write_bytes(ROOMBA_UART, ((const char*)&opcodes[op].opcode), 1);

  va_start(ap, op);

  for (uint8_t i = 0; i < opcodes[op].nargs; i++) {
    char arg = (char)va_arg(ap, int);
    uart_write_bytes(ROOMBA_UART, &arg, 1);
  }

  va_end(ap);
}
