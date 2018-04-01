#ifndef ROOMBABRAIN_ROOMBA_H
#define ROOMBABRAIN_ROOMBA_H

#include <stdint.h>

typedef struct {
  const uint8_t opcode;
  const int8_t nargs;
  const char* description;
} opcode_t;

typedef enum {
  OP_START,
  OP_RESET,
  OP_STOP,
  OP_BAUD,
  OP_SAFE,
  OP_FULL,
  OP_CLEAN,
  OP_MAX,
  OP_SPOT,
  OP_SEEK_DOCK,
  OP_SCHEDULE,
  OP_SETTIME,
  OP_POWER,
  OP_DRIVE,
  OP_DRIVE_DIRECT,
  OP_DRIVE_PWM,
  OP_MOTORS,
  OP_MOTORS_PWM,
  OP_LEDS,
  OP_SCHEDULE_LEDS,
  OP_DIGIT_LEDS,
  OP_DIGIT_LEDS_ASCII,
  OP_BUTTONS,
  OP_SONG,
  OP_PLAY,
  OP_SENSORS,
  OP_QUERY_LIST,
  OP_STREAM,
  OP_PAUSE
} roomba_opcode_t;

typedef enum {
  BAUD_300   = 0,
  BAUD_600   = 1,
  BADD_1200  = 2,
  BAUD_2400  = 3,
  BAUD_4800  = 4,
  BAUD_9600  = 5,
  BAUD_14400 = 6,
  BAUD_19200 = 7,
  BAUD_28800 = 8,
  BAUD_38400 = 9,
  BAUD_57600 = 10,
  BAUD_115200 = 11
} op_baud_code_t;

typedef enum {
  DAY_SAT = 6,
  DAY_FRI = 5,
  DAY_THU = 4,
  DAY_WED = 3,
  DAY_TUE = 2,
  DAY_MON = 1,
  DAY_SUN = 0
} roomba_schedule_day_t;

void roomba_init();
void send_roomba_cmd(roomba_opcode_t op, int8_t len, ...);

#endif //ROOMBABRAIN_ROOMBA_H
