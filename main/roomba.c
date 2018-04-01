#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <driver/uart.h>
#include <esp_log.h>
#include <driver/gpio.h>

#include "roomba.h"

#define ROOMBA_UART UART_NUM_1
#define BUF_SIZE 1024

static const char *TAG = "roomba";

QueueHandle_t roomba_queue;

const opcode_t opcodes[255] = {
    [OP_START]            {.opcode = 128, .nargs =  0, .description = "start"                    },
    [OP_RESET]            {.opcode =   7, .nargs =  0, .description = "reset"                    },
    [OP_STOP]             {.opcode = 173, .nargs =  0, .description = "stop"                     },
    [OP_BAUD]             {.opcode = 129, .nargs =  1, .description = "baud"                     },
    [OP_SAFE]             {.opcode = 131, .nargs =  0, .description = "mode/safe"                },
    [OP_FULL]             {.opcode = 132, .nargs =  0, .description = "mode/full"                },
    [OP_CLEAN]            {.opcode = 135, .nargs =  0, .description = "clean/clean"              },
    [OP_MAX]              {.opcode = 136, .nargs =  0, .description = "clean/max"                },
    [OP_SPOT]             {.opcode = 134, .nargs =  0, .description = "clean/spot"               },
    [OP_SEEK_DOCK]        {.opcode = 143, .nargs =  0, .description = "clean/dock"               },
    [OP_SCHEDULE]         {.opcode = 167, .nargs = 15, .description = "clean/schedule"           },
    [OP_SETTIME]          {.opcode = 168, .nargs =  3, .description = "clean/settime"            },
    [OP_POWER]            {.opcode = 133, .nargs =  0, .description = "clean/power"              },
    [OP_DRIVE]            {.opcode = 137, .nargs =  4, .description = "actuator/drive"           },
    [OP_DRIVE_DIRECT]     {.opcode = 145, .nargs =  4, .description = "actuator/drive_direct"    },
    [OP_DRIVE_PWM]        {.opcode = 146, .nargs =  4, .description = "actuator/drive_pwm"       },
    [OP_MOTORS]           {.opcode = 138, .nargs =  1, .description = "actuator/motors"          },
    [OP_MOTORS_PWM]       {.opcode = 144, .nargs =  3, .description = "actuator/motors_pwm"      },
    [OP_LEDS]             {.opcode = 139, .nargs =  3, .description = "actuator/leds"            },
    [OP_SCHEDULE_LEDS]    {.opcode = 162, .nargs =  2, .description = "actuator/schedule_leds"   },
    [OP_DIGIT_LEDS]       {.opcode = 163, .nargs =  4, .description = "actuator/digit_leds"      },
    [OP_DIGIT_LEDS_ASCII] {.opcode = 164, .nargs =  4, .description = "actuator/digit_leds_ascii"},
    [OP_BUTTONS]          {.opcode = 165, .nargs =  1, .description = "actuator/buttons"         },
    [OP_SONG]             {.opcode = 140, .nargs = -1, .description = "actuator/song"            },
    [OP_PLAY]             {.opcode = 141, .nargs =  1, .description = "actuator/play"            },
    [OP_SENSORS]          {.opcode = 142, .nargs =  1, .description = "input/sensors"            },
    [OP_QUERY_LIST]       {.opcode = 149, .nargs = -1, .description = "input/query_list"         },
    [OP_STREAM]           {.opcode = 148, .nargs = -1, .description = "input/stream"             },
    [OP_PAUSE]            {.opcode = 150, .nargs =  1, .description = "input/pause"              }
};

void roomba_uart_task(void *pvParameters) {
  uart_event_t event;
  size_t buffered_size;
  //uint8_t* dtmp = (uint8_t*)malloc(BUF_SIZE);

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
  //free(dtmp);
  //dtmp = NULL;
  vTaskDelete(NULL);
}

void roomba_init() {
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity    = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
  };

  uart_param_config(ROOMBA_UART, &uart_config);
  gpio_set_direction(GPIO_NUM_26, GPIO_MODE_OUTPUT);
  uart_set_pin(ROOMBA_UART, GPIO_NUM_26, GPIO_NUM_27, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_driver_install(ROOMBA_UART, BUF_SIZE * 2, 0, 10, NULL, 0);

  //xTaskCreate(roomba_uart_task, "roomba_uart_task", 2048, (void*)ROOMBA_UART, 12, NULL);
}

void send_roomba_cmd(roomba_opcode_t op, int8_t len, ...) {
  va_list ap;

  ESP_LOGI(TAG, "Sending command %s to roomba\n", opcodes[op].description);
  char bfr[len+1];
  bfr[0] = opcodes[op].opcode;

  va_start(ap, len);
  for (int8_t i = 1; i <= len; i++) {
     char arg = (char)va_arg(ap, int);
     bfr[i] = arg;
  }
  va_end(ap);

  uart_write_bytes(ROOMBA_UART, bfr, (size_t) (len + 1));
}
