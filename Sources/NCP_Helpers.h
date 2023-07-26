#ifndef NCP_HELPERS_H_
#define NCP_HELPERS_H_

#include "./src/BlynkRpcClient.h"
#include "./src/BlynkRpcUartFraming.h"

/* Fill in information from your Blynk Template here */
/* Read more: https://bit.ly/BlynkInject */
#define BLYNK_TEMPLATE_ID           "TMPL2wYwAWFl2"
#define BLYNK_TEMPLATE_NAME         "Bottle Filler"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_FIRMWARE_BUILD_TIME     "7.20.23"

#define BLYNK_FIRMWARE_TYPE BLYNK_TEMPLATE_ID

#define BLYNK_NCP_BAUD 38400 // 38400

#define BLYNK_PARAM_KV(k, v)    k "\0" v "\0"
/*
 * Implement the UART interface
 */

//#define SerialNCP   BLYNK_NCP_SERIAL

int rpc_uart_available(void);
int rpc_uart_read(void);
size_t rpc_uart_write(uint8_t data);

void rpc_uart_flush(void);

uint32_t millis(void);
uint32_t rpc_system_millis(void);

const char* ncpGetStateString(uint8_t state);

void virtualWrite(int virtualPin, const char* value);

void NCP_init(void);

void sendPeriodicMessage(void);

// Handle Blynk Virtual Pin value updates
void rpc_client_blynkVPinChange_impl(uint16_t vpin, buffer_t param);

// Define the callback for the NCP state change event
void rpc_client_blynkStateChange_impl(uint8_t state);

// Handle various NCP events
void rpc_client_processEvent_impl(uint8_t event);


#endif /* NCP_HELPERS_H_ */
