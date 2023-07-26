#ifndef BLYNK_RPC_CLIENT_H
#define BLYNK_RPC_CLIENT_H

#include "BlynkRpc.h"

/*
 * Infra
 */

static uint16_t _rpc_seq;

/*
 * Shims
 */

#include "idl/rpc_shim_ncp.h"
#include "idl/rpc_shim_hw.h"
#include "idl/rpc_shim_blynk.h"

#include "idl/rpc_handler_mcu.h"
#include "idl/rpc_handler_client.h"

rpc_handler_t rpc_find_uid_handler(uint16_t uid);

RpcStatus rpc_invoke_handler(uint16_t uid, MessageBuffer* buff);

bool rpc_mcu_hasUID_impl(uint16_t uid);

#endif /* BLYNK_RPC_CLIENT_H */
