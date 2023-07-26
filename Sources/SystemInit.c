/*
  * SystemInit.c
 *
 *  Created on: Feb 20, 2017
 *      Author: Scott Wang
 *      This module contains all functions for the system initialization. 
 *      and should be performed before entering mail loop.
 */

#include "SystemInit.h"
#include "IRSensor.h"
#include "TouchSensor.h"
#include "UART.h"
#include "Timing.h"
#include "PowerSupply.h"
#include "Operation.h"
#include "BLE.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "Ant.h"

// variables must stay for partial start, these are only initialized at full start
__attribute__ ((section (".NonVolatileData"))) SolisBLEParaType NVsOpPara;				// General Operation parameters
__attribute__ ((section (".NonVolatileData"))) uint8 NVbCheckBatteryType;				// Flag to check battery type at reset
__attribute__ ((section (".NonVolatileData"))) uint8 NVbSkipReset;						// Flag to skip full start
__attribute__ ((section (".NonVolatileData"))) uint8 NVbPowerupSts;						// Power up status 
__attribute__ ((section (".NonVolatileData"))) uint8 NVbInShippingSts;					// Flag of shipping status
__attribute__ ((section (".NonVolatileData"))) uint8 NVbSolenoidSts;					// Solenoid state
__attribute__ ((section (".NonVolatileData"))) BLEDongleParaType NVsBLEPara;			// BLE related Operation parameters
__attribute__ ((section (".NonVolatileData"))) uint8 NVbBLEDongleIn;					// flag of BLE Dongle installed
__attribute__ ((section (".NonVolatileData"))) uint16 NVwHandWaveIR;					// IR level for detecting handwave
__attribute__ ((section (".NonVolatileData"))) BatCalType NVsOffset;					// Battery calibration Data

__attribute__ ((section (".NonVolatileData"))) ANTDongleParaType NVsANTPara;			// ANT parameters
__attribute__ ((section (".NonVolatileData"))) uint8 NVbRFEnDisableModeIn;		// RF dongle RF enable/Disable, mode in 3/30/23 
                                                                                // 0 =  in normal mode,  1 = enable RF, 2 =  processed enable RF, 3 = Disable RF, 4 = processed disable RF 
__attribute__ ((section (".NonVolatileData"))) uint8 NVbDongleDeepSlpModeIn;	// RF Dongle deep sleep active/deactive, mode in
//__attribute__ ((section (".NonVolatileData"))) uint8 NVbFeatureEnable;			// 0 = disable, 1= enable
//__attribute__ ((section (".NonVolatileData"))) uint8 NVbOccupancyStaus;				// 0 = Occupancy not supported, 1= enter, 2= vacant (3 = standing, 4 = sitting)= not supported in v7




// Global Variables, Must initialized at any reset
SolisBLEParaType GVsTempPara;    		// Temporary used for real parameter transition	
OperationStsType GVeOperationSts;		// Operation state 
UserStsType GVeUserSts;					// User state
ButtonStsType GVsButtonSts;				// Button state
BatteryStsType GVeBatterySts;			// Battery state
SentinelTimeType GVsSentinelTime;		// Sentinel timer
SolenoidOnTimeType GVsTurnOnTM;			// Solenoid turn on time
PowerTHType GVsBatteryTH;				// Battery threshold
uint8 GVbFlushRequest;					// Type of flush
uint8 GVbDutyRate;						// IR scan duty rate
uint8 GVbBDisconCT;						// count of battery in disconnection
uint8 GVbSentinelFlush;					// Sentinel flag
uint8 GVbBLEEnabled;					// Flag of BLE donggle enable
uint8 GVbWakeBLE;						// flag to wake BLE
uint8 GVbInDiag;						// Flag of unit in diagnostics state
uint16 GVwBLENoActionTimer;				// Timer to tracking no communication from BLE
uint8 GVwBLEDone;						// flag of BLE done


uint8 debugdata;						// just for dubug
uint16 wBattest[20];		

uint16 debugStartReason; 					// sheng debug restart reason

/* Sheng MVP Mod begin */
// SR UART vars
uint8 GVbFlushActivate;				// Flag, Activate flush (Sheng)
uint8 GVbSRModuleStatus; 			// SR Module status list
uint8 GVbSRRefreshCmd; 				// SR refresh command 69 (0x45)
uint8 GVbStorePreviousCmd; 			// Store previous command
uint8 GVbUpdateANT;					// flag to update RF
//uint8 GVbWakeANT;					// flag to wake ANT
//uint8 GVbANTEnabled;					// Flag of ANT donggle enable
uint8 GVbBattRequest;				// Battery level change flag request
uint8 GVbUartFlushRequest;			// Flush flag request
uint8 GVbStoreFlushRequest;			// store Flush flag request

uint8 GVbDIAGLEDFLG;			// Diagnosis BLink LED flag
uint8 GVbDIAGVALVEFLG;			// Diagnosis VALVE flag

uint8 GVbDiagSensorStat;			// Diagnosis sensor status value
uint8 GVbDiagValveStat;			// Diagnosis valve status value
uint8 GVbDiagSolarStat;			// Diagnosis Solar status value

//uint8 GVbCallRFEnDisable;       // call rf enable/disable function
//uint8 GVbRFEnDisabState;		// RF dongle RF enable/Disable 
//uint8 GVbPreRFEnDisabState;		// previous RF dongle RF enable/Disable 

uint8 GVbOccupRoutineEnDisable; // call occupancy enable/disable function
uint8 GVbOccupState;			// Occupancy State 
uint8 GVbPreOccupState;			// previous Occupancy state 

//uint8 GVbRFEnDisabFlg;		// RF dongle RF enable/Disable 
//uint8 GVbShipDeepSlpFlg;		// RF Dongle deep sleep active/deactive flag

/*
** =========================================================================================================================
**     Method      :PullUpUartWakePin(void){
**     Description :
**         This method pull up Uart Wake pin so that connection status can be detected correctly
**     Parameters  : 
**     Returns     : Nothing
** =========================================================================================================================
*/
void PullUpUartWakePin(void){
	
	PORTC_PCR6 = (uint32_t)(PORTC_PCR6 | (uint32_t)(
	            		   PORT_PCR_PS_MASK |
	            		   PORT_PCR_PE_MASK)
	            		   );
} 
/* Sheng MVP Mod end */

/*
** =========================================================================================================================
**     Method      :PullUpComTriggerPin(void){
**     Description :
**         This method pull up communication cable TRIGGER pin so that connection status can be detected correctly
**     Parameters  : 
**     Returns     : Nothing
** =========================================================================================================================
*/
void PullUpComTriggerPin(void){
	
	PORTC_PCR6 = (uint32_t)(PORTC_PCR6 | (uint32_t)(
	            		   PORT_PCR_PS_MASK |
	            		   PORT_PCR_PE_MASK)
	            		   );
}

/*
** =========================================================================================================================
**     Method      :PullUpSentinelSetPin(void){
**     Description :
**         This method pull up SentinelSet pin so that Sentinel setting can be detected correctly
**     Parameters  : 
**     Returns     : Nothing
** =========================================================================================================================
*/
void PullUpSentinelSetPin(void){
	
	PORTA_PCR18 = (uint32_t)(PORTA_PCR18 | (uint32_t)(
	            		   PORT_PCR_PS_MASK |
	            		   PORT_PCR_PE_MASK)
	            		   );	
}

/*
** =========================================================================================================================
**     Method      :DisablePullUpSentinelSetPin(void){
**     Description :
**         This method disable the pull up /down, so it will not consume power if the external is pull up/down.
**     Parameters  : 
**     Returns     : Nothing
** =========================================================================================================================
*/
void DisablePullUpSentinelSetPin(void){
	
	 PORTA_PCR18 = (uint32_t)(PORTA_PCR18 & (uint32_t)~(uint32_t)(
			 	 	 	 	 PORT_PCR_PS_MASK |
			 	 	 	 	 PORT_PCR_PE_MASK)
			 	 	 	 	 );	
}

/*
** =========================================================================================================================
**     Method      :PullUpTXPin(void)
**     Description :
**         This method pull up Uart TX pin
**     Parameters  : 
**     Returns     : Nothing
** =========================================================================================================================
*/
void PullUpTXPin(void){
	
	PORTA_PCR2 = (uint32_t)(PORTA_PCR2 | (uint32_t)(
	            		   PORT_PCR_PS_MASK |
	            		   PORT_PCR_PE_MASK)
	            		   );	
}

/*
** =========================================================================================================================
**     Method      :DisablePullUpTXPin(void){
**     Description :
**         This method disable the pull up /down, so it will not consume power if the external is pull up/down.
**     Parameters  : 
**     Returns     : Nothing
** =========================================================================================================================
*/
void DisablePullUpTXPin(void){
	
	 PORTA_PCR2 = (uint32_t)(PORTA_PCR2 & (uint32_t)~(uint32_t)(
			 	 	 	 	 PORT_PCR_PS_MASK |
			 	 	 	 	 PORT_PCR_PE_MASK)
			 	 	 	 	 );	
}

/*
** =========================================================================================================================
**     Method      :ConnectUART(void)
**     Description :
**         This method configure PORTA1 and PORTA2 as UART
**     Parameters  : 
**     Returns     : Nothing
** =========================================================================================================================
*/
void ConnectUART(void){
	PORTA_PCR1 = (uint32_t)((PORTA_PCR1 & (uint32_t)~(uint32_t)(
               PORT_PCR_ISF_MASK |
               PORT_PCR_MUX(0x05)
              )) | (uint32_t)(
               PORT_PCR_MUX(0x02)
              ));
 /* PORTA_PCR2: ISF=0,MUX=2 */
	PORTA_PCR2 = (uint32_t)((PORTA_PCR2 & (uint32_t)~(uint32_t)(
               PORT_PCR_ISF_MASK |
               PORT_PCR_MUX(0x05)
              )) | (uint32_t)(
               PORT_PCR_MUX(0x02)
              ));
}

/*
** =========================================================================================================================
**     Method      :DisableUART(void)
**     Description :
**         This method disconnects UART and set both RX and Tx pins as disabled. to save power
**     Parameters  : 
**     Returns     : Nothing
** =========================================================================================================================
*/
void DisableUART(void){
	
	PORTA_PCR1 = (uint32_t)((PORTA_PCR1 & (uint32_t)~(uint32_t)(
	               PORT_PCR_MUX(0x07))));
	
	PORTA_PCR2 = (uint32_t)((PORTA_PCR2 & (uint32_t)~(uint32_t)(
		               PORT_PCR_MUX(0x07))));
}

/*
** =========================================================================================================================
**     Method      :DisableLATCHCurrentSensing(void)
**     Description :
**         This method set Latch current feedback pin disconected to save power.
**     Parameters  : 
**     Returns     : Nothing
** =========================================================================================================================
*/
void DisableLATCHCurrentSensing(void){
	
	LatchCurrent_Disable(LatchCurrent_DeviceData);
	
	PORTC_PCR2 = (uint32_t)((PORTC_PCR2 & (uint32_t)~(uint32_t)(
	               PORT_PCR_MUX(0x07))));
	
}

/*
** =========================================================================================================================
**     Method      :EnableLATCHCurrentSensing(void)
**     Description :
**         This method set Latch current feedback pin conected.
**     Parameters  : 
**     Returns     : Nothing
** =========================================================================================================================
*/
void EnableLATCHCurrentSensing(void){
	
	  LatchCurrent_Init(LatchCurrent_DeviceData); 
	  
}

void EnableLATCHCurrentSensingold(void){
	 /* Initialization of Port Control registers */
	  /* PORTC_PCR2: ISF=0,MUX=1 */
	 PORTC_PCR2 = (uint32_t)((PORTC_PCR2 & (uint32_t)~(uint32_t)(
	                PORT_PCR_ISF_MASK |
	                PORT_PCR_MUX(0x06)
	               )) | (uint32_t)(
	                PORT_PCR_MUX(0x01)
	               ));
	  /* PORTC_PCR2: ISF=1,IRQC=9 */
	  PORTC_PCR2 = (uint32_t)((PORTC_PCR2 & (uint32_t)~(uint32_t)(
	                PORT_PCR_IRQC(0x06)
	               )) | (uint32_t)(
	                PORT_PCR_ISF_MASK |
	                PORT_PCR_IRQC(0x09)
	               ));
	  /* NVIC_IPR7: PRI_31=0x80 */
	  NVIC_IPR7 = (uint32_t)((NVIC_IPR7 & (uint32_t)~(uint32_t)(
	               NVIC_IP_PRI_31(0x7F)
	              )) | (uint32_t)(
	               NVIC_IP_PRI_31(0x80)
	              ));
	  /* NVIC_ISER: SETENA|=0x80000000 */
	  NVIC_ISER |= NVIC_ISER_SETENA(0x80000000);
	
}

/*
** =========================================================================================================================
**     Method      :CheckSentinelSetting(void)
**     Description :
**         This method check if sentinel pin is pulled down externally. if so, sentinel flush is required.
**         It also set the pin as output low after detection so that it would not leak current.
**         For BLE units,sentinel is set through APP, ignor the jumper
**     Parameters  : 
**     Returns     : TRUE or FALSE
** =========================================================================================================================
*/
uint8 CheckSentinelSetting(void){
	
	uint8 bResult;
#ifdef WITHBLE
	if(NVsOpPara.SentinalTM == 0){
		bResult = NOTSENTINEL;			// No sentinel 
	}
	else{
		bResult = YESSENTINEL;			// sentinel requested
	}
	return bResult;
#else	
		Sentinel_SetDir(FALSE);  		// set the pin as input
		PullUpSentinelSetPin();  		// pull up internally
		if(Sentinel_GetVal()){			// no external pull down. (sentinel jump is not in)
			bResult = NOTSENTINEL;		// No sentinel 
		}
		else{							// setting jump connected
			bResult = YESSENTINEL;		// sentinel requested
		}
		DisablePullUpSentinelSetPin(); 	// prevent current leak
		Sentinel_SetDir(TRUE);  		// set pin as output
		Sentinel_ClrVal();				// set output to 0.
		return bResult;
		
#endif
}

/*
** =========================================================================================================================
**     Method      :GetSoftwareBuildDate(uint8 * Bday, uint8* BMonth, uint16* Byear)
**     Description :
**         This method get software build date
**     Parameters  : point to day, month and year to store the results
**     Returns     : Nothing
** =========================================================================================================================
*/
void GetSoftwareBuildDate(uint8 * Bday, uint8* BMonth, uint16* Byear){
	
	char StrMonth[] = "Jan";
	int BdDay;
	uint8 BdMonth;
	int BdYear;
	char DateStr[] = __DATE__;		// Date provided by compiler	
		sscanf(DateStr,"%s%d%d",StrMonth,&BdDay,&BdYear);
		if(!strcmp(StrMonth,"Jan")) BdMonth = 1;
		if(!strcmp(StrMonth,"Feb")) BdMonth = 2;
		if(!strcmp(StrMonth,"Mar")) BdMonth = 3;
		if(!strcmp(StrMonth,"Apr")) BdMonth = 4;
		if(!strcmp(StrMonth,"May")) BdMonth = 5;
		if(!strcmp(StrMonth,"Jun")) BdMonth = 6;
		if(!strcmp(StrMonth,"Jul")) BdMonth = 7;
		if(!strcmp(StrMonth,"Aug")) BdMonth = 8;
		if(!strcmp(StrMonth,"Sep")) BdMonth = 9;
		if(!strcmp(StrMonth,"Oct")) BdMonth = 10;
		if(!strcmp(StrMonth,"Nov")) BdMonth = 11;
		if(!strcmp(StrMonth,"Dec")) BdMonth = 12;
		*Bday = (uint8)BdDay;
		*BMonth = (uint8)BdMonth;
		*Byear = (uint16)BdYear;	
}

/*
** =========================================================================================================================
**     Method      :GetSoftwareVersion(uint8 * VMaj, uint8* VMin)
**     Description :
**         This method get software version
**     Parameters  : point to major and minor to store the results
**     Returns     : Nothing
** =========================================================================================================================
*/
void GetSoftwareVersion(uint8 * VMaj, uint8* VMin){
	
	char StrVMin[] = VERISION_MINOR;		// Macro, defined in SystemInit.h
	char StrVMaj[] = VERISION_MAJOR;		// Macro, defined in SystemInit.h
	int Min;
	int Maj;
	
		sscanf(StrVMin,"%d",&Min);
		sscanf(StrVMaj,"%d",&Maj);
	
		*VMaj = (uint8)Maj;
		*VMin = (uint8)Min;
	
}


/*
*
** =========================================================================================================================
**     Method      :BatCalType LoadBatteryCalFromFlash
**     Description :
**         This method get battery calibration data from flash. if data in flash is corrupted (or blank), use default instead.
**     Parameters  : Nothing
**     Returns     : Parameter struck
** =========================================================================================================================
*/
BatCalType LoadBatteryCalFromFlash(void){
	
	uint16 len;
	BatCalType sPara;

		IFsh1_EnableEvent();																			// Enable flash process events
		len = sizeof sPara;
		while(IFsh1_GetBlockFlash(FLASHBATTERYADDRESS, (IFsh1_TDataAddress) &sPara, len)!= ERR_OK); 	// wait until load parameters from flash done
		IFsh1_DisableEvent();																			// Flash process done.
		if(sPara.bCalFlag != CALIBRATED){
			sPara.wOff1 = 0;
			sPara.wOff2 = 0;
		}
		
		return sPara;
}

/*
** ===================================================================================================================================================================
**     Method      :void SaveBatCalParaToFlash(BatCalType sPara)
**     Description :
**         This method save battery calibration data  to flash. Also the data CRC is calculated. The CRC and a written flag are stored at the end of parameter
**     Parameters  : parameter
**     Returns     : Nothing
** ===================================================================================================================================================================
*/
void SaveBatCalParaToFlash(BatCalType sPara){
	uint16 len;
	
		len = sizeof sPara;
		IFsh1_EnableEvent();		//Enable flash process events
		// wait until write done
		while(IFsh1_SetBlockFlash((IFsh1_TDataAddress) &sPara, FLASHBATTERYADDRESS, len)!= ERR_OK); // let watch dog reset it if something is wrong

		IFsh1_DisableEvent();
		
}

/*
** =========================================================================================================================
**     Method      :SolisParaType LoadParaWithDefault(void)
**     Description :
**         This method set Parameters with default valve based on SKU to be built
**     Parameters  : none
**     Returns     : parameter struck
** =========================================================================================================================
*/
/*
** =========================================================================================================================
**     Method      :SolisParaType LoadParaWithDefault(void)
**     Description :
**         This method set Parameters with default valve based on SKU to be built
**     Parameters  : none
**     Returns     : parameter struck
** =========================================================================================================================
*/
SolisBLEParaType LoadParaWithDefault(void){
	 
	uint8 VMaj;
	uint8 VMin;
	uint8 BdDay;
	uint8 BdMonth;
	uint16 BdYear;
	SolisBLEParaType sPara;
	
	GetSoftwareVersion(&VMaj, &VMin);
	GetSoftwareBuildDate(&BdDay, &BdMonth, &BdYear);
#ifdef WITHBLE
	switch(SKU){
		case 1:
		case 2:
		case 21:
			sPara.Mode = CLOSETMODE;
			sPara.IRLevel  = 1700;  
			sPara.OpenTM = 28;
			sPara.ArmTM = 16;
			sPara.ONDelayTM = 2;
			
		break;
		case 3:
			sPara.Mode = CLOSETMODE;
			sPara.IRLevel  = 1700;  
			sPara.OpenTM = 20;
			sPara.ArmTM = 16;
			sPara.ONDelayTM = 2;
			
		break;
		case 8:
			sPara.Mode = CLOSETMODE;
			sPara.IRLevel  = 1700;  
			sPara.OpenTM = 28;
			sPara.ArmTM = 16;
			sPara.ONDelayTM = 2;
		break;
		case 10:
			sPara.Mode = URINALMODE;
			sPara.IRLevel  = 1200; 
			sPara.OpenTM = 12;
			sPara.ArmTM = 8;
			sPara.ONDelayTM = 1;
		break;
		case 11:
		case 12:
		case 13:
			sPara.Mode = URINALMODE;
			sPara.IRLevel  = 1200; 
			sPara.OpenTM = 22;
			sPara.ArmTM = 8;
			sPara.ONDelayTM = 1;
		break;
		default: // Closet mode
			sPara.Mode = CLOSETMODE;
			sPara.IRLevel  = 1700;  
			sPara.OpenTM = 28;
			sPara.ArmTM = 16;
			sPara.ONDelayTM = 2;
		break;
		
	}
#else
	switch(SKU){  // Non BLE units
			case 1:		// Closet
				sPara.Mode = CLOSETMODE;
				sPara.IRLevel  = 1700;  
				sPara.OpenTM = 28;
				sPara.ArmTM = 16;
				sPara.ONDelayTM = 2;
			break;
			
			case 2:		// 1.1 GPF Closet
				sPara.Mode = CLOSETMODE;
				sPara.IRLevel  = 1700;  
				sPara.OpenTM = 20;
				sPara.ArmTM = 16;
				sPara.ONDelayTM = 2;
			break;
			
			case 3:		// Urinal
				sPara.Mode = BALLPARKMODE;
				sPara.IRLevel  = 1200; 
				sPara.OpenTM = 22;
				sPara.ArmTM = 8;
				sPara.ONDelayTM = 1;
			break;
			
			case 4:		// 0.125 Urinal
				sPara.Mode = BALLPARKMODE;
				sPara.IRLevel  = 1200; 
				sPara.OpenTM = 12;
				sPara.ArmTM = 8;
				sPara.ONDelayTM = 1;
			break;
			
			case 5: // Dual Buttons
				sPara.Mode = DUALMODE;
				sPara.IRLevel  = 1700;  
				sPara.OpenTM = 28;
				sPara.ArmTM = 16;
				sPara.ONDelayTM = 2;
			break;
			
			case 6:  	// Conceal Closet 1.6 GPF
				sPara.Mode = CLOSETMODE;
				sPara.IRLevel  = 1700;  
				sPara.OpenTM = 14;
				sPara.ArmTM = 16;
				sPara.ONDelayTM = 2;
			break;
			
			case 7:  	// Conceal Closet 1.28 GPF
				sPara.Mode = CLOSETMODE;
				sPara.IRLevel  = 1700;  
				sPara.OpenTM = 18;
				sPara.ArmTM = 16;
				sPara.ONDelayTM = 2;
			break;
			
			case 8:  	// Conceal Urinal 1.0 GPF
				sPara.Mode = BALLPARKMODE;
				sPara.IRLevel  = 1200; 
				sPara.OpenTM = 14;
				sPara.ArmTM = 8;
				sPara.ONDelayTM = 1;
			break;
			
			case 9:  	// Conceal Urinal 0.5 GPF
				sPara.Mode = BALLPARKMODE;
				sPara.IRLevel  = 1200; 
				sPara.OpenTM = 28;
				sPara.ArmTM = 8;
				sPara.ONDelayTM = 1;
			break;
			
			default: // Closet mode
				sPara.Mode = CLOSETMODE;
				sPara.IRLevel  = 1700;  
				sPara.OpenTM = 28;
				sPara.ArmTM = 16;
				sPara.ONDelayTM = 2;
			break;
			
		}
#endif
	// The same for all modes
	sPara.CalibrationFlag = 1;
	sPara.VerMajor = VMaj;
	sPara.VerMinor = VMin;
	sPara.BuildM = BdMonth;
	sPara.BuildD = BdDay;
	sPara.BuildY = BdYear;
	sPara.PinResetCT = 0;
	sPara.PORResetCT = 0;
	sPara.TotalActivation = 0;
	sPara.RSecond = 0;
	sPara.RMinute = 0;
	sPara.RHour = 0;
	sPara.RDay = 0;
	sPara.RYear = 0;
	sPara.BVolt = 0;
	sPara.LBActivationCT = 0;
	sPara.MaxBackground = 14000;
	sPara.ResetCause = 0;
	sPara.CalibrationEcho = 4800;
	sPara.WaitDelayTM = 3;	
	sPara.SentinalTM = 0;
	sPara.BadSolisDay = 0;
	sPara.UrinalIR = 70;
	sPara.UserStepInTH = 1500;
	sPara.MinUserTH = 5000 ;
	sPara.TargetStableRange = 200;
	sPara.StableTimeTH = 480;
	sPara.ConfirmTimeTH = 14400;
	sPara.CleanBackground = 4000;
	sPara.SittingTH = 12000;
	sPara.IRCalibrationTH = 4800;
	sPara.TouchTH = 50;
	sPara.MaxIRTH = 3000;
	sPara.MinIRTH = 850;
	sPara.ResetCT = 0;
	sPara.BType = ALKLINE;
	sPara.FlushVolumeM = 0;
	sPara.DistanceAdjustedCT = 0;
	sPara.AdjudtedFailCT = 0;
	sPara.ConfirmedBackground = 0;
	sPara.ButtonActivations = 0;
	sPara.UpdateM = BdMonth;
	sPara.UpdateD = BdDay;
	sPara.UpdateY = BdYear;	
	return sPara;
		
}


/*
** =========================================================================================================================
**     Method      :uint8 CalculateCRC(uint8* pBlock, uint16 len)
**     Description :
**         This method calculate CRC of a block bytes. CRC i sthe xor of the bytes.
**     Parameters  : pointer to the data block
**     			   : length of the block
**     Returns     : CRC
** =========================================================================================================================
*/
uint8 CalculateCRC(uint8* pBlock, uint16 len){
	uint8 lp;
	uint8* pTemp;
	uint8 bTempCRC;
	
		pTemp = pBlock;									// start from the beginning
		bTempCRC = *pTemp;								// first byte
		for(lp = 1; lp < len; lp++){					
			bTempCRC = bTempCRC  ^ (*(pTemp + lp)); 	// Checksum
		}
		return bTempCRC;

}

/*
** =========================================================================================================================
**     Method      :SolisParaType LoadParaFromFlash(void)
**     Description :
**         This method get sensor operational parameters from flash. if data in flash is corrupted (or blank), use default instead.
**     Parameters  : Nothing
**     Returns     : Parameter struck
** =========================================================================================================================
*/
SolisBLEParaType LoadParaFromFlash(void){
	
	uint16 len;
	uint8 bTempCRC;
	uint8 VMaj;
	uint8 VMin;
	uint8 BdDay;
	uint8 BdMonth;
	uint16 BdYear;
	SolisBLEParaType sPara;
	uint8 bTempFlag;

		IFsh1_EnableEvent();																			// Enable flash process events
		len = sizeof sPara;
		while(IFsh1_GetBlockFlash(FLASHDATAADDRESS, (IFsh1_TDataAddress) &sPara, len)!= ERR_OK); 		// wait until load parameters from flash done
		while(IFsh1_GetBlockFlash((FLASHDATAADDRESS + len),&bTempCRC, 1)!= ERR_OK); 					// get stored CRC.
		while(IFsh1_GetBlockFlash((FLASHDATAADDRESS + (len+1)),&bTempFlag, 1)!= ERR_OK); 				// get written flag.
		IFsh1_DisableEvent();																			// Flash process done.
		
		if(((CalculateCRC((uint8*)&sPara, len)) == bTempCRC) && (bTempFlag == WRITTENFLAG)){			// data in flash is good // using the data from flash
			GetSoftwareVersion(&VMaj, &VMin);															
			GetSoftwareBuildDate(&BdDay, &BdMonth, &BdYear);
			sPara.VerMajor = VMaj;
			sPara.VerMinor = VMin;
			sPara.UpdateM = BdMonth;																	// update date in case firmware is updated through bootloader with command "U"
			sPara.UpdateD = BdDay;
			sPara.UpdateY = BdYear;	
		}
		else{// data in flash corrupted or blank
			sPara = LoadParaWithDefault();																// use default instead
		}
		
		return sPara;
}

/*
** ===================================================================================================================================================================
**     Method      :void SaveParaToFlash(SolisParaType sPara){
**     Description :
**         This method save parameters to flash. Also the data CRC is calculated. The CRC and a written flag are stored at the end of parameter
**     Parameters  : parameter
**     Returns     : Nothing
** ===================================================================================================================================================================
*/
void SaveParaToFlash(SolisBLEParaType sPara){
	uint16 len;
	uint8 bTempCRC;	
	uint8 bTempFlag = WRITTENFLAG;
	
		len = sizeof sPara;
		bTempCRC = CalculateCRC((uint8*)&sPara, len);
		IFsh1_EnableEvent();		//Enable flash process events
		// wait until write done
		while(IFsh1_SetBlockFlash((IFsh1_TDataAddress) &sPara, FLASHDATAADDRESS, len)!= ERR_OK); // let watch dog reset it if something is wrong
		// write CRC at the end
		while(IFsh1_SetBlockFlash(&bTempCRC, (FLASHDATAADDRESS + len), 1) !=  ERR_OK); 
		// write written flag at the end
		while(IFsh1_SetBlockFlash(&bTempFlag, (FLASHDATAADDRESS + (len + 1) ), 1) !=  ERR_OK); 	
		IFsh1_DisableEvent();
}

/*
** ===================================================================================================================================================================
**     Method      :uint16 SetSensingRange(void){
**     Description :
**         This method set sensing range by setting NVsOpPara.MinUserTH. It is only need to be called when range changing requested from BLE.
**     Parameters  : parameter
**     Returns     : Nothing
** ===================================================================================================================================================================
*/
void SetSensingRange(char* sRange){
	
	uint16 iTemp;
	
	iTemp = atoi(sRange); 					// Convert from ascii to number
	
	if(iTemp < 1) iTemp = 1;
	if(iTemp > 5) iTemp = 5;
	
	switch(iTemp){
		case 1:
			NVsOpPara.MinUserTH = 6000;		// Closest
		break;
		case 2:
			NVsOpPara.MinUserTH = 5500;	
		break;
		case 3:
			NVsOpPara.MinUserTH = 5000;	
		break;	
		case 4:
			NVsOpPara.MinUserTH = 4500;	
		break;	
		case 5:
			NVsOpPara.MinUserTH = 4000;	  // Fartest
		break;
		default:
			NVsOpPara.MinUserTH = 5000;	
		break;
	}
}

void SetSensingRangeOld(char* sRange){
	
	uint16 iTemp;
	
	iTemp = atoi(sRange); 					// Convert from ascii to number
	
	if(iTemp < 1) iTemp = 1;
	if(iTemp > 5) iTemp = 5;
	
	switch(iTemp){
		case 1:
			NVsOpPara.MinUserTH = 1540;		// Closest
		break;
		case 2:
			NVsOpPara.MinUserTH = 1520;	
		break;
		case 3:
			NVsOpPara.MinUserTH = 1500;	
		break;	
		case 4:
			NVsOpPara.MinUserTH = 1480;	
		break;	
		case 5:
			NVsOpPara.MinUserTH = 1460;	  // Fartest
		break;
		default:
			NVsOpPara.MinUserTH = 1500;	
		break;
	}

}

/*
**===================================================================================================================================================================
**		Method      : void FullStart(void)
**     	Description :
**         This method do the full start up
**         Battery type will only be set in 3 situations:
**         1.		POR Reset
**         2.		Low Power Reset
**         3.		NVbCheckBatteryType is set (Battery has been disconnected for more than predefined time)
**		Note: if com cable is connected, skip the sequence to save time for production
**		All Non BSS vaiables must be ititialized here
**     	Parameters  : Nothing
**     	Returns     : Nothing
**     variables must stay for partial start, these are only initialized at full start
__attribute__ ((section (".NonVolatileData"))) SolisBLEParaType NVsOpPara;				// General Operation parameters
__attribute__ ((section (".NonVolatileData"))) uint8 NVbCheckBatteryType;				// Flag to check battery type at reset
__attribute__ ((section (".NonVolatileData"))) uint8 NVbSkipReset;						// Flag to skip full start
__attribute__ ((section (".NonVolatileData"))) uint8 NVbPowerupSts;						// Power up status 
__attribute__ ((section (".NonVolatileData"))) uint8 NVbInShippingSts;					// Flag of shipping status
__attribute__ ((section (".NonVolatileData"))) uint8 NVbSolenoidSts;					// Solenoid state
__attribute__ ((section (".NonVolatileData"))) BLEDongleParaType NVsBLEPara;			// BLE related Operation parameters
**     
**     
** ===================================================================================================================================================================
*/
void FullStart(void){
	
	uint8 bLP;
	uint8 bTempBType;
	
	
		//GVbGetSRStatFlag = TRUE;  // no longer useful -remove 4/7/23
		
		NVsOffset = LoadBatteryCalFromFlash(); // must load befor any battery red
			
		while(ReadBattery() < STARTVOLTAGE){ // do not continue if battery voltage is less than start up voltage
			SleepMS(1000); 				
		}
	
		if(NVsOffset.bInFactoryFlag != OUTOFFACTORY){  	// In factory, go to check if there is any non-full battery
		
	//Test 7/14/23 while(IsThereNonFullBattery()){				// stay here if there is a non full battery installed
				LED_SetVal();							// Turn on LED to indicate status
				SleepMS(1000); 		
	/*test*/	SleepMS(30000); 
	//Test 7/14/23}
			LED_ClrVal();								// Turn it off
		}
	
		while(ReadBattery() < STARTVOLTAGE){ // do not continue if battery voltage is less than start up voltage
			SleepMS(3000); 				
		}
		
		// LED on 3 seconds to indicate start
		LED_SetVal();
		SleepMS(3000);
		LED_ClrVal();
		
		// One unlatch operation to shut off solenoid
		UnLatchSolenoid();									// NVbSolenoidSts is also set inside
		
		// Get Operation parameters

		NVsOpPara = LoadParaFromFlash();					// Get parameters from flash
		NVsBLEPara = LoadBLEParaFromFlash();				// BLE related parameters
		NVsANTPara = LoadANTParaFromFlash();				// ANT related parameters

		
		if(COMCable_GetVal()){	// communication cable not present. Normal operation
			
			SleepMS(3000);  	// interval for next LED blink
			
			// blink LED every 3 seconds for 14 times to match the patten of other product like G2
			for(bLP = 0; bLP < 14; bLP++){
				BlinkLED(1);
				while(ReadBattery() < STARTVOLTAGE); 			// do not continue if battery voltage is not in normal operation range
				if(COMCable_GetVal()==0){
					break;
				}
				SleepMS(3000);
			}
			
			//three more unlatch actions
			for(bLP = 0; bLP < 3; bLP++){
				BlinkLED(1);
				UnLatchSolenoid();
				if(COMCable_GetVal()==0){
					break;
				}
				SleepMS(3000);									// Give enough time to recharges the Cap
				
			}
			
			// Check Reset reason and record results
			NVsOpPara.ResetCause = StartReason;					// record reset cause
			NVsOpPara.ResetCT += 1;								// update reset count
			
			// External PIN reset
			if((StartReason & RSTSRC_PIN) == RSTSRC_PIN){ 		// reset maybe multiple reasons
				NVsOpPara.PinResetCT += 1; 						// update count			
			}	
			
			// POR reset
			if((StartReason & RSTSRC_POR) == RSTSRC_POR){ 		// reset maybe multiple reasons
				NVsOpPara.PORResetCT += 1; 						// update count
				NVbCheckBatteryType = TRUE;						// update battery type as this is considered as a new battery 	
				
			}
			
			// low power reset
			if((StartReason & RSTSRC_LVD) == RSTSRC_LVD){ 		// reset from low voltage detected
				NVbCheckBatteryType = TRUE; 					// update battery type as this is considered as a new battery 					
				
			}
						
			// Check battery type
			bTempBType = CheckBatteryType();					// check battery type
			
			if(bTempBType == LITHIUM){                          // still high voltage
				NVsOpPara.BType = bTempBType;  					// For sure it is lithium, no matter if this is a new installation
			} 
			else{// Voltage below the lithium threshold, it may not be a battery installation, but just a reset
				if(NVbCheckBatteryType != SKIPCHECKTYPE){ 		// skip flag has been cleared. the flag could be cleared if battery disconnection is long enough or cpu lost power
					NVsOpPara.BType = bTempBType;  				// update battery type as this is considered as a new battery
				}	
			}
			NVbCheckBatteryType = SKIPCHECKTYPE;				// set flag to skip check battery type.
			
			//Blink LED to indicate battery type
			if(NVsOpPara.BType == LITHIUM){ // lithium, 
				//LED on 2 seconds once
				LED_SetVal();
				SleepMS(2000);
				LED_ClrVal();
			}
			else{ // Alkaline. 
				//LED on 2 seconds twice	
				LED_SetVal();
				SleepMS(2000);
				LED_ClrVal();
				SleepMS(1000);
				LED_SetVal();
				SleepMS(2000);
				LED_ClrVal();
			}
			//GVeOperationSts = SetOperationState(NORMAL);	// start from NORMAL at any reset 
		}
		else{ // Communication cable connected
			//ToutchButtonInit();							// will take 6 seconds with 2 led blinks
		}
		// all non-initialized Global variables during reset. The variables should not be changed during short power outrage
		NVbPowerupSts = NOTDONE;							// Power up status. It should not be reset for partial reset
		NVbInShippingSts = FALSE;							// clear in shipping flag
		NVbSkipReset = SKIPSTARTER;							// Default to skip full start for next time. It will only be reset during stop or CPU lost power
		IRSensorInit();										// initialize all the data related to IR sensing algorithm
#ifdef WITHBLE
	#ifndef WITHANT  // MVP add	  
		NVwHandWaveIR = GetIRLevelForDetectingHangwave();	// Get IR level for handwave detecting
	#endif			// MVP add
#endif
}

/*
** ===================================================================================================================================================================
**     Method      : PartialStart(void)
**     Description :
**         This method perform partial start up, without LED blinks
**     Parameters  : Nothing
**     Returns     : Nothing
** ===================================================================================================================================================================
*/
void PartialStart(void){
	
	// One unlatch operation to shut off solenoid to prevent run-on when external PIN reset happens during solenoid turned on.
	UnLatchSolenoid();
	// Check Reset reason and record results
	NVsOpPara.ResetCause = StartReason;					// record reset cause
	NVsOpPara.ResetCT += 1;								// update reset count
	NVsOpPara.PinResetCT += 1; 							// update count, only external pin reset will come here						
}

/*
** ===================================================================================================================================================================
**     Method      : void SystemInit(void)
**     Description :
**         This method is for the system initialization.
**         FullStart or PartialStart is based on start condition.
**         All the global variables are initialized, but all non-volatile global variables are only initialized in full start.
**         During normal operation, if battery lost contacts temporary due to unit vibration,
**         then hardware reset signal will generated (since solenoid is connected, this reset is always generated), then this routine will be called,
**         if this happened, just skip normal start. All global varables are cleared to 0 in start for any reset, so they need to be intialised before getting normal operation.
**     Parameters  : Nothing
**     Returns     : Nothing
**     
** ===================================================================================================================================================================
*/
void SystemInit(void){
	
///* MVP Mod begin */
//	uint8 bTemp; 
///* MVP Mod end */
//	
//	uint16 wTemp;
//	
//		//TurnOffAllPeripheral();								// save power when it is not needed
//
//		debugdata = NVbSkipReset;							// for debuging
//		debugStartReason = StartReason;						//Sheng start Reason
//				
//		if(((StartReason & RSTSRC_PIN) == RSTSRC_PIN) && (NVbSkipReset == SKIPSTARTER)){ // Battery lost contact temporary during normal operation
//			PartialStart();	
//			
//		}
//		else{ // any other system start
//			FullStart();
//			NCP_init(); // NCP init 7/20/23
//		 
//		}
		NCP_init(); // NCP init 7/20/23
		
///* MVP Mod begin */		
//		PullUpUartWakePin(); 								// configure wakeup pin to high
///* MVP Mod end */
//		
//		// Initialize all global variables. These are BSS data and were cleared before getting here
//		
//		//Set Battery threshold based on battery type, Battery type is only being set on certain situations  
//		if(NVsOpPara.BType == LITHIUM){ 					// lithium, 
//			GVsBatteryTH.wLowVolt = LOWVOLTWANLIT;			// Low 					
//			GVsBatteryTH.wStopVolt = LOWVOLTSTOPLIT;		// Stop
//			GVsBatteryTH.wWakeupVolt = WAKEUPLIT;			// Wake up 
//		}
//		else{ // Alkaline. or anything else 
//			GVsBatteryTH.wLowVolt = LOWVOLTWRNALK;			// low battery threshold 
//			GVsBatteryTH.wStopVolt = LOWVOLTSTPALK;			// stop battery thresholds
//			GVsBatteryTH.wWakeupVolt = WAKEUPALK;			// Wake up 
//		} 
//		GVeBatterySts = CheckBattery(& wTemp);				// check battery
//		NVsOpPara.BVolt = wTemp;							// Battery voltage
//		SaveParaToFlash(NVsOpPara);							// store new cycle operation parameters to flash
//		GVbBDisconCT = 0;									// count of battery in disconnection
//		GVbFlushRequest = NOFLUSH;							// No flush requested
//		GVsTurnOnTM = SetValveOnTime(NVsOpPara.OpenTM);		// set valve turn on time for all types of flush. There are 4 type flush.
//		GVbSentinelFlush = CheckSentinelSetting();			// Check sentinel jumper
//		GVsSentinelTime = ResetSentinelTimer();				// reset sentinel timer	
//		GVsTempPara = NVsOpPara; 							// make a copy used as a temporary for testing and setting operation
//		GVeUserSts = NOTPRESENT;							// No User
//		if(NVbPowerupSts == NOTDONE ){						// In power up period
//			GVbDutyRate = POWERUPDUTYRATE;					// power up IR scan duty rate,
//		}
//		else{
//			GVbDutyRate = NORMALDUTYRATE;					// Normal IR scan rate
//		}
//		//PullUpComTriggerPin();							// must be pulled up, do not need it if external pull exist	
//		GVeOperationSts = SetOperationState(NORMAL);		// start from NORMAL at any reset 
//		ToutchButtonInit();									// initialize all data related to touch button	
//
// 
///* MVP Mod begin */
//		GVbSRRefreshCmd = 0;
//		GVbBattRequest = FALSE;				  	// Battery level change flag request
//		GVbUartFlushRequest = FALSE;				// Flush flag request
//		GVbStoreFlushRequest = STDFULLBT;
//		GVbFlushActivate = 0;
//		//GVbActRptThold = 1;
//		GVbActRptThold = NVsANTPara.iActivationRptTh;
//		GVbActRptTholdCnt = 1;
//		GVbDIAGLEDFLG = FALSE;
//		GVbDIAGVALVEFLG = FALSE;			// Diagnosis VALVE flag
//		GVbFactoryRestFlg = FALSE;
//		//strcpy(GVbSrange, SRANGEVALUE); // init sensor range setting
//		
//		if(wTemp >= FULLBATTERY){
//			bTemp = 100;
//		}
//		else{
//			if(wTemp <= ENDBATTERY){
//				bTemp = 1;
//			}
//			else{
//				bTemp = (wTemp - ENDBATTERY) / STEPS;
//			}
//		}
//		
//		GVbBatLevelStore = bTemp;
//		GVbBatLevelChkCnt = 0; // 10/28/20
//		
//		//GVbRFEnDisabFlg = 0;		// 0 = RF enable
//		//GVbShipDeepSlpFlg = 0;      // 0 = Deep sleep deactivated
//		//GVbCallRFEnDisable = 0;       // call rf enable/disable function
//		//GVbRFEnDisabState = 0;
//		//GVbPreRFEnDisabState = 0;		// previous RF dongle RF enable/Disable 
//		
//		GVbOccupRoutineEnDisable = 0;  // call occupancy enable/disable function
//		GVbOccupState = 2;			   // Occupancy State  2 = vacant
//		GVbPreOccupState = 2;		   // previous Occupancy state
//		
//		if(NVsANTPara.iShipModeDeepSleep == 1)
//		{
//			NVbDongleDeepSlpModeIn = 3; 
//		}else {
//			NVbDongleDeepSlpModeIn = 0;
//		}
//		
//		NVbRFEnDisableModeIn = 0;     // normal mode
		
//		if(NVsANTPara.iRFDongleEnDisable)  // if RF disable 
//		{
//			NVbRFEnDisableModeIn = 1;     // enable RF 
//		}else {
//			NVbRFEnDisableModeIn = 0;     // normal mode
//		}
		
/* MVP Mod end */
		
}	


