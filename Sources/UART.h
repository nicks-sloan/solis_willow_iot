/*
 * UART.h
 *
 *  Created on: Feb 17, 2017
 *      Author: Test1
 */

#ifndef UART_H_
#define UART_H_

#include "SystemInit.h"


#define PREAMP 			0xA5
#define ACK    			0x5A
#define RECEIVEOK     	0x55
#define READDATA 		0x10
#define WRITEDATA 		0x20
#define WRITEBOARDINFO  0x30
#define EXITSET 		0x40
//#define COMMUNICATIONLIMIT	150	//980ms @1ms rate. The maxiumum time of one command from BLE
#define COMMUNICATIONLIMIT	980	//980ms @1ms rate
//#define COMMUNICATIONLIMIT	1980	//980ms @1ms rate
extern bool volatile SendComplete;

//public functions
void UARTOutMultiCurveData(uint16 d1,uint16 d2,uint16 d3);
void UARTOutASCIIData(uint16 d);
void ProcessASCIIRequest(void);
void UARTOutHexASCIIBlock(uint8 * src, uint8 len);
void UARTOutEnterSettingModePrompt(void);
void ProcessSettingCmd(void);
void ProcessTestCmd(void);
void UARTOutDebugData(void);
void UARTOutDebugInfor(void);
void UARTOutDebugTem(uint16 wT1,uint16 wT2,uint16 wT3);
void UARTOutBLEInfor(void);
void UARTOutHandWaveData(void);
void ConvertByteToHexStr(uint8 bByte, char* pHexStr);
void Convert16bitsToHexStr(uint16 iData, char* pHexStr);
void Convert32bitsToHexStr(uint32 lData, char* pHexStr);
void ConvertBlockToHexStr(uint8* pBlock, uint8 len, char* pHexStr);
uint8 HexCharToNum(char HexChar, uint8* bNum);
uint8 HexByteToNum(char* HexByte, uint8* bNum);
uint8 HexStrToBlock(char* HexStr, uint8* bDataBlock, uint16 len);
void UARTOutStr(char* src);
void UARTOutDebugDataOn(void);
void UARTOutHexInt(uint16 OutInt); 
void UARTOutHexByte(uint8 OutByte);



#endif /* UART_H_ */
