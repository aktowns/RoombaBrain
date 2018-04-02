#ifndef ROOMBABRAIN_RPC_H
#define ROOMBABRAIN_RPC_H

#include <stdint.h>
#include <stddef.h>

uint8_t *perform_rpc_request(const unsigned char *data, size_t* size);

#endif //ROOMBABRAIN_RPC_H
