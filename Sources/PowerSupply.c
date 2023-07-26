/*
 * PowerSupply.c
 *
 *  Created on: Feb 17, 2017
 *      Author: Scott Wang
 *      This module contains all functions for power supply managment
 */

#include "PowerSupply.h"
#include "Timing.h"

/*
** =====================================================================================================================
**     Method      :  ReadSolarCell(void)
**     Description :
**         This method read Solar voltage
**        
**     Parameters  : Nothing
**     Returns     : Solar voltage A/D in 16 digits
** =====================================================================================================================
*/
uint16 ReadSolarCell(void){
	
	uint16 Result;
	uint8 bADCh;
	
		bADCh = SVOLTAGE;								// Set A/D channel to solar voltage
		AD1_Enable();									// enable A/D
		SolarCheck_SetVal(); 							// Turn on power check switch 
		DelayUS(500);									// Stable, let the static leakage discharged
		while(AD1_MeasureChan(TRUE, bADCh) != ERR_OK);	// Start AD and waiting for the result
		AD1_GetChanValue16(bADCh, &Result);				// Get result
		SolarCheck_ClrVal(); 							// Turn off power check switch
		AD1_Disable();									// Disable A/D to save power
		
		return Result;
		
}


/*
** =====================================================================================================================
**     Method      :  ReadBatteryFull(void)
**     Description :
**         This method read battery voltage at full point (total voltage)
**         The A/D reading doesn't include offset on switch drop
**     Parameters  : Nothing
**     Returns     : battery voltage A/D in 16 digits
** =====================================================================================================================
*/
uint16 ReadBatteryFull(void){
	
	uint16 Result;
	uint8 bADCh;
	
		bADCh = BVOLTAGE;								// Set A/D channel to battery voltage
		AD1_Enable();									// enable A/D
		BatteryCheck_SetVal(); 							// Turn on power check switch 
		DelayUS(30);									// let it stable
		while(AD1_MeasureChan(TRUE, bADCh) != ERR_OK);	// Start AD and waiting for the result
		AD1_GetChanValue16(bADCh, &Result);				// get result
		BatteryCheck_ClrVal(); 							// Turn off power check switch
		AD1_Disable();									// Disable A/D to save power
		//Result += GVwOffSet1;							// adding offset
		return Result;		
}

/*
** =====================================================================================================================
**     Method      :  ReadBatteryHalf(void)
**     Description :
**         This method read battery voltage at mid contact (half voltage)
**        
**     Parameters  : Nothing
**     Returns     : battery voltage A/D in 16 digits
** =====================================================================================================================
*/
uint16 ReadBatteryHalf(void){
	
	uint16 Result;
	uint8 bADCh;
	
		bADCh = BMIDVOLTAGE;							// Set A/D channel to battery voltage
		AD1_Enable();									// enable A/D
		BatteryMidCheck_SetVal(); 						// Turn on power check switch 
		DelayUS(30);									// let it stable
		while(AD1_MeasureChan(TRUE, bADCh) != ERR_OK);	// Start AD and waiting for the result
		AD1_GetChanValue16(bADCh, &Result);				// get result
		BatteryMidCheck_ClrVal(); 						// Turn off power check switch
		AD1_Disable();									// Disable A/D to save power
		//Result += GVwOffSet2;							// adding a offset
		return Result;
		
}

/*
** =====================================================================================================================
**     Method      : uint16 CalculateMinBattery(uint16 wWhole,uint16 wMid)
**     Description :
**         This method calculate minimum single battery voltage. 
**         The result is already include offset, so the real voltage should be result/65535*3.5/RATIO
**         
**     Parameters  : 
**     				uint16 wWhole full batery voltage A/D
**     				uint16 wMid	A/D value from mid battery contact
**     Returns     : 
**     				uint16 equivalent Minimum battrey value
**     	
** =====================================================================================================================
*/
uint16 CalculateMinBattery(uint16 wWhole,uint16 wMid){
	uint16 wTemp,wTemp2, wMin, wResult;
	
	if(wWhole < wMid){					// Verify input
		return 0;						// terminate with 0
	}

	if(wMid < DISCONNECTED){			// Verify input, no mid power
		return wWhole;					// terminate with whole 
	}
	wTemp = wWhole - wMid;				// another half battery voltage
	if(wTemp > wMid){					// minium battery in measured half
		wTemp2 = (wTemp >> 1);			// Average single battery voltage in second half
		if(wMid > wTemp2){
			wMin = wMid - wTemp2;      	// single Minimum voltage battery on measured half
		}
		else{							// it shouldn't happen
			wMin = (wMid>>1);			// half of this middle voltage
		}
		//wResult = ADJUST1 + (wMin<<2);	// 4 times singleand adjustment
		wResult = (wMin<<2);			// 4 times single
	}
	else{								// Minimum battery in another half.
		wTemp2 = (wMid >> 1);			// Average single battery voltage in measured half
		if(wTemp > wTemp2){	
			wMin = wTemp - wTemp2 ; 	// minimum volatge on second half
			
		}
		else{
			wMin = (wTemp>>1);			// half of this middle voltage
		}
		
		//wResult = (wMin<<2)- ADJUST2;	// 4 times single and adjustment
		wResult = (wMin<<2);			// 4 times single and adjustment
	}	
	
	if(wResult > wWhole){
		wResult = wWhole;				// calculated voltage shouldn't greater than measured whole voltage.
	}
	
// extra consideration 
	if(wResult > wWhole){				// calculated greater than whole measured, there is ameasuring error
		wResult = wWhole;
	}
	else{
		if((wResult <= LOWVOLTWRNALK) && ((wResult + 1700) > wWhole)){ 	// when both calculated and measured whole are closing warning threshold
			wResult = wWhole;											// measured whole is more accurate.
		}
	}
	
	return wResult;
}

/*
** =====================================================================================================================
**     Method      :  ReadBattery(void)
**     Description :
**         This method read battery voltage, the voltage is calculated as 4 X minimum single battery voltage
**        
**     Parameters  : Nothing
**     Returns     : battery voltage A/D in 16 digits
** =====================================================================================================================
*/
uint16 ReadBattery(void){
	
	uint16 wTempF, wTempM, wMin;
		
		wTempF = ReadBatteryFull();						// Read at full point
		wTempM = ReadBatteryHalf(); 					// Read at mid point
		
		if(wTempF < DISCONNECTED){
			return wTempF;								// don't add offset incase overflow
		}
		wTempF += NVsOffset.wOff1;						// add offset
		
		if(wTempM <= DISCONNECTED){						// hardwire 
			return wTempF;								// Read at full point									
		}
		wTempM += NVsOffset.wOff2;						// Add offset
		wMin = CalculateMinBattery(wTempF,wTempM);		// calculate equivalent minimum battery
		return wMin;		
}

/*
** =====================================================================================================================
**     Method      : uint8 IsThereNonFullBattery(void)
**     Description :
**         This method check if there is a non-full battery installed
**         
**     Parameters  : 
**     				none
**     				
**     Returns     : 
**     				FALSE: no
**     				TRUE: yes
**     	
** =====================================================================================================================
*/
bool IsThereNonFullBattery(void){

	bool bResult;
	
	bResult = TRUE;	// set default
	
	if(ReadBattery() > GOODFULLBATTERY){ 	// computed battery are good
	
		bResult = FALSE;
	}
	if(ReadBatteryHalf() <= DISCONNECTED){	// It's hardwire power
		bResult = FALSE;
	}
	return bResult;
}

bool IsThereNonFullBatteryold(void){

	bool bResult;
	
	bResult = TRUE;	// set default
	if((ReadBatteryFull() + NVsOffset.wOff1) > GOODFULLBATTERY){ // battery at full point greater than threshhold
		bResult = FALSE;
	}
	if(ReadBatteryHalf() <= DISCONNECTED){					// It's hardwire power
		bResult = FALSE;
	}
	return bResult;
}

/*
** =====================================================================================================================
**     Method      :  CheckBatteryType
**     Description :
**         This method determines battery type based on voltage
**         This is only valid when battery is brand new
**     Parameters  : Nothing
**     Returns     : battery type, Alkaline or lithium
** =====================================================================================================================
*/
uint8 CheckBatteryType(void){
	
	if(ReadBattery() >= LITHIUMBATVOLT){
		return LITHIUM;
	}
	return ALKLINE;
	
}

/*
** =====================================================================================================================
**     Method      : BatteryStsType CheckBattery(uint16* wBvolt)
**     Description :
**         This method determines battery status based on reading of battery voltage
**         
**     Parameters  : point to store battery reading
**     Returns     : battery status
**     	
** =====================================================================================================================
*/
BatteryStsType CheckBattery(uint16* wBvolt){
	uint16 wTemp;
	BatteryStsType eTempBSts;
	
		wTemp = ReadBattery();							// Read battery voltage
		*wBvolt = wTemp;								// report reading
		if(wTemp > GVsBatteryTH.wLowVolt){				// voltage is greater than low voltage threshold
			eTempBSts = BNORMAL;
		}
		else{											// voltage less than o equal to warning threshold
			if(wTemp > GVsBatteryTH.wStopVolt){			// greater than stop but less than low
				eTempBSts = BWARNING;
			}
			else{										// voltage voltage less than stop threshold
				if(wTemp > DISCONNECTED){				// greater than disconnected but less than stop
					eTempBSts = BSTOP;
				}
				else{									// voltage less than disconnected threshold
					eTempBSts = BDISCONNECT;
				}
			}
		}
		return eTempBSts;
}

/*
** =====================================================================================================================
**     Method      : BatteryStsType ConfirmBattery(uint16* wBvolt)
**     Description :
**         This method read battery 4 times to get average to confirm the battery status
**         
**     Parameters  : point to store battery reading
**     Returns     : battery status
**     	
** =====================================================================================================================
*/
BatteryStsType ConfirmBattery(uint16* wBvolt){
	uint16 wTemp;
	BatteryStsType eTempBSts;
	uint32 dSum;
	uint8 bLp;
	
	dSum = 0;
	for (bLp = 0 ; bLp < 4; bLp++){
		dSum += ReadBattery();		
	}
	wTemp = (uint16)(dSum >> 2);    // Average
	*wBvolt = wTemp;				// update
	
	if(wTemp > GVsBatteryTH.wLowVolt){			// voltage is greater than low voltage threshold
		eTempBSts = BNORMAL;
	}
	else{
		if(wTemp > GVsBatteryTH.wStopVolt){	// voltage is greater than stop but less than low
			eTempBSts = BWARNING;
		}
		else{						// voltage is less than stop threshold
				eTempBSts = BSTOP;
			}
	}
	return eTempBSts;
}

