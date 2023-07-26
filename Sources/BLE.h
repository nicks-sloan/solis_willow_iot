/*
 * BLE.h
 *
 *  Created on: Sep 5, 2019
 *      Author: Test1
 */

#ifndef BLE_H_
#define BLE_H_

#include "SystemInit.h"


// public functions
void TestBLECommand(void);
void BLEProcess(void);
//BLEReadOnlyType GetEngineeringReadOnlyData(void);
//BLEReadWriteType GetEngineeringReadWriteData(void);
//void SetEngineeringWriteData(void);
void SaveBLEParaToFlash(BLEDongleParaType sPara);
void SaveBoardInfoToFlash(uint8* psBoardInfo, uint8 bLen);
void GetBoardInfoFromFlash(void);
void SetSKURelatedDeftOpPara(uint8 bSKUIndex);
BLEDongleParaType LoadBLEParaFromFlash(void);
void SetDefaultOpParas(void);
BLEDongleParaType LoadDefaultBLEPara(void);
uint8 IsBLEInstalled(void);

#endif /* BLE_H_ */

