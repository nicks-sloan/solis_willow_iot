/*
 * IRSensor.c

 *
 *  	Created on: Feb 17, 2017
 *      Author: Scott Wang
 *      This module contains all functions for detecting user through IR sensor
 */


#include "IRSensor.h"
#include "Timing.h"
#include "Operation.h"
#include "PowerSupply.h"
#include "stdlib.h"

// module variable, these have to stay when battery losing contacts shortly
__attribute__ ((section (".NonVolatileData"))) static uint16 MGVwIRReceiverOffset;  			// IR receiver output offset
__attribute__ ((section (".NonVolatileData"))) static uint16 MGVwNoiseLevel;					// Noise level
__attribute__ ((section (".NonVolatileData"))) static uint16 MGVwIRBackground;  				// Current live Background 
__attribute__ ((section (".NonVolatileData"))) static uint16 MGVwIRInstant;  					// Instant IR echo signal
__attribute__ ((section (".NonVolatileData"))) static uint16 MGVwConfirmedBackground;			// confirmed background value
__attribute__ ((section (".NonVolatileData"))) static uint16 MGVwStableValue;					// stable value
__attribute__ ((section (".NonVolatileData"))) static uint16 MGVwMaxDiff;						// signal change
__attribute__ ((section (".NonVolatileData"))) static uint16 MGVwStableTime;					// Stable time
__attribute__ ((section (".NonVolatileData"))) static uint8 MGVbTGRange;						// Target range
__attribute__ ((section (".NonVolatileData"))) static uint8 MGVbTargetType;						// Type of target
__attribute__ ((section (".NonVolatileData"))) static uint8 MGVbUserRange;						// user range
__attribute__ ((section (".NonVolatileData"))) static uint8 MGVbStableTG;						// flag of stable target
__attribute__ ((section (".NonVolatileData"))) static uint8 MGVbUserSitDown;                   	// flag of user sit down
__attribute__ ((section (".NonVolatileData"))) static uint8 MGVbConfirmedBack;					// flag of confirmed background
__attribute__ ((section (".NonVolatileData"))) static uint8 MGVbConfirmedNB;					// number of times confirmed background repeated 
__attribute__ ((section (".NonVolatileData"))) static BackgroundRecordType MGVsBackRecord[4];  	// last stable background record

// Just for debug
uint16 DwCurrentEcho;
uint16 DwPreEcho;	
uint16 DwIncrese;
uint8 DbHandin;

/*
** ==================================================================================================================================
**     Method      :uint16 GetIRRecOffset(void)
**     Description :
**         This method only export value from the module
**         
**     Parameters  : 			
**     Returns     :	offset in A/D.
** ==================================================================================================================================
*/
uint16 GetIRRecOffset(void){
	
	return MGVwIRReceiverOffset;
}

/*
** ==================================================================================================================================
**     Method      :SetIRRecOffset(uint16 wOffset)
**     Description :
**         This method set value from outside module
**         
**     Parameters  :background in A/D. 			
**     Returns     :
** ==================================================================================================================================
*/
void SetIRRecOffset(uint16 wOffset){
	
	MGVwIRReceiverOffset = wOffset;
}

/*
** ==================================================================================================================================
**     Method      :uint16 GetStableBackground(void)
**     Description :
**        This method only export value from the module
**         
**     Parameters  : None			
**     Returns     :background in A/D.
** ==================================================================================================================================
*/
uint16 GetStableBackground(void){
	
		return MGVwConfirmedBackground;	
}

/*
** ==================================================================================================================================
**     Method      :uint16 GetBackground(void)
**     Description :
**         This method only export value from the module
**         
**     Parameters  :  	None			
**     Returns     :	background in A/D.
** ==================================================================================================================================
*/
uint16 GetBackground(void){
	
		return MGVwIRBackground;	
}

/*
** ==================================================================================================================================
**     Method      :void SetBackground(uint16 wBack)
**     Description :
**         This method set background from outside module
**         
**     Parameters  :background in A/D. 			
**     Returns     :
** ==================================================================================================================================
*/
void SetBackground(uint16 wBack){
	
	if(wBack > MGVwConfirmedBackground){		// Background can't be less than confirmed background 
		MGVwIRBackground = wBack;				// store the data
	}

}

/*
** ==================================================================================================================================
**     Method      :uint8 GetTargetRange(void)
**     Description :
**         This method export target range from the module
**         
**     Parameters  : 	None			
**     Returns     :	Target range
** ==================================================================================================================================
*/
uint8 GetTargetRange(void){
	
		return MGVbTGRange;
	
}

/*
** ==================================================================================================================================
**     Method      :uint8 GetUserRange(void)
**     Description :
**         This method export user range from the module
**         
**     Parameters  : 	None			
**     Returns     :	User range
** ==================================================================================================================================
*/
uint8 GetUserRange(void){
	
		return MGVbUserRange;
	
}

/*
** ==================================================================================================================================
**     Method      :uint16 GetNoiseLevel(void)
**     Description :
**         This method export Noise level from the module
**         
**     Parameters  : 	None			
**     Returns     :	Noise level
** ==================================================================================================================================
*/
uint16 GetNoiseLevel(void){
	
	return MGVwNoiseLevel;
	
}

/*
** ==================================================================================================================================
**     Method      :uint16 GetInstant(void)
**     Description :
**         This method export instant IR echo signal from the module
**         
**     Parameters  :	None			
**     Returns     :	Instant Echo signal
** ==================================================================================================================================
*/
uint16 GetInstant(void){
	
		return MGVwIRInstant;	
}

/*
** ==================================================================================================================================
**     Method      :uint8 IsStableTG(void)
**     Description :
**         This method export target stable flag from the module
**         
**     Parameters  :  	none			
**     Returns     :	result of stable target
** ==================================================================================================================================
*/
uint8 IsStableTG(void){
	
	return MGVbStableTG;
	
}

/*
** ==================================================================================================================================
**     Method      :uint16 GetStableTime(void)
**     Description :
**         This method export signal has stable time from the module
**         
**     Parameters  :	None			
**     Returns     :	Instant Echo signal
** ==================================================================================================================================
*/
uint16 GetStableTime(void){
	
		return MGVwStableTime;	
}

/*
** ==================================================================================================================================
**     Method      :uint16 GetStableValue(void)
**     Description :
**         This method export stable signal from the module
**         
**     Parameters  :	None			
**     Returns     :	Instant Echo signal
** ==================================================================================================================================
*/
uint16 GetStableValue(void){
	
		return MGVwStableValue;	
}

/*
** ==================================================================================================================================
**     Method      :uint16 GetMaxDiff(void)
**     Description :
**         This method export stable signal variation from the module
**         
**     Parameters  :	None			
**     Returns     :	Instant Echo signal
** ==================================================================================================================================
*/
uint16 GetMaxDiff(void){
	
		return MGVwMaxDiff;	
		
}

/*
** ==================================================================================================================================
**     Method      :uint8 GetMGVbConfirmedBack(void)
**     Description :
**         This method export confirmed background flag from the module
**         
**     Parameters  :  	none			
**     Returns     :	flag
** ==================================================================================================================================
*/
uint8 GetMGVbConfirmedBack(void){
	
	return MGVbConfirmedBack;
	
}

/*
** ==================================================================================================================================
**     Method      : uint8 GetMGVbConfirmedNB
**     Description :
**         This method export number of repeated background from the module
**         
**     Parameters  :  	none			
**     Returns     :	repeated number
** ==================================================================================================================================
*/
uint8 GetMGVbConfirmedNB(void){
	
	return MGVbConfirmedNB;
	
}

/*
** ==================================================================================================================================
**     Method      :uint8 IsUserSitDown(void)
**     Description :
**         This method export user sit down flag from the module
**         
**     Parameters  : 
**     		
**     				
**     Returns     :background in A/D.
** ==================================================================================================================================
*/
uint8 IsUserSitDown(void){
	
	return MGVbUserSitDown;
	
}

/*
** ==================================================================================================================================
**     Method      :  BackgroundRecordType GetStableBackRecord(uint8 bIndex)
**     Description :
**         export background record from module
**         
**     Parameters  : record index
**     Returns     : background record
** ==================================================================================================================================
*/
BackgroundRecordType GetStableBackRecord(uint8 bIndex){
	
	return MGVsBackRecord[bIndex];
	
}

/*
** ==================================================================================================================================
**     Method      :  uint16 ReadIREcho(void)
**     Description :
**         This method read IREcho voltage.
**         It also handle A/D Enable/Disable to prevent A/D unnecessary enabled
**     Parameters  : 
**     Returns     : IREcho A/D results. It is a 16 bits data reference at VCC
** ==================================================================================================================================
*/
uint16 ReadIREcho(void){
	
	uint16 wResult;
	uint8 bADCh;
		
		bADCh = IRECHO; 								// Set A/D channel
		AD1_Enable();									// enable A/D
		while(AD1_MeasureChan(TRUE,bADCh ) != ERR_OK);	// Start AD and waiting for the result
		AD1_GetChanValue16(bADCh, &wResult);			// Get result
		AD1_Disable();									// Turn off A/D to save power
		return wResult;		
}

/*
** ==================================================================================================================================
**     Method      :  uint16 ReadCurrent(void)
**     Description :
**         This method reads IR current.(The voltage of IR current pass through a one ohm resister)
**         A/D enable/disable is also handled.
**     Parameters  : none
**     Returns     : IR current A/D reading
** ==================================================================================================================================
*/
uint16 ReadCurrent(void){
	
	uint16 wResult;
	uint8 bADCh; 
	
		bADCh = IRCURRENT;								// set A/D channel
		AD1_Enable();	
		while(AD1_MeasureChan(TRUE, bADCh) != ERR_OK);	// start AD and waiting for the result
		AD1_GetChanValue16(bADCh, &wResult);
		AD1_Disable();
		return wResult;		
}

/*
** ==================================================================================================================================
**     Method      :  void TurnONIRPulse(uint16 wIRLevel)
**     Description :
**         This method turns on IR pulse at input level.
**         Pulse width IRPULSEWIDTH is changeable at built time by #define
**     Parameters  : IR Driver level
**     Returns     : Nothing
** ==================================================================================================================================
*/
void TurnONIRPulse(uint16 wIRLevel){
	
	IRDRV_Enable(IRDRV_DeviceData);				// Enable D/A
	IRDRV_SetValue(IRDRV_DeviceData, wIRLevel);	// IR pulse start. 
	DelayUS(IRPULSEWIDTH);					    // IR width
	IRDRV_SetValue(IRDRV_DeviceData,0);			// IR Pulse stop
	IRDRV_Disable(IRDRV_DeviceData);			// Disable D/A
}

/*
** ==================================================================================================================================
**     Method      :  void MeasureIRCurrent(uint16 IRLevel)
**     Description :
**         This method turns on IR pulse at input level and then measure the IR current.
**         It is only used during PCB test.
**     Parameters  : IR Driver level
**     Returns     : IR current A/D reading. The voltage cross 1 ohm resister R89
** ==================================================================================================================================
*/
uint16 MeasureIRCurrent(uint16 IRLevel){
	
	uint16 wResult;
	
		IRDRV_Enable(IRDRV_DeviceData);				// Enable D/A
		IRDRV_SetValue(IRDRV_DeviceData, IRLevel);	// IR pulse start. 
		DelayUS(IRPULSEWIDTH);						// IR width.
		wResult = ReadCurrent();					// Measure the current
		IRDRV_SetValue(IRDRV_DeviceData,0);			// IR Pulse stop
		IRDRV_Disable(IRDRV_DeviceData);			// Disable D/A
		
		return wResult;
}

/*
** ==================================================================================================================================
**     Method      :  uint16 GetIRLevelForDetectingHangwave(void){
**     Description :
**         This method get minium IR level to achive IR current greater than 500 (around 25 mA)
**         
**     Parameters  : 
**     Returns     : IR level
** ==================================================================================================================================
*/
uint16 GetIRLevelForDetectingHangwave(void){
	
	uint16 wResult;
	uint16 wCurrent;
	uint8 bRep;
	wResult = 700;										// start from  minium
	wCurrent =  MeasureIRCurrent(wResult);				// check the current
	bRep = 0;
	while(wCurrent < 500){								// make sure current is greater than requirment
		SleepMS(10);									// give sometime off between sampleing
		wResult += 10 ;									// increase the IR
		wCurrent  = MeasureIRCurrent(wResult);			// measure the current
		while((wCurrent >= 500) && (bRep < 3)){			// confirming							
			bRep += 1;
			SleepMS(10);								// give sometime off between sampleing
			wCurrent  = MeasureIRCurrent(wResult);		// measure the current
		}
		if(wResult >= NVsOpPara.MaxIRTH){				// high limit
			break;
		}
		bRep = 0;										// reset verification counts
	}
	return wResult;
}

/*
** ==================================================================================================================================
**     Method      : uint8 IRScan(uint16 IRLevel,uint16 *pBeforeIR, uint16 *pAfterIR)
**     Description :
**         This method read receiver outputs before and after IR emiting.
**   
**     Parameters  : 
**     				uint16 IRLevel:  	IR Driver level
**     				uint16 *pBeforeIR:  point to store receiver output data before IR
**     				uint16 *pAfterIR:	point to store receiver output data after IR
**     Returns     : if Data is valid. 
**     				The caller is responsible for processing the result data. 
**     				Even data is invalid, but results are still output
** ==================================================================================================================================
*/
uint8 IRScan(uint16 IRLevel,uint16 *pBeforeIR, uint16 *pAfterIR){
	
	uint16 wEcho1, wEcho2;
	uint8 bResult;
			
		IRRecPwr_ClrVal(); 				// Turn on Receiver
		SleepMS(3);						// Let receiver stable
		wEcho1 = ReadIREcho(); 			// Read offset before IR emitted
		TurnONIRPulse(IRLevel);			// Emit IR
		//DelayUS(15);					// Don't delay
		wEcho2 = ReadIREcho();			// Read echo	
		IRRecPwr_SetVal(); 				// Turn off Receiver	
			
		*pBeforeIR = wEcho1;			// output
		*pAfterIR = wEcho2;				// Output
		if(wEcho1 >= wEcho2){			// Data valid
			bResult = 1;				// Data is good
		}
		else{							// invalid
			bResult  = 0;				// Set data is invalid
		}	
		return bResult;
}

/*
** ==================================================================================================================================
**     Method      : uint8 IRScanTest(uint16 IRLevel,uint16 *pOffset, uint16 *pEcho, uint8 bDel)
**     Description :
**         This method is only used to test when is the best time to read echo after IR pulse.
**  	Parameters  : 
**     				uint16 IRLevel:  IR Driver level
**     				uint16 *pOffset:  Receiver output before IR
**     				uint16 *pEcho:	  Receiver output after IR
**     				uint8 bDel:   Time delay before to read echo after end of IR pulse
**     Returns     : if Data is valid. 
**     				The caller is responsible for processing the result data. 
**     				Even data is invalid, but results are still output
** ==================================================================================================================================
*/
uint8 IRScanTest(uint16 IRLevel,uint16 *pOffset, uint16 *pEcho, uint8 bDel){
	
	uint16 wEcho1, wEcho2;
	uint8 bResult;
		//AD1_Enable();			
		IRRecPwr_ClrVal(); 				// Turn on Receiver
		SleepMS(3);						// Let receiver stable
		wEcho1 = ReadIREcho(); 			// Read offset before IR emitted
		TurnONIRPulse(IRLevel);			// Emit IR
		if(bDel > 0){
			DelayUS(bDel);
		}
		//DelayUS(15);					// Don't delay
		wEcho2 = ReadIREcho();			// Read echo	
		IRRecPwr_SetVal(); 				// Turn off Receiver	
			
		*pOffset = wEcho1;				// output
		*pEcho = wEcho2;				// Output
		if(wEcho1 >= wEcho2){			// Data valid
			bResult = 1;				// Data is good
		}
		else{							// invalid
			bResult  = 0;				// Set data is invalid
		}	
		//AD1_Disable();			
		return bResult;
}

/*
** ==================================================================================================================================
**     Method      : uint16 MeasureEchoVolt(uint16 IRLevel)
**     Description :
**         This method measures echo difference between before and after IR emit.
**         It is target echo 
**     Parameters  : 
**     				uint16 IRLevel:  IR Driver level
**     				
**     Returns     :Echo voltage (Offset-Echo after IR emit).
** ==================================================================================================================================
*/
uint16 MeasureEchoVolt(uint16 IRLevel){
	
	uint16 wEcho1, wEcho2;
	uint16 wResult;
			
		if(IRScan(IRLevel,&wEcho1,&wEcho2)){ 	// IR scan is good
			wResult = wEcho1- wEcho2;			// Valid result, otherwise it is 0 (invalid result)
		}
		else{
			wResult  = 0;						// Set data is invalid
		}
		return wResult;
}

/*
** ==================================================================================================================================
**     Method      : uint8 IRScanForHandWave(void)
**     Description :
**         This method detect if handwave happened.
**   
**     Parameters  : 
**     				
**     Returns     : true if handwave happened
** ==================================================================================================================================
*/
uint8 IRScanForHandWave(void){
	
	uint8 bResult;
	uint16 wCurrentEcho;
	static uint16 wPreEcho = 0;	
	static uint16 wIncrese = 0;
	static uint8 bHandIn = FALSE;
	
	wCurrentEcho = MeasureEchoVolt(NVwHandWaveIR);		// Read echo
	DwCurrentEcho = wCurrentEcho;  	// debug
	DwPreEcho = wPreEcho;			// debug
	
	if(wCurrentEcho > wPreEcho ){
		wIncrese += (wCurrentEcho - wPreEcho);
	}
	else{
		wIncrese = 0;
		if(wPreEcho > (wCurrentEcho + WAVEDIF) || (wCurrentEcho < HANDOUTTHD)){  // move out
			bHandIn = FALSE; // hand is out
		}
	}
	
	DwIncrese = wIncrese;         // debug
	
	if(((wCurrentEcho > HANDINTHD) || (wIncrese > WAVEDIF)) && (bHandIn == FALSE)){  // hand from out to in, it is a hand wave
		bResult = TRUE;
		bHandIn = TRUE;			// hand is in
		wIncrese = 0;
	}
	else{
		bResult = FALSE;
	}
	
	DbHandin = bHandIn;
	wPreEcho = wCurrentEcho;
	return bResult;
}

/*
** ==================================================================================================================================
**     Method      :  uint16 MeasureIRRecOffset(void)
**     Description :
**         This method really measure the offset and initialize the variable
**     Parameters  : 
**     Returns     : A/D of channel IR Echo
** ==================================================================================================================================
*/
uint16 MeasureIRRecOffset(void){
	
	uint8 lp;
	uint16 wTemp1;
	uint32 wTemp2;
	
		wTemp1 =0;
		wTemp2 =0;
		for(lp = 0; lp < 4; lp++){		// Multiple samples
			IRRecPwr_ClrVal(); 			// Turn on Receiver
			SleepMS(3);					// let receiver stable
			wTemp1 = ReadIREcho(); 		// Read echo before IR emitted
			wTemp2 += wTemp1;			// sum
			IRRecPwr_SetVal(); 			// Turn off Receiver
			SleepMS(250);				// let receiver stable
		}	
		wTemp1 = (uint16)(wTemp2 >> 2); // average
		MGVwIRReceiverOffset = wTemp1;	// set the variable
		return wTemp1;
}

/*
** ==================================================================================================================================
**     Method      :MeasureBackground(uint16 wIRLevel,uint16 wInterval, uint8 bLED)
**     Description :
**         This method measure average background from 8 samples.
**         The result is the difference between offset and echo after IR emit
**         The result is the signal from background (environment) and used as a reference of target.
**         This method is part of system start procedure, so LED blink paten is built in
**     Parameters  : 
**     				uint16 IRLevel: 	IR Driver level
**     				uint16 wInterval:	Time interval between samples
**     				uint8 bLED:			Flag of LED blinking
**     Returns     :background in A/D.
** ==================================================================================================================================
*/
uint16 MeasureBackground(uint16 wIRLevel,uint16 wInterval, uint8 bLED){
	
	uint16 wResult;
	uint16 wTemp,wTemp2;
	uint32 dSum;
	uint8 lp;
		
		dSum = 0;
		
		for(lp = 0; lp < 8; lp++){ 						// 8 samples
			if(bLED){									// Used at start up
				BlinkLED(1);
			}
			wTemp = MeasureEchoVolt(wIRLevel);      	// sampling
			dSum += wTemp;
			do{	// start up procedure	
				SleepMS(wInterval);
				wTemp2 = ReadBattery();
			} while(wTemp2 < LOWVOLTWRNALK); 			// stop if battery voltage is not in normal operation range
		}
		wResult = (uint16)(dSum >> 3); 					// Average
		return wResult;	
}

/*
** ==================================================================================================================================
**     Method      : uint8 SearchTargetIRLevel(uint16 wTGThs,uint16 wLowLimit,uint16 wHighLimit,uint16* pIRLevel, uint16* pIREcho)
**     Description :
**         This method uses binary search to find out minimum IR DAC value so that echo is greater than inputed target threshold
**         
**     Parameters  : 
**     				uint16 wTGThs: 		Target threshold
**     				uint16 wLowLimit: 	search low limit
**     				uint16 wHighLimit: 	high limit
**     				uint16* pIRLevel: 	point to store IR level
**     				uint16* pIREcho: 	point to store echo voltage
**     Returns     : Search result
** ==================================================================================================================================
*/
uint8 SearchTargetIRLevel(uint16 wTGThs,uint16 wLowLimit,uint16 wHighLimit,uint16* pIRLevel, uint16* pIREcho){
	
	uint16 wLowEnd, wHighEnd, wTempIR, wHalf;
	uint16 wTempEcho;
	uint8 bResult;
	
		wLowEnd = wLowLimit - 20;				// start at extended range
		wHighEnd = wHighLimit + 20;				// start at extended range
			
		while((wHighEnd - wLowEnd) > 10){ 		// stop condition to continuous binary search 
			wHalf = (wHighEnd - wLowEnd) >> 1;	// Half
			wTempIR = wLowEnd + wHalf;			// mid point
			wTempEcho = MeasureEchoVolt(wTempIR);
			if(wTempEcho >= wTGThs){ 			// IR is too strong
				wHighEnd = wTempIR;	 			// continue on lower half
			}
			else{								// Need increase IR
				wLowEnd = wTempIR;   			// continue on top half
			}
					
			WaitMS(2);							// add interval during scans	
		}
		
		bResult = 0;							// default to not good
		if((wTempIR >= wLowLimit) && (wTempIR <= wHighLimit)){ // in the allowable range
			
			*pIRLevel = wTempIR;				// store the data
			*pIREcho =  wTempEcho;
			bResult = 1;						// result is good
		}
		else if(wTempIR < wLowLimit){			// less than lower limit
			*pIRLevel = wLowLimit;				// use low limit
			*pIREcho =  wTGThs;
		}
		else if(wTempIR > wHighLimit){			// greater than high limit
			*pIRLevel = wHighLimit;				// use high limit
			*pIREcho =  wTGThs;
		}
			
		return bResult;
}

/*
** ==================================================================================================================================
**     Method      : uint8 CalibrateTarget(uint16 wTGThs,uint16* pIRLevel, uint16* pIREcho, uint16* pIROffset)
**     Description :
**         This method find out minimum IR DAC value to get target echo greater than inputed threshold
**         IR level is a 12 bit D/A number. 
**     Parameters  : 
**     				wTGThs: Target threshold
**     				uint16* pIRLevel: point to store IR level
**     				uint16* pIREcho:  point to store echo voltage
**     				uint16* pIROffset: point to store IR receiver offset
**     Returns     : calibration result
** ==================================================================================================================================
*/
uint8 CalibrateTarget(uint16 wTGThs,uint16* pIRLevel, uint16* pIREcho, uint16* pIROffset){
		
	uint16 wTempIR,wTempEcho;
	uint8 bResult;
	uint32 dIRSum, dEchoSum;
	uint8 lp;
				
		dIRSum = 0;
		dEchoSum = 0;
		
		*pIROffset = GetIRRecOffset();
		
		for(lp = 0; lp < 4; lp++){ // repeat 4 times and get average
			
			if(SearchTargetIRLevel(wTGThs,NVsOpPara.MinIRTH,NVsOpPara.MaxIRTH,&wTempIR, &wTempEcho)){//make sure search is correct
				dEchoSum += wTempEcho;
				dIRSum += wTempIR ;	
				bResult = 1;
			}
			else{
				
				dEchoSum = 0;		// special number to indicate fail
				dIRSum = 0;			// special number to indicate fail
				bResult = 0;
				break;
			}
		}
		
		*pIRLevel = (uint16)(dIRSum >> 2);		// average
		*pIREcho =  (uint16)(dEchoSum >> 2);	// average
		
		return bResult;
}

/*
** ================================================================================================================================
**     Method      :  uint16 DefineUserTheshold(uint16 wBackground)
**     Description :
**         This method calculate user threshold based on current background and current user status
**         NVsOpPara.CalibrationEcho is the real echo value when unit was calibrated at 33.5 inch (factory black box) without cover.
**         NVsOpPara.MinUserTH = NVsOpPara.CalibrationEcho + CALOFFSET
**              
**     Parameters  : wBackground: current background
**     Returns     : user threshold
** ================================================================================================================================
*/
uint16 DefineUserTheshold(uint16 wBackground){
	
	uint16 wTemp;
	
	if((NVsOpPara.CalibrationFlag & BACKGRONDCAL) == BACKGRONDCAL){ 	// fixed distance
		wTemp = NVsOpPara.MinUserTH;									// fixed threshold. The value is measured at field calibration.
	}
	else{																// not fixed
		if(wBackground > NVsOpPara.CalibrationEcho){					// strong background, no hysteresis
			if(GVeUserSts != ARMED){
				wTemp = wBackground + NVsOpPara.UserStepInTH;
			}
			else{
				wTemp = wBackground + NVsOpPara.UserStepInTH;
			}
		}
		else{															// clean background
			if(GVeUserSts != ARMED){
				wTemp = NVsOpPara.MinUserTH + OUTHYTERESIS;				// user in
			}
			else{ // Armed
				wTemp = NVsOpPara.MinUserTH + OUTHYTERESIS;				// user in
				if(wBackground < NVsOpPara.CleanBackground){			// Adding Hysteresis on very clean background
					wTemp = NVsOpPara.MinUserTH;						// user out
				}
			}
		}
	}
	return wTemp;	
}

/*
** ==================================================================================================================================
**     Method      : DetermineUserRange(uint16 wIRInst, uint16 wIRBaseline, uint16 wNoise);
**     Description :
**         This method determine the user range based on IR echo, IR baseline and IR noise
**         
**     Parameters  : 
**     				wIRInst	:		Current IR reading
**     				wIRBaseline: 	Baseline
**     				wNoise:			Current noise reading
**     		
**     Returns     : User Range
** ==================================================================================================================================
*/
uint8 DetermineUserRange(uint16 wIRInst, uint16 wIRBaseline, uint16 wNoise){
	
	uint8 bResult;
	uint32 dNoiseImu;
	uint16 wUserTH;
	static uint8 bLEDTick = 0 ;
	
		dNoiseImu = (wNoise << 1);  						// noise immunity, two times the noise
		
		bResult = USERNOTIN;								// default, range 6. so is the range in noise condition
		
		if(wIRInst > dNoiseImu){ 							// make sure signal not in noise range
			
			wUserTH = DefineUserTheshold(wIRBaseline); 		// set user threshold
			
			if(wIRInst > wUserTH){ 							// range 5
				bResult = USERIN;	
			}
			if(wIRInst >= wUserTH + USERHYSIN){				// range 4
				bResult = USERFAR;	
			}
			if(wIRInst >= wUserTH + USERFARTHD){			// range 3
				bResult = USERMID;
			}
			
			if(wIRInst >= wUserTH + USERMIDTHD){			// range 2
				bResult = USERNEAR;
			}
			
			if(wIRInst >= wUserTH + NVsOpPara.SittingTH){	// range 1
				bResult = USERSIT;
			}

		}
		
		
	//	else{ // noise range
			if(wNoise > NOISEENV ){		// noise big enough, blink LED. This is to match G2 behavier
				bLEDTick += GVbDutyRate;
				if(bLEDTick >= 12){		// every 3 seconds
					bLEDTick = 0 ;
					BlinkLED(1);
				}
			}
		//}
		
		return bResult;
				
}

/*
** ==================================================================================================================================
**     Method      : uint8 DetermineTargetRange(uint16 wIRInst)
**     Description :
**         This method determine the target range based on the signal.
**         Note target is the combination of user and background.
**         
**     Parameters  : 
**     				wIRInst	:	Current IR reading 		
**     Returns     : Target range
** ==================================================================================================================================
*/
uint8 DetermineTargetRange(uint16 wSignal){
	
	uint8 bResult;
	
		bResult = ZONE6; 							// Range 6
		if(wSignal > CLEANBACK){ 					// range 5
			bResult = ZONE5;	
		}
		if(wSignal >= HIGHBACK){					// range 4
			bResult = ZONE4;	
		}
		if(wSignal >= STANDFAR){					// range 3
				bResult = ZONE3;
		}
		if(wSignal >= STANDCLOSE){					// range 2
				bResult = ZONE2;
		}
		if(wSignal >= SITDOWN){						// range 1
			bResult = ZONE1;
		}

		return bResult;
				
}

/*
** ==================================================================================================================================
**     Method      :  uint8 VerifyConfirmedBackground(uint16 wNew)
**     Description :
**         This method verify if the input is a confirmed background.
**         The value has to be greater than existing confirmed background and less than predefined maximum background
**         The same value has to repeat 3 times to be a confirmed background.
**         The valid value will be stored in record
**         
**     Parameters  : uint16 wNew : new value
**     Returns     : true or false if the new value has repeated predefined times
** ==================================================================================================================================
*/
uint8 VerifyConfirmedBackground(uint16 wNew){
	
	uint8 bLp;
	
		if((wNew <= MGVwConfirmedBackground) ||(wNew > NVsOpPara.MaxBackground)) {	// Make sure the new value is greater than current confirmed and less than limit
			return FALSE;
		}
		
		bLp = 0;
		do{// compare with record
			if(abs(wNew - MGVsBackRecord[bLp].wStableValue) <  NVsOpPara.TargetStableRange){  	// it appeared before
				if(MGVsBackRecord[bLp].bRepeatCT < 100){						// set up limit
					MGVsBackRecord[bLp].bRepeatCT++;							// update repeat number
				}
				MGVbConfirmedNB = MGVsBackRecord[bLp].bRepeatCT;				// for debugging 
				MGVsBackRecord[bLp].wStableValue = wNew;						// update record
				if(MGVsBackRecord[bLp].bRepeatCT >= 3){							// Already confirmed
					MGVwConfirmedBackground = wNew; 							// update to real 
					NVsOpPara.ConfirmedBackground = MGVwConfirmedBackground; 	// record it
					return TRUE;
				}
				break;
			}
			bLp++;	
		}while (bLp < 4);
		
		if(bLp == 4){ 													// no match value in records, not the repeat value.
			for(bLp = 3; bLp >= 1; bLp--){								// update record
				MGVsBackRecord[bLp] = MGVsBackRecord[bLp-1];			// shift data
			}
			MGVsBackRecord[0].bRepeatCT = 1;							// push new data in
			MGVsBackRecord[0].wStableValue = wNew;	
		}
		return FALSE;
}

/*
** ==================================================================================================================================
**     Method      : uint16 UpdateBackground(uint8 bTGType, uint8 bUserrange, uint16 wSignal, uint8 bScanRate)
**     Description : Manage all background related data and update background
**     Parameters  : 
**     				uint8 bTGType:		Target type
**     				uint8 bUserrange:	user range
**     				uint16 wSignal:		Current signal
**     				uint8 bScanRate:	Scan rate
**     Returns     : Background
** ====================================================================================================================================
*/
uint16 UpdateBackground(uint8 bTGType, uint8 bUserRange, uint16 wSignal, uint8 bScanRate){
	
	uint16 wTempBack;
	uint16 wTempDiff;
	static uint8 bBaselineDownCT = 0; 		// count for signal is continuously less than background
	static uint8 bBaselineUpCT = 0;			// count for signal is continuously greater than background
	static uint8 bStableTG = 0;				// flag of short time stable target, used to identify a user
	static uint16 wStableCT = 0;			// count of continuously stable signal
	static uint16 wStayInCT = 0;			// count of user has been detected
	static uint8 bSatbleDebunce = 0;		// debunce for stable signal
	 
		wTempBack = MGVwIRBackground;   			// default to previous background
		
		// 1. Signal less than current background
		if(wSignal <= MGVwIRBackground){			// current signal less than background, need tracing background to current reading
			bBaselineDownCT += bScanRate; 			// add debunce
			if(bBaselineDownCT > BASEDOWNBUNCE){
				wTempBack = wSignal;				// tracing to new background
				bBaselineDownCT = 0;
			}
			bBaselineUpCT = 0;						// restart baseline up debunce
		}
		// 2. Signal greater than current background
		else{										// current signal is greater than background
			if(bUserRange > USERIN){ 				// there is no user, need to update background 
				bBaselineUpCT += bScanRate;			// add debunce
				if(bBaselineUpCT > BASEUPBUNCE){
					wTempBack = GetDataMovingEverage(wSignal,wTempBack);		// Background moving average.
					bBaselineUpCT = 0;
				}
			}
			bBaselineDownCT = 0;       				 // reset baseline down count
		}
		
		// 3. user has been detected too long
		if(bTGType == ISUSER){
			wStayInCT += bScanRate; 				// update stay time
			if(wStayInCT >= MAXSTAYTIME){			// Target has been detected too long, it is possible that environment changed
				wTempBack = wSignal;				// Tracing current signal
				wStayInCT = 0;						// reset count
			}
		}
		else{
			wStayInCT = 0;							// reset count
		}
		
		// 4. signal is stable
		wTempDiff = (uint16) abs(wSignal - MGVwStableValue);
		if(wTempDiff < NVsOpPara.TargetStableRange){ 	// stable signal
			// 4.1 Signal stable for short period of  time, target maybe the background, set as current background, but it can be changed afterward
			if(!bStableTG){			// only update if the flag has not set yet to make sure below action is only done once.
				wStableCT += bScanRate; 							// increase count
				if(wStableCT >= NVsOpPara.StableTimeTH){			// long enough
					bStableTG = TRUE;								// set flag
					if(DetermineTargetRange(wSignal) >= ZONE4){ 	// update back ground only when target is in certain range
						wTempBack = wSignal;						// treat current target is background
					}
					
				}
			}
			// 4.2 signal stable for long time, target is sure background, once set, it can't be reduced.
			if(!MGVbConfirmedBack){									// not confirmed yet
				MGVwStableTime += bScanRate;						// increase time count
				if(MGVwStableTime >= NVsOpPara.ConfirmTimeTH){ 		// long enough to confirm it is background
					MGVbConfirmedBack = TRUE;						// set the flag
					if(VerifyConfirmedBackground(wSignal)){			// check to verify if this is a confirmed background.
						wTempBack = wSignal;						// it is new confirmed background
					}
				}
			}
			
			if(wTempDiff > MGVwMaxDiff){
				MGVwMaxDiff = wTempDiff;
			}
		}
		else{ // not stable signal. 
			if(bSatbleDebunce <= 2){ // add debunce
				bSatbleDebunce++;
			}
			else{// start of new verification cycle
				MGVwStableValue = wSignal;			// restart reference of stable signal
				bStableTG = FALSE;					// clear flag for short time stable
				wStableCT = 0;						// reset count for short period stable 
				MGVwStableTime = 0;					// reset count for long time stable
				MGVbConfirmedBack = FALSE;			// clear flag for long time stable
				MGVwMaxDiff = 0;
				bSatbleDebunce = 0 ;				// reset debunce
			}
		}
		
		// finally make sure the background should not be less than confirmed background
		if(wTempBack > MGVwConfirmedBackground){				// Background is greater than confirmed background 
			MGVwIRBackground = wTempBack;						// Set the new background
		}
		else{// background reduced
			wTempBack = MGVwConfirmedBackground; 				// keep the higher confirmed background to prevent the problem in case higher background back again after usage 
			MGVwIRBackground = MGVwConfirmedBackground; 		// Set the background
		}
		
		MGVbStableTG = bStableTG;
		return wTempBack;

}

/*
** ==================================================================================================================================
**     Method      : uint8 ScanTarget(uint16 IRLevel, UserStsType eTsts, uint8 bScanRate)
**     Description : IR Scan. Manage all IR scan related data, and finally determine if the target is a user or the background
**     Parameters  : 
**     				IRLevel:  		IR Driver level D/A
**     				eTsts:			Current user state
**     				bScanRate:		Scan rate
**     Returns     : Target type
** ====================================================================================================================================
*/
uint8 ScanTarget(uint16 IRLevel, UserStsType eTsts, uint8 bScanRate){
	
	uint16 wTemp, wEcho1, wEcho2, wTempOffset, wTempBack, wTempSG, wTempNoise;
	uint8 bTemp, bTRange, bUserRange, bTGType;
	static uint16 wSitDownCT = 0;							// count of user in sit down range
	static uint16 wStandUpCT = BENDINGTM;					// count of user has out off sit down range
	static uint8 bDetSitDown = TRUE;						// Flag to detect sit down user
	
		bTGType = MGVbTargetType; 							// default to previous type. so if scan failed, the type should stay unchanged
		
		// IR scan to get data
		bTemp = IRScan(IRLevel,&wEcho1,&wEcho2);			// IR scan
				
		// Determine optical and EMI noise. This is the echo variance before IR emitted
		wTemp = MGVwIRReceiverOffset;						// historic offset
		wTempNoise = (uint16)abs(wEcho1 - wTemp);			// noise is the difference between current reading and historic value 
		MGVwNoiseLevel = wTempNoise;						// store noise data
		wTempOffset = GetDataMovingEverage(wEcho1,wTemp);  	// get filter offset	
		MGVwIRReceiverOffset = wTempOffset;					// update and store the result
		
		// Determine Background and target type
		if(bTemp){	// process data only when scan is valid, otherwise only offset and noise data would be updated from this scan
			
			wTempBack = MGVwIRBackground;   				// previous background
			wTempSG = wEcho1 - wEcho2;						// real IR response signal
			MGVwIRInstant = wTempSG;						// store result
			bTRange = DetermineTargetRange(wTempSG); 		// Determine target range
			MGVbTGRange = bTRange;							// store the data
			bUserRange	= DetermineUserRange(wTempSG, wTempBack, wTempNoise); // determine user range
			MGVbUserRange = bUserRange;						// store the data
			
			// determine if the user sit down
			switch(eTsts){
				case  NOTPRESENT:
					if(bUserRange == USERSIT){					// in sit down range
						bDetSitDown = FALSE;					// Do not detect sit down user
					}
					else{
						bDetSitDown = TRUE;						// Set Flag to detect sit down user
					}
				break;
				case ENTER:
					if(bUserRange == USERSIT){					// in sit down range
						bDetSitDown = FALSE;					// Do not detect sit down user
					}
				break;
				case NONEARMED:
					if(bUserRange == USERSIT){					// in sit down range
						bDetSitDown = FALSE;					// Do not detect sit down user
					}
					wSitDownCT = 0;								// clear count for new cycle
					wStandUpCT = BENDINGTM;						// clear count for new cycle
					MGVbUserSitDown = FALSE;					// clear flag for new cycle
				break;
				case ARMED:
					if(bDetSitDown == TRUE){
						if(bUserRange == USERSIT){				// in sit down range
							if(MGVbUserSitDown == FALSE){		// sit down has not confirmed yet
								wSitDownCT += bScanRate; 		// increase count
								if(wSitDownCT >= SITDOWNDELAY){ // in sit down range long enough 
									MGVbUserSitDown = TRUE;		// set flag
									wStandUpCT = 0 ;			// start stand up count
								}
								if(wStandUpCT <= BENDINGTM){	// user was sit down before, treat it as bending, user is still sit down but just bended.  
									MGVbUserSitDown = TRUE;		// set flag	
									wStandUpCT = 0 ;			// clear stand up count
								}
							}	
						}
						else{  // user not in sit down range	
							if(wStandUpCT <= BENDINGTM){		// left sit down range not long enough, may be just bending
								wStandUpCT += bScanRate; 		// increase out range count
							}
							MGVbUserSitDown = FALSE;			// reset flag
							wSitDownCT = 0;						// reset count
						}
					}	
				break;
				
				default:
				break;
					
			}
							
			// Determine target Type
			if(bUserRange == USERNOTIN){						// only in this range, target is considered as background only
				bTGType = ISBACKGROUND;							// target is background
			}
			else{												// any other range
				bTGType = ISUSER;								// target is a user
			}
			
			// update background
			UpdateBackground(bTGType, bUserRange, wTempSG, bScanRate);
				
		}
		
		return bTGType;
}

/*
** ==================================================================================================================================
**     Method      :  void IRSensorInit(void)
**     Description :
**         This method initialize all module scope data in this module related to IR sensor detection algorithm.
**     Parameters  : Nothing
**     Returns     : Nothing
** ==================================================================================================================================
*/
void IRSensorInit(void){
	
	uint8 bLp;
	
		MGVwIRReceiverOffset = MeasureIRRecOffset();			// MGVwIRReceiverOffset;  
		MGVwNoiseLevel = 0;										// noise level
		MGVwIRBackground = MeasureEchoVolt(NVsOpPara.IRLevel); 	// live background
		MGVwIRInstant = MGVwIRBackground;						// IR instant signal
		MGVwConfirmedBackground = FALSE;						// confirmed background
		MGVwStableValue = 1000;									// confirmed background value
		MGVwMaxDiff = 0;										// signal change
		MGVwStableTime = 0;										// Stable time
		MGVbTGRange = ZONE6;									// Target range 
		MGVbTargetType = ISBACKGROUND;							// current target is a background
		MGVbUserRange = USERNOTIN;								// user range
		MGVbStableTG = FALSE;									// target stable flag		
		MGVbUserSitDown = FALSE;								// user sit down flag
		MGVbConfirmedBack = FALSE;								// flag of confirmed background
		MGVbConfirmedNB = 0;									// number of times confirmed background repeated 
		for(bLp = 0;bLp < 4;bLp++){								// confirmed background records
			MGVsBackRecord[bLp].bRepeatCT = 0;
			MGVsBackRecord[bLp].wStableValue = 0;
		}		
		if(MGVwIRBackground > NVsOpPara.MaxBackground){ 		// make sure it is not out of limit
			MGVwIRBackground = NVsOpPara.MaxBackground;
		}
		
}

