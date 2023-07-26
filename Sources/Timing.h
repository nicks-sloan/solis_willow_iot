/*
 * Timing.h
 *
 *  Created on: Feb 17, 2017
 *      Author: Scott Wang
 */

#ifndef TIMING_H_
#define TIMING_H_

#include "SystemInit.h"

//public functions
void SleepForever(void);
void SleepMS(uint16 ms);
void SleepSecond(uint8 bScnd);
void WaitMS(uint16 ms);
void DelayMS(uint16 ms);
void StopLPTMR(void);
void DelayUS(uint16 dlyTick);
uint8 IsPowerupDone(void);
void UpdateOperationTime(void);
SentinelTimeType ResetSentinelTimer(void);
void UpdateSentinelTime(void);

void SleepMSWithUART(uint16 ms);

#endif /* TIMING_H_ */
