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
void send_roomba_cmd(roomba_opcode_t op, ...);

#endif //ROOMBABRAIN_ROOMBA_H
