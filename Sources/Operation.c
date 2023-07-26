/*
 * Operation.c
 *
 *  	Created on: Feb 20, 2017
 *      Author: Scott Wang
 *      This module contains all the operations in all states
 */


#include "Operation.h"
#include "PowerSupply.h"
#include "IRSensor.h"
#include "TouchSensor.h"
#include "UART.h"
#include "Timing.h"
#include "BLE.h"
#include "Ant.h"

#include "NCP_Helpers.h"

// Just for debug
uint8 DbHandWave;
uint8 DbHandWaveCT;
uint8 DbInterv;


#define CURRENTFEEDBACK

void LatchSolenoid(void){
	uint16 wTemp;
	BatteryStsType sTempBSts;
	
	sTempBSts = CheckBattery(&wTemp);		// Check battery before latching the solenoid to prevent failure of unlatch.
	if(sTempBSts > BSTOP){ 					// Make sure power supply is good
		
#ifdef CURRENTFEEDBACK
		LatchCheck_SetVal();    			// power on current feedback circuitry
		EnableLATCHCurrentSensing();		// connect latch current feedback
		WaitMS(2);							// let it stable, it is measured as 4 ms	
		NVbSolenoidSts = SOLENOIDON;		// Set solenoid on flag
		LATCH_SetVal();						// turn latch gate on
		DelayMS(1);							// first 1 ms to skip interrupt
		Latchhappened = FALSE;				// Clear latch flag
		WaitMS(6);							// waiting for current feedback or timer expire 
		if(Latchhappened){
			DelayUS(300);					// Extra 300 micro second delay
		}
		else{
			DelayUS(500);					// Extra 0.5 ms delay
		}
		LATCH_ClrVal();						// turn off latch gate
		LatchCheck_ClrVal();				// power off current feedback circuitry
		DisableLATCHCurrentSensing();		// Disable the pin to prevent current leak
#else
		NVbSolenoidSts = SOLENOIDON;		// Set solenoid on flag
		LATCH_SetVal();						// turn latch gate on
		DelayMS(7);							// 7 ms
		DelayUS(500);						// Extra 0.5 ms total 7.5 ms
		LATCH_ClrVal();						// turn off latch gate
#endif	
		
		// Handle activation number here
		NVsOpPara.TotalActivation += 1;		// Increase activation #
		if(sTempBSts == BWARNING){
			NVsOpPara.LBActivationCT += 1; 	// activation in low battery
		}
		else{
			NVsOpPara.LBActivationCT = 0; 
		}
		
		/*MVP*/
			if (GVbDIAGVALVEFLG){
				GVbDiagValveStat = 1; 
			}
	}	
}

/*
** ===========================================================================================================================
**     Method      : LatchSolenoid(void)
**     Description :
**         This method turns solenoid on. On pulse width depends on current feedback circuit, but set a maxim with 8 ms
**         Check battery before turn on
**        
**     Parameters  : Nothing
**     Returns     : Nothing
** ===========================================================================================================================
*/
void LatchSolenoidold(void){
	uint16 wTemp;
	BatteryStsType sTempBSts;
	
	sTempBSts = CheckBattery(&wTemp);		// Check battery before latching the solenoid to prevent failure of unlatch.
	if(sTempBSts > BSTOP){ 					// Make sure power supply is good
		LatchCheck_SetVal();    			// power on current feedback circuitry
		//EnableLATCHCurrentSensing();		// connect latch current feedback
		WaitMS(2);							// let it stable, it is measured as 4 ms
		NVbSolenoidSts = SOLENOIDON;		// Set solenoid on flag
		LATCH_SetVal();						// turn latch gate on
		WaitMS(8);							// waiting for current feedback or timer expire 
		//DelayUS(500);						// Extra 0.5 ms delay
		LATCH_ClrVal();						// turn off latch gate
		LatchCheck_ClrVal();				// power off current feedback circuitry
		//DisableLATCHCurrentSensing();		// Disable the pin to prevent current leak
		// Handle activation number here
		NVsOpPara.TotalActivation += 1;		// Increase activation #
		if(sTempBSts == BWARNING){
			NVsOpPara.LBActivationCT += 1; 	// activation in low battery
		}
		else{
			NVsOpPara.LBActivationCT = 0; 
		}
		
		/*IoT*/
		if (GVbDIAGVALVEFLG){
			GVbDiagValveStat = 1; 
		}
	}	
}

/*
** ===========================================================================================================================
**     Method      : void UnLatchSolenoid(void)
**     Description :
**         This method turn solenoid off. turn off pulse is fixed width at 7.5 ms    
**     Parameters  : Nothing
**     Returns     : Nothing
** ===========================================================================================================================
*/
void UnLatchSolenoid(void){
	
	UNLATCH_ClrVal();					// turn on unlatch gate
	//WaitMS(7);			
	DelayMS(7);							// pulse width
	DelayUS(500);						// total 7.5 ms	
	UNLATCH_SetVal();					// turn off unlatch gate
	NVbSolenoidSts = SOLENOIDOFF;		// Clear solenoid on flag
}


/*
** =========================================================================================================================
**     SolenoidOnTimeType SetValveOnTime(uint8 bSFullOn)
**     Description :
**         This method find out valve on time of Grand mode full, grand mode lite, and standard lite from predefined table
**     Parameters  : Standard full flush time
**     Returns     : Nothing
** =========================================================================================================================
*/
SolenoidOnTimeType SetValveOnTime(uint8 bSFullOn){
	uint8 bIndex;
	uint8 aSFull[7] = {18,20,22,24,26,28,30};
	uint8 aGFull[7] = {21,24,27,29,32,34,37};
	uint8 aSLite[7] = {10,10,12,14,15,16,18};
	uint8 aGLite[7] = {13,14,17,19,21,22,25};
	SolenoidOnTimeType sTemp;
	
		bIndex = 0;
		while((bSFullOn > aSFull[bIndex]) && (bIndex < 7)){
			bIndex++;
		}	
		sTemp.bStandardFull = bSFullOn;
		sTemp.bStandardLite = aSLite[bIndex];
		sTemp.bGrandFull = aGFull[bIndex];
		sTemp.bGrandLite = aGLite[bIndex];
		return sTemp;
}

/*
** ===========================================================================================================================
**     Method      : void Flush100ms(uint16 Hms)
**     Description :
**         This method activates a flush.
**                
**     Parameters  : Turn on time in hundred milli seconds
**     Returns     : Nothing
** ===========================================================================================================================
*/
void Flush100ms(uint16 Hms){
	
	uint16 lp;
	uint16 runt;
	
		if((Hms < 1) || (Hms > 100)){	// to prevent run on if accidently 0 on time
			Hms = 30;
		}
		runt = 0 ;
		for(lp = 0; lp < Hms; lp++){ 						// set On time
			runt += 100;									// 100 milli second
		}
		LatchSolenoid();									// Latch solenoid
		if(GVbBLEEnabled){
			for(lp = 0; lp < Hms; lp++){ 					// set On time
				DelayMS(100);     							// Keep uart active
			}
		}
		else{
			SleepMS(runt);									// On time	
		}
		UnLatchSolenoid();									// unlatch solenoid
		// this feature may be needed in future
		/*
		if(NVsOpPara.LBActivationCT >= MAXLOWBACT){
			GVeOperationSts = SetOperationState(STOPOP);	// change to stop operation
		}
		*/
}

/*
** ===========================================================================================================================
**     Method      : void ToggleLED(void)
**     Description :
**         This method toggle LED
**      
**     Parameters  : Nothing
**     Returns     : Nothing
** ===========================================================================================================================
*/
void ToggleLED(void){
	
	if(LED_GetVal()){	// LED already On
		LED_ClrVal();	// Turn it off
	}
	else{				// LED was off
		LED_SetVal();	// Turn it on
	}
	
}

/*
** ===========================================================================================================================
**     Method      : void BlinkLED(uint16 ms)
**     Description :
**         This method turn on LED for inputed ms
**           
**     Parameters  : on time in ms
**     Returns     : Nothing
** ===========================================================================================================================
*/
void BlinkLED(uint16 ms){
	
	LED_SetVal();
	DelayMS(ms);
	LED_ClrVal();
	
}

/*
** ===========================================================================================================================
**     Method      : void TurnOnLED(uint16 ms)
**     Description :
**         This method turn on LED for inputed ms
**         
**        
**     Parameters  : on time in ms
**     Returns     : Nothing
** ===========================================================================================================================
*/
void TurnOnLED(uint16 ms){
	
	LED_SetVal();
	SleepMS(ms);
	LED_ClrVal();
	
}

/*
** ===========================================================================================================================
**     Method      :  uint16 GetDataMovingEverage(uint16 NewData,uint16 OldAvg)
**     Description :
**         This method calculate moving average
**         newAverage = (NewData + OldAvg *3)/4 
**     Parameters  : new data and existing average
**     Returns     : new average
** ===========================================================================================================================
*/
uint16 GetDataMovingEverage(uint16 NewData,uint16 OldAvg){
	
	uint32 lTemp1;
	uint16 wTemp2;
	
		lTemp1 = NewData;
		lTemp1 += OldAvg;			
		lTemp1 += OldAvg;			
		lTemp1 += OldAvg;			
		wTemp2 = (uint16) (lTemp1>>2);
		return wTemp2;
}

/*
** ===========================================================================================================================
**     Method      : OperationStsType SetOperationState(OperationStsType State)
**     Description :
**         This method sets all necessary settings for the new operation state
**         This is only called when changing state
**         System will start operating in new state at next wake up
**        
**     Parameters  : New state
**     Returns     : New state
** ===========================================================================================================================
*/
OperationStsType SetOperationState(OperationStsType State){
			
	switch(State){
	
		case NORMAL:
			TI1_Disable();
			AS1_Enable();
			//AS1_SetBaudRateMode(ASerialLdd1_BM_38400BAUD);
			AS1_TurnRxOn();
			AS1_TurnTxOn();
			AS1_ClearRxBuf();
			AS1_ClearTxBuf();
			//AS1_TurnRxOff();
			//AS1_TurnTxOff();
			//AS1_Disable();
			LED_ClrVal();					// in case it was turned on
			DisableUART();					// Reconfigure UART as disabled pin to prevent current leak
		#ifndef WITHANT /* IoT add*/	
			GVbBLEEnabled = FALSE;  		// flag of BLE enabled
			GVbWakeBLE = FALSE;				// flag to wake up BLE	
			GVbInDiag = FALSE;				// flag of BLE in diagnostics
			GVwBLENoActionTimer = 0; 		// reset time out 
			GVwBLEDone = FALSE;				// Flag of BLE operation done  
		#endif	/* IoT add */
		break;
		
		case COMMUNICATION:
			ConnectUART();					// Configure UART
			TI1_Disable();
			AS1_Enable();
		//	AS1_SetBaudRateMode(ASerialLdd1_BM_57600BAUD);
			AS1_TurnRxOn();
			AS1_TurnTxOn();
			AS1_ClearRxBuf();
			AS1_ClearTxBuf();
			LED_ClrVal();					// in case it was turned on
			
		break;

/* IoT begin */		
		case ANT:
			ConnectUART();					// Configure UART
			TI1_Disable();
			AS1_Enable();
			//AS1_SetBaudRateMode(ASerialLdd1_BM_57600BAUD);
			AS1_TurnRxOn();
			AS1_TurnTxOn();
			AS1_ClearRxBuf();
			AS1_ClearTxBuf();
			LED_ClrVal();					// in case it was turned on
				
		break;
		
		case UPDATEANT:
			ConnectUART();			// Configure UART
			TI1_Disable();
			AS1_Enable();
	//		AS1_SetBaudRateMode(ASerialLdd1_BM_57600BAUD);
			AS1_TurnRxOn();
			AS1_TurnTxOn();
			AS1_ClearRxBuf();
			AS1_ClearTxBuf();
			LED_ClrVal();			// in case it was turned on
			
			GVbWakeBLE= FALSE;		// Reset flag to get in this state
			GVbBLEEnabled = FALSE;  // flag of hardware setting. Need to pull down the hardware pin
			//GVbInDiag = FALSE;
			GVbUpdateANT = FALSE;
					
		break;
/* IoT end */
			
		case BLE:
			ConnectUART();					// Configure UART
			TI1_Disable();
			AS1_Enable();
	//		AS1_SetBaudRateMode(ASerialLdd1_BM_9600BAUD);
			AS1_TurnRxOn();
			AS1_TurnTxOn();
			AS1_ClearRxBuf();
			AS1_ClearTxBuf();
			LED_ClrVal();					// in case it was turned on
			GVwBLEDone = FALSE;				// Flag of BLE operation done
			GVbWakeBLE= FALSE;				// Reset flag to get in this state
			GVbBLEEnabled = FALSE;  		// flag of hardware setting. Need to pull down the hardware pin
			GVbInDiag = FALSE;
			GVwBLENoActionTimer = 0; 		// reset time out 			
		break;		
		
		case CALIBRATION:
			LED_ClrVal();					// in case it was turned on	 
		break;
		
		case TESTING:						// no need to set UART, it keeps the same as in communication state
			LED_ClrVal();					// in case it was turned on
		break;
		 
		case SETTING:						// no need to set UART, it keeps the same as in communication state
			GVsTempPara = NVsOpPara; 		// use temporary in case something wrong during test
			LED_ClrVal();					// in case it was turned on
		break;
		
		case SHIPPING:
			LED_ClrVal();					// in case it was turned on
			NVbInShippingSts = INSHIPPING;	// set in shipping flag. This is to put unit stay in shipping if battery lost contacts shortly during shipping 
			//GVbBLEEnabled = FALSE;  		// Just in case 
			NVsOffset.bInFactoryFlag = OUTOFFACTORY; 	// set outoff factory flag
			SaveBatCalParaToFlash(NVsOffset);			// store it		
			
		break;
		
		case STOPOP:
			LED_ClrVal();					// in case it was turned on
			GVbBDisconCT = 0;				// count of battery in disconnection
			NVbCheckBatteryType = SKIPCHECKTYPE;	// flag to skip check battery type at reset
		break;
		
		default:							// something wrong if get here. stop operation and waiting for watch dog to reset
			while(1); 						// Restart
		break;	
	}
	return State;
}

/*
** ===========================================================================================================================
**     Method      :void PowerProcess(void)
**     Description :
**         This process includes battery related tasks.
**         This routine is called at 1 Hz  
**         
**     Parameters  : Nothing
**     Returns     : Nothing
** ===========================================================================================================================
*/
void PowerProcess(void){
	
/* IoT begin */
	uint8 bTemp; // added in Poc 2
	uint8 bBatReportCnt; // add 4/9/23
	uint8 bBatReportCntPlus; // add 4/13/23
/* IoT end */
	
	uint16 wTemp;

	GVeBatterySts = CheckBattery(&wTemp);					// check battery
	if(GVeBatterySts <= BSTOP){								// battery drop below stop level
		GVeBatterySts = ConfirmBattery(&wTemp);				// Confirm battery is dead
		if(GVeBatterySts <= BSTOP){							// Confirmed bad
			UnLatchSolenoid();								// Failed Save Mode  				
			GVeOperationSts = SetOperationState(STOPOP);	// change to stop operation  3/31/23 
		}
	}else //IoT add else to fix short power up issue
	{
		#ifdef WITHANT 
		
			if(wTemp >= FULLBATTERY){
				bTemp = 100;
			}
			else{
				if(wTemp <= ENDBATTERY){
					bTemp = 1;
				}
				else{
					bTemp = (wTemp - ENDBATTERY) / STEPS;
				}
			}
			
			if ((NVsANTPara.iShipModeDeepSleep == 0) && (bTemp <= 2)) //add 6/28/23 fix memory corruption when add in Bstop
			{
				NVbRFEnDisableModeIn = 3;  						// set RF disable	
				NVbDongleDeepSlpModeIn = 1;	// shipping mode put dongle into deep sleep mode, go in to deepsleep 4/7/23
				GVeOperationSts = SetOperationState(UPDATEANT);	// change system status 4/7/23
				
			} else {
				
				if (NVsANTPara.iShipModeDeepSleep == 0){ //add 7/11/23 for memory corruption
					if(GVeBatterySts == BWARNING){  // report battery every 1% if warning mode 4/9/23
						bBatReportCnt = BATRPTLOWTWOEPERCENT; 
						bBatReportCntPlus = BATRPTPLUSLOWPERCENT; // add 4/13/23
					}else{
						bBatReportCnt = BATRPTNOMFIVEPERCENT; 
						bBatReportCntPlus = BATRPTPLUSNOMPERCENT; // add 4/13/23
					}
					
					//if ((bTemp <= (GVbBatLevelStore - 5)) || (bTemp >= (GVbBatLevelStore + 5)))
					//if ((bTemp <= (GVbBatLevelStore - bBatReportCnt)) || (bTemp >= (GVbBatLevelStore + bBatReportCnt)))  //remove 4/13/23
					if ((bTemp <= (GVbBatLevelStore - bBatReportCnt)) || (bTemp >= (GVbBatLevelStore + bBatReportCntPlus)))	 //add 4/13/23
					{
						// fix battery level report on power dip 10/28/20
						if (GVbBatLevelChkCnt >= BATLEVELCHKTIME)
						{
							// enable Battery update RF dongle 
							GVbBattRequest = TRUE;// Batery flag request
							GVbUpdateANT = TRUE;
							GVbBatLevelStore = bTemp;
							GVbBatLevelChkCnt = 0;
						} else
						{
							GVbBatLevelChkCnt++;
						}
						// end fix 10/28/20 
						
					}
				}
			}
		#endif
	}

	NVsOpPara.BVolt = wTemp;								// store battery reading
 		
}

/*
** ====================================================================================================================================================================================
**     Method      :uint8 CheckToWakeupBLE(void)
**     Description :
**         This method check if user wave hand 3 time in HANDWAVEINT period.
**         
**     Parameters  : 	
**     Returns     :result
** ====================================================================================================================================================================================
*/
uint8 CheckToWakeupBLE(void){
	uint8 bResult;
	bResult = FALSE; 
	static uint8 bHandWave;
	static uint8 bHandWaveCT;
	static uint8 bInterv;
	
	bHandWave = IRScanForHandWave();
	if(bHandWave == TRUE){			// hand present in front
		bHandWaveCT++;				// count it as a one wave
		if(bHandWaveCT  >= HANDWAVECNT){
			bResult = TRUE;
			bInterv = 0;
			bHandWaveCT = 0;
		}
	}
	if(bHandWaveCT > 0){				// hand wave has been detected
		bInterv += 1;					// count time since first hand wave
			if(bInterv >= HANDWAVEINT){	// time over
				bHandWaveCT = 0;		// reset
				bInterv = 0;
			}
	}
	
	// data for debug
	DbHandWave = bHandWave;
	//DbHandWaveCT = bHandWaveCT;
	if(bHandWave){
		DbHandWaveCT += 1;
		if(DbHandWaveCT > 10){
			DbHandWaveCT = 0 ;
		}
	}
	DbInterv = bInterv;
	return bResult;
}
/*
** ====================================================================================================================================================================================
**     Method      :UserStsType TargetProcess(UserStsType eTsts, BatteryStsType eBsts)
**     Description :
**         This method detect user and take actions based on user status.
**         This involves all the tasks related to IR sensing
**     Parameters  : 
**     				UserStsType eTsts:			user state
**     				BatteryStsType eBsts:	 	battery state
**     		
**     Returns     :current user state
** ====================================================================================================================================================================================
*/
UserStsType TargetProcess(UserStsType eTsts, BatteryStsType eBsts){
	
	uint16 wTempIRLevel;							// for IR DAC
	uint16 wTemp;
	UserStsType eTempTSts;							// for user state
	uint8 bTargetType;
	static uint8 bLeftTime = 0;						// time for target left after target armed 
	static uint16 wStayTime = 0;					// target stay time
	static uint8 bFlushed = 0;						// count for ball park flush, maxim continuous flush time is 5
	static uint16 wBallParkWaitTime = 0;			// Timer for a ball park flush
	static uint8 bWarningCT = 0;					// Count of LED blink
	static uint8 bDelayExt = 0;						// On delay. add it to prevent flush if sit user has been detected 
	
	eTempTSts = eTsts;                  			// default to Previous user state if battery level is below stop or IR scan get invalid result.
	if(eBsts > BSTOP){// skip if battery voltage is in stop range
		
		wTempIRLevel = NVsOpPara.IRLevel;			// IR DAC
		bTargetType = ScanTarget(wTempIRLevel, eTempTSts,GVbDutyRate);	//Scan for target

		switch(eTsts){								// New user status is determined by 3 factors, previous status, current status and timing.
			case NOTPRESENT:						// Idle, nothing happened before. every things are clear
				if (bTargetType == ISUSER){			// User in, the beginning of the new cycle, 
					//if((GVsButtonSts.bBigButton == NOTOUCH) && (NVbPowerupSts == NOTDONE) && (eBsts != BWARNING )){	// blink LED during power up period if button is not touched
					//if(((GVsButtonSts.bBigButton == NOTOUCH) && (NVbPowerupSts == NOTDONE) && (eBsts != BWARNING ) ) || (GVbBLEEnabled == TRUE)){	
					if((GVsButtonSts.bBigButton == NOTOUCH) && (((NVbPowerupSts == NOTDONE) && (eBsts != BWARNING ) ) || (GVbBLEEnabled == TRUE)|| (GVbDIAGLEDFLG== TRUE))){	// don't blink LED at any conditions if button is touched
						BlinkLED(1); 
						GVbDiagSensorStat = 1;  //MVP - IR sensor working 
						
					}
				
					if (NVsANTPara.iFeatureOccupEnDisable)
					{
						GVbOccupRoutineEnDisable = 1;  // call occupancy enable/disable function
						GVbOccupState = 1;				// Occupancy State  1 =Enter zone
					}
					
					eTempTSts = ENTER;	 					// user state changed. This is the time to reset all count and flags for new cycle
					wStayTime = (GVbDutyRate >> 1);			// Initialize timer,not 0 but count half time of scan interval.
					wBallParkWaitTime = (GVbDutyRate >> 1);	// Reset target stay time for ball park
					bFlushed = 0;      						// clear flush count for ball park	
					
					if(NVbPowerupSts == POWERUPDONE){ 		// after power up period
						GVbDutyRate = USERINDUTYRATE; 		// Speed up scan. Scan rate does not need change during power up period.
					}
				}
				
			
			break;
			
			case ENTER:
				if (bTargetType == ISUSER){			// User in confirmed. It is the second detection of the target.
					
					//if((GVsButtonSts.bBigButton == NOTOUCH) && (NVbPowerupSts == NOTDONE) && (eBsts != BWARNING ) ){	// blink LED during power up period if button is not touched
					//if(((GVsButtonSts.bBigButton == NOTOUCH) && (NVbPowerupSts == NOTDONE) && (eBsts != BWARNING ) ) || (GVbBLEEnabled == TRUE)){
					if((GVsButtonSts.bBigButton == NOTOUCH) && (((NVbPowerupSts == NOTDONE) && (eBsts != BWARNING ) ) || (GVbBLEEnabled == TRUE) || (GVbDIAGLEDFLG== TRUE))){		
						BlinkLED(1); 
						GVbDiagSensorStat = 1;  //IoT - IR sensor working
					}
					
					bDelayExt = 0;                      // Clear extend on delay
					eTempTSts = NONEARMED;	 			// Status changed. 
					wStayTime += GVbDutyRate;			// Increase timer
					wBallParkWaitTime += GVbDutyRate; 	// Increase count
					//bFlushed = 0;      					// clear flush count for ball park
					bWarningCT = 0;						// clear battery waring blink count
					if(NVbPowerupSts == POWERUPDONE){	// after power up period
						GVbDutyRate = USERINDUTYRATE; 	// Speed up
					}
					
				}
				else{									// not a user
					
					if (NVsANTPara.iFeatureOccupEnDisable)
					{
						GVbOccupRoutineEnDisable = 1;  // call occupancy enable/disable function
						GVbOccupState = 2;				// Occupancy State  2 = User Left (vacant)
					}
					
					eTempTSts = NOTPRESENT;				// back to idle status
					if(NVbPowerupSts == POWERUPDONE){ 	// after power up period
						GVbDutyRate = NORMALDUTYRATE; 	// back to low duty rate
					}
						
				}
		
			break;
			
			case NONEARMED:
				if (bTargetType == ISUSER){				// user stay
					
					if(GVsButtonSts.bBigButton == NOTOUCH){     // Blink LED only when button is not touched
						if(eBsts == BWARNING){			// Battery in Warning Stage
							if(bWarningCT < 4){ 		// blink LED during battery voltage is in warning stage and only 4 times
								BlinkLED(1);
								bWarningCT += 1;  		// increase count
							}
						}
						else{							// Battery is Normal
							//if(NVbPowerupSts == NOTDONE){// blink LED during power up period if button is not touched
							if((NVbPowerupSts == NOTDONE) || (GVbBLEEnabled == TRUE)|| (GVbDIAGLEDFLG == TRUE)|| (GVbDIAGLEDFLG== TRUE)){	
								BlinkLED(1);
							}
						}
					}
					
					wStayTime += GVbDutyRate;					// Increase target stay time count
					wBallParkWaitTime += GVbDutyRate; 			// Increase time count for ball park flush
					
					
					if (wStayTime >= (NVsOpPara.ArmTM << 2)){	// Target stays in long enough for armed
						eTempTSts = ARMED;						// switch status
						bLeftTime = 0;     						// reset target leave count
					}
					
				}
				else{ 											// not a user or user left
					if (NVsANTPara.iFeatureOccupEnDisable)
					{
						GVbOccupRoutineEnDisable = 1;  // call occupancy enable/disable function
						GVbOccupState = 2;				// Occupancy State  2 = User Left (vacant)
					}
					
					LED_ClrVal();								// Turn off LED in case it is ON.
					eTempTSts = ENTER;							// back to enter statues, not to the idle status so that scan rate is still high
					wStayTime = 1;								// restart arm verification
				}
			
				break;
		
			case ARMED:
				if (bTargetType == ISUSER){			// user stay
					//if((GVsButtonSts.bBigButton == NOTOUCH) && (NVbPowerupSts == NOTDONE) && (eBsts != BWARNING )){// blink LED during power up period if button is not touched
					//if(((GVsButtonSts.bBigButton == NOTOUCH) && (NVbPowerupSts == NOTDONE) && (eBsts != BWARNING ) ) || (GVbBLEEnabled == TRUE)){
					if((GVsButtonSts.bBigButton == NOTOUCH) && (((NVbPowerupSts == NOTDONE) && (eBsts != BWARNING ) ) || (GVbBLEEnabled == TRUE)|| (GVbDIAGLEDFLG == TRUE))){		
						BlinkLED(1);
					}
					bLeftTime = 0 ; 							// reset it. count for On Delay
				
					if(IsUserSitDown()){						// if user sit down has been detected
						bDelayExt = 20;							// add extent on delay. 5 seconds
					}
					else{							// out of sit down
						if(bDelayExt >= GVbDutyRate){			// out of sit down not long enough
							bDelayExt -= GVbDutyRate;			// decrease  extend delay
						}
						else{
							if(bDelayExt > 0){
								bDelayExt--;
							}
						}
					}
					
					if(wStayTime < FULLFLUSHSTAY){ 							// Make sure count below limit
						wStayTime += GVbDutyRate;							// Increase user stay time count
					}
					if((NVsOpPara.Mode == BALLPARKMODE) && (bFlushed < MAXBPFLUSH)){ 	// Urinal ball park and flush number has not reach maximum
						wBallParkWaitTime += GVbDutyRate; 					// Increase time count for ball park flush
						if(wBallParkWaitTime >= BALLPARKFLUSHTIME){			// It's the time for flush
							GVbFlushRequest = STDFULLIR;					// set Flush request
							eTempTSts = ENTER;	 							// Start from new cycle
							wStayTime = 0;									// Initialize timer
							wBallParkWaitTime = 0; 							// Reset target stay time for ball park
							bFlushed += 1;									// Increase activation count
						}
					}
				}
				else{ //user left
					
					if (bLeftTime >= ((NVsOpPara.ONDelayTM << 2 ) + bDelayExt)){	// On delay expired
						GVbFlushRequest = STDFULLIR;					// set Flush request
						#ifdef BUILDDUAL
							if(wStayTime >= FULLFLUSHSTAY){ 			// Full flush
								if(NVsOpPara.FlushVolumeM == 1){ 		// Grand mode
									GVbFlushRequest = GRDFULLIR;
								}
								else{									// stand mode
									GVbFlushRequest = STDFULLIR;
									}
							}
							else{										// reduced flush
								if(NVsOpPara.FlushVolumeM == 1){ 		// Grand mode
									GVbFlushRequest = GRDLITEIR;
								}
								else{									// stand mode
									GVbFlushRequest = STDLITEIR;
								}
							}												// use default instead
						#endif
						/*
						if(NVsOpPara.Mode == DUALMODE){					// for dual flush operation
							if(wStayTime >= FULLFLUSHSTAY){ 			// Full flush
								if(NVsOpPara.FlushVolumeM == 1){ 		// Grand mode
									GVbFlushRequest = GRDFULLIR;
								}
								else{									// stand mode
									GVbFlushRequest = STDFULLIR;
								}
							}
							else{										// reduced flush
								if(NVsOpPara.FlushVolumeM == 1){ 		// Grand mode
									GVbFlushRequest = GRDLITEIR;
								}
								else{									// stand mode
									GVbFlushRequest = STDLITEIR;
								}
							}
						}
						*/
						if (NVsANTPara.iFeatureOccupEnDisable)
						{
							GVbOccupRoutineEnDisable = 1;  // call occupancy enable/disable function
							GVbOccupState = 2;				// Occupancy State  2 = User Left (vacant)	
						}
						
						eTempTSts = NOTPRESENT;							// Go to idle status
						if(NVbPowerupSts == POWERUPDONE){ 				// after power up period
							GVbDutyRate = NORMALDUTYRATE; 				// back to low duty rate
						}
						wTemp = GetInstant();
						SetBackground(wTemp);							// set current signal as background  
					}
					bLeftTime += GVbDutyRate;							// update timer	
				}
				
			break;
			
			default:
			break;		
		}
			
	} // target state would change if battery below stop level
	
	return eTempTSts;
}

/*
** ===========================================================================================================================
**     Method      :void FlushProcess(void)
**     Description :
**         This method process the real flush. Different flush have different solenoid open time
**     Parameters  : Nothing
**     Returns     : Nothing
** ===========================================================================================================================
*/

void FlushProcess(uint8 bRequest){
	
	if(GVbInDiag == FALSE){ // don't flush during BLE diagnostics
		switch(bRequest){
			case STDFULLIR: // IR sensing full Flush
				Flush100ms(GVsTurnOnTM.bStandardFull);				// Turn on solenoid
				if(GVbBLEEnabled == FALSE){
					SleepSecond(NVsOpPara.WaitDelayTM);   				// Delay time, do nothing
					NVsOpPara.RSecond += (3 + NVsOpPara.WaitDelayTM); 	// count the time of this flush action.(On time + wait Delay)
				}
			break;
			case STDLITEIR:	// IR sensing lite Flush
				Flush100ms(GVsTurnOnTM.bStandardLite);
				//NVsOpPara.ReduceFlushActivation += 1;
				NVsBLEPara.ReduceFlushActivation += 1;
				if(GVbBLEEnabled == FALSE){
					SleepSecond(NVsOpPara.WaitDelayTM);   				// Delay time, do nothing
					NVsOpPara.RSecond += (3 + NVsOpPara.WaitDelayTM); 	// count the time of this flush action.(On time + wait Delay)
				}
			break;
			case GRDFULLIR: // IR sensing Grand full Flush
				Flush100ms(GVsTurnOnTM.bGrandFull);	
				if(GVbBLEEnabled == FALSE){
					SleepSecond(NVsOpPara.WaitDelayTM);   				// Delay time, do nothing
					NVsOpPara.RSecond += (3 + NVsOpPara.WaitDelayTM); 	// count the time of this flush action.(On time + wait Delay)
				}
			break;
			case GRDLITEIR: // IR sensing Grand lite Flush
				Flush100ms(GVsTurnOnTM.bGrandLite);
				if(GVbBLEEnabled == FALSE){
					SleepSecond(NVsOpPara.WaitDelayTM);   				// Delay time, do nothing
					NVsOpPara.RSecond += (3 + NVsOpPara.WaitDelayTM); 	// count the time of this flush action.(On time + wait Delay)
				}
			break;
			case STDFULLBT: // Button standard full Flush
				Flush100ms(GVsTurnOnTM.bStandardFull);	
				NVsOpPara.ButtonActivations += 1;
				if(GVbBLEEnabled == FALSE){
					SleepSecond(5);           							// Delay time, do nothing, this is the delay after specially for button activation
					NVsOpPara.RSecond += 8;								// Ontime + 5
				}
			break;
			case STDLITEBT: // Button standard lite Flush
				Flush100ms(GVsTurnOnTM.bStandardLite);					
				NVsOpPara.ButtonActivations += 1;
				//NVsOpPara.ReduceFlushActivation += 1;
				NVsBLEPara.ReduceFlushActivation += 1;
				if(GVbBLEEnabled == FALSE){
					SleepSecond(5);           							// Delay time, do nothing, this is the delay after specially for button activation
					NVsOpPara.RSecond += 8;								// Ontime + 5
				}
			break;
			case GRDFULLBT: // Button Grand full Flush
				Flush100ms(GVsTurnOnTM.bGrandFull);			
				NVsOpPara.ButtonActivations += 1;
				if(GVbBLEEnabled == FALSE){
					SleepSecond(5);           							// Delay time, do nothing, this is the delay after specially for button activation
					NVsOpPara.RSecond += 8;								// Ontime + 5
				}
			break;
			case GRDLITEBT: // Button Grand lite Flush
				Flush100ms(GVsTurnOnTM.bGrandLite);				
				NVsOpPara.ButtonActivations += 1;
				if(GVbBLEEnabled == FALSE){
					SleepSecond(5);           							// Delay time, do nothing, this is the delay after specially for button activation
					NVsOpPara.RSecond += 8;								// Ontime + 5
				}
			break;	
			case SENTINALACT: // Sentinel Flush
				Flush100ms(NVsOpPara.OpenTM);							// sentinel flush
				//NVsOpPara.SentinalActivation += 1;
				NVsBLEPara.SentinalActivation += 1;
			break;
			default:	 
			break;
		}
			
		GVsSentinelTime = ResetSentinelTimer();				// reset sentinel timer
	}
}

/*
** ====================================================================================================================================================================================
**     Method      :void RoutinProcess(void)
**     Description :
**         This process includes the tasks after wake up during none shipping and battery in operation range.
**         Duty rate is set for button scan rate, but IR scan can be slower
**         Let big button scan during sleep time to save power, so two buttons are scan at the different time.
**     Parameters  : Nothing
**     Returns     : Nothing
** ====================================================================================================================================================================================
*/
void RoutinProcess(void){
	
	static uint8 bIRScanTick = 0; 								// count for reduce rate for IR scan
	static uint8 bSecondTick = 0;                               // count for a second
	uint8 bBigButtonNewSts;										// Temporary big button state
	uint8 bSmallButtonNewSts;									// Temporary small button state
		
		WDReset();												// kick dog
		if(NVbSolenoidSts != SOLENOIDOFF){						// Safe check to prevent water run on. There is no reason that solenoid is on at this moment.
			UnLatchSolenoid();									// Turn off the solenoid
		}
		GVbFlushRequest = NOFLUSH;								// reset flush request
		
		// 1. Check hand wave for BLE request
#ifdef WITHBLE
	#ifndef WITHANT	// IoT add
		#ifdef OUTHANDWAVE
			if((GVeOperationSts == NORMAL) || (GVeOperationSts == COMMUNICATION)){	//	for debug
		#else
			//if(GVeOperationSts == NORMAL && (NVbBLEDongleIn == TRUE)){		// only during normal operation and dongle installed
			if(GVeOperationSts == NORMAL){				// only during normal operation and dongle installed
		#endif
				GVbWakeBLE = CheckToWakeupBLE();    				// Check if there is a wake BLE request.
			}
	#endif //MVP add
#endif
				
		// 2. tasks every second
		bSecondTick += 1;
		if(bSecondTick >= 4){									// one second
			bSecondTick = 0;  									// reset
			
			// 2.1. update power up status
			if(NVbPowerupSts != POWERUPDONE){					// power up period not expired yet
				NVbPowerupSts = IsPowerupDone();				// check to see if the time expire
			}
				
			// 2.2 battery related tasks
			PowerProcess();
			
			// 2.3 update operation time clock
			UpdateOperationTime();								// update time clock
			
			// 2.4 Sentinel process
			GVbSentinelFlush = CheckSentinelSetting();			// Check sentinel setting
			if(GVbSentinelFlush == YESSENTINEL){				// sentinel required
				UpdateSentinelTime();
				if(GVsSentinelTime.bSentinelHour >= NVsOpPara.SentinalTM){ // is it the time for sentinel
					GVbFlushRequest = SENTINALACT;				// set flush flag
				}			
			}
			
		}
	
		if(GVeOperationSts != STOPOP) {      					// skip if battery voltage is found in stop range from above step.
			
			// 3. Button Sensing, at 4 HZ
			bBigButtonNewSts = IsButtonTouched(BIGBUTTON);		// Get big button current states, scan started before sleep and it should be down during sleep.
			GVsButtonSts.bBigButton = bBigButtonNewSts;
			/*
			if(NVsOpPara.Mode == DUALMODE){						// for dual flush operation
				TButtonStartScan(SMALLBUTTON);					// start second button scan 
			}
			 */
			#ifdef BUILDDUAL
			TButtonStartScan(SMALLBUTTON);						// start second button scan 
			#endif
			
			// 4. IR related process, this is running at reduced duty rate
			bIRScanTick ++;
			if(bIRScanTick >= GVbDutyRate){						// IR scan for different rate than button scan
				bIRScanTick = 0;								// new cycle
				//if(bBigButtonNewSts != TOUCHED){				// No IR scan if button touched. It may cause the problem if touch sensor is not working properly
				GVeUserSts = TargetProcess(GVeUserSts, GVeBatterySts);	// IR Scan process
				//}	
			}
			else{				// not the IR scan duty
				SleepMS(3);		// the same time as target scan, keep time consistent for IR scan and non scan. Let button scan, 
			}
			
			// 5. Button processing
			bSmallButtonNewSts = NOTOUCH;							// single button, never used
			#ifdef BUILDDUAL
				while((TSI0_GENCS & TSI_GENCS_EOSF_MASK) == 0);		// waiting for small button scan complete
				bSmallButtonNewSts = IsButtonTouched(SMALLBUTTON); 	// Get small button new state		
			#endif
		/*	
			if(NVsOpPara.Mode == DUALMODE){							// for dual buttons, go to get small button status
				while((TSI0_GENCS & TSI_GENCS_EOSF_MASK) == 0);		// waiting for small button scan complete
				bSmallButtonNewSts = IsButtonTouched(SMALLBUTTON); 	// Get small button new state	
			}
			else{													// for single button units
				bSmallButtonNewSts = NOTOUCH;						// single button, never used
			}
		*/
			GVsButtonSts.bCombination = TwoButtonProcess(bSmallButtonNewSts, bBigButtonNewSts); 	// go to process button related tasks

			// 6. Process flush request
			if(GVbFlushRequest){ // flush flag set
				
/* IoT begin */				
				/* IoT comment out */
				//FlushProcess(GVbFlushRequest); 					// go to flush
			
				#ifdef WITHANT
					//Uart flush logic 
					GVbStoreFlushRequest = GVbFlushRequest;     	//store flush flag 
					
					// enable flush RF dongle 
					GVbUartFlushRequest = TRUE; // Flush flag request
					GVbWakeBLE = TRUE;
				#else
					FlushProcess(GVbFlushRequest); 					// go to flush
				#endif
/* IoT end */

			}
			
			#ifdef CYCLETEST    // For cycle test	
			if(GVeOperationSts == NORMAL){	
				GVbFlushRequest = STDFULLIR;	// cycle test
			}
			#endif	
			
		}
		
}

/*
** ===========================================================================================================================
**     Method      :IsSolarCellGetPower
**     Description :
**         This method detects if solar cell get voltage. 
**     Parameters  : Nothing
**     Returns     : result true or false
** ===========================================================================================================================
*/
uint8 IsSolarCellgetPower(void){
	uint8 bTemp;
	bTemp = FALSE;
	//if(ReadSolarCell() > 31000){	// greater than 4.33V
	if(ReadSolarCell() > 25000){	// There is a time delay to let leak voltage discharge, this is not exact voltage of solar cell for normal load
		bTemp = TRUE;
	}
	return bTemp;
}

/*
** ===========================================================================================================================
**     Method      :IsSolarCellCovered
**     Description :
**         This method detects if solar cell covered by tape. 
**     Parameters  : Nothing
**     Returns     : result true or false
** ===========================================================================================================================
*/
uint8 IsSolarCellCovered(void){
	uint8 bTemp;
	bTemp = TRUE;
	if(ReadSolarCell() > 15000){	// Normally there is a leak voltage on solar cell
		bTemp = FALSE;
	}
	return bTemp;
}

/*
** ===========================================================================================================================
**     Method      :IsWindowCovered(void)
**     Description :
**         This method detects if IR Window covered by tape.  
**     Parameters  : Nothing
**     Returns     : result true or false
** ===========================================================================================================================
*/
uint8 IsWindowCovered(void){
	uint8 bResult;
	uint16 wTempEcho;
	
		bResult = TRUE; 									// preset
		wTempEcho = MeasureEchoVolt(NVsOpPara.IRLevel);		// Measure it
		if(wTempEcho >= WINDOWCOVERED){						// check the value
			bResult = FALSE;								// set flag
		}
		return bResult;
}


/*
** ===========================================================================================================================
**     Method      :IsSystemInShipCondition
**     Description :
**         This method check if it is in shipping condition
**         
**        
**     Parameters  : Nothing
**     Returns     : result
** ===========================================================================================================================
*/
uint8 IsSystemInShipCondition(void){
	uint8 bTemp;
	uint8 bLp;
	
	bTemp = FALSE;		// default

	for(bLp=0; bLp < 5; bLp++){ // Give 5 chances
		if(IsSolarCellCovered() && IsWindowCovered()){ // both solar panel and IR window covered
			bTemp = TRUE;		// set flag
			break;
		}
	}
	
	return bTemp;
}

/*
** ===========================================================================================================================
**     Method      :IsSystemOutShipCondition
**     Description :
**         This method check if it is out shipping condition
**         To prevent accident reading, 4 consecutive readings of out shipping has to be confirmed.
**        
**     Parameters  : Nothing
**     Returns     : result
** ===========================================================================================================================
*/
uint8 IsSystemOutShipCondition(void){
	uint8 bTemp;
	uint8 bLp;
	
	bTemp = TRUE;							// default
	
	for(bLp=0; bLp < 4; bLp++){ 			// Give 4 chances
		if(IsWindowCovered()){ 				// IR window covered
			bTemp = FALSE;					// set flag
			break;
		}
		else{
			SleepMS(250);
		}
	}
	return bTemp;
}

/*
** ===========================================================================================================================
**     Method      :TurnOffAllPeripheral(void)
**     Description :
**         This method turns off all peripherals
**         
**        
**     Parameters  : Nothing
**     Returns     : Nothing
** ===========================================================================================================================
*/
void TurnOffAllPeripheral(void){
	
	AD1_Disable();								// Disable A/D
	TI1_Disable();								// Timer
	AS1_Disable();								// UART
	AS1_TurnRxOff();							// UART Receiver
	AS1_TurnTxOff();							// UART Transmit
	IRDRV_Disable(IRDRV_DeviceData);			// Disable D/A
	LED_ClrVal();								// Turn off LED
	LATCH_ClrVal();								// Turn Off Latch
	UNLATCH_SetVal();							// Turn Off Unlatch
	IRRecPwr_SetVal();							// Turn off IR detector
	DisableUART();								// Disable UART 
	
	DisableLATCHCurrentSensing();
}

/*
** ===========================================================================================================================
**     Method      :void CheckCommunicationCable(void){
**     Description :
**         This method check communication cable status and set operation states accordingly
**         
**        
**     Parameters  : Nothing
**     Returns     : Nothing
** ===========================================================================================================================
*/
void CheckCommunicationCable(void){
	
	uint8 bTemp = 0; 
	
		bTemp = COMCable_GetVal(); 										// check if communication cable present
		
		switch(GVeOperationSts){										// Compare with current operation status
			case COMMUNICATION:
				if(bTemp == 1){                                         // cable disconnected.

/*IoT begin */
					#ifdef WITHANT
						// added PoC 2 
						UartWake_SetVal(); //pull high
						UartWake_SetInput(); // reset UartWake as input (pin 27) 
						UartWakeInt_Enable(); // re enable interrupt
					#endif
/* IoT end */
															
					GVeOperationSts = SetOperationState(NORMAL);		// change to normal operation
					ToutchButtonInit();									// reset button baseline since communication cable will effect
				}
			break;
				
			case NORMAL: 
				if(bTemp == 0){											// cable connected.			
					
/* IoT begin */
					#ifdef WITHANT
						// added PoC 2 disable interrupt on UARTWAKE pin
						UartWakeInt_Disable(); // disable interrupt
						
						/* Check Wake up pin high before send 4/4/23-mod 4/10/23*/
						if(UartWake_GetVal()){
							UartWake_SetOutput(); //set UartWake to output (pin 27)	
							UartWake_ClrVal();  // pull low
						}
						//--
							
							//org code
//						UartWake_SetOutput(); //set UartWake to output (pin 27)	
//						UartWake_ClrVal();  // pull low
							//--end
					#endif
/* IoT end */
 
					GVeOperationSts = SetOperationState(COMMUNICATION);	//change to communication mode	
					ToutchButtonInit();									// reset button baseline since communication cable will effect
				}
			break;
				
			case SETTING:    
				if(bTemp == 1){											// cable disconnected.
					GVeOperationSts = SetOperationState(NORMAL);		// change to normal operation
					ToutchButtonInit();									// reset button baseline since communication cable will effect
				}
			break;
				
			case TESTING: 
				if(bTemp == 1){											// cable disconnected.
					LED_ClrVal();										// turn off LED, in case it was turned on during test
					UnLatchSolenoid();									// Unlatch solenoid in case it was latched during test
					GVeOperationSts = SetOperationState(NORMAL);		// change to normal operation
					ToutchButtonInit();									// reset button baseline since communication cable will effect
			}				
			break;

/* IoT begin */
			case ANT: 
				if(bTemp == 0){											// cable connected.		
					GVeOperationSts = SetOperationState(NORMAL);		// change to normal operation
					ToutchButtonInit();									// reset button baseline since communication cable will effect	
				}
			break;
			
			case UPDATEANT: 
				if(bTemp == 0){											// cable connected.		
					GVeOperationSts = SetOperationState(NORMAL);		// change to normal operation
					ToutchButtonInit();									// reset button baseline since communication cable will effect	
				}
			break;
/* IoT end */
			
			case BLE: 
				if(bTemp == 0){											// cable connected.		
					GVeOperationSts = SetOperationState(NORMAL);		// change to normal operation
					ToutchButtonInit();									// reset button baseline since communication cable will effect	
				}
			break;
			
			case SHIPPING: 
					
			break;
			
			case STOPOP: 
							
			break;
			
			case CALIBRATION: 
								
			break;
				
			default:	
				while(1); //something wrong if get here. stop operation and waiting for watch dog to reset
			break;	
		}			
		
}

/*
** ====================================================================================================================================================================
**     Method      : NormalOperation(void)
**     Description :
**         This is the operation in normal state. 
**         It takes about 3 ms for the duty, so sleep 247 ms after duty to keep 4 Hz duty rate   
**     Parameters  : Nothing
**     Returns     : Nothing
** ====================================================================================================================================================================
*/
void NormalOperation(void){
	
	//RoutinProcess();				// basic tasks
	//CheckCommunicationCable();		// Check if communication cable connected.
	
	
	rpc_run();
	sendPeriodicMessage();
	
//#ifdef WITHBLE
//	
//	/* IoT begin */ 
//	#ifdef WITHANT
//		
//		// check wake up ANT request
//		if(GVbWakeBLE == TRUE)
//		{					
//			GVeOperationSts = SetOperationState(ANT);		// change operation state
//			return;											// terminate right away
//		} else
//		{
//			//if((GVbUpdateANT == TRUE)|| (GVbGetSRStatFlag == 1) || (NVbDongleDeepSlpModeIn == 3) || (GVbOccupRoutineEnDisable == 1)||(NVbRFEnDisableModeIn == 1) || (NVbRFEnDisableModeIn == 3)) // GVbGetSRStatFlag remove 4/7/23
//			//if((GVbUpdateANT == TRUE) || (NVbDongleDeepSlpModeIn == 3) || (GVbOccupRoutineEnDisable == 1)||(NVbRFEnDisableModeIn == 1) || (NVbRFEnDisableModeIn == 3))
//			if((GVbUpdateANT == TRUE) || (NVbDongleDeepSlpModeIn == 3) || (GVbOccupRoutineEnDisable == 1))
//			{
//				GVeOperationSts = SetOperationState(UPDATEANT);
//				return;
//			}  
//		}
//	#else  /* IoT add end */
//		// check wake up BLE request
//		if(GVbWakeBLE == TRUE){
//			GVeOperationSts = SetOperationState(BLE);		// change operation state
//			return;											// terminate right away
//		}
//	#endif // IoT Add
//#endif
	
	//TButtonStartScan(BIGBUTTON);	// Start big button scan, scan will complete during sleep.
	//SleepMS(247);					// Completed duty. Go to sleep, Duty rate is always 4 Hz, duty time is about 3 ms. It can't be slower due to touch sensing			
}

/*
** ====================================================================================================================================================================
**     Method      : CommunicationEnabledOperation
**     Description :
**         This operation is for communication cable connected    
**     Parameters  : Nothing
**     Returns     : Nothing
**====================================================================================================================================================================
*/
void CommunicationEnabledOperation(void){
		
	RoutinProcess();			// The same tasks as in normal non communication operation
	ProcessASCIIRequest();  	// Check user request from UART
	
#ifdef	OUTCURVE				// for debug and turning purpose
	uint16 wTempInst, wTempBack;
		wTempInst = GetInstant();
		wTempBack = GetBackground();
		UARTOutMultiCurveData(wTempInst,wTempBack,(wTempInst-wTempBack)) ;
		//UARTOutMultiCurveData(GVwIRInstant,GVwIRBackground,GVwIRReceiverOffset) ;
#endif
		
#ifdef	OUTBUTTON1				// for debug and turning purpose
		uint16 wTempInst, wTempBack, wTempFilteredSignal;
		wTempInst = GetBigButtonCapInstantValue(); 
		wTempBack = GetBigButtonBaselineValue();
		//wTempFilteredSignal = GetFilteredValue();
		//wTempFilteredSignal = dUpdate;
		//UARTOutMultiCurveData(wTempInst,wTempBack,wTempFilteredSignal);
		
		
		if(wTempInst >= wTempBack){
			UARTOutMultiCurveData(wTempInst,wTempBack,(wTempInst-wTempBack)) ;
		}
		else{
			UARTOutMultiCurveData(wTempInst,wTempBack,0);
		}
		
#endif
		
#ifdef	OUTBUTTON2				// for debug and turning purpose
		uint16 wTempInst, wTempBack, wTempFilteredSignal;
		wTempInst = GetSmallButtonCapInstantValue(); 
		wTempBack = GetSmallButtonBaselineValue();
		//wTempFilteredSignal = GetFilteredValue();
		//wTempFilteredSignal = dUpdate;
		//UARTOutMultiCurveData(wTempInst,wTempBack,wTempFilteredSignal);
		
		
		if(wTempInst >= wTempBack){
			UARTOutMultiCurveData(wTempInst,wTempBack,(wTempInst-wTempBack)) ;
		}
		else{
			UARTOutMultiCurveData(wTempInst,wTempBack,0);
		}
		
#endif		
		
#ifdef	OUTCURVES				// for debug and turning purpose
		uint16 wTempInst, wTempBack;
		wTempInst = GetInstant();
		wTempBack = GetBackground();
		if(GVbSentinelFlush == NOTSENTINEL){
			UARTOutMultiCurveData(wTempInst,wTempBack,(wTempInst-wTempBack)) ;
		}
		else{
			//UARTOutMultiCurveData(GVwBigCapInstant,GVwBigButtonBaseline,(GVwBigCapInstant-GVwBigButtonBaseline)) ;
			UARTOutDebugData();
			//UARTOutDebugInfor();
		}
#endif
	
#ifdef OUTDATA
	//UARTOutDebugData();
	//UARTOutDebugDataOn();
#endif
	
#ifdef OUTHANDWAVE
	UARTOutHandWaveData();
#endif		
	
	CheckCommunicationCable();		// check communication cable status 
	TButtonStartScan(BIGBUTTON);	// start button scan, scan will complete during sleep.
	WaitMS(250);					// Not complete sleep so that UART is still active	
}

/* IoT begin */

/*
** ====================================================================================================================================================================
**     Method      : void ANTEnabledOperation(void)
**     Description :
**         This operation is for ANT    
**     Parameters  : Nothing
**     Returns     : Nothing
**====================================================================================================================================================================
*/
void ANTEnabledOperation(void){
	
	bool TimeOut;
	RoutinProcess();				// The same tasks as in normal non communication operation	
		
	if (GVbUartFlushRequest) // manual flush
	{
		
		//Disable enable pins interrupt and set output 
		UartWakeInt_Disable(); // disable interrupt
		
		/* Check Wake up pin high before send 4/4/23 -mod 4/10/23*/
			if(UartWake_GetVal()){
				
				UartWake_SetOutput(); //set UartWake to output (pin 27)
				UartWake_ClrVal(); // pull low
				
				FlushProcess(GVbStoreFlushRequest); 					// go to flush
				
				//enable pins interrupt and set input 
				UartWake_SetVal();   // pull up
				UartWake_SetInput(); // reset UartWake as input (pin 27) 
			}else
			{
				FlushProcess(GVbStoreFlushRequest); 					// go to flush
			}
			//--
		
			//org code
//		UartWake_SetOutput(); //set UartWake to output (pin 27)
//		UartWake_ClrVal(); // pull low
//		
//		FlushProcess(GVbStoreFlushRequest); 					// go to flush
//		
//		//enable pins interrupt and set input 
//		UartWake_SetVal();   // pull up
//		UartWake_SetInput(); // reset UartWake as input (pin 27) 
		//org cod end

		UartWakeInt_Enable(); // re enable interrupt
						
		// check state of RF dongle status before send 
		
		if ((BYPASSCHKSRSTATUS == 1 || GVbSRModuleStatus == BDGNNETNSERVER) || (GVbSRModuleStatus == ROUTERNNETWORK))  // remove always 3 -- 4/9/23  --add bypass 4/10/23
		{

		DelayMS(100); //500
				
			if (GVbActRptTholdCnt == GVbActRptThold)
			{
				// update command total activation to dashboard (58)hex 0x3A 
				if (NVsANTPara.iShipModeDeepSleep == 0){ //add 7/13/23
					UARTProcessSendRequest(ACTSINCEINSTALL);
				}
				GVbActRptTholdCnt = 1;
			}else
			{
				GVbActRptTholdCnt++; 
			}
	
		} // remove always 3 -- 4/9/23
		
		
		//add in 1.5 to try to fix double flush
		GVeOperationSts = SetOperationState(NORMAL);		// change to normal operation
		GVbWakeBLE = FALSE;
		GVbBLEEnabled = FALSE;
		
		GVbUartFlushRequest = FALSE;
		
	} else {
		TimeOut = UARTProcess();  					// Every task related to UART
		
		if (TimeOut)
		{		
			GVeOperationSts = SetOperationState(NORMAL);		// change to normal operation
			GVbWakeBLE = FALSE;
			GVbBLEEnabled = FALSE;
		}
		
		/* send factory rest data
		 	data ArmTM - ARMTIMER, OpenTM - MRTOTIMER, ONDelayTM - IRUPDATETHOLD, 
		 	SentinalTM - FLUSHSENTINTIME, sSensorRange - SENSORRANGE 
		 */
		if (GVbFactoryRestFlg == 1)
		{
			GVbUARTInCMDList[0] = 0x05;  	// number of cmd
			GVbUARTInCMDList[1] = 0x01;
			GVbUARTInCMDList[2] = 0x0B;  	// sensor range
			GVbUARTInCMDList[3] = 0x01;
			GVbUARTInCMDList[4] = 0x0D; 	// open timer
			GVbUARTInCMDList[5] = 0x01;
			GVbUARTInCMDList[6] = 0x0F;		// sentinal timer
			GVbUARTInCMDList[7] = 0x01;
			GVbUARTInCMDList[8] = 0x46;		// Arm timer
			GVbUARTInCMDList[9] = 0x01;
			GVbUARTInCMDList[10] = 0x56;	// on delay timer	
			GVbUARTInCMDList[11] = '\0';
			
			GVbMultiPDUPart = 1;
			UARTProcessSendRequest(MULTIPDU);
			GVbFactoryRestFlg = 0;
		}
		
		// remote refresh flag set
		if (GVbSRRefreshCmd == 1)
		{
			GVbMultiPDUPart = 1;
			UARTProcessSendRequest(MULTIPDU);
			DelayMS(100);
			if(GVbMultiPDUPart == 2){
				UARTProcessSendRequest(MULTIPDU);
			}		
			GVbSRRefreshCmd = 0;
		}
		
		// flush request handler (remote flush)
		if (GVbFlushActivate == 1)
		{	
	/*		PoC 2 comment out
			//Turn on LED for 500ms for status
			LED_SetVal();
			SleepMS(800); //2000
			LED_ClrVal();
		*/	
			//diagnosis
			if(GVbDIAGLEDFLG)
			{
				UARTProcessSendRequest(SOLARPANELSTAT);
				DelayMS(100);
				UARTProcessSendRequest(SENSORSTAT);
			
				GVbDIAGLEDFLG = FALSE;
			}
			// do actually flush
			
			//Disable enable pins interrupt and set output 
			UartWakeInt_Disable(); // disable interrupt
			
			/* Check Wake up pin high before send 4/4/23 -mod 4/10/23*/
			if(UartWake_GetVal()){
				UartWake_SetOutput(); //set UartWake to output (pin 27)
				UartWake_ClrVal(); //pull low
				
				FlushProcess(GVbStoreFlushRequest); 					// go to flush
		
				//enable pins interrupt and set input 
				UartWake_SetVal();   // pull up
				UartWake_SetInput(); // reset UartWake as input (pin 27) 
			} else
			{
				FlushProcess(GVbStoreFlushRequest); 					// go to flush
			}
			//--
			
			//org code
//			UartWake_SetOutput(); //set UartWake to output (pin 27)
//			UartWake_ClrVal(); //pull low
//			
//			FlushProcess(GVbStoreFlushRequest); 					// go to flush
//	
//			//enable pins interrupt and set input 
//			UartWake_SetVal();   // pull up
//			UartWake_SetInput(); // reset UartWake as input (pin 27) 
			//org code end
			
			UartWakeInt_Enable(); // re enable interrupt
			
			//diagnosis
			if(GVbDIAGVALVEFLG)
			{
				UARTProcessSendRequest(VALVESTAT);
				GVbDIAGVALVEFLG = FALSE;
			}
						
			// check state of RF dongle status before send 
			
			if ((BYPASSCHKSRSTATUS == 1 || GVbSRModuleStatus == BDGNNETNSERVER) || (GVbSRModuleStatus == ROUTERNNETWORK)) // remove always 3 -- 4/9/23 --add bypass 4/10/23
			{
				DelayMS(100);  //500
				
				if (GVbActRptTholdCnt == GVbActRptThold)
				{
					// update command total activation to dashboard (58)hex 0x3A 
					if (NVsANTPara.iShipModeDeepSleep == 0){ //add 7/13/23
						UARTProcessSendRequest(ACTSINCEINSTALL);
					}
					GVbActRptTholdCnt = 1;
				}else
				{
					GVbActRptTholdCnt++; 
				}
			} // remove always 3 -- 4/9/23
		
			
			GVbFlushActivate = 0;
			
			// add for 1.5 try to fix double activate flush
			GVeOperationSts = SetOperationState(NORMAL);		// change to normal operation
			GVbWakeBLE = FALSE;
			GVbBLEEnabled = FALSE;
		}
	}
	
	CheckCommunicationCable();
	TButtonStartScan(BIGBUTTON);	// start button scan, scan will complete during sleep.
	SleepMS(250);					// sleep 2 seconds
	
}

/*
** ====================================================================================================================================================================
**     Method      : UARTRFUdateOperation
**     Description :
**         This operation is for communication cable connected    
**     Parameters  : Nothing
**     Returns     : Nothing
**====================================================================================================================================================================
*/
void UARTANTUdateOperation(void){
		
	if ((NVbDongleDeepSlpModeIn == 1) || (NVbDongleDeepSlpModeIn == 3)) //shipping mode
	{
		
		UARTProcessSendRequest(SHIPMODEDEEPSLP);
		
		if((NVbDongleDeepSlpModeIn == 2) && (NVbRFEnDisableModeIn != 3)) 
		{
			Flush100ms(10);	
			GVeOperationSts = SetOperationState(SHIPPING);	// change system status
		} 
		
	} 
	else
	{
		
		RoutinProcess();				// The same tasks as in normal non communication operation
		
		// after power up and init, get rf dongle statue, if connected to internet send all command to dashboard  --remove 4/7/23
//		if(GVbGetSRStatFlag == 1)
//		{
//			GVbGetSRStatFlag = 0;  // reset init flag
//			UARTProcessSendRequest(SRSTATUS);  // get RF Dongle Status 
//			
//		}
	
		if(GVbOccupRoutineEnDisable)
		{
			GVbOccupRoutineEnDisable = 0;
			if(GVbOccupState != GVbPreOccupState )
			{
				GVbPreOccupState = GVbOccupState;
				if ((BYPASSCHKSRSTATUS == 1 || GVbSRModuleStatus == BDGNNETNSERVER) || (GVbSRModuleStatus == ROUTERNNETWORK))  // check RF dongle status = GVbSRModuleStatus  3 or 5 with internet connection 			
				{ // remove always 3 -- 4/9/23 --add bypass 4/10/23
					UARTProcessSendRequest(OCCUPANCY);  
				}// remove always 3 -- 4/9/23
			}
		}
				
		
		// Battery request is true then do something  
		if (GVbBattRequest == TRUE)
		{
			if ((BYPASSCHKSRSTATUS == 1 ||GVbSRModuleStatus == BDGNNETNSERVER) || (GVbSRModuleStatus == ROUTERNNETWORK))  // check RF dongle status = GVbSRModuleStatus  3 or 5 with internet connection 			
			{ // remove always 3 -- 4/9/23 --add bypass 4/10/23
				UARTProcessSendRequest(BATLEVEL);	
			} // remove always 3 -- 4/9/23
			
			GVbBattRequest = FALSE;
		}
		
		
		GVbUpdateANT = FALSE;
		GVeOperationSts = SetOperationState(NORMAL);
		
		
		CheckCommunicationCable();
		TButtonStartScan(BIGBUTTON);	// start button scan, scan will complete during sleep.
		SleepMS(250);					// sleep 2 seconds
	
	}
	
}
/* IoT end */
 
/*
** ====================================================================================================================================================================
**     Method      : void BLEEnabledOperation(void)
**     Description :
**         This operation is for BLE    
**     Parameters  : Nothing
**     Returns     : Nothing
**====================================================================================================================================================================
*/
void BLEEnabledOperation(void){
	
	RoutinProcess();										// The same tasks as in normal non operation
	GVwBLEDone = FALSE;										// clear the flag  before processing BLE
	BLEProcess();  											// All tasks related to BLE
	
	GVwBLENoActionTimer++;									// Timer would reset during BLEProcss if command received from BLE
	if(GVwBLENoActionTimer > BLETIMEOUT ){					// 
		GVwBLEDone = TRUE;									// Terminate BLE
	}
	
	if(GVwBLEDone){
		GVeOperationSts = SetOperationState(NORMAL);		// change to normal operation
		return;	
	}
	CheckCommunicationCable();
	TButtonStartScan(BIGBUTTON);							// start button scan, scan will complete during sleep.
	SleepMS(250);											// sleep 2 seconds
}

/*
** ===========================================================================================================================
**     Method      : void CalibrationOperation(void)
**     Description :
**         This operation is for distance calibration when button was touched for 20 to 40 seconds
**         If calibration successful, blink LED at  1 Hz for 10 seconds , otherwise, blink LED at 4 Hz for 10 seconds
**        
**     Parameters  : Nothing
**     Returns     : Nothing
** ===========================================================================================================================
*/
void CalibrationOperation(void){
		
	static uint8 bCount = 0;
	uint16 wTemp1, wTemp2, wTemp3;

	uint8 bLp;

	if(bCount > 15){						// time to start calibrating. Duty rate is 2 second, waiting for 30 seconds
		bCount = 0;							// reset it
		if(CalibrateTarget(NVsOpPara.IRCalibrationTH,&wTemp1, &wTemp2,&wTemp3)){ // use new calibrated data only if calibration successful 
			NVsOpPara.CalibrationFlag = CURRENTCAL;
			NVsOpPara.IRLevel = wTemp1;
			NVsOpPara.CalibrationEcho = wTemp2;
			//NVsOpPara.MinUserTH = NVsOpPara.CalibrationEcho + NVsOpPara.CalibrationOffset;
			NVsOpPara.MinUserTH = NVsOpPara.CalibrationEcho + CALOFFSET;
			NVsOpPara.DistanceAdjustedCT += 1;
			
			for(bLp=0; bLp < 10; bLp++){	// indicate calibration successful
				BlinkLED(10); 
				SleepMS(1000);				// sleep 1 seconds
			}
		}
		else{								// calibration failed
					
			for(bLp=0; bLp < 40; bLp++){	// indicate calibration fail
				BlinkLED(10); 
				SleepMS(250);				// sleep 1/4 seconds
			}
			
			LED_ClrVal();					// in case it was turned on
			NVsOpPara.AdjudtedFailCT += 1 ;	
		}
		
		SaveParaToFlash(NVsOpPara);			// store calibration results to flash
		//PartialInit();						// Calibration done, Exit calibration mode
		while(1); 			// restart
		
	}
	else{					// not the time yet, keep using LED to tell user to leave
		
		BlinkLED(1); 
		bCount++;
		SleepMS(2000);		// sleep 2 seconds
		
	}
	
}


/*
** ===========================================================================================================================
**     Method      : void CalibrationOperation(void)
**     Description :
**         This operation is for distance calibration when button was touched for 20 to 40 seconds
**         At the end of calibration,If calibration successful, LED blink for 10 seconds in HZ.
**         If failed, LED blink for 10 seconds in 4 HZ
**        
**     Parameters  : Nothing
**     Returns     : Nothing
** ===========================================================================================================================
*/

/*
void CalibrationOperation(void){
		
	static uint8 bCount = 0;
	uint16 wTemp1;
	uint8 bLp;

	if(bCount >= 7){												// time to start calibrating. Duty rate is 2 second, waiting for 30 seconds
		bCount = 0;													// reset it
		wTemp1 = MeasureBackground(NVsOpPara.IRLevel,2000,TRUE);	// go to measure it.
		if(wTemp1 <= NVsOpPara.MaxBackground){ 						// Value is in valid range
			NVsOpPara.CalibrationFlag |= FIELDCAL; 					// Set flag
			NVsOpPara.MinUserTH = wTemp1 + NVsOpPara.UserStepInTH; 	// user threshold will depends on this
			NVsOpPara.DistanceAdjustedCT += 1;						// increase the counter
			for(bLp = 0; bLp < 10; bLp++){							// indicate calibration successful
				BlinkLED(1); 
				SleepMS(1000);										// sleep 1 seconds
			}
		}
		else{								// calibration failed
			for(bLp = 0; bLp < 40; bLp++){	// indicate calibration fail
				BlinkLED(1); 
				SleepMS(250);				// sleep 1/4 seconds
			}
			LED_ClrVal();					// in case it was turned on
			NVsOpPara.AdjudtedFailCT += 1 ;	
		}
		
		SaveParaToFlash(NVsOpPara);			// store calibration results to flash
		//PartialInit();					// Calibration done, Exit calibration mode
		while(1); 	// restart
		
	}
	else{	// not the time yet, keep using LED to tell user to leave
		BlinkLED(1); 
		bCount++;
		SleepMS(2000);		// sleep 2 seconds
		
	}
	
}
*/

/*
** ===========================================================================================================================
**     Method      : TestOperation(void)
**     Description :
**         This operation is performed when test cmd is received from UART  
**     Parameters  : Nothing
**     Returns     : Nothing
** ===========================================================================================================================
*/
void TestOperation(void){
	
	ProcessTestCmd();			// Go to process the commands
	CheckCommunicationCable();	// If cable is removed, out of this test mode automatically.
	WaitMS(250);	
			
}
 
/*
** ===========================================================================================================================
**     Method      : SetOperation(void)
**     Description :
**         This operation is performed when set cmd is received from UART.
**         It is for service monitor.
**         Process command from UART   
**     Parameters  : Nothing
**     Returns     : Nothing
** ===========================================================================================================================
*/
void SetOperation(void){
	
	ProcessSettingCmd();
	CheckCommunicationCable();
	WaitMS(250);	
}

/*
** ===========================================================================================================================
**     Method      : void ShipOperation(void)
**     Description :
**         	This operation is during in shipping mode
**       	It checks if ship condition still present, if condition does not exist (either solar cover or IR window caver removed) 
**       	then restart through watchdog reset.
**     Note:	During shipping, if battery contact lost connection shortly, unit will reset, all the global vaiabls would be cleared.
**     			So don't use global vaiables in shipping operation. 
**     Parameters  : Nothing
**     Returns     : Nothing
** =============================================================================================================================
*/
void ShipOperation(void){
		
	WDReset();							// kick dog
	if(IsSystemOutShipCondition()){		// wake up
		NVbInShippingSts = FALSE;		// clear in shipping flag
		NVbSkipReset = FALSE;			// clear skip reset flag
		//NVbDongleDeepSlpModeIn = 3; 	// goout deepsleep mode
		while(1);						// waiting for reset. start from beginning
	}
	else{								// still in ship
				
		SleepMS(5000);					// sleep 5 seconds
	}

}

/*
** ============================================================================================================================================================================
**     Method      : StopOperation(void)
**     Description :
**         	This operation is during in stop mode
**       	It checks if battery voltage back to normal every 5 seconds.
**       	if so, restart.
**       	It also check battery disconnecting time, two flags will be set based on this time.
**       	1. NVbSkipReset : if more than 6 seconds(2 wakeups), set this flag.
**       	2. NVbCheckBatteryType : set if more than 20 seconds
**        
**     Parameters  : Nothing
**     Returns     : Nothing
** =============================================================================================================================================================================
*/
/*
void StopOperation(void){
	
	uint16 wTemp;

		WDReset();										// kick dog
		GVeBatterySts = CheckBattery(&wTemp);			// check battery
		if(NVbCheckBatteryType != CHECKTYPE){ 			// It was cleared when get in stop state
			if(GVeBatterySts == BDISCONNECT){			// Battery disconnected
				GVbBDisconCT++;							// it was cleared when get in this state
				if(GVbBDisconCT >= 2){					// 5 to 6 second seconds
					NVbSkipReset = 0;					// need reset procedure if reset from PIN. It was set after previous reset procedure
					
				}
				if(GVbBDisconCT >= 5){					// 20 to 21 seconds
					NVbCheckBatteryType = CHECKTYPE;	// set the flag, need to check battery type at start
				}
			
			}
		}
		
		if(NVbCheckBatteryType == CHECKTYPE){   		// Battery has been disconnected more than 21 second
			if(wTemp >= STARTVOLTAGE){					// wake up?
				wTemp = ReadBattery();					// Double check it
			}
			if(wTemp >= STARTVOLTAGE){					// confirmed
				while(1);								// waiting for reset. start from beginning
			}
		}
		else{											// Battery disconnected less than 21 seconds, using different wake up voltage
			if(wTemp >= GVsBatteryTH.wWakeupVolt){		// wake up?
				wTemp = ReadBattery();					// Double check it
			}
			if(wTemp >= GVsBatteryTH.wWakeupVolt){		// confirmed
				while(1);								// waiting for reset. start from beginning
			}
		}
		
		SleepMS(5000);									// sleep for 5 seconds
			
}
*/

void StopOperation(void){
	
	uint16 wTemp;

		WDReset();										// kick dog
		GVeBatterySts = CheckBattery(&wTemp);			// check battery
		if(NVbCheckBatteryType == SKIPCHECKTYPE){ 		// It was set when get in stop state
			if(GVeBatterySts == BDISCONNECT){			// Battery disconnected
				GVbBDisconCT++;							// it was cleared when get in this state
				if(GVbBDisconCT >= 2){					// 5 to 6 second seconds
					NVbSkipReset = 0;					// need reset procedure if reset from PIN. It was set after previous reset procedure
					
				}
				if(GVbBDisconCT >= 5){					// 20 to 21 seconds
					NVbCheckBatteryType = TRUE;			// clear SKIPCHECKTYPE flag, need to check battery type at start
				}			
			}
		}
		
		if(NVbCheckBatteryType == TRUE){   				// Battery has been disconnected more than 21 second
			if(wTemp >= STARTVOLTAGE){					// wake up?
				wTemp = ReadBattery();					// Double check it
			}
			if(wTemp >= STARTVOLTAGE){					// confirmed
				while(1);								// waiting for reset. start from beginning
			}
		}
		else{											// Battery disconnected less than 21 seconds, using different wake up voltage
			if(wTemp >= GVsBatteryTH.wWakeupVolt){		// wake up?
				wTemp = ReadBattery();					// Double check it
			}
			if(wTemp >= GVsBatteryTH.wWakeupVolt){		// confirmed
				while(1);								// waiting for reset. start from beginning
			}
		}
		SleepMS(5000);									// sleep for 5 seconds
			
}


