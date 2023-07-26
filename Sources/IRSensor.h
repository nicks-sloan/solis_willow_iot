/*

 * IRSensor.h
 *
 *  Created on: Feb 17, 2017
 *      Author: Scott Wang
 */

#ifndef IRSENSOR_H_
#define IRSENSOR_H_

#include "SystemInit.h"

// stable background record
typedef struct StableBack{
	uint8 bRepeatCT;
	uint16 wStableValue;
}BackgroundRecordType;

// Operation constants
#define IRPULSEWIDTH    16			// IR pulse width. 22 measured as 25 us, 16 measured as 20 us

#define WINDOWCOVERED   2500		// Threshold for window been covered
#define BASEUPBUNCE		60			// Time to wait for updating background if signal is greater than background in 1/4 second
#define BASEDOWNBUNCE	20			// Time to wait for updating background when signal is less than background in 1/4 second
#define MAXSTAYTIME 	14400		// in 1/4 seconds. 60 minute if target has been detected longer than this, go to update IRBackground
#define SITDOWNDELAY 	40         	// Time threshold of defining sit down in 1/4 second. 10 seconds 
#define BENDINGTM 		12          // Extended on delay for sitting user in 1/4 second. 3 seconds
#define CALOFFSET       400         // Calibration offset
#define NOISEENV		2000		// Noise envirment threshold

// target range definition
#define ZONE6			6
#define CLEANBACK     	4800
#define ZONE5			5
#define HIGHBACK		6000
#define ZONE4			4
#define STANDFAR        10000
#define ZONE3			3
#define STANDCLOSE      12000
#define ZONE2			2
#define SITDOWN         15000
#define ZONE1			1
#define HANDWAVE        22000
#define WAVEDIF         5000
#define HANDINTHD       5000
#define HANDOUTTHD      1500
#define SCANHANDCURRET  900
#define HANDWAVECNT     3
#define HANDWAVEINT		40

// user range definition
#define OUTHYTERESIS    1000
#define TGINVALID		7
#define USERNOTIN		6
#define USERIN			5
#define USERHYSIN		1000
#define USERFAR			4
#define USERFARTHD		4000
#define USERMID			3
#define USERMIDTHD		8000
#define USERNEAR		2
#define USERNEARTHD		12000
#define USERSIT			1

#define MINIUMIR        850
#define MAXIUMIR        3000

// Public functions
uint16 ReadIREcho(void);
uint8 ScanTarget(uint16 IRLevel, UserStsType eTsts, uint8 bScanRate);
uint16 MeasureIRCurrent(uint16 IRLevel);
uint8 CalibrateTarget(uint16 wTGThs,uint16* pIRLevel, uint16* pIREcho, uint16* pIROffset);
uint16 MeasureEchoVolt(uint16 IRLevel);
uint16 GetBackground(void);
uint8 IRScanTest(uint16 IRLevel,uint16 *pOffset, uint16 *pEcho, uint8 bDel);
uint8 IsUserSitDown(void);
uint16 GetInstant(void);
uint16 GetIRRecOffset(void);
uint16 GetNoiseLevel(void);
uint8 GetUserRange(void);
uint8 GetTargetRange(void);
uint16 MeasureBackground(uint16 wIRLevel,uint16 wInterval, uint8 bLED);
uint8 IsStableTG(void);
void SetBackground(uint16 wBack);
void IRSensorInit(void);
uint16 GetStableBackground(void);
uint8 GetMGVbConfirmedBack(void);
uint8 GetMGVbConfirmedNB(void);
BackgroundRecordType GetStableBackRecord(uint8 bIndex);
uint16 GetStableTime(void);
uint16 GetStableValue(void);
uint16 GetMaxDiff(void);
uint8 IRScanForHandWave(void);
uint16 GetIRLevelForDetectingHangwave(void);

extern uint16 DwCurrentEcho;
extern uint16 DwPreEcho;
extern uint16 DwIncrese;
extern uint8 DbHandin;

#endif /* IRSENSOR_H_ */
