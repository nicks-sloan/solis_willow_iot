/*
 * SystemInit.h
 *
 *  Created on: Feb 20, 2017
 *      Author: Test1
 */

#ifndef SYSTEMINIT_H_
#define SYSTEMINIT_H_

// System level Declaration
#include "PE_LDD.h"		// Include all the components
#include "IO_Map.h"
#include "Cpu.h"
#include "Events.h"

/* 
---------------------------------------------------------------------------------------------------------------------------
 	Build options for IoT:
 	Below items must be set for building defferent units
 	1.	#define WITHANT
 	2.	#define WITHBLE
 	3.	SKU:
 		Products
 										SKU											Hex File
 			Closet 2.4              #define SKU 21   							SolisIoT_SKU_30_Verxxxx				
 	 		Closet	1.6				#define SKU 1								SolisIoT_SKU_31_Verxxxx
 	 	 	Closet 1.28				#define SKU 2								SolisIoT_SKU_32_Verxxxx
 	 	 	Closet 1.1				#define SKU 3								SolisIoT_SKU_33_Verxxxx
 	 	 	Dual	1.6				#define SKU 8 and #define BUILDDUAL			SolisIoT_SKU_38_Verxxxx
 	 	 	Urinal 0.125			#define SKU 10								SolisIoT_SKU_40_Verxxxx
 	 	 	Urinal 0.25				#define SKU 11								SolisIoT_SKU_41_Verxxxx
 	 	 	Urinal 0.5				#define SKU 12								SolisIoT_SKU_42_Verxxxx
 	 	 	Urinal 1.0				#define SKU 13								SolisIoT_SKU_43_Verxxxx
-----------------------------------------------------------------------------------------------------------------------------
*/

#define WITHANT // ANT Protocol  --Sheng
#define WITHBLE // BLE protocol 

// Closets
#define SKU 1  // Closet	1.6
//#define SKU 2  // Closet	1.28
//#define SKU 3  // Closet	1.1
//#define SKU 21 // Closet	2.4

// Urinals
//#define SKU 10 // Urinal 0.125
//#define SKU 11 // Urinal 0.25
//#define SKU 12 // Urinal 0.5
//#define SKU 13 // Urinal 1.0

// Dual Flush
//#define SKU 8
//#define BUILDDUAL

//#define CONCEAL

/*----------------------------------------------------------------------------------------------------------------------------
 Software Version
 ---------------------------------------------------------------------------------------------------------------------------*/
#define VERISION_MAJOR "01"	//00	
#define VERISION_MINOR "11"	// v3 boot fix and factory reset; v4 sku remap, diagnosis implement, turbine no response; v5 final;
							// v6 short power fix 10/27/20 -complete
							// v6 battery level check 3 time before sent - complete 10/28/20

							// ---in progress 
                            // v7 ship mode(deepsleep) add  2/25/21    
							//    -sleep mode add but not use since max turn RF on 2 minute when off 2/25/21
							//    -occupancy and feature enable cmd 2/25/21
							// v8 disable occupancy, add 2.4 GPF on 06/22/2022 (Scott)
							// v104 shut off dongle RF when stop mode, remove SR status request, modify enable pin check 4/10/23
							// v106 increase multiple command from 5 to 8 - fix retry response from dongle 4/11/23
							// v107 check max payload size for multiple command 4/12/23 
							//      --change + battery send threshold
							// V110 - add disconnect to power process
							// V110 - Remove shipping flag update to flash 7/11/23
/*---------------------------------------------------------------------------------------------------------------------------
 * Debug Options:
 * CYCLETEST: for cycle solenoid test
 * OUTCURVE:	for IR multicurve
 * OUTBUTTON1:	for Button multicurve
 * OUTDATA:		for ASCii data
 * OUTCURVES:	for IR multicurve or ASCii data based on sentinel setting jumper
 * ACTIONDATA:	for Only output action data	
 * OUTHANDWAVE:	for handwave detecting
 * -------------------------------------------------------------------------------------------------------------------------- */
//#define CYCLETEST 
//#define OUTCURVE
//#define OUTBUTTON1
//#define OUTDATA
//#define OUTCURVES
//#define ACTIONDATA
//#define OUTHANDWAVE



// Memory address
#define FLASHDATAADDRESS	(0x0001FC00)	// Parameter address in Flash
#define FLASHBOARDADDRESS	(0x0001FD00)	// Board info address in flash
#define FLASHBLEADDRESS		(0x0001FE00)	// BLE info address in flash
#define FLASHBATTERYADDRESS	(0x0001FDF0)	// battery calibration info address in flash
//MVP2
#define FLASHANTADDRESS		(0x0001FF00)	// ANT data address flash -v7 2/25/21

// board info
#define BOARDINFOLEN   		22  			// include crc and flag at the end   

// A/D channels
#define SVOLTAGE	4
#define IRCURRENT	1
#define IRECHO		2	
#define BVOLTAGE	3
#define BMIDVOLTAGE	0

// Operation Mode
#define CLOSETMODE 		0
#define URINALMODE 		1
#define BALLPARKMODE 	2
#define DUALMODE 		3

// Sentinel flush
#define YESSENTINEL 1
#define NOTSENTINEL 2

// button status
#define VERIFY		1
#define	TOUCHED		2
#define NOTOUCH 	3

// button combination status
#define BUSU		1
#define BUST		2
#define BTSU		3
#define BTST		4

// power up status
#define	POWERUPDONE	1
#define	NOTDONE		2

// Battery Flags
#define OUTOFFACTORY 	0X55		// Flag of out of factory 
#define CALIBRATED		0xA0		// Flag of battery calibration

// in shipping status
#define INSHIPPING 		0X55		// Flag of solenoid on 

// Solenoid status
#define SOLENOIDON 		0X55		// Flag of solenoid on       
#define SOLENOIDOFF     0XAA		// Flag of solenoid off

// Duty Rate 
#define POWERUPDUTYRATE 1			// 4 HZ , during power up period
#define USERINDUTYRATE 	4			// 1 HZ,  when target present
#define NORMALDUTYRATE 12			// 1/3 HZ, no target detected

//Start Procedure
#define SKIPSTARTER 0xAA			// Flag to skip normal starting procedure

//Power Up Period
#define POWERUPMIN 9      			// power up period in minutes, total 10 minutes including 1 minute for start initialize

#define BALLPARKFLUSHTIME   360		// flush period (in 4 HZ) for ball park. 90 seconds
#define MAXBPFLUSH 5				// maxim flush number for ball park

#define ISBACKGROUND 0X55			// Flag of confirmed background        
#define ISUSER       0XAA			// Flag of confirmed user

#define WRITTENFLAG 0x55

//Button action allowed
#define ALLOWED   1
#define NOTALLOW  2

// Dual flush time
#define FULLFLUSHSTAY   260   		// time threshold (in 4 HZ) for a full flush

// Flush Types
#define NOFLUSH   0
#define STDFULLIR 1
#define STDLITEIR 2
#define GRDFULLIR 3
#define GRDLITEIR 4
#define STDFULLBT 5
#define STDLITEBT 6
#define GRDFULLBT 7
#define GRDLITEBT 8
#define SENTINALACT	9

// calibration flag
#define CURRENTCAL	0xA0				// Flag of current calibrated
#define BACKGRONDCAL	0x05			// flag of background calibrated

// Low battery actions
#define MAXLOWBACT  2000

// BLE Time out
#define BLETIMEOUT 2400

// Parameters to be communicated with remote service monitor
#pragma pack(push,1)

typedef struct Solis{
	uint8 CalibrationFlag;				// index 0,				including both factory and field calibration
	uint16 IRLevel;						// index 1,2,			IR driver DAC	
	uint8 VerMajor;						// index 3,				Software version Major
	uint8 VerMinor;						// index 4,				Software version Minor
	uint8 BuildM;						// index 5,				Software build month
	uint8 BuildD;						// index 6,				Software build day
	uint16 BuildY;						// index 7,8,			Software build year
	uint16 PinResetCT;					// index 9,10,			Count of external Pin reset
	uint16 PORResetCT;					// index 11,12,			Count of POR reset
	uint32 TotalActivation;				// index 13,14,15,16,	Count of total activations
	uint8 RSecond;						// index 17,			Time of unit has run seconds
	uint8 RMinute;						// index 18,			Time of unit has run minutes
	uint8 RHour;						// index 19,			Time of unit has run hours
	uint16 RDay;						// index 20,21,			Time of unit has run days
	uint8 RYear;						// index 22,			Time of unit has run years
	uint16 BVolt;						// index 23,24, 		Battery voltage
	uint16 LBActivationCT;				// index 25,26,			Count of activations after battery in warning
	uint16 MaxBackground;				// index 27,28,			Maxim background
	uint16 ResetCause;					// index 29,30,			Cause of reset
	uint16 CalibrationEcho;				// index 31,32,			Echo at factory calibration
	uint8 Mode;							// index 33,			Operation mode
	uint8 ArmTM;						// index 34,			Arm time in second
	uint8 OpenTM;						// index 35,			Open time in 100 ms
	uint8 ONDelayTM;					// index 36,			On delay time in second
	uint8 WaitDelayTM;					// index 37,			Wait delay time in second
	uint8 SentinalTM;					// index 38,			Sentinel time in hour
	uint16 BadSolisDay;					// index 39,40,			number of days when solis is bad
	uint8 UrinalIR;						// index 41,			percentage of urial vs closet for IR driver level
	uint16 UserStepInTH;				// index 42,43,			Threshold of user get in
	uint16 MinUserTH;					// index 44,45,			Threshold of user on clean background environment
	uint16 TargetStableRange;			// index 46,47			Range limit for stable target
	uint16 StableTimeTH;				// index 48,49			Time limit for stable target in 1/4 second
	uint16 ConfirmTimeTH;				// index 50,51			Time threshold for confirmed background in 1/4 second
	uint16 CleanBackground;				// index 52,53			Clean background
	uint16 SittingTH;					// index 54,55			Threshold for sit down
	uint16 IRCalibrationTH;				// index 56,57,			Distance calibration target
	uint16 TouchTH;						// index 58,59			Button touch threshold
	uint16 MaxIRTH;						// index 60,61,			Maxim IR drive level
	uint16 MinIRTH;						// index 62,63,			Minimum IR drive level
	uint16 ResetCT;						// index 64,65			Count of total reset
	uint8 BType;						// index 66,			Battery type
	uint8 FlushVolumeM;					// index 67,			Flag of grand flush
	uint8 DistanceAdjustedCT;			// index 68,			Count of field calibrated
	uint8 AdjudtedFailCT;				// index 69,			Count of failed field calibration
	uint16 ConfirmedBackground;			// index 70,71,			Confirmed background
	uint32 ButtonActivations;			// index 72,73,74,75,	Count of button activations 
	uint8 UpdateM;						// index 76,			Software update month
	uint8 UpdateD;						// index 77,			Software update day
	uint16 UpdateY;						// index 78,79,			Software update year
} SolisBLEParaType;

typedef struct SolisBLE{
	char sBoardSN[11];					// ID 1,	serial Number
	char sBoardDATE[7];					// ID 2,	build data
	char sBoardREV[5];					// ID 3,	Board revision
//	char sFirmwareREV[5];				// ID 4,	Firmware revision
	char sBoardSKU[3];					// ID 5,	SKU
	uint8 bSKUIndex;					// SKU Index 
	char sSensorRange[2];				// ID 7, 	Sensor Range
	char sGPF[6];						// ID 10,	Flush Volum
//	char sEngineeringData1[41];			// ID 17,   Enginerring Data1
//	char sEngineeringData2[21];			// ID 18,   Enginerring Data2
	uint32 SentinalActivation;			// ID 21,   sentinal flush count
	uint32 ReduceFlushActivation;		// ID 23,   Reduced fluah count
} BLEDongleParaType;

typedef struct BLERead{
	uint16 iEngReadOnly[5];
	uint8 bEngReadOnly[10];	
} BLEReadOnlyType;

typedef struct BLERW{
	uint16 iEngReadWrite[3];
	uint8 bEngReadWrite[4];	
} BLEReadWriteType;

typedef struct SolisANT{
	uint16 iActivationRptTh;			// ID 1,2,	Activation reporting Threshold
	uint8 iShipModeDeepSleep;			// ID 3,	Deep sleep shipping mode
	uint8 iFeatureOccupEnDisable;		// ID 4,	Feature enable/Disable Occupancy 
	uint8 iRFDongleEnDisable;		    // ID 5,	flag RF dongle status enable/Disable, 1= RF is disable, 0 = RF is enable 3/31/23
} ANTDongleParaType;

typedef struct BATCalibration{
	int16 wOff1;
	int16 wOff2;
	uint8 bCalFlag;
	uint8 bInFactoryFlag;
} BatCalType;
#pragma pack(pop)


// Solenoid turn on time
typedef struct SolenoidOn{
	uint8 bStandardFull;				// valve on time for standard full flush
	uint8 bStandardLite;				// valve on time for standard lite flush
	uint8 bGrandFull;					// valve on time for grand full flush
	uint8 bGrandLite;					// valve on time for grand lite flush
} SolenoidOnTimeType;

// sentinel time
typedef struct Sentinel{
	uint8 bSentinelHour;
	uint16 wSentinelSecond;
} SentinelTimeType;

// Power Threshold
typedef struct PowerTH{
	uint16 wLowVolt;					// Battery low voltage threshold
	uint16 wStopVolt;					// Battery stop voltage threshold
	uint16 wWakeupVolt;					// Wake up voltage threshold
} PowerTHType;

// Button status
typedef struct Buttonsts{
	uint8 bBigButton;					// Big button status
	uint8 bSmallButton;					// Small button status
	uint8 bCombination;					// buttons combination status	
} ButtonStsType;

// Operation status 
typedef enum {
	NORMAL,								// standard normal operation. unit is in this mode most of the time
	COMMUNICATION,						// uart is avalabe

/* Sheng MVP Mod begin */	
	ANT,								// ANT is active
	UPDATEANT,							// Update ANT is active
/* Sheng MVP Mod end */
	
	BLE,								// BLE is active
	CALIBRATION,						// During IR calibration
	TESTING,							// Test mode after 'T' command from UART. unit accept ASCii command
	SETTING,							// The mode working with service monitor
	SHIPPING,							// In shipping
	STOPOP								// No regular operation, battery is below operation range.
} OperationStsType;

// Target Status
typedef enum {
	NOTPRESENT = 0,
	ENTER = 1,
	NONEARMED = 2,
	ARMED = 3
} UserStsType;

// battery status
typedef enum {
	BNORMAL = 4,
	BWARNING = 3,
	BSTOP = 2,
	BDISCONNECT = 1
} BatteryStsType;

// Global variables
extern SolisBLEParaType NVsOpPara;    		// Operating parameters
extern uint8 NVbPowerupSts;					// Power up status 
extern uint8 NVbCheckBatteryType;			// Flag to check battery type at reset
extern uint8 NVbSkipReset;					// Flag to skip the reset process
extern uint8 NVbInShippingSts;				// Flag of shipping status
extern uint8 NVbSolenoidSts;				// Solenoid state

extern SolenoidOnTimeType GVsTurnOnTM;		// Solenoid turn on time
extern SolisBLEParaType GVsTempPara;    		// Temporary used for real parameter transition
extern OperationStsType GVeOperationSts;	// Operation status 
extern UserStsType GVeUserSts;				// Target status
extern BatteryStsType GVeBatterySts;		// Battery status
extern PowerTHType GVsBatteryTH;			// Battery threshold
extern SentinelTimeType GVsSentinelTime;	// Sentinel timer
extern ButtonStsType GVsButtonSts;			// Button status
extern uint8 GVbSentinelFlush;				// Sentinel flag
extern uint8 GVbFlushRequest;				// type of flush requested
extern uint8 GVbDutyRate;					// IR scan duty rate
extern uint8 GVbBDisconCT;					// count of battery in disconnection
extern uint8 GVbWakeBLE;					// flag to wake BLE
extern uint8 GVbBLEEnabled;					// Flag of BLE donggle enable
extern uint8 GVbInDiag;						// Flag of unit in diagnostics state
extern uint16 GVwBLENoActionTimer;			// Timer to tracking no communication from BLE
extern uint8 GVwBLEDone;					// flag of BLE done
extern BLEDongleParaType NVsBLEPara;		// Operation parameters
extern uint8 NVbBLEDongleIn;				// flag of dongle installed
extern uint16 NVwHandWaveIR;				// IR level for detecting handwave


extern uint16 debugStartReason; 			// sheng debug start reason
extern BatCalType NVsOffset;				// Battery calibration Data
extern uint8 debugdata;
extern uint16 wBattest[20];
/* Sheng MVP Mod begin */
//SR UART Variables
extern uint8 GVbFlushActivate;				// Flag, Activate flush (Sheng)
extern uint8 GVbSRModuleStatus; 			// SR Module status list
extern uint8 GVbSRRefreshCmd; 				// SR refresh command 69 (0x45)
extern uint8 GVbStorePreviousCmd; 			// Store previous command
extern uint8 GVbUpdateANT;					// flag to update ANT
//extern uint8 GVbWakeANT;					// flag to wake ANT
//extern uint8 GVbANTEnabled;					// Flag of ANT donggle enable
extern uint8 GVbBattRequest;				// Battery level change flag request
extern uint8 GVbUartFlushRequest;			// Flush flag request
extern uint8 GVbStoreFlushRequest;			// store Flush flag request

extern uint8 NVbRFEnDisableModeIn;			// RF dongle RF enable/Disable mode  3/31/23 
											// 0 =  in normal mode,  1 = enable RF, 2 =  processed enable RF, 3 = Disable RF, 4 = processed disable RF 
extern uint8 NVbDongleDeepSlpModeIn;		// RF Dongle deep sleep active/deactive, mode in
											// 0 = in normal mode, 1 = go in deep sleep mode, 2 = in deep sleep mode; 3 = go out deep sleep mode

//extern uint8 NVbFeatureEnable;				// 0 = disable, 1= enable
//extern uint8 NVbOccupancyStaus;				// 0 = Occupancy not supported, 1= enter, 2= vacant (3 = standing, 4 = sitting)= not supported in v7

extern ANTDongleParaType NVsANTPara;   		// ANT parameters
// SR End 
/* Sheng MVP Mod end */

// Public functions
void SystemInit(void);
SolisBLEParaType LoadParaWithDefault(void);
SolisBLEParaType LoadParaFromFlash(void);
void SaveParaToFlash(SolisBLEParaType sPara);
void DisableUART(void);
void ConnectUART(void);
uint8 CalculateCRC(uint8* pBlock, uint16 len);

void DisableLATCHCurrentSensing(void);
void EnableLATCHCurrentSensing(void);
void SetSensingRange(char* sRange);
uint8 CheckSentinelSetting(void);

BatCalType LoadBatteryCalFromFlash(void);
void SaveBatCalParaToFlash(BatCalType sPara);

#endif /* SYSTEMINIT_H_ */
