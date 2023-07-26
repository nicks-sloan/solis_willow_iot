/*
 * NCP_Helpers.c
 *
 *  Created on: Jul 21, 2023
 *      Author: DengS1
 */

#include "NCP_Helpers.h"
#include "Events.h"
#include "Operation.h"

int rpc_uart_available(void) {
  // Return the amount of data in the UART input buffer
  return AS1_GetCharsInRxBuf();
}
int rpc_uart_read(void) {
  // Get a byte from the UART input buffer
  uint8 bTemp;
  AS1_RecvChar(&bTemp);
  return bTemp;
}

size_t rpc_uart_write(uint8_t data) {
  // Put a byte to the UART output buffer
	AS1_SendChar(data);
	return 1; 
}

void rpc_uart_flush(void) {
  // Wait for UART to output all the data
  //SerialNCP.flush();
	while(SendComplete == 0);				// Waiting for Tx complete
	SendComplete = 0;	
}

uint32_t millis(void) {
	return millisTick;
}

uint32_t rpc_system_millis(void) {
  // Return uptime in milliseconds
  // This is used to implement the RPC timeout
  return millis();
}

/*
 * Embed the info tag into the MCU firmware binary
 * This structure is used to identify the firmware type
 * and version during the OTA upgrade
 */

//volatile const char firmwareTag[] PROGMEM = "blnkinf\0"
//    BLYNK_PARAM_KV("mcu"    , BLYNK_FIRMWARE_VERSION)
//    BLYNK_PARAM_KV("fw-type", BLYNK_FIRMWARE_TYPE)
//    BLYNK_PARAM_KV("build"  , BLYNK_FIRMWARE_BUILD_TIME)
//    BLYNK_PARAM_KV("blynk"  , BLYNK_RPC_LIB_VERSION)
//    "\0";

/*
 * Implement some helpers
 */
uint32_t timeout = 10000;

bool ncpSetupSerial(timeout) {
  RpcUartFraming_init();

  const long baudTarget = BLYNK_NCP_BAUD;
  const long baudRates[3] = { 38400, 115200, baudTarget };
  unsigned baudIdx = 0;

  const uint32_t tbeg = millis();
  while (millis() - tbeg < timeout) {
    long baud = baudRates[baudIdx++ % 3];
 //   SerialNCP.begin(baud);
    if (RPC_STATUS_OK == rpc_ncp_ping()) {
      if (baud != baudTarget) {
        // Upgrade baud rate
        if (rpc_hw_setUartBaudRate(baudTarget)) {
       //   SerialNCP.flush();
       //   SerialNCP.begin(baudTarget);
      //    baud = baudTarget;
        //  delay(20);
          if (RPC_STATUS_OK != rpc_ncp_ping()) {
    //        SerialDbg.println(F("Changing NCP baud failed"));
            return false;
          }
        }
      }
  //    SerialDbg.print(F("Blynk.NCP ready, baud: "));
  //    SerialDbg.println(baud);
      return true;
    }
  }
  //SerialDbg.println(F("NCP not responding"));
  return false;
}

const char* ncpGetStateString(uint8_t state) {
  switch (state) {
  case BLYNK_STATE_IDLE             : return "Idle";
  case BLYNK_STATE_CONFIG           : return "Configuration";
  case BLYNK_STATE_CONNECTING_NET   : return "Connecting Network";
  case BLYNK_STATE_CONNECTING_CLOUD : return "Connecting Cloud";
  case BLYNK_STATE_CONNECTED        : return "Connected";

  case BLYNK_STATE_NOT_INITIALIZED  : return "Not Initialized";
  case BLYNK_STATE_OTA_UPGRADE      : return "NCP Upgrade";
  case BLYNK_STATE_ERROR            : return "Error";

  default                           : return "Unknown";
  }
}


void virtualWrite(int virtualPin, const char* value) {
  buffer_t val = { (uint8_t*)value, strlen(value) };
  rpc_blynk_virtualWrite(virtualPin, val);
}

//void virtualWrite(int virtualPin, int32_t value) {
//  char buff[16];
//  itoa(value, buff, 10);
//  virtualWrite(virtualPin, buff);
//}

// Wait for serial console, up to 3 seconds
//template <typename T>
//void waitSerialConsole(T& ser) {
//#if defined(LINUX)
//  // Not needed on linux
//  (void) ser;
//#else
//  const uint32_t tstart = millis();
//  while (!ser && (millis() - tstart < 2900)) { delay(1); }
//  delay(100);
//#endif
//}

void NCP_init(void){
	
	if (!ncpSetupSerial()) {
	    return;
	}
	 
  const char* ncpFwVer = "unknown";
  if (rpc_blynk_getNcpVersion(&ncpFwVer)) {
 //   SerialDbg.print(F("NCP firmware: "));
  //  SerialDbg.println(ncpFwVer);
//	  UARTOutStrPlusNull2(ncpFwVer, 3);
  }

  // Setup the indicator LED, user button (if needed)
  //rpc_hw_initUserButton(0, true);
  //rpc_hw_initLED(19, false);
  //rpc_hw_setLedBrightness(160);


  // Set config mode timeout to 30 minutes, for testing purposes
  rpc_blynk_setConfigTimeout(30*60);


  // Provide MCU firmware info.
  // This info is mainly used for the Primary MCU OTA updates
  rpc_blynk_setFirmwareInfo(BLYNK_FIRMWARE_TYPE,
                            BLYNK_FIRMWARE_VERSION,
                            BLYNK_FIRMWARE_BUILD_TIME,
                            BLYNK_RPC_LIB_VERSION);

  // White labeling (use this ONLY if you have a branded Blynk App)
  //rpc_blynk_setVendorPrefix("MyCompany");
  //rpc_blynk_setVendorServer("dashboard.mycompany.com");

  // Product setup
  if (!rpc_blynk_initialize(BLYNK_TEMPLATE_ID, BLYNK_TEMPLATE_NAME)) {
  //  SerialDbg.println(F("rpc_blynk_initialize failed"));
  }
  
}

void sendPeriodicMessage(void)
{
  static uint32_t last_change;
  last_change = millis();
  if (millis() - last_change > 10000) {
    last_change += 10000;

    // Send a value to Blynk Virtual Pin 1
  //  virtualWrite(1, millis());
  }
}

//void loop()
//{
//  rpc_run();
//
//  sendPeriodicMessage();
//}

// Handle Blynk Virtual Pin value updates
void rpc_client_blynkVPinChange_impl(uint16_t vpin, buffer_t param)
{
    // NOTE: we could copy the buffer, but we use 0-copy instead
    // But we need to 0-terminate it, overwriting the CRC8
    param.data[param.length] = '\0';

   // SerialDbg.print(F("Got data on Virtual Pin "));
    //SerialDbg.println(vpin);

    // Param format. Most values will be plain strings: "Hello world", "1234", "123.456", etc.
    // However, sometimes the value contains multiple items (an array). In this case, the values are separated using a 0x00 byte, i.e:
    // "First\0Second\0Third"
}

// Define the callback for the NCP state change event
void rpc_client_blynkStateChange_impl(uint8_t state)
{
 // SerialDbg.print(F("NCP state: "));
 // SerialDbg.println(ncpGetStateString(state));
  if ((RpcBlynkState)state == BLYNK_STATE_CONNECTED) {
    // Send a value to Virtual Pin 1
    virtualWrite(1, "hello world!");
  }
}

// Handle various NCP events
void rpc_client_processEvent_impl(uint8_t event)
{
    switch ((RpcEvent)event) {
    case RPC_EVENT_NCP_REBOOTING:
  //    SerialDbg.println(F("NCP is rebooting. TODO: reinitialize NCP"));
      break;
    case RPC_EVENT_HW_USER_CLICK:      break;
    case RPC_EVENT_HW_USER_DBLCLICK:   break;
    case RPC_EVENT_HW_USER_LONGPRESS:  break;
    case RPC_EVENT_HW_USER_CONFIGRESET: break;
    case RPC_EVENT_BLYNK_PROVISIONED:  break;
    case RPC_EVENT_BLYNK_TIME_SYNC:    break;
    case RPC_EVENT_BLYNK_TIME_CHANGED: break;
    default: break;
    }
}
