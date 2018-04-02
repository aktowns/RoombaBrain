#include "rpc.h"

#include <esp_log.h>

#include "roomba.h"

#include "rpc/request.pb.h"
#include "rpc/response.pb.h"

#include <pb_encode.h>
#include <pb_decode.h>
#include <stdlib.h>

static const char *TAG = "rpc";

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

#define RESP_BUF ((sizeof(uint8_t) * 120))

uint8_t *perform_rpc_request(const unsigned char *data, size_t* size) {
  RPC_Request rpc_request = RPC_Request_init_zero;
  pb_istream_t stream = pb_istream_from_buffer(data, strlen((const char *) data));
  pb_decode(&stream, RPC_Request_fields, &rpc_request);
  handle_rpc_request(&rpc_request);

  RPC_Response resp = RPC_Response_init_zero;
  resp.id = rpc_request.id;

  uint8_t *resp_buf = malloc(RESP_BUF);

  pb_ostream_t ostream = pb_ostream_from_buffer(resp_buf, RESP_BUF);
  pb_encode(&ostream, RPC_Response_fields, &resp);

  *size = ostream.bytes_written;

  return resp_buf;
}