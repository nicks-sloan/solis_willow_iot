/*
 * Ant.h
 *
 *  Created on: Mar 21, 2023
 *      Author: WANGS1
 */

#ifndef ANT_H_
#define ANT_H_
#endif /* ANT_H_ */

#include "SystemInit.h"

/* Sheng MVP Mod begin */

#define TIMERWAITLIMIT	300	//300ms @1ms rate
#define MAXWAITTIME		3000 	// 3 second - 3000ms @1ms rate

//Payload
#define STARTBYTE 		0xF7 
#define ENDBYTE			0xF8

#define MULTIPDUCMDLP	8  		// 5 // Multiple command counts allow 4/11/23
#define MULTIPDUHEXCMD	0x08  	// 0x05 // Multiple command counts allow 4/11/23
#define MAXDATASIZE 	89 		// Max message length < 96 bytes 4/12/23
								// Start Bytes, Length, CMD, Datasize, checksum, end byte, /0
								// 95 - 6 bytes(start byte, Length, CMD, checksum, end byte, /0)  = 89

//MVP
//common cmd list
#define SERIALNUM		1 		// Serial Number (0x01)
#define MFGDATE			2 		// Manufacturing Date                             
#define HWREVISION		3 		// Hardware Revision                              
#define FMWREVISION		4 		// Firmware Revision                              
#define ANTSKU			5		// SKU (0x05)  
/*
// Delphian take care
#define RFSERIALNUM	    6 		// RF Dongle Serial Number        --Don't need to implement on device
#define RFMFGDATE		7 		// RF Dongle Manufacture Date     --Don't need to implement on device
#define RFHWREVISION	8 		// RF Dongle hardware version     --Don't need to implement on device
#define RFFMWREVISION	9 		// RF dongle firmware version     --Don't need to implement on device
 */
#define FACTORYRESET	10 		// Factory Reset                                  
#define SENSORRANGE		11 		// Sensor Range (0x0B) 
#define MODESELECTION	12 		// Mode Faucet: "0" = On-demand, "1" = Metered    
								// Solis "0": closet; "1": urinal; "2": urinal with ballpark

#define MRTOTIMER		13 		// Metered Run Time(faucet)/ Open Timer (flushometer) (0x0D)
#define FLUSHACT		14 		// Flush Activate (0x0E)
#define FLUSHSENTINTIME	15 		// Flush Time / Sentinel Time                     
                                // Faucet '"3"-"180" seconds; Flushometer "1"-"200" hours

#define LPMGPF			16 		// LPM (faucet) / GPF Value (flushometer) (0x10)
#define DIAGNOSISINIT	17 		// Diagnosis Init                                 
#define BLEENABLE		18 		// BLE Enable                                     
#define SENSORSTAT		19 		// Sensor Status (0x13)
#define VALVESTAT		20 		// Valve Status (0x14)
// sku determine unit have turbine or solar
#define TURBINESTAT		21 		// Turbine Status                                 
#define SOLARPANELSTAT	22 		// Solar Panel Status                             
#define BATLEVEL		23 		// Battery Level (0x17)
#define OPHRSINCEINSTALL	24 		// Operation Hours since install (0x18)
#define ACTSINCEINSTALL	25 		// Activation since install (0x19)
/*
//Handle on Argos
#define DNTLSTFACTRESET	26 		// Date & time of last factory reset
#define DNTLSTRANGECHG	27 		// Date & time of last range change
#define DNTLSTMODECHG	28 		// Date & time of last mode change
#define DNTLSTSENTINALCHG 29 	// Date & time of last Sentinel change
#define DNTLSTDIAG		30 		// Date & time of last diagnostic
*/
#define SRSTATUS		31 		// RF Dongle Status (0x1F)  //always return 3 - obsolete
#define MULTIPDU		32 		// Multiple command Value Read/write (0x20)
#define FLUSHACTREQUEST	33 		// Flush Activate request (0x21)
#define ACTRPTTHOLD		34 		// Number of Activation reporting threshold (0x22)
// #define AUTONOTIF	35 		// Auto Flush Notification Event -Dongle handle

// v7 2/25/21
//#define DONGLERFENDISABLE 36 	// RF Dongle RF Enable/Disable (0x24)   --remove 6/23/23
								// 0x31 = “1” = RF Disable. 0x30 = “0” = RF Enable.
#define SHIPMODEDEEPSLP 37 		// Ship Mode Activate/ Deactivate (deep sleep)(0x25)
								// 0x31 = “1” = Shipping Mode Activate. 
								// 0x30 = “0” = Shipping Mode deactivate.
#define OCCUPANCY 		38 		// Occupancy (0x26)
								//"0" = Occupancy sensor not supported
								//"1" = enter sensor zone
								//"2" = vacant
								//“3” = standing
								//“4” = sitting

#define FEATUREENABLE	39		// "01" = Occupancy off , "11" = occupancy on   (0x27)
								// 1st Byte = on/off; 2nd byte = feature (1 = occupancy) 

//-------------------------------

//Engineer data
// use for generic Engineer data depend on device
#define ARMTIMER		70 		// Arm Timer

/* 
 * not MVP1 but coded
#define PORRESETCNT		71 		// POR Reset Counter
#define UIRCURRATIO		72 		// Urinal IR Current Ratio
#define NOISEFLOOR		73 		// Noise Floor
#define ECHODIFFERENCE	74 		// Echo difference
#define TGTINTHOLD		75 		// Target in threshold
#define TGTOUTTHOLD		76 		// Target out threshold
#define TGTSTAYINTHOLD	77 		// Target Stay in threshold
#define TGTSTAYOUTTHOLD	78 		// Target stay out threshold
#define TGTSTAYOUTTHOLD1	79 	// Target stay out threshold1
#define TGTBACKTHOLD	80 		// Target back threshold
#define TGTSTAYTHOLD	81 		// Target stay threshold
#define BACKTOIDLETHOLD	82 		// Back to Idle threshold
*/

#define SITTINGUSRTHOLD	83 		// Sitting user threshold;											Solis = NVsOpPara.MinUserTH
#define BGCHANGETHOLD	84 		// Background change threshold; 									Solis = NVsOpPara.TouchTH
#define IRTGTTHOLD		85 		// IR Target Threshold;												Solis = NVsOpPara.MaxBackground
#define IRUPDATETHOLD	86 		// IR update threshold;												Solis = NVsOpPara.ONDelayTM

/*
 * Not MVP1 but coded 
#define IRCHANGETHOLD	87 		// IR change Threshold
*/

#define MAXIRTHOLD		88 		// other = Maximum IR Threshold; 									Solis =  NVsOpPara.MaxIRTH 

/*
 * not MVP1 but coded 
#define MINIRTHOLD		89 		// Minimum IR Threshold
#define MINBG			90 		// Minimum background
*/

#define TOFDISTCALFACTOR	91 	// Other = Time of flight distance calibration factor; 				Solis = NVsOpPara.CalibrationFlag
#define TOFOFFSETCALFACTOR	92 	// Other = Time of flight offset calibration factor;				Solis = NVsOpPara.CalibrationEcho
#define DISTADJCNT		93 		// Distance adjust counter;											Solis = NVsOpPara.ConfirmTimeTH
#define DISTADJFAILCNT	94 		// Distance adjust fail counter;									Solis = NVsOpPara.AdjudtedFailCT
#define SALKDNIRRECCAL	95 		// Sensor A (Look down) IR current from recently calibration or default;  	Solis = NVsOpPara.BVolt

/* not MVP1 but coded
#define SBLKUPIRRECCAL	96 		// Sensor B (Look up) IR current from recently calibration or default;  	Solis = NVsOpPara.ONDelayTM
*/

#define SALKDNIRMFGCAL	97 		// Sensor A (Look down) IR current from manufacture calibration or default;  	Solis = NVsOpPara.WaitDelayTM

/*
 * not MVP1 but coded
#define SALKUPIRMFGCAL	98 		// Sensor B (Look up) IR current from manufacture calibration or default
#define SALKDNMAXNLVL	99 		// Sensor A (Look down) Maximum noise level
#define SBLKUPMAXNLVL	100 		// Sensor B (Look up) Maximum noise level
 */
#define SALKDNBGCURUSED	101 		// Sensor A (Look down) background for current used;			Solis = NVsOpPara.CleanBackground

/* not MVP1 but coded
#define SBLKUPBGCURUSED	102 		// Sensor B (Look up) background for current used
#define TOTACTCNTLOWBAT	103 		// Total activation count in low battery
#define MANUALACTTIMES	104 		// Manual Activation times
*/
#define DNTLSTFMWUPDATE	105 		// Date & time of last firmware update
//--------------------------------------------------------------

//status, error code
#define ECSUCCESS		0x00 	// success
#define ECINVPARAM	 	0x01	// Invalid Parameter(s)
#define ECOPTIMEOUT		0x02 	// Operation Timout

/*
 * SR module connection status
 *
 *  bit 2 , bit 1 ->	00 = Device not connected in network
 *  					01 = Device connected with Bridge and in network
 *  					10 = Device is connected with router and in network
 *
 *  bit 0 -> 	0 = No server connectivity
 *  			1 = Server connectivity available
 *
 * Useful status 0x02 to 0x05 (bit 0010 to 0101)
 */
#define BRIDGENNETWORK	0x02 	// (0010) Device connected with bridge and in network
#define BDGNNETNSERVER  0x03	// (0011) Device connected with bridge, in network and server connectivity available
#define ROUTERNNETWORK 	0x05    // (0101) Device is connected with router and in network and server connectivity available
#define BYPASSCHKSRSTATUS 1     //by pass check SR status 4/10/23

// battery level check time
#define BATLEVELCHKTIME 3
#define BATRPTNOMFIVEPERCENT 5  // report battery change -5% -add 4/10/23
#define BATRPTLOWTWOEPERCENT 2  // report battery change -2% in warning mode -add 4/10/23
#define BATRPTPLUSNOMPERCENT 15  // report battery change +15% in normal mode -add 4/13/23
#define BATRPTPLUSLOWPERCENT 6  // report battery change +6% in warning mode -add 4/13/23

// sloan status
//#define SLNOTENABLE		0x00 	// Not enable
//#define SLENABLE	 	0x01	// Active/Enable/Okay/Init

//extern char GVbSrange[3];			//  Sensor Range 
extern uint8 GVbMultiPDUPart; 		//  Part of PDU
extern uint8 GVbMultiPDULocCnt; 	//  Store Part two array location 
extern uint8 GVbBatLevelStore; 		//  Save battery level for compare
extern uint8 GVbBatLevelChkCnt; 	//  number of battery level compare  10/28/20
extern uint8 GVbBatLowChkCnt; 		//  number of battery low compare  7/11/23
extern uint16 GVbActRptThold; 		//  store activation reporting threshold 
extern uint16 GVbActRptTholdCnt; 	//  store activation reporting threshold count number 
//extern uint8 GVbCmdListArr[15];  	// array command list
extern uint8 GVbCmdListArr[41];  	// array command list
extern uint8 GVbUartCmdStateNerrFlg;    // 0 - Unused, 1 - SR Request time out, 2 - SR Request payload exceed, 3 - SR Request invalid checksum, 
									 	// 4 - SR Request invalid cmd, 5 - Cmd multipdu received, 6 - SR Response ACK time out,
										// 7 - SR Response ACK payload exceed, 8 - SR Response Ack invalid cmd, 9 - SR Response ACK retry twice fail
										// 10 - SR Response ACK received
extern uint8 GVbUARTSensorCMDNerrFlg;   // 0 - Unused, 1 - SR Response ACK time out, 2 - SR Response ACK payload exceed, 3 - Unused 
										// 4 - SR Response ACK invalid cmd, 5 - SR Response ACK retry twice fail

extern char GVbUARTInCMDList[97];  		// store cmds for Multi PDU cmd 
//extern uint8 GVbGetSRStatFlag;			// get sr status flag  //remove 4/7/23
			 
extern uint8 GVbDIAGLEDFLG;				// Diagnosis BLink LED flag
extern uint8 GVbDIAGVALVEFLG;			// Diagnosis VALVE flag

extern uint8 GVbFactoryRestFlg; 		// Factory reset flag
extern uint8 GVbDiagSensorStat;			// Diagnosis sensor status value
extern uint8 GVbDiagValveStat;			// Diagnosis valve status value
extern uint8 GVbDiagSolarStat;			// Diagnosis Solar status value

//extern uint8 GVbCallRFEnDisable;       	// call rf enable/disable function
//extern uint8 GVbRFEnDisabState;			// RF dongle RF enable/Disable 
//extern uint8 GVbPreRFEnDisabState;		// previous RF dongle RF enable/Disable 

extern uint8 GVbOccupRoutineEnDisable;  // call occupancy enable/disable function
extern uint8 GVbOccupState;				// Occupancy State 
extern uint8 GVbPreOccupState;			// previous Occupancy state 

//extern uint8 GVbShipDeepSlpFlg;		// RF Dongle deep sleep active/deactive flag


//extern char sTempbuffer[20];
/* SR end define */

//void UARTOutStrPlusNull(char* src, int buflen);

// SR functions
uint8 batConvert16to8(void);  							// SR function
uint8 UARTProcess(void);									// SR Function
void UARTProcessSendRequest(uint8 CmdID); // SR Function	
ANTDongleParaType LoadANTParaFromFlash(void);
ANTDongleParaType LoadDefaultANTPara(void);
void SaveANTParaToFlash(ANTDongleParaType sPara);
/* Sheng MVP Mod end */
