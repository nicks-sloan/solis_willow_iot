/*
 * Operation.h
 *
 *  Created on: 4/1/2017
 *      Author: Scott Wang
 */

#ifndef OPERATION_H_
#define OPERATION_H_

/* Including shared modules, which are used for whole project */
#include "SystemInit.h"


extern uint8 DbHandWave;
extern uint8 DbHandWaveCT;
extern uint8 DbInterv;
//public functions
#define WDReset() WDog1_Clear(WDog1_DeviceData)
void BlinkLED(uint16 ms);
void ToggleLED(void);
uint16 GetDataMovingEverage(uint16 NewData,uint16 OldAvg);
void LatchSolenoid(void);
void UnLatchSolenoid(void);
void Flush100ms(uint16 Hms);
void TurnOffAllPeripheral(void);
uint8 IsSystemInShipCondition(void);
SolenoidOnTimeType SetValveOnTime(uint8 bSFullOn);
uint8 IsSolarCellgetPower(void);
OperationStsType SetOperationState(OperationStsType State);
void NormalOperation(void);
void CommunicationEnabledOperation(void);
void TestOperation(void);
void SetOperation(void);
void ShipOperation(void);
void StopOperation(void);
void CalibrationOperation(void);
void BLEEnabledOperation(void);

/* Sheng MVP Mod begin */
//SR Uart Functions
void ANTEnabledOperation(void);
void UARTANTUdateOperation(void);
/* Sheng MVP Mod end */

#endif /* OPERATION_H_ */
