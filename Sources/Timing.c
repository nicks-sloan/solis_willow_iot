/*
 * Timing.c
 *
 *  Created on: Feb 17, 2017
 *      Author: Scott Wang
 *      This module contains all functions for timing
 */

#include "Timing.h"
#include "Operation.h"
#include "BLE.h"


/*
** =========================================================================================================================
**     Method      :  void SleepMS(uint16 ms)
**     Description :
**         This method put CPU in sleep mode for inputed time in milliseconds.
**     Parameters  : sleep time in milliseconds
**     Returns     : Nothing
** =========================================================================================================================
*/
void SleepMS(uint16 ms){
	
	LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;					// reset timer
	LPTMR0_CSR |= LPTMR_CSR_TIE_MASK;  					// Enable interrupt
	if(ms > 2){
	  LPTMR0_CMR = (ms-2); 								// set the sleep time
	}
	else{
		LPTMR0_CMR = ms; 								// set the  sleep time	
	}
	LPTMR0_CSR |= LPTMR_CSR_TEN_MASK; 					// start timer
	
	if(GVeOperationSts == BLE){							// can't be full sleep incase to miss something from BLE 
		Cpu_SetOperationMode(DOM_SLEEP, NULL, NULL);	// Stop mode, uart0 is still active
	}
	else{
		Cpu_SetOperationMode(DOM_STOP, NULL, NULL); 	// LLS Mode
	}
}

/*
** =========================================================================================================================
**     Method      :  void SleepMS(uint16 ms)
**     Description :
**         This method put CPU in sleep mode for inputed time in milliseconds.
**     Parameters  : sleep time in milliseconds
**     Returns     : Nothing
** =========================================================================================================================
*/
void SleepMSWithUART(uint16 ms){
	  
	LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;			// reset timer
	LPTMR0_CSR |= LPTMR_CSR_TIE_MASK;  			// Enable interrupt
	if(ms > 2){
	  LPTMR0_CMR = (ms-2); 						// set the sleep time
	}
	else{
		LPTMR0_CMR = ms; 						// set the  sleep time	
	}
	LPTMR0_CSR |= LPTMR_CSR_TEN_MASK; 			// start timer
	Cpu_SetOperationMode(DOM_SLEEP, NULL, NULL); // go sleep
}


/*
** =========================================================================================================================
**     Method      :  SleepSecond(uint8 bScnd)
**     Description :
**         This method put CPU in sleep mode for inputed time in seconds.
**     Parameters  : sleep time in seconds
**     Returns     : Nothing
** =========================================================================================================================
*/
void SleepSecond(uint8 bScnd){
	uint8 bLp;
	for(bLp = 0; bLp < bScnd; bLp++){
		SleepMS(1000);
	}
}

/*
** =========================================================================================================================
**     Method      :  SleepForever(void)
**     Description :
**         This method put CPU in sleep mode and shall not be waked up until a hardware reset.
**     Parameters  : Nothing
**     Returns     : Nothing
** =========================================================================================================================
*/
void SleepForever(void){
	  
	LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;			// stop timer and clear flag to prevent waking up CPU
	Cpu_SetOperationMode(DOM_STOP, NULL, NULL); // go sleep
}

/*
** =========================================================================================================================
**     Method      :  void WaitMS(uint16 ms)
**     Description :
**          This method put CPU in wait mode for inputed time in milliseconds.
**          Note that beside LPTMR, any other interrupt can also wake up CPU from wait mode.
**          Wait time can be terminated by other interrupt.
**     Parameters  : wait time in milliseconds
**     Returns     : Nothing
** =========================================================================================================================
*/
void WaitMS(uint16 ms){
	
	LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;			// reset timer
	LPTMR0_CSR |= LPTMR_CSR_TIE_MASK;  			// Enable interrupt
	if(ms > 2){
		  LPTMR0_CMR = (ms-2); 					// set the timer
	}
	else{
		LPTMR0_CMR = ms; 						// set the  sleep time	
	}
	LPTMR0_CSR |= LPTMR_CSR_TEN_MASK; 			// start timer
	WDReset();
	Cpu_SetOperationMode(DOM_WAIT, NULL, NULL); // go wait, and let the timer wake me up
	
}

/*
void DelayMS(uint16 ms){
	
	WDReset();
	SIM_SCGC5|=SIM_SCGC5_LPTMR_MASK;// Make sure the clock to the LPTMR is enabled 
	LPTMR0_CSR=0;//reset timer, disable interrupt
	//LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;	//reset timer
	LPTMR0_CMR = ms; // set the  sleep time	
	LPTMR0_CSR |= LPTMR_CSR_TEN_MASK; // start timer
	while (!(LPTMR0_CSR & LPTMR_CSR_TCF_MASK));//Wait for counter to reach compare value
	LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;	//stop timer and clear flag
	
	  
}
*/

/*
** =========================================================================================================================
**     Method      :  void DelayMS(uint16 ms)
**     Description :
**         This method delay in milliseconds. 
**         It uses regular timer module but not LPTM, so it is much more accurate than WaitMS.
**     Parameters  : delay time in milliseconds
**     Returns     : Nothing
** =========================================================================================================================
*/
void DelayMS(uint16 ms){
	
	WDReset();						// kick the dog 
	TI1_Enable();
	wTimerTick = 0; 				// Clear timer
	while (wTimerTick < ms);		// watchdog will rest if ms is biger than watchdog time
	
	wTimerTick = 0; 				// reset flag
	TI1_Disable(); 
};

/*
** =========================================================================================================================
**     Method      :  void StopLPTMR(void)
**     Description :
**         This method stop LPTMR.
**     Parameters  : Nothing
**     Returns     : Nothing
** =========================================================================================================================
*/
void StopLPTMR(void){
	
	LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;	//stop timer and clear flag
	  
}

/*
** =========================================================================================================================
**     Method      : void Delay(uint16 dlyTick)
**     Description :
**         This method delay for some ticks. used for microseconds delay.
**     Parameters  : ticks for delay
**     Returns     : Nothing
** =========================================================================================================================
*/
void DelayUS(uint16 dlyTick)
{
  uint16 i;
  for(i = 0; i < dlyTick; i++){
	  __asm("nop");
  }
}

/*
** =========================================================================================================================
**     Method      :uint8 IsPowerupDone(void)
**     Description :
**         This method check if power up expired.
**         This is called every second
**     Parameters  : nothing
**     Returns     : power up status
** =========================================================================================================================
*/
uint8 IsPowerupDone(void){
	
	static uint8 bPowerupSec = 0;
	static uint8 bPowerupMin = 0;
	uint8 bSts;
	
		bSts = NOTDONE;
		bPowerupSec += 1;
		if(bPowerupSec >= 60){						// one minute
			bPowerupSec = 0;
			bPowerupMin += 1;					
			if(bPowerupMin >= POWERUPMIN){			// the moment of power up period expire
				bSts = POWERUPDONE; 				// change the status
				bPowerupMin = 0;					// Reset for next  
				LED_ClrVal(); 						// in case it was set
				if(GVeUserSts == NOTPRESENT){
					GVbDutyRate = NORMALDUTYRATE; 	// set to normal duty rate
				}
				else{	
					GVbDutyRate = USERINDUTYRATE; 	// set to target in duty rate
				}
			}
		}
	return bSts;
}

/*
** =========================================================================================================================
**     Method      : UpdateOperationTime(void) 
**     Description :
**         This method is called at 1 HZ, it updates unit run time.
**         It also store operation data to flash every day.
**         It check solar power every hour.
**     Parameters  : Nothing
**     Returns     : Nothing
** =========================================================================================================================
*/
void UpdateOperationTime(void) {
	
	static uint8 bSolisPower = FALSE;
	static uint8 bPCount = 0;
	 
		NVsOpPara.RSecond += 1;   				// update run time
		if(NVsOpPara.RSecond >= 60){			// every minute
			NVsOpPara.RSecond -= 60;			// the second maybe increased more than 1 in flush action
			NVsOpPara.RMinute += 1;
			if(NVsOpPara.RMinute >= 60){ 		// every hour
				NVsOpPara.RMinute = 0;
				NVsOpPara.RHour += 1;
				if(bSolisPower == FALSE){
					if(IsSolarCellgetPower()){ 	// Check solar cell
						bPCount += 1;			// Count of getting good power
						if(bPCount >= 3){
							bSolisPower = TRUE;	// Solar power good
						}
					}
				}
				if(NVsOpPara.RHour >= 24){ 		// every day
					NVsOpPara.RHour = 0;
					NVsOpPara.RDay += 1;
					if(NVsOpPara.RDay >= 365){	// every year
						NVsOpPara.RDay = 0;
						NVsOpPara.RYear += 1;
					}
					if(bSolisPower == FALSE){	// store solar cell bad days
						NVsOpPara.BadSolisDay += 1;
					}
					bSolisPower = FALSE; 		// reset for next day
					bPCount = 0;				// reset for next day
					SaveParaToFlash(NVsOpPara);	// save data to flash every 24 hours
					SaveBLEParaToFlash(NVsBLEPara); // save BLE related data
				}
			}
		}
}

/*
** =========================================================================================================================
**     Method      : void UpdateSentinelTime(void)
**     Description :
**         This method update sentinel time.
**     Parameters  : Nothing
**     Returns     : Nothing
** =========================================================================================================================
*/
void UpdateSentinelTime(void) {
		
	GVsSentinelTime.wSentinelSecond += 1;   		// update run time
	if(GVsSentinelTime.wSentinelSecond >= 3600){	// pass a hour
		GVsSentinelTime.wSentinelSecond = 0; 		// reset seconds
		GVsSentinelTime.bSentinelHour += 1;			// increase a hour	
	}
		
}

/*
** =========================================================================================================================
**     Method      :SentinelTimeType ResetSentinelTimer(void)
**     Description :
**         This method clear sentinel timer.
**     Parameters  : sentinel timer
**     		
**     Returns     : Nothing
** =========================================================================================================================
*/
SentinelTimeType ResetSentinelTimer(void){
	SentinelTimeType sTemp;	
		sTemp.bSentinelHour = 0;
		sTemp.wSentinelSecond = 0;
		return sTemp;
	
}
