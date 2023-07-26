/*
 * PowerSupply.h
 *
 *  	Created on: Feb 17, 2017
 *      Author: Scott Wang
 */

#ifndef POWERSUPPLY_H_
#define POWERSUPPLY_H_

#include "SystemInit.h"


#define ALKLINE 1	// battery type alkaline
#define LITHIUM 2	// battery type lithium

//Battery voltage constant

/*
 *********************************************************************************************************
for hardware adding mid point battery read
This volue are for calculation and doesn't include offset. offset should be canceled in calibration
Value = ((Volt)*4.02/(4.02 + 4.99))/3.5 * 65535
************************************************************************************************************
*/

#define DISCONNECTED		0x20F2	//1 V battery disconnected 
#define LITHIUMBATVOLT     	0xE46F	//7 V, 	Lithium battery
#define GOODFULLBATTERY		0xCA54  //6.2 V
#define LOWVOLTSTPALK		0x8288	//4.0V, Alkaline battery stop operation
#define LOWVOLTWRNALK		0x890F	//4.2V, Alkaline battery warning 
#define STARTVOLTAGE		0x890F	//4.2V, 
#define WAKEUPALK			0xA32B  //5 V,  Alkaline wake up
#define LOWVOLTSTOPLIT		0xA32B	//5.0V, Lithium battery stop
#define LOWVOLTWANLIT		0xBD46	//5.8V, Lithium battery warning
#define WAKEUPLIT			0xC3CD  //6 V,  Lithium wake up
#define CALIBRATIONVOL6		0xC3CD  //6.0 V BAT_read1 calibration voltage
#define CALIBRATIONVOL3		0x61E6  //3.0 V BAT_read2 calibration voltage
#define CALIBRATIONLIMIT    1500	// Calibration limit about 0.2 V
#define FULLBATTERY			0xC3CD	//6.0 V
#define ENDBATTERY 			0x8288	//4.0V
#define STEPS               184     // step for battery % (FULLBATTERY-ENDBATTERY)/100define FULLBATTERY			0xC3CD  //6 V,  

#define FULLSINGLEVOLTAGE   0x3438  //6.4/4 volt
#define RATIO 0.4462
#define BATTERYPOWER	0x55	// Battery power
#define HARDWIREPOWER	0xAA	// Battery power

#define SKIPCHECKTYPE       0x55	// to check battery type

//Public Functions
uint16 ReadSolarCell(void);
uint16 ReadBattery(void);
uint8 CheckBatteryType(void);
BatteryStsType CheckBattery(uint16* wBvolt);
BatteryStsType ConfirmBattery(uint16* wBvolt);
bool IsThereNonFullBattery(void);
uint16 ReadBatteryFull(void);
uint16 ReadBatteryHalf(void);
uint16 CalculateMinBattery(uint16 wWhole,uint16 wMid);
float CalculateMinBatteryUseFloatNumber(uint16 wWhole,uint16 wMid);


#endif /* POWERSUPPLY_H_ */
