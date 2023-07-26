/*
 * URAT.c
 *
 *  Created on: Feb 17, 2017
 *      Author: Scott Wang
 *      This module contains communication through UART0 port
 */

#include "UART.h"
#include "Operation.h"
#include "boot.h"
#include "PowerSupply.h"
#include "IRSensor.h"
#include "TouchSensor.h"
#include "Timing.h"
#include "TI1.h"
#include "BLE.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Ant.h"

/*
** =================================================================================================================================
**     Method      :  uint8 NibbleByteToHexAscii(uint8 hex)
**     Description :
**         This method converts  low nibble of input to a Hex ASCII Char.
**     Parameters  :  byte to convert
**     Returns     : Converted Hex ASCII Char
** =================================================================================================================================
*/
uint8 NibbleByteToHexAscii(uint8 hex)
{
  hex &= 0x0F;
  return hex + ((hex <= 9) ? '0' : ('A'-10));
}

/*
** =================================================================================================================================
**     Method      :  ConvertByteToHexStr(uint8 bByte, uint8* pHexStr)
**     Description :
**         This method convert one byte number toHex ASCii chars (two chars).
**     Parameters  : The number to be converted.
**     				: pointer to converted char
**     Returns     : Nothing
** =================================================================================================================================
*/
void ConvertByteToHexStr(uint8 bByte, char* pHexStr) 
{

	*pHexStr = 	NibbleByteToHexAscii(bByte >> 4); 	// high nibble
	*(pHexStr + 1) = NibbleByteToHexAscii(bByte); 	// low nibble
	*(pHexStr + 2) = '\0';                          // terminator string
	
}

/*
** =================================================================================================================================
**     Method      :  Convert16bitsToHexStr(uint16 iData, uint8* pHexStr) 
**     Description :
**         This method convert 2 byte number to Hex ASCii string (four chars).
**     Parameters  : The number to be converted.
**     				: pointer to converted char
**     Returns     : Nothing
** =================================================================================================================================
*/
void Convert16bitsToHexStr(uint16 iData, char* pHexStr) 
{
	uint8 bByte[2];
	char* pChar;
	
		bByte[1] =	(uint8) (iData);		// low byte
		bByte[0] =	(uint8) (iData >> 8);	// high byte	
		pChar = pHexStr;					// pointer to string
		ConvertByteToHexStr(bByte[0], pChar);
		pChar = pHexStr + 2;                 // next byre
		ConvertByteToHexStr(bByte[1], pChar);
}


/*
** =================================================================================================================================
**     Method      :  Convert32bitsToHexStr(uint32 lData, uint8* pHexStr) 
**     Description :
**         This method convert 4 byte number to Hex ASCii string (8 chars).
**     Parameters  : The number to be converted.
**     				: pointer to converted char
**     Returns     : Nothing
** =================================================================================================================================
*/
void Convert32bitsToHexStr(uint32 lData, char* pHexStr) 
{

	uint16 iData[2];
	char* pChar;
		
		iData[1] =	(uint16) (lData);		// lower two bytes
		iData[0] =	(uint16) (lData >> 16);	// high two bytes	
		pChar = pHexStr;					// pointer to string
		Convert16bitsToHexStr(iData[0], pChar);
		pChar = pHexStr + 4;                // next two bytes
		Convert16bitsToHexStr(iData[1], pChar);
	
}


/*
** =================================================================================================================================
**     Method      :  ConvertBlockToHexStr(uint8* pBlock, uint8 len, uint8* pHexStr) 
**     Description :
**         This method converts block bytes to hexascii str
**     Parameters  : pointer of data block
**     				 the number of bytes
**     				 point to hexascii string
**     				 
**     Returns     : Nothing
** =================================================================================================================================
*/
void ConvertBlockToHexStr(uint8* pBlock, uint8 len, char* pHexStr) 
{
	uint8 lp;
	char * pTemp;

		pTemp = pHexStr;
		for(lp=0;lp < len;lp++){
			ConvertByteToHexStr(*(pBlock + lp), pTemp);
			pTemp += 2;    		// two byte hax ascii
		}
		*(pTemp) = '\0';  		// terminator
	
}

/*
** =================================================================================================================================
**     Method      :  uint8 HexCharToNum(char HexChar, uint8* bNum) 
**     Description :
**         This method convert a hexadecimal character into a decimal value. Note this is half byte
**     Parameters  :  Hex Ascii char, pointer to data
**     Returns     : Concert result
** =================================================================================================================================
*/
uint8 HexCharToNum(char HexChar, uint8* bNum) {

	uint8 bResult;

		bResult = FALSE;

		if (HexChar >='0' && HexChar <='9') {
			*bNum = (unsigned char)(HexChar -'0');
			bResult = TRUE;
		} else if (HexChar>='a' && HexChar <= 'f') {
			*bNum = (unsigned char)(HexChar-'a'+10);
			bResult = TRUE;
		} else if (HexChar>='A' && HexChar<='F') {
			*bNum = (unsigned char)(HexChar-'A'+10);
			bResult = TRUE;
		}
  return bResult;
}

/*
** =================================================================================================================================
**     Method      :  uint8 HexByteToNum(char* HexByte, uint8* bNum) {
**     Description :
**         This method convert a hexadecimal byte (two chars) into a decimal value
**     Parameters  :  pointer to first hex char (there should be 2 chars for a byte)
**     Returns     : Converte result
** =================================================================================================================================
*/
uint8 HexByteToNum(char* HexByte, uint8* bNum) {

	uint8 bResult;
	uint8 bTemp;
	char HexChar;
	
		bResult = FALSE;
		HexChar = *HexByte;	// high nibble
		if(HexCharToNum(HexChar, &bTemp)){
			*bNum = bTemp << 4;
			HexChar = *(HexByte + 1);	// low nibble
			if(HexCharToNum(HexChar, &bTemp)){
				*bNum += bTemp;
				bResult = TRUE;		
			}	
		}	
		return bResult;
}

/*
** =================================================================================================================================
**     Method      :  uint8 HexStrToBlock(char* HexStr, uint8* bDataBlock, uint16 len) {
**     Description :
**         This method convert array of hex Ascii string to a block out decimal data.
**     Parameters  :  pointer to hex string, pointer to data block, length of data
**     Returns     : Converte result
** =================================================================================================================================
*/
uint8 HexStrToBlock(char* HexStr, uint8* bDataBlock, uint16 len) {

	uint8 bResult;
	uint8 lp;
	
		bResult = FALSE;
		
		for (lp=0;lp<len; lp++){	
			if(HexByteToNum(HexStr, bDataBlock)){
				HexStr += 2;		// two chars for a byte
				bDataBlock += 1;	// next byte of the data
				bResult = TRUE;	
			}
			else{
				break;
			}
			bResult = TRUE;	
		}
  return bResult;
}

/*
** =================================================================================================================================
**     Method      :  UARTPutCRLF(void)
**     Description :
**         This method sends "\r\n".
**     Parameters  : Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTPutCRLF(void){
	
	uint16 sd;
	
		AS1_ClearTxBuf();  
		SendComplete = 0;
		AS1_SendBlock((byte*)"\r\n",2, &sd);
		
		while(SendComplete == 0); 
		SendComplete = 0;
   	
		//while(AS1_GetCharsInTxBuf() != 0);
}

/*
** =================================================================================================================================
**     Method      :  void UARTOutMultiCurveData(uint16 d1,uint16 d2,uint16 d3)
**     Description :
**         This method sends out MultiCurve formated data for displaying on MultiCurve software.
**     Parameters  : 3 uint16 data
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutMultiCurveData(uint16 d1,uint16 d2,uint16 d3){
	
	uint8 out[2];
	uint16 buffer;
	
		AS1_ClearTxBuf();  
		SendComplete = 0;
		AS1_SendBlock((byte*)"\r\n",2, &buffer);
		//first data
		out[0]= d1 >> 8;  			// high byte first
		out[1] = d1 & 0xff; 		// low byte
		AS1_SendBlock((byte*)out,2, &buffer);
		//second data
		out[0]= d2 >> 8;  			// high byte first
		out[1] = d2 & 0xff; 		// low byte
		AS1_SendBlock((uint8 *)out, 2, &buffer);
		//3rd data
		out[0]= d3 >> 8;  			// high byte first
		out[1] = d3 & 0xff; 		// low byte
		AS1_SendBlock((uint8 *)out,2, &buffer);
		// data termination		
		AS1_SendChar(0x00);
		AS1_SendChar(0xFf);
		AS1_SendChar(0xFF);
		
		while(SendComplete == 0); 	// Waiting for transmit completion
		SendComplete = 0;			// Clear flag for next

		//while(AS1_GetCharsInTxBuf() != 0);
}

/*
** =================================================================================================================================
**     Method      :  void UARTOutASCIIData(uint16 d)
**     Description :
**         This method sends out data in decimal ASCII.
**     Parameters  : data to be sent
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutASCIIData(uint16 d){
	
	uint8 ASCStr[5];	// Maximum 5 digits for uint16
	uint8 pl;
	uint16 buffer; 
	
		AS1_ClearTxBuf();
		SendComplete = 0;
		UARTPutCRLF();
		for(pl=0;pl<5;pl++){
			ASCStr[pl]=0;
		}
		UTIL1_Num16uToStr(ASCStr,5,d);    // Covert data to decimal string
		AS1_SendBlock(ASCStr,5, &buffer);
		
		while(SendComplete==0); 
		SendComplete = 0;
		
		//while(AS1_GetCharsInTxBuf() != 0);
}

/*
** =================================================================================================================================
**     Method      :UARTOutStr(uint8* src)
**     Description :
**         This method sends out input string.
**     Parameters  : point to the string
**     				: length of string
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutStr(char* src){
	uint16 len;	// length of string
	uint16 sd; 	//number char sent
		WDReset();								// Kick dog
		len = strlen(src);
		AS1_ClearTxBuf();	
		AS1_SendBlock((byte*)src, len, &sd);
		
		while(SendComplete == 0);				// Waiting for Tx complete
		SendComplete = 0;
		
		//while(AS1_GetCharsInTxBuf() != 0);
		AS1_ClearTxBuf();
		AS1_ClearRxBuf();	
}

/*
** =================================================================================================================================
**     Method      : UARTOutSoftwareVersion(void
**     Description :
**         This method sends out software version.
**     Parameters  : Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutSoftwareVersion(void){
	
		UARTOutStr("\r\nVersion: ");
		UARTOutStr(VERISION_MAJOR);
		AS1_SendChar('.'); //Separator
		UARTOutStr(VERISION_MINOR);
		UARTOutStr("\r\n");
		AS1_ClearTxBuf();
		AS1_ClearRxBuf();
	
}

/*
** =================================================================================================================================
**     Method      :UARTPromptWrongInput(void)
**     Description :
**         Prompt "\r\n???\r\n "
**     Parameters  : Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTPromptWrongInput(void){
	
	UARTOutStr("\r\n ???\r\n");
		
}

/*
** =================================================================================================================================
**     Method      : UARTOutEnterSettingModePrompt
**     Description :
**         Prompt "\r\nEntering Service Mode: "
**     Parameters  : Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutEnterSettingModePrompt(void){
	
	UARTOutStr("\r\nEntering Service Mode: ");
}

/*
** =================================================================================================================================
**     Method      : UARTOutEnterTestingModePrompt(void)
**     Description :
**         Prompt "\r\nEntering Service Mode: "
**     Parameters  : Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutEnterTestingModePrompt(void){

	UARTOutStr("\r\nEntering Test Mode:\r\n ");
	
}

/*
** =================================================================================================================================
**     Method      :  UARTOutHexByte(uint8 OutByte) 
**     Description :
**         This method sends out one byte in Hex ASCii format.
**     Parameters  :	Byte
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutHexByte(uint8 OutByte) 
{
	
	uint16 sd; //number char sent
	uint8 hexString[2];

		AS1_ClearTxBuf();
		SendComplete = 0;
		WDReset();											// Kick dog
		hexString[1] = NibbleByteToHexAscii(OutByte);  		// low nibble
		hexString[0] = NibbleByteToHexAscii(OutByte>>4);	// high nibble
		AS1_SendBlock(hexString, 2, &sd);					// send out one byte
		while(SendComplete == 0);							// waiting for Tx complete    
		AS1_ClearTxBuf();
		SendComplete = 0;
  
		//while(AS1_GetCharsInTxBuf() != 0);
}

/*
** =================================================================================================================================
**     Method      :  UARTOutHexInt(uint16 OutInt) 
**     Description :
**         This method sends out one int in Hex ASCii format. High byte first
**     Parameters  :	int number
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutHexInt(uint16 OutInt) 
{
	
	uint8 LowByte, HighByte;
	
		LowByte = (uint8) OutInt;			// low byte
		HighByte = (uint8) (OutInt >> 8);	// high byte
		
		UARTOutHexByte(HighByte);			// send high byte first
		UARTOutHexByte(LowByte);  
}

/*
** =========================================================================================================================
**     Method      :DisplayFlashData(void)
**     Description :
**         This method send out operation parameters in flash
**     Parameters  : Nothing
**     Returns     : Nothing
** =========================================================================================================================
*/
void DisplayFlashData(void){
	
	uint8 sParaData[83];
	uint8 sBoardData[23];
	uint8 sBLEParaData[51];

		IFsh1_EnableEvent();																			// Enable flash process events
		while(IFsh1_GetBlockFlash(FLASHDATAADDRESS, (IFsh1_TDataAddress) sParaData, 82)!= ERR_OK); 		// wait until load parameters from flash done
		sParaData[82] = '\0';
		while(IFsh1_GetBlockFlash(FLASHBOARDADDRESS,sBoardData, 22)!= ERR_OK); 					// get stored CRC.
		sBoardData[22] = '\0';
		while(IFsh1_GetBlockFlash(FLASHBLEADDRESS,sBLEParaData, 50)!= ERR_OK); 				// get written flag.
		sBLEParaData[51] = '\0';
		IFsh1_DisableEvent();// Flash process done.
		UARTOutStr("\r\n");
		UARTOutHexASCIIBlock(sParaData,82);
		UARTOutStr("\r\n");
		UARTOutHexASCIIBlock(sBoardData,22);
		UARTOutStr("\r\n");
		UARTOutHexASCIIBlock(sBLEParaData,50);
		
}

/*
** =========================================================================================================================
**     Method      :void DisplayNonBSSData(void)
**     Description :
**         This method send out NonBSS data for debuging
**     Parameters  : Nothing
**     Returns     : Nothing
**     
// variables must stay for partial start, these are only initialized at full start
__attribute__ ((section (".NonVolatileData"))) SolisBLEParaType NVsOpPara;				// General Operation parameters
__attribute__ ((section (".NonVolatileData"))) uint8 NVbCheckBatteryType;				// Flag to check battery type at reset
__attribute__ ((section (".NonVolatileData"))) uint8 NVbSkipReset;						// Flag to skip full start
__attribute__ ((section (".NonVolatileData"))) uint8 NVbPowerupSts;						// Power up status 
__attribute__ ((section (".NonVolatileData"))) uint8 NVbInShippingSts;					// Flag of shipping status
__attribute__ ((section (".NonVolatileData"))) BLEDongleParaType NVsBLEPara;			// BLE related Operation parameters
__attribute__ ((section (".NonVolatileData"))) BLEReadOnlyType NVsBLEEngDataROnly;		// ReadOnly Enginerring data
__attribute__ ((section (".NonVolatileData"))) BLEReadWriteType NVsBLEEngDataRWrite;	// ReadWrite Engineering data
** =========================================================================================================================
*/
void DisplayNonBSSData(void){
		UARTOutStr("\r\n");
		UARTOutHexASCIIBlock((uint8*) &NVsOpPara,80);
		UARTOutStr("\r\n");
		UARTOutHexByte(NVbCheckBatteryType);
		UARTOutStr(", ");
		UARTOutHexByte(NVbSkipReset);
		UARTOutStr(", ");
		UARTOutHexByte(NVbPowerupSts);
		UARTOutStr(", ");
		UARTOutHexByte(NVbInShippingSts);
		UARTOutStr("\r\n");
		UARTOutHexASCIIBlock((uint8*) &NVsBLEPara,43);
		UARTOutStr("\r\n");

}

/*
** =========================================================================================================================
**     Method      :void DisplayBSSData(void)
**     Description :
**         This method sends out BSS data for debuging
**     Parameters  : Nothing
**     Returns     : Nothing
**     
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
uint8 GVbSolenoidSts;					// Solenoid state

uint8 GVbHandWave;						// flag of handwave
uint8 GVbHandWaveINT;					// flag of hand move away
uint8 GVbHandWaveCT;					// Count of handwave
uint8 GVbWakeBLE;						// flag to wake BLE
uint8 GVbInterv;						// count for interval time between handwave
uint8 GVbBLEEnabled;					// Flag of BLE donggle enable
uint8 GVbInDiag;						// Flag of unit in diagnostics state
uint16 GVwBLENoActionTimer;				// Timer to tracking no communication from BLE
uint8 GVbSensingRange;					// sensor sensing Range
uint8 GVwBLEDone;						// flag of BLE done

** =========================================================================================================================
*/
void DisplayBSSData(void){
		UARTOutStr("\r\n");
		UARTOutStr("GVeOperationSts: ");
		UARTOutHexByte(GVeOperationSts);
		UARTOutStr("\r\n");
		
		UARTOutStr("GVbWakeBLE: ");
		UARTOutHexByte(GVbWakeBLE);
		UARTOutStr("\r\n");
		
		UARTOutStr("GVbBLEEnabled: ");
		UARTOutHexByte(GVbBLEEnabled);
		UARTOutStr("\r\n");
		
		UARTOutStr("GVbSentinelFlush: ");
		UARTOutHexByte(GVbSentinelFlush);
		UARTOutStr("\r\n");		
}
/*
** =========================================================================================================================
**     Method      :void DisplayBoardData(void)
**     Description :
**         This method send out boardinfo data
**     Parameters  : Nothing
**     Returns     : Nothing
** =========================================================================================================================
*/
void DisplayBoardData(void){
	

	uint8 sBoardData[23];
	
		IFsh1_EnableEvent();																	// Enable flash process events
		while(IFsh1_GetBlockFlash(FLASHBOARDADDRESS,sBoardData, 22)!= ERR_OK); 					// get stored CRC.
		sBoardData[22] = '\0';
		IFsh1_DisableEvent();// Flash process done.
		
		UARTOutStr("\r\n");
		sBoardData[20] = '\0';		// terminate the string
		UARTOutStr((char*)sBoardData);
		UARTOutStr("\r\n");
		
}

/*
** =================================================================================================================================
**     Method      :  void UARTOutDebugInfor(void)
**     Description :
**         This method sends out debug information.
**     Parameters  :	Nothing
**     Returns     : 	Nothing
** =================================================================================================================================
*/
void UARTOutDebugInfor(void)
{
	uint8 i;
	//uint8 bData[7];
	uint16 iData[14];
	BackgroundRecordType sTemp;
	
				//bData[0] = GVeOperationSts;
				//bData[0] = NVbCheckBatteryType;
				//bData[1] = GVbBDisconCT;
				//bData[3] = GVsButtonSts.bBigButton;
				//bData[4] = GVeBatterySts;
				//bData[5] = NVbPowerupSts;
				//bData[6] = GVbDutyRate;
		/*
				bData[0] = GetTargetRange();
				bData[1] = GVeUserSts;
				bData[2] = GetUserRange();
				bData[3] = IsStableTG();
				bData[4] = GetMGVbConfirmedBack();
				bData[5] = GetMGVbConfirmedNB();
				for(i = 0; i < 6; i++){
					UARTOutHexByte(bData[i]);
					UARTOutStr(", ");
				}
				UARTOutStr("\r\n");
				*/
				
				/*
				iData[0] = GVwIRReceiverOffset;
				iData[1] = GVwIRBackground;
				iData[2] = GVwIRInstant;	
				//iData[3] = GVwBigButtonBaseline;
				//iData[4] = GVwBigCapInstant;
				iData[3] = GVwNoiseLevel;
				iData[4] = NVsOpPara.NorseFloor;
				iData[5] = NVsOpPara.IRLevel;
				*/
				/*
				iData[0] = GetIRRecOffset();
				iData[1] = GetNoiseLevel();
				iData[2] = GetStableBackground();
				iData[3] = GetBackground();
				iData[4] = GetInstant();
				*/
				//iData[4] = GetNoiseLevel();
				//iData[5] = NVsOpPara.IRLevel;
				sTemp = GetStableBackRecord(0);
				iData[0] = sTemp.wStableValue;
				iData[1] = sTemp.bRepeatCT;
				sTemp = GetStableBackRecord(1);
				iData[2] = sTemp.wStableValue;
				iData[3] = sTemp.bRepeatCT;
				sTemp = GetStableBackRecord(2);
				iData[4] = sTemp.wStableValue;
				iData[5] = sTemp.bRepeatCT;
				sTemp = GetStableBackRecord(3);
				iData[6] = sTemp.wStableValue;
				iData[7] = sTemp.bRepeatCT;
				//iData[7] = GetBigButtonBaselineValue();
				//iData[8] = GetBigButtonCapInstantValue();
				iData[8] = GetInstant();
				iData[9] = GetStableValue();
				iData[10] = GetMaxDiff();
				iData[11] = NVsOpPara.ConfirmTimeTH;
				iData[12] = GetStableTime();
				for(i = 0; i < 13; i++){
					UARTOutHexInt(iData[i]);
					UARTOutStr(", ");
				}
				UARTOutStr("\r\n");
				AS1_ClearTxBuf();
				AS1_ClearRxBuf();		
}


/*
** =================================================================================================================================
**     Method      :  void UARTOutDebugData(void)
**     Description :
**         This method sends out all status in hex for debug.
**     Parameters  :Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutDebugDataOn(void)
{
	
	UARTOutStr("\r\nButton Full Flush\r\n");
	UARTOutStr("Start From Reset:  ");
	UARTOutHexInt(StartReason);
	UARTOutStr("\r\nSkip Start: ");
	UARTOutHexByte(debugdata);
			//DisplayFlashData();
			//DisplayNonBSSData();
	DisplayBSSData();
	
	switch(GVbFlushRequest){
		case STDFULLIR: // IR sensing Flush
			UARTOutStr("\r\nIR Sensor Full Flush\r\n\r\n");
		break;
		case STDLITEIR: // IR sensing Flush
			UARTOutStr("\r\nIR Sensor Lite Flush\r\n\r\n");
		break;
		case STDFULLBT:	// Button Flush
			UARTOutStr("\r\nButton Full Flush\r\n");
			UARTOutStr("Start From Reset:  ");
			UARTOutHexInt(StartReason);
			UARTOutStr("\r\nSkip Start: ");
			UARTOutHexByte(debugdata);
			//DisplayFlashData();
			//DisplayNonBSSData();
			DisplayBSSData();
		break;
		case STDLITEBT:	// Button Flush
			UARTOutStr("\r\nButton Lite Flush\r\n\r\n");
		break;
		case GRDFULLIR: // IR sensing Flush
			UARTOutStr("\r\nIR Sensor Grand Full Flush\r\n\r\n");
		break;
		case GRDLITEIR: // IR sensing Flush
			UARTOutStr("\r\nIR Sensor Grand Lite Flush\r\n\r\n");
		break;
		case GRDFULLBT:	// Button Flush
			UARTOutStr("\r\nButton Grand Full Flush\r\n\r\n");
		break;
		case GRDLITEBT:	// Button Flush
			UARTOutStr("\r\nButton Grand Lite Flush\r\n\r\n");
		break;
		case SENTINALACT: // Sentinal Flush
			UARTOutStr("\r\nSentinal Flush\r\n\r\n");
		break;
		default:	 
		break;	
	}
}
	

/*
** =================================================================================================================================
**     Method      :  void UARTOutDebugData(void)
**     Description :
**         This method sends out all status in hex for debug.
**     Parameters  :Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutDebugData(void)
{
	uint8 i;
	uint8 bData[10];
	uint16 iData[9];
	static uint8 bResetInfo = 0; 
	static uint8 bTick = 0;
	static uint8 bNewInfo = 0;
	//BackgroundRecordType sTemp;
	
	if(bResetInfo == 0){
		UARTOutStr("\r\nStart From Reset:  ");
		UARTOutHexInt(StartReason);
		UARTOutStr("\r\n");
	//	UARTOutDebugInfor();
		DisplayFlashData();
		
		bResetInfo = 1;
	}

	switch(GVbFlushRequest){
		case STDFULLIR: // IR sensing Flush
			UARTOutStr("\r\nIR Sensor Full Flush\r\n\r\n");
		break;
		case STDLITEIR: // IR sensing Flush
			UARTOutStr("\r\nIR Sensor Lite Flush\r\n\r\n");
		break;
		case STDFULLBT:	// Button Flush
			UARTOutStr("\r\nButton Full Flush\r\n\r\n");
		break;
		case STDLITEBT:	// Button Flush
			UARTOutStr("\r\nButton Lite Flush\r\n\r\n");
		break;
		case GRDFULLIR: // IR sensing Flush
			UARTOutStr("\r\nIR Sensor Grand Full Flush\r\n\r\n");
		break;
		case GRDLITEIR: // IR sensing Flush
			UARTOutStr("\r\nIR Sensor Grand Lite Flush\r\n\r\n");
		break;
		case GRDFULLBT:	// Button Flush
			UARTOutStr("\r\nButton Grand Full Flush\r\n\r\n");
		break;
		case GRDLITEBT:	// Button Flush
			UARTOutStr("\r\nButton Grand Lite Flush\r\n\r\n");
		break;
		case SENTINALACT: // Sentinal Flush
			UARTOutStr("\r\nSentinal Flush\r\n\r\n");
		break;
		default:	 
		break;
	}
	
	if(GVeUserSts == 0){
		bNewInfo = 0;
	}
	
	bTick++;				
	if(bTick >= GVbDutyRate){
		bTick = 0;
	}
	
	if((GVeUserSts == 1) && (bNewInfo == 0)){
		UARTOutStr("\r\nNew Event\r\n\r\n");
		bNewInfo = 1;
		 bTick = 0;
	}
	

	#ifdef ACTIONDATA
		if(((GVeUserSts != NOTPRESENT)||(GVsButtonSts.bBigButton != NOTOUCH)) && (bTick == 0)){	
	#endif
			if(bTick == 0){
			
				bData[0] = GVeUserSts;
				bData[1] = GetTargetRange();
				bData[2] = GetUserRange();
				//bData[3] = IsUserSitDown();
				bData[3] = GVbSentinelFlush;

				bData[7] = GVbWakeBLE;						// flag to wake BLE
				
				bData[9] = GVbBLEEnabled;					// Flag of BLE donggle enable	
			
				for(i = 0; i < 10; i++){
					UARTOutHexByte(bData[i]);
					UARTOutStr(", ");
				}
				
				iData[0] = GetBigButtonBaselineValue();
				iData[1] = GetBigButtonCapInstantValue();
				iData[2] = GetSmallButtonBaselineValue();
				iData[3] = GetSmallButtonCapInstantValue();
				for(i = 0; i < 4; i++){
					UARTOutHexInt(iData[i]);
					UARTOutStr(", ");
				}
				/*
				iData[0] = GetIRRecOffset();
				iData[1] = GetNoiseLevel();
				iData[2] = GetStableBackground();
				iData[3] = GetBackground();
				iData[4] = GetInstant();
				iData[5] = NVsOpPara.IRLevel;
				
				for(i = 0; i < 6; i++){
					UARTOutHexInt(iData[i]);
					UARTOutStr(", ");
				}
				*/
				AS1_ClearTxBuf();
				AS1_ClearRxBuf();
			}
	#ifdef ACTIONDATA
		}	
	#endif	
		
		
}

/*
** =================================================================================================================================
**     Method      :  void UARTOutHandWaveData(void)
**     Description :
**         This method sends out all status in hex for debug.
**     Parameters  :Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutHandWaveData(void)
{
	uint8 i;
	uint8 bData[10];
	uint16 iData[9];
	static uint8 bResetInfo = 0; 
	
		if(bResetInfo == 0){
			UARTOutStr("\r\nStart From Reset:  ");
			UARTOutHexInt(StartReason);
			UARTOutStr("\r\n");
			//UARTOutDebugInfor();
			bResetInfo = 1;
		}
	
		switch(GVbFlushRequest){
			case STDFULLIR: // IR sensing Flush
				UARTOutStr("\r\nIR Sensor Full Flush\r\n\r\n");
			break;
			case STDLITEIR: // IR sensing Flush
				UARTOutStr("\r\nIR Sensor Lite Flush\r\n\r\n");
			break;
			case STDFULLBT:	// Button Flush
				UARTOutStr("\r\nButton Full Flush\r\n\r\n");
			break;
			case STDLITEBT:	// Button Flush
				UARTOutStr("\r\nButton Lite Flush\r\n\r\n");
			break;
			case GRDFULLIR: // IR sensing Flush
				UARTOutStr("\r\nIR Sensor Grand Full Flush\r\n\r\n");
			break;
			case GRDLITEIR: // IR sensing Flush
				UARTOutStr("\r\nIR Sensor Grand Lite Flush\r\n\r\n");
			break;
			case GRDFULLBT:	// Button Flush
				UARTOutStr("\r\nButton Grand Full Flush\r\n\r\n");
			break;
			case GRDLITEBT:	// Button Flush
				UARTOutStr("\r\nButton Grand Lite Flush\r\n\r\n");
			break;
			case SENTINALACT: // Sentinal Flush
				UARTOutStr("\r\nSentinal Flush\r\n\r\n");
			break;
			default:	 
			break;
		}
	
		bData[0] = DbHandWave;
		bData[1] = DbHandWaveCT;
		bData[2] = DbInterv;
		bData[3] =DbHandin;
		//bData[3] = GVbBLEEnabled;
	
		for(i = 0; i < 4; i++){
			UARTOutHexByte(bData[i]);
			UARTOutStr(", ");
		}
		iData[0] = DwPreEcho;
		iData[1] = DwCurrentEcho;
		iData[2] = DwIncrese;
		iData[3] = NVwHandWaveIR;
		for(i = 0; i < 4; i++){
			UARTOutHexInt(iData[i]);
			UARTOutStr(", ");
		}
		UARTOutStr("\r\n");
	//	AS1_ClearTxBuf();
	//	AS1_ClearRxBuf();		
}

/*
** =================================================================================================================================
**     Method      :  UARTOutHexASCIIBlock(uint8 * src, uint8 len) 
**     Description :
**         This method sends out block memory in hex ascii.
**     Parameters  : point to Data to start of block
**     			   : length of block
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutHexASCIIBlock(uint8 * src, uint8 len) 
{
	uint8 i,j;
	j=0;
	UARTPutCRLF(); 					// new line	
	for (i=0; i < len; i++){
		UARTOutHexByte(*src);		// put out one byte
		AS1_SendChar(','); 			// data seperator 
		src++;						// next byte
		j++;
		if(j>=10){ 					// 10 bytes per line
			j=0;
			UARTPutCRLF(); 			// new line	
		}
	}
}

/*
** =================================================================================================================================
**     Method      :  UARTOutParametersHexASCII(SolisParaType* Para)
**     Description :
**         This method sends out parameters in hex ASCII.
**     Parameters  : point to the parameter
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutParametersHexASCII(SolisBLEParaType* Para){
	
	uint8 * src;
	uint8 len;
	
		src = (uint8*)Para;
		len = sizeof( *Para);
		UARTOutHexASCIIBlock(src,len); 
}

/*
** =================================================================================================================================
**     Method      :  UARTOutParametersHexASCII(SolisParaType* Para)
**     Description :
**         This method sends out parameters in hex ASCII.
**     Parameters  : point to the parameter
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutReadOnlyEngDataHexASCII(BLEReadOnlyType* Para){
	
	uint8 * src;
	uint8 len;
	
		src = (uint8*)Para;
		len = sizeof( *Para);
		UARTOutHexASCIIBlock(src,len); 
}

/*
** =================================================================================================================================
**     Method      :  UARTOutParametersHexASCII(SolisParaType* Para)
**     Description :
**         This method sends out parameters in hex ASCII.
**     Parameters  : point to the parameter
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutReadWriteEngDataHexASCII(BLEReadWriteType* Para){
	
	uint8 * src;
	uint8 len;	
		src = (uint8*)Para;
		len = sizeof( *Para);
		UARTOutHexASCIIBlock(src,len); 
}

/*
** =================================================================================================================================
**     Method      : void UARTOutCommandList(void)
**     Description :
**         This method sends out Command list in communication mode.
**     Parameters  : Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutCommandList(void){
	
	UARTOutStr("\r\n V: Software Version");
	UARTOutStr("\r\n I: PCB Information");
	UARTOutStr("\r\n H: Parameters In Flash");
	UARTOutStr("\r\n L: Parameters In RAM");
	UARTOutStr("\r\n T: Go To Test Mode");
	UARTOutStr("\r\n S: Go To Set Mode");
	UARTOutStr("\r\n B: Boot Loader");
	UARTOutStr("\r\n");
	AS1_ClearTxBuf();
	AS1_ClearRxBuf();
	
}


/*
** =================================================================================================================================
**     Method      :  OutHexByte(uint8 OutByte) 
**     Description :
**         This method sends out one byte raw data.
**     Parameters  : Byte
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutRawByte(uint8 OutByte) 
{
	
	AS1_ClearTxBuf();
	SendComplete = 0;
	WDReset();					// Kick dog
	AS1_SendChar(OutByte); 		// Send out
    while(SendComplete == 0);	// Waiting for Tx complete    
    AS1_ClearTxBuf();
    SendComplete = 0;
  
}

/*
** =================================================================================================================================
**     Method      	:  OutRawBlock(uint8 * src, uint8 len) 
**     Description 	:
**         This method sends out a block raw data. Data being send out should be already include CRC
**         
**     Parameters  	:point to start of the block
**     				: length of the block
**     Returns     	: Nothing
** =================================================================================================================================
*/
void UARTOutRawBlock(uint8 * src, uint8 len) 
{
	
	uint8 i;

	for(i = 0; i < len; i++){	
		WDReset();				// Kick Dog
	    UARTOutRawByte(*src);
	    src++;					// next byte	    
	 }
	AS1_ClearTxBuf();
 
}

/*
** =================================================================================================================================
**     Method      :  ProcessSettingCmd(void)
**     Description :
**         This method process the command from service monitor during setting mode.
**        
**     Parameters  : Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/
void ProcessSettingCmd(void){
	
	uint8  k, temp, data[85], cmd;
	uint16 length, ln;
	uint8 * DataSrc;
	
	WDReset();											// kick dog
	if(AS1_GetCharsInRxBuf() > 0){ 						// there is a input
		AS1_RecvChar(&temp); 							// receive data
		if(temp == PREAMP){								// Communication request
			UARTOutRawByte(ACK);						// prompt
			WDReset();									// Kick dog
			wTimerTick = 0;								// reset timer
			TI1_Enable();								// Enable timer to terminate if something wrong
			while(AS1_GetCharsInRxBuf() < 4){ 			// should get 4 byte
				WDReset();								// kick dog
				if(wTimerTick >= COMMUNICATIONLIMIT){	// check if time run out
					break; 								// can't receive whole package. break
				}
			}
			TI1_Disable();								// stop timer to save power and 
			AS1_RecvBlock(data, 4, &length);			// get 4 byte
			if((data[0] ^ data[1]) == data[2]){	 		// command valid
				length = data[1]; 						// length of data to be communicated. total one byte ID + data block + checksum
				cmd = data[0];							// Command
				UARTOutRawByte(ACK);					// Prompt, command received
			}
			AS1_ClearRxBuf();
			AS1_ClearTxBuf();
			DelayUS(100);
					
			switch(cmd){
				case READDATA:									// send data to monitor
					WDReset();									// kick dog
					data[0] = '0';  							// first byte package ID
					temp = data[0];								// for calculate check sum
					DataSrc = (uint8 *)&NVsOpPara;
					for(k = 1; k <= (length-2); k++){
						data[k] = (*(DataSrc +(k-1))) + 1; 		// encode by adding 1
						temp = temp ^ data[k]; 					// Checksum
					}
					data[length-1] = temp; 						// append checksum at the end
					data[length] = 0; 							// append 0 at the end
					UARTOutRawBlock(data,length+1);				//
					wTimerTick = 0;								// reset timer
					TI1_Enable();	
					while(AS1_GetCharsInRxBuf() < 1){			// waiting for return
						WDReset();								// kick dog
						if(wTimerTick >= COMMUNICATIONLIMIT){	// check if time run out
							break; 								// can't receive whole package. break
						}
					}
					TI1_Disable();								// Disable timer
					AS1_ClearRxBuf();
					AS1_ClearTxBuf();
				break;
			
				case WRITEDATA:									// Receive settings from Monitor
					WDReset();									// kick dog
				   wTimerTick = 0;								// reset timer
				   TI1_Enable();								// start timer to exit below loop if unexpected thing happen
				   while(AS1_GetCharsInRxBuf() < length){   	// wait for whole package
					   WDReset();								// kick dog
					   if(wTimerTick >= COMMUNICATIONLIMIT){	// check if time run out
						   break; 								// can't receive whole package. break
						}
				   } 											// end of wait for receiving
				   TI1_Disable();	//
				   AS1_RecvBlock(data, length, &ln);
				   temp = data[0];
				   for(k = 1; k < (length-1); k++){
					   temp = temp ^ data[k]; 					// CHECK SUM
				   }
				   if(temp == data[length-1]){ 					// check sum ok?
					   DelayUS(500);
					   UARTOutRawByte(RECEIVEOK);				// Prompt Data received OK
					   DataSrc = (uint8 *)&GVsTempPara;			// write data to temporary parameters
					   for(k = 0; k < length-2; k++){
						   *(DataSrc + k) = data[k+1]; 			// skip first byte; 
					   }
					   
					   if((NVsOpPara.Mode != GVsTempPara.Mode) && ((NVsOpPara.CalibrationFlag & CURRENTCAL) == CURRENTCAL)){	// mode change requested and distance calibrated before
						   if(NVsOpPara.Mode == 0){														// request change from closet to urinal
							   GVsTempPara.IRLevel = NVsOpPara.IRLevel * GVsTempPara.UrinalIR /100; 	// modify IR level   
						   }
						   if(GVsTempPara.Mode == 0){				                                    // request change from Urinal to closet
							   GVsTempPara.IRLevel = NVsOpPara.IRLevel *100 / GVsTempPara.UrinalIR;     // modify IR level     
						   }   
					   }
					  NVsOpPara = GVsTempPara;						// update to real 
					  SetValveOnTime(NVsOpPara.OpenTM);				// set valve turn on time for all types of flush. There are 4 global variables.
					  SaveParaToFlash(NVsOpPara);					// store to flash
	
				   }
				   AS1_ClearRxBuf();
				   AS1_ClearTxBuf();
										
				break;
				
				case WRITEBOARDINFO:							// Receive settings from board configer
					WDReset();									// kick dog
				   wTimerTick = 0;								// reset timer
				   TI1_Enable();								// start timer to exit below loop if unexpected thing happen
				   while(AS1_GetCharsInRxBuf() < length){   	// wait for whole package
					   WDReset();								// kick dog
					   if(wTimerTick >= COMMUNICATIONLIMIT){	// check if time run out
						   break; 								// can't receive whole package. break
						}
				   } 											// end of wait for receiving
				   TI1_Disable();	//
				   AS1_RecvBlock(data, length, &ln);
				   temp = CalculateCRC(data, (length-1));		// calculate crc of received data,the last byte of received data is suppose CRC. don't inclide this byte when calculate the CRC. 
				   if(temp == data[length-1]){ 					// check sum ok?
					   DelayUS(500);
					   UARTOutRawByte(RECEIVEOK);				// Prompt Data received OK
					   SaveBoardInfoToFlash(data, length);		// Data including CRC
					   NVsBLEPara = LoadDefaultBLEPara();		// update BLE related parameters
					   SetDefaultOpParas();						// update Operational parameters
				   }
				   AS1_ClearRxBuf();
				   AS1_ClearTxBuf();
										
				break;
			
				case EXITSET:									//Exit setting mode
					//GVeOperationSts = SetOperationState(COMMUNICATION);	//just back to ASCii communication mode
					//GVeOperationSts = SetOperationState(NORMAL);//go normal mode, it will go communication mode if cable is still connected
					GVeOperationSts = COMMUNICATION;            // just change state without initilization
				break;	
				
				default:
				break;
			}	

		}
	}
}


/*
** ========================================================================================================================================================
**     Method      :  uint8 InputBoardInfor(void)
**     Description :
**         This method receives board information from UART.
**         Note: Board information is a 17 chars long string
**     Parameters  : Nothing
**     Returns     : TRUE if 17 chars have been received
** =========================================================================================================================================================
*/
uint8 InputBoardInfor(void){	

	char sReceicved[18]; 	// buffer to put incoming string
	char sBoardInfor[22];
	uint8 lp;
	uint8 temp;
	uint16 ln;				// index of receiving char
	WDReset();				// kick dog

	
	wTimerTick = 0;							// start to count time
	TI1_Enable();							// start timer to exit below loop if unexpected thing happen
	while(AS1_GetCharsInRxBuf() < 17){   	// wait for whole package
		WDReset();							// kick dog
		if(wTimerTick >= 60000){			// check if time run out
			UARTOutStr("\r\nTime Out\r\n");
			TI1_Disable();	//
			return FALSE;
		}
	} 										// end of wait for receiving
	TI1_Disable();	//				  
	AS1_RecvBlock((uint8*)sReceicved, 17, &ln);

	AS1_ClearRxBuf();
	AS1_ClearTxBuf();
	sReceicved[17] = '\0'; // terminate
	UARTOutStr("\r\nReceived: ");
	UARTOutStr(sReceicved);
	UARTOutStr("\r\n");
	NVsBLEPara = LoadBLEParaFromFlash();	// BLE related parameters
	for(lp =  0; lp <3; lp++){
		sBoardInfor[lp] = NVsBLEPara.sBoardSN[lp];
	}
	for(lp =  0; lp < 17; lp++){
		sBoardInfor[lp+3] = sReceicved[lp];
	} 
	
	temp = CalculateCRC((uint8*)sBoardInfor, 20);		// calculate crc of board infor
	sBoardInfor[20] = temp;				  
	SaveBoardInfoToFlash((uint8*)sBoardInfor, 21);		// Data including CRC
	NVsBLEPara = LoadDefaultBLEPara();					// update BLE related parameters
	//SetDefaultOpParas();								// update Operational parameters
	return TRUE;
}


/*
** ========================================================================================================================================================
**   For debug only
** =========================================================================================================================================================
*/

void UARTOutBatteryDebugData(void)
{
	uint16 wTemp1;
	float fWhole;
	char Outstring[40];
	static uint8 i=0;
	
	if(i==0){
	
	wTemp1 = NVsOpPara.BVolt;						// Read battery voltage
	fWhole = (wTemp1*3.5/65535.0)/ RATIO;
	sprintf(Outstring,"%2.3f",fWhole);
	UARTPutCRLF();
	UARTOutStr(Outstring); 
	}
	i++;
	if(i>=4){
		i=0;
	}	
}

/*
** ========================================================================================================================================================
**   For debug only
** =========================================================================================================================================================
*/
void UARTOutBatteryStoredData(void)
{
	uint8 i;
	float fBat;
	char Outstring[40];
	
	for(i=0;i<20;i++){
		fBat = (wBattest[i]*3.5/65535.0)/ RATIO;	
		sprintf(Outstring,"%2.3f",fBat);
		UARTPutCRLF();
		UARTOutStr(Outstring); 
	}
}

/*
** ========================================================================================================================================================
**   For debug only
** =========================================================================================================================================================
*/
void UARTOutBatteryMeasuremnet(void)
{
	uint16 wTemp1,wTemp2,wTemp3;
	float fWhole,fMid,fBat;
	char Outstring[40];

	wTemp1 = ReadBatteryFull()+ NVsOffset.wOff1;						// Read battery voltage
	wTemp2 = ReadBatteryHalf() + NVsOffset.wOff2;						// Read half battery voltage
	if(wTemp2 <= DISCONNECTED){						// hardwire 
		wTemp3 = wTemp1;										
	}
	else{
		wTemp3 = CalculateMinBattery(wTemp1,wTemp2);// calculate minimum single battery
	}
	fWhole = (wTemp1*3.5/65535.0)/ RATIO;
	fMid = (wTemp2*3.5/65535.0)/ RATIO;
	fBat = (wTemp3*3.5/65535.0)/ RATIO;
	
	sprintf(Outstring,"%2.3f, %2.3f, %2.3f,",fWhole,fMid,fBat);
	UARTPutCRLF();
	UARTOutStr(Outstring); 
	
}

/*
** =================================================================================================================================
**     Method      : void UARTOutTestCommandList(void);
**     Description :
**         Prompt commands in test mode
**     Parameters  : Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutTestCommandList(void){
	
#ifndef WITHBLE		
	UARTOutStr("\r\n C: Set Uint To Closet Mode");
	UARTOutStr("\r\n u: Set Uint To Urinal Mode");
	UARTOutStr("\r\n p: Set Uint To Urinal With Ball Park Mode");
#endif
	UARTOutStr("\r\n A: Read Battery at Full Point");
	UARTOutStr("\r\n a: Compare Battery Read Calibration Data");
	UARTOutStr("\r\n B: Read Battery");	
	UARTOutStr("\r\n W: Read Battery at Half Point");		
	UARTOutStr("\r\n Y: Battery Read Calibration");
	UARTOutStr("\r\n l: Load Battery Calibration Data");
	UARTOutStr("\r\n E: Set Off_Factory Flag");
	UARTOutStr("\r\n e: Clear Off_Factory Flag");
	UARTOutStr("\r\n S: Read Solar Cell");	
	UARTOutStr("\r\n d: Turn On LED");
	UARTOutStr("\r\n b: Turn Off LED");
	UARTOutStr("\r\n O: Latch Solenoid");
	UARTOutStr("\r\n U: UnLatch Solenoid");
	UARTOutStr("\r\n P: End Power Up Period");
	UARTOutStr("\r\n c: Calibrate Unit");
	UARTOutStr("\r\n s: Save Calibration");
	UARTOutStr("\r\n I: Check Maxmium IR Current");
	UARTOutStr("\r\n K: Check Active IR Current");
	UARTOutStr("\r\n J: Check IR Current Over Entire Range");
	UARTOutStr("\r\n V: Software Version");
	UARTOutStr("\r\n H: Parameters In Flash");
	UARTOutStr("\r\n L: Parameters In RAM");
	UARTOutStr("\r\n z: Put Unit In Sleep");
	UARTOutStr("\r\n M: Set PCB Information");
	UARTOutStr("\r\n m: Read PCB Information");
	UARTOutStr("\r\n Q: Quit Test");
	UARTOutStr("\r\n");

	AS1_ClearTxBuf();
	AS1_ClearRxBuf();

}


/*
** =================================================================================================================================
**     Method      :  ProcessTestCmd(void)
**     Description :
**         This method process ASCII command from UART during testing mode.
**        
**     Parameters  : Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/
void ProcessTestCmd(void){
	
	uint8 bReceicved;
	uint16 wTemp1,wTemp2, wTemp3,wTemp;
	uint16 wDrv;
	BatCalType BatTempOffset;					// Battery calibration Data
	float fWhole,fMid,fBat;
	char Outstring[40];
	
	WDReset();	// kick dog
	
	if(AS1_RecvChar(&bReceicved) == ERR_OK){	// Something received  		   
		AS1_ClearRxBuf();						// Clear Rx buffer for next usage
		AS1_ClearTxBuf();						// Clear Tx buffer to make tx clean
		
		switch(bReceicved){	
#ifndef WITHBLE
			case 'C':	// set to closet mode
				if(NVsOpPara.Mode != CLOSETMODE ){												// change requested
//					GVsTempPara = LoadParaWithDefault(CLOSETMODE);								// Change to closet mode and Load Closet  default
					GVsTempPara.Mode = CLOSETMODE;
					GVsTempPara.IRLevel  = 1700;  
					GVsTempPara.OpenTM = 28;
					GVsTempPara.ArmTM = 16;
					GVsTempPara.ONDelayTM = 2;
					if((NVsOpPara.CalibrationFlag & CURRENTCAL) == CURRENTCAL){					// Distance calibrate before for the urinal
						GVsTempPara.IRLevel = NVsOpPara.IRLevel *100 / GVsTempPara.UrinalIR; 	// calculate IR level for Closet from urinal calibration
						GVsTempPara.CalibrationFlag = CURRENTCAL;   							// set calibration flag
					}
					NVsOpPara = GVsTempPara ;   												// Update to real
					SaveParaToFlash(NVsOpPara);		  											// store to flash
				}	
				UARTOutStr("\r\n Closet Mode"); 												// prompt	
			break;
						
			case 'u':	// set to urinal mode
				if(NVsOpPara.Mode != URINALMODE ){													// change requested
//					GVsTempPara = LoadParaWithDefault(URINALMODE);									// Change to closet mode and Load Closet  default
					GVsTempPara.Mode = URINALMODE;
					GVsTempPara.IRLevel  = 1200; 
					GVsTempPara.OpenTM = 12;
					GVsTempPara.ArmTM = 8;
					GVsTempPara.ONDelayTM = 1;
					if((NVsOpPara.CalibrationFlag & CURRENTCAL) == CURRENTCAL){					// Distance calibrated.
						if(NVsOpPara.Mode == CLOSETMODE){											// Was calibrated for closet
							GVsTempPara.IRLevel = NVsOpPara.IRLevel * GVsTempPara.UrinalIR / 100; // calculate IR level for urinal from closet calibration
						}
						else{																	// was calibrated for urinal
							GVsTempPara.IRLevel = NVsOpPara.IRLevel;							// keep original calibration
						}
						GVsTempPara.CalibrationFlag = CURRENTCAL;   							// set calibration flag
					}
					NVsOpPara = GVsTempPara ;   												// Update to real
					SaveParaToFlash(NVsOpPara);		  											// store to flash
				}	
				UARTOutStr("\r\n Urinal Mode"); 												//prompt			
			break;
						
			case 'p':	// set to urinal with ball park
				if(NVsOpPara.Mode != BALLPARKMODE ){												// change requested
					//GVsTempPara = LoadParaWithDefault(BALLPARKMODE);								// Change to urinal with BP mode and Load default
					GVsTempPara.Mode = BALLPARKMODE;
					GVsTempPara.IRLevel  = 1200; 
					GVsTempPara.OpenTM = 12;
					GVsTempPara.ArmTM = 8;
					GVsTempPara.ONDelayTM = 1;
					if((NVsOpPara.CalibrationFlag & CURRENTCAL) == CURRENTCAL){					// Distance was calibrated.
						if(NVsOpPara.Mode == CLOSETMODE){											// Was calibrated for closet
							GVsTempPara.IRLevel = NVsOpPara.IRLevel * GVsTempPara.UrinalIR / 100; // calculate IR level for urinal from closet calibration
						}
						else{																	// was calibrated for urinal
							GVsTempPara.IRLevel = NVsOpPara.IRLevel;							// keep original calibration
						}
						GVsTempPara.CalibrationFlag = CURRENTCAL;   							// set calibration flag
					}
					NVsOpPara = GVsTempPara ;   												// Update to real
					SaveParaToFlash(NVsOpPara);		  											// store to flash
				}
				UARTOutStr("\r\n Urinal with Ball Park Mode"); 									//prompt		
			break;
#endif					
			case 'A':	// test battery at full point
				SleepMS(250);									// make sure battery read at MCU wake up moment
				UARTPutCRLF();
				UARTOutHexInt(ReadBatteryFull()); 
				UARTPutCRLF();
			break;
			
			case 'Y':	// battery read calibration 
					
				SleepMS(250);									// make sure battery read at MCU wake up moment
				wTemp1 = ReadBatteryFull();						// Read battery voltage
				wTemp2 = ReadBatteryHalf();						// Read half battery voltage
							
				BatTempOffset.wOff1 = CALIBRATIONVOL6-wTemp1;
				BatTempOffset.wOff2 = CALIBRATIONVOL3-wTemp2;
				BatTempOffset.bCalFlag = CALIBRATED;
				
				UARTPutCRLF();
				if((abs(BatTempOffset.wOff1)< CALIBRATIONLIMIT) && (abs(BatTempOffset.wOff2) < CALIBRATIONLIMIT)){
					NVsOffset = BatTempOffset;
					SaveBatCalParaToFlash(NVsOffset);
					UARTOutStr("Pass, ");
				}
				else{
					UARTOutStr("Fail, ");
				}
				sprintf(Outstring,"%d, %d",BatTempOffset.wOff1,BatTempOffset.wOff2);
				UARTOutStr(Outstring); 
				
			break;
						
			case 'B':	// test battery
				SleepMS(250);									// make sure battery read at MCU wake up moment
				UARTPutCRLF();
				UARTOutHexInt(ReadBattery()); 
				UARTPutCRLF();
			break;
			
			case 'W':	// test battery at half point
				SleepMS(250);									// make sure battery read at MCU wake up moment
				UARTPutCRLF();
				UARTOutHexInt(ReadBatteryHalf()); 
				UARTPutCRLF();

			break;
			
			case 'l':	// load battery calivbration data from flash
				
				NVsOffset = LoadBatteryCalFromFlash();
				if(NVsOffset.bCalFlag == CALIBRATED){
					UARTOutStr("\r\nCalibrated\r\n"); 
				}
				else{
					UARTOutStr("\r\nNot_Calibrated\r\n");
				}
				if(NVsOffset.bInFactoryFlag == OUTOFFACTORY){  	// In factory, go to check if there is any non-full battery
					UARTOutStr("Out_Factory\r\n"); 		// prompt
				}
				else{
					UARTOutStr("In_Factory\r\n"); 		// prompt
				}
				sprintf(Outstring,"%d, %d",NVsOffset.wOff1,NVsOffset.wOff2);
				UARTOutStr(Outstring);
			break;
	
			case 'a':	// Compare battery read calibration Data
				SleepMS(250);									// make sure battery read at MCU wake up moment
				wTemp1 = ReadBatteryFull();						// Read battery voltage
				wTemp2 = ReadBatteryHalf();						// Read half battery voltage
				if(wTemp2 <= DISCONNECTED){						// hardwire 
					wTemp3 = wTemp1;										
				}
				else{
					wTemp3 = CalculateMinBattery(wTemp1,wTemp2);	// calculate minimum single battery
					//wTemp3 = (wTemp << 2);                      // report battery as 4 X minimum
				}
				UARTPutCRLF();
				UARTOutStr("Without Calibration: ");
				fWhole = (wTemp1*3.5/65535.0)/ RATIO;
				fMid = (wTemp2*3.5/65535.0)/ RATIO;
				fBat = (wTemp3*3.5/65535.0)/ RATIO;	// don't include offset as it already included in adjusment
				sprintf(Outstring,"%2.3f, %2.3f, %2.3f,",fWhole,fMid,fBat);
				UARTOutStr(Outstring); 
				
				if(wTemp3>GVsBatteryTH.wLowVolt){
					UARTOutStr("  Normal"); 		// prompt
					//UARTPutCRLF();
				}
				else if(wTemp3>GVsBatteryTH.wStopVolt){
					UARTOutStr(" Warning"); 		// prompt
				}
				else{
					UARTOutStr("  Stop"); 		// prompt
				}
			
				wTemp1 += NVsOffset.wOff1;		
				wTemp2 += NVsOffset.wOff2;	
				if(wTemp2 <= DISCONNECTED){						// hardwire 
					wTemp3 = wTemp1;										
				}
				else{
					wTemp3 = CalculateMinBattery(wTemp1,wTemp2);	// calculate minimum single battery
				}
				
				UARTPutCRLF();
				UARTOutStr("With Calibration:    ");

				fWhole = (wTemp1*3.5/65535.0)/ RATIO;
				fMid = (wTemp2*3.5/65535.0)/ RATIO;
				fBat = (wTemp3*3.5/65535.0)/ RATIO;	// don't include offset as it already included in adjusment
				sprintf(Outstring,"%2.3f, %2.3f, %2.3f,",fWhole,fMid,fBat);
				UARTOutStr(Outstring); 
				
				if(wTemp3>GVsBatteryTH.wLowVolt){
					UARTOutStr("  Normal"); 		// prompt
					//UARTPutCRLF();
				}
				else if(wTemp3>GVsBatteryTH.wStopVolt){
					UARTOutStr(" Warning"); 		// prompt
				}
				else{
					UARTOutStr("  Stop"); 		// prompt
				}
				
				
			break;
			
			case 'r':	// repeat battery read
				
				for(wTemp=0; wTemp< 100; wTemp++)	{
					SleepMS(1000);
					wTemp1 = ReadBatteryFull();						// Read battery voltage
					wTemp2 = ReadBatteryHalf();						// Read half battery voltage
					if(wTemp2 <= DISCONNECTED){						// hardwire 
					wTemp3 = wTemp1;										
					}
					else{
						wTemp3 = CalculateMinBattery(wTemp1,wTemp2);	// calculate minimum single battery
					
					}
				
					wTemp1 += NVsOffset.wOff1;		
					wTemp2 += NVsOffset.wOff2;	
					if(wTemp2 <= DISCONNECTED){						// hardwire 
						wTemp3 = wTemp1;										
					}
					else{
						wTemp3 = CalculateMinBattery(wTemp1,wTemp2);	// calculate minimum single battery
					}
				
					fWhole = (wTemp1*3.5/65535.0)/ RATIO;
					fMid = (wTemp2*3.5/65535.0)/ RATIO;
					fBat = (wTemp3*3.5/65535.0)/ RATIO;	// don't include offset as it already included in adjusment
					sprintf(Outstring,"%2.3f, %2.3f, %2.3f,",fWhole,fMid,fBat);
					UARTPutCRLF();
					UARTOutStr(Outstring); 
					
				}
					
			break;	
			
			case 'E':	// set OUT off Factory flag
			
				NVsOffset.bInFactoryFlag = OUTOFFACTORY;
				SaveBatCalParaToFlash(NVsOffset);
				UARTOutStr("\r\nOut_Factory Flag Set"); 		// prompt		
			break;
			
			case 'e':	// set OUT off Factory flag		
				NVsOffset.bInFactoryFlag = 0;
				SaveBatCalParaToFlash(NVsOffset);
				UARTOutStr("\r\nOut_Factory Flag Cleared"); 		// prompt		
			break;
			
			case 'R':	// stored data
							
				UARTOutBatteryStoredData();
								
			break;	
						
			case 'S':	// test solar cell
				UARTPutCRLF();
				UARTOutHexInt(ReadSolarCell()); 
				UARTPutCRLF();
			break;
			
			case 'd': 	// Turn On LED
				LED_SetVal();						// turn on LED
				UARTOutStr("\r\nLED On\r\n"); 		// prompt				
			break;
				  
			case 'b':	// Turn off LED
				LED_ClrVal();						// turn off LED
				UARTOutStr("\r\nLED Off\r\n"); 		// prompt	
			break;
			 
			case 'c':	// Calibration
				GVsTempPara = NVsOpPara; 			// use temporary in case something wrong during test
				CalibrateTarget(NVsOpPara.IRCalibrationTH,&wTemp1, &wTemp2, &wTemp3);
				UARTPutCRLF();
				UARTOutHexInt(wTemp1); 		
				UARTOutStr(", "); 				   	// Separator	
				UARTOutHexInt(wTemp2); 
				//UARTOutStr(", "); 				// separator	
				//UARTOutHexInt(wTemp3); 
				GVsTempPara.CalibrationFlag = CURRENTCAL;
				GVsTempPara.IRLevel = wTemp1;
				GVsTempPara.CalibrationEcho = wTemp2;
				//GVsTempPara.MinUserTH = GVsTempPara.CalibrationEcho + GVsTempPara.CalibrationOffset;
				GVsTempPara.MinUserTH = GVsTempPara.CalibrationEcho + CALOFFSET;
				//GVsTempPara.NorseFloor = wTemp3;
			break;
							
			case 's':	// save Calibration
				
				NVsOpPara = GVsTempPara;              			// update to active 
				SaveParaToFlash(GVsTempPara);		  			// store to flash
				UARTOutStr("\r\nCalibration stored\r\n"); 		// Prompt
				
			break;			   	
							
			case 'I':	// Check maximum IR current
				
				wTemp1 = MeasureIRCurrent(NVsOpPara.MaxIRTH);
				wTemp2 = MeasureEchoVolt(NVsOpPara.MaxIRTH);
				UARTPutCRLF();
				UARTOutHexInt(NVsOpPara.MaxIRTH); 	// IR Level D/A	
				UARTOutStr(", "); 					// data separator 
				UARTOutHexInt(wTemp1); 				// IR Current
				UARTOutStr(", "); 					// data separator 
				UARTOutHexInt(wTemp2); 				// IR Echo
				
			break;
					
			case 'J':	// Check IR current over entire range
				UARTPutCRLF();
				for(wDrv=NVsOpPara.MinIRTH; wDrv <= NVsOpPara.MaxIRTH; wDrv += 50){
					wTemp1 = MeasureIRCurrent(wDrv);
					wTemp2 = MeasureEchoVolt(wDrv);
					UARTPutCRLF();
					UARTOutHexInt(wDrv); 			// IR Level D/A
					UARTOutStr(", "); 				// data seperator 
					UARTOutHexInt(wTemp1); 			// IR Current
					UARTOutStr(", "); 				// data seperator 
					UARTOutHexInt(wTemp2); 			// IR Echo

				}
				
			break; 
			
			case 'j':	// Check IR current over entire range of handwave detecting
							UARTPutCRLF();
							for(wDrv = 700; wDrv <= 900; wDrv += 10){
								wTemp1 = MeasureIRCurrent(wDrv);
								wTemp2 = MeasureEchoVolt(wDrv);
								UARTPutCRLF();
								UARTOutHexInt(wDrv); 			// IR Level D/A
								UARTOutStr(", "); 				// data seperator 
								UARTOutHexInt(wTemp1); 			// IR Current
								UARTOutStr(", "); 				// data seperator 
								UARTOutHexInt(wTemp2); 			// IR Echo
								SleepMS(50);
							}
							
						break; 
				
			case 'K':	// Check Active IR current
				wTemp1 = MeasureIRCurrent(NVsOpPara.IRLevel);
				wTemp2 = MeasureEchoVolt(NVsOpPara.IRLevel);
				UARTPutCRLF();
				UARTOutHexInt(NVsOpPara.IRLevel); 	// IR Level D/A
				UARTOutStr(", "); 					// data seperator 
				UARTOutHexInt(wTemp1); 				// IR Current
				UARTOutStr(", "); 					// data seperator 
				UARTOutHexInt(wTemp2); 				// IR Echo

			break; 
			
			case 'x':	// Test echo
				//IRScanTest(NVsOpPara.IRLevel, &wTemp1,&wTemp2,2);
				for(bReceicved = 0; bReceicved <50; bReceicved++){	   		
					IRScanTest(NVsOpPara.IRLevel, &wTemp1,&wTemp2,bReceicved);
					wTemp3 = wTemp1-wTemp2;
					UARTPutCRLF();
					UARTOutHexInt(wTemp1); 				// IR Current
					UARTOutStr(", "); 					// data seperator 
					UARTOutHexInt(wTemp2); 				// IR Echo
					UARTOutStr(", "); 					// data seperator 
					UARTOutHexInt(wTemp3); 				// IR Echo
					SleepMS(247);
				}
				
			break; 
					
			case 'O':	// latch solenoid 
				LatchSolenoid();					// latch solenoid
				UARTOutStr("\r\nSolenoid Latched\r\n"); //prompt	
			break;
								
			case 'U':	// unlatch solenoid 
				UnLatchSolenoid();// Unlatch solenoid
				UARTOutStr("\r\nSolenoid UnLatched\r\n"); //prompt	
			break;
			
			case 'V':	//show software revision
				UARTOutSoftwareVersion();
			break;	
			
			case 'P':	// put power up timer expire, so standby current can be measured
				NVbPowerupSts = POWERUPDONE;
				UARTOutStr("\r\nEnd of Power Up"); 	
				
			break;	
					
			case 'z': 	// everything off
				UARTOutStr("\r\nEverything is Off. Reset Power to Restart\r\n"); //prompt	
				UnLatchSolenoid();		// Unlatch solenoid
				TurnOffAllPeripheral();
				SleepForever();
			
			break;
									
			case 'L':	// show all parameters from RAM
				UARTOutParametersHexASCII(&NVsOpPara);
				break;
				
			case 'H':	// show all parameters from Flash
				GVsTempPara = LoadParaFromFlash();	// Get parameters from flash
				UARTOutParametersHexASCII(&GVsTempPara);
				break;
				
			case 'M': // 17 bytes SSSSSSSYYMMDDVVVV
				UARTOutStr("\r\nInput Board Info:\r\n"); //prompt
				InputBoardInfor();
				break;
				
			case 'm': //
				DisplayBoardData();
				break;				
				
			case 'Q': // quit to the main manual
				
				UARTOutStr("\r\nEnd of Test\r\n"); //prompt
				//GVeOperationSts = SetOperationState(COMMUNICATION);
				GVeOperationSts = COMMUNICATION;            // just change state without initilization
				break;
				
			
			default:
				UARTOutTestCommandList();
			break;
		}
	}
}

/*
** =================================================================================================================================
**     Method      :  ProcessASCIIRequest(void)
**     Description :
**         This method check UART inputs.
**         Note: use this only on AS1 is enabled. this method does not handle AS1 enable/disable
**     Parameters  : Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/
void ProcessASCIIRequest(void){	

	uint8 Receicved;

	
	if(AS1_RecvChar(&Receicved) == ERR_OK){	// Received something 		   
		AS1_ClearRxBuf();					// Clear Rx buffer for next usage
		switch(Receicved){	
			case 'V':	// Request to show software revision
				UARTOutSoftwareVersion();
			break;
				
			case 'H':	// Request All Parameters from Flash
				GVsTempPara = LoadParaFromFlash();	// Get parameters from flash
				UARTOutParametersHexASCII(&GVsTempPara);
				UARTOutStr("\r\n");
			break;
				
			case 'L':	// Request All parameters From RAM
				UARTOutParametersHexASCII(&NVsOpPara);
				UARTOutStr("\r\n");
			break;
				
			case 'T':	// Request to go to test mode
				GVeOperationSts = SetOperationState(TESTING);	// change to Test operation
				UARTOutEnterTestingModePrompt();
			break;
				
			case 'S':	// Request to go setting Mode
				GVeOperationSts = SetOperationState(SETTING);
				UARTOutEnterSettingModePrompt();				
			break;
				
			case 'B':	// request to go to boot loader
				RunApp = 0xAA;
				Boot_Reset();
			break;
			
			case 'd':
			case 'D':	// Request debug information
				//UARTOutDebugInfor();
				//TestBLECommand();
				DisplayFlashData();
			break;
			
			case 'h':	// Request debug information
				//UARTOutDebugInfor();
				UARTOutHandWaveData();
			break;
			
			
			case 'I':	// Request board info
				DisplayBoardData();	
			break;
			
			default:	// not a command
				//UARTOutCommandList(); // Send out command list
			break;
		}
	}
}
