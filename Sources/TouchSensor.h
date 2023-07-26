/*
 * touch.h
 *
 *  Created on: Feb 17, 2017
 *      Author: Test1
 */

#ifndef TOUCH_H_
#define TOUCH_H_


#include "SystemInit.h"


#define CAPDATA  TSI0_DATA & 0xFFFFu    // the TSI data register


#define SMALLBUTTON 1					// small touch button
#define BIGBUTTON 2					    // big touch button

#define TOUCHTHD 50					// Threshold of touch
#define TBUPDATETHD 20					// Threshold for updating baseline moving average 
//#define TBUPDATETHD 50				// Threshold for updating baseline moving average 

#define FORSHIP 60      				// touch time for enter in shipping
#define FORRESET 260      				// time to reset baseline
#define CALLOW	80						// calibration request window low
#define CALHIGH 160						// calibration request window high
#define ENABLELED 40					// Enable led request window low

#define MAXBTACT 5						// button actions allowed in window time
#define WINDOWTM 120					// window time 30 minutes
#define LOCKOUT  240					// lockout time of button

#define MODELOW 40						// time to display mode
#define MODECHANGELOW 60				// Time to change mode lower 
#define MODECHANGEHIGH 100				// Time to change mode higher

//public functions
void TButtonStartScan(uint8 chan);
void TButtonClr(void);
void ToutchButtonInit(void);
uint8 SingleButtonProcess(uint8 bNewSts);
uint8 TwoButtonProcess(uint8 bSMNewStatus,uint8 bBGNewStatus);
uint8 IsButtonTouched(uint8 chan);
uint16 GetBigButtonBaselineValue(void);
uint16 GetBigButtonCapInstantValue(void);
uint16 GetSmallButtonBaselineValue(void);
uint16 GetSmallButtonCapInstantValue(void);
uint16 GetFilteredValue(void);
void ResetButtonData(void);

#endif /* TOUCH_H_ */
