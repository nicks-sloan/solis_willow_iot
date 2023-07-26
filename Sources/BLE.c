/*
 * BLE.c
 *
 *  Created on: Sep 5, 2019
 *      Author: Scott Wang
 *      Includes all BLE related 
 */

#include "BLE.h"
#include "UART.h"
#include "Operation.h"
#include "PowerSupply.h"
#include "Timing.h"
#include "TI1.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// GPF constants
char* sGPFSTring[21] = {
	"1.6",		//SKU 1
	"1.28",		//SKU 2
	"1.1",		//SKU 3
	"1.6",		//SKU 4
	"1.28",		//SKU 5
	"1.28",		//SKU 6
	"1.1",		//SKU 7
	"1.6",		//SKU 8
	"1.6",		//SKU 9
	"0.125",	//SKU 10
	"0.25",		//SKU 11
	"0.5",		//SKU 12
	"1.0",		//SKU 13
	"0.125",	//SKU 14
	"0.25",		//SKU 15
	"0.5",		//SKU 16
	"1.0",		//SKU 17
	"0.125",	//SKU 18
	"0.25",		//SKU 19
	"0.5",		//SKU 20
	"2.4"};		//SKU 21


BLEReadOnlyType sBLEEngDataROnly;		// ReadOnly Enginerring data
BLEReadWriteType sBLEEngDataRWrite;		// ReadWrite Engineering data

/*
** ===================================================================================================================================================================
**     Method      :void SaveBoardInfoToFlash(uint8* psBoardInfo, uint8 bLen)
**     Description :
**         This method save board info to flash. Board info is set from configurator during production.
**         Board info should include CRC at the end. 
**         One more byte written flag will be added at the end
**     Parameters  : point to board info, length of info
**     Returns     : Nothing
** ===================================================================================================================================================================
*/
void SaveBoardInfoToFlash(uint8* psBoardInfo, uint8 bLen){

	uint8 bFlag = WRITTENFLAG;

		IFsh1_EnableEvent();		//Enable flash process events
		// write board infor
		while(IFsh1_SetBlockFlash((IFsh1_TDataAddress) psBoardInfo, FLASHBOARDADDRESS, bLen)!= ERR_OK); // let watch dog reset it if something is wrong
		// write Flag at the end
		while(IFsh1_SetBlockFlash(&bFlag, (FLASHBOARDADDRESS + bLen), 1) !=  ERR_OK); 				
		IFsh1_DisableEvent();			
}

/*
** =========================================================================================================================
**     Method      :uint8 IsBoardInfoValid(uint8* psBoardInfo)
**     Description :
**         This method check if the board info from flash is valid. the last two bytes are CRC and written Flag
**     Parameters  : pointer to board info string
**     Returns     : True if valid
** =========================================================================================================================
*/
uint8 IsBoardInfoValid(uint8* psBoardInfo){
	uint8 lp;
	uint8* pTemp;
	uint8 bTempCRC;
	
		pTemp = psBoardInfo;							// start from the beginning
		bTempCRC = *pTemp;								// first byte
		for(lp = 1; lp < (BOARDINFOLEN- 2); lp++){					
			bTempCRC = bTempCRC  ^ (*(pTemp + lp)); 	// Checksum
		}
		if((bTempCRC != *(psBoardInfo + (BOARDINFOLEN- 2))) || (*(psBoardInfo + (BOARDINFOLEN- 1)) != WRITTENFLAG)){
			return FALSE;
		}
		return TRUE;
}

/*
** ===================================================================================================================================================================
**     Method      :void SaveBLEParaToFlash(BLEDongleParaType sPara){
**     Description :
**         This method save BLE related parameters to flash. Also the data CRC is calculated and stored at the end of parameter.
**         A written flag is also stored at the end.
**     Parameters  : parameter
**     Returns     : Nothing
** ===================================================================================================================================================================
*/
void SaveBLEParaToFlash(BLEDongleParaType sPara){
	uint16 len;
	uint8 bTempCRC;	
	uint8 bTempFlag = WRITTENFLAG;
	IFsh1_TAddress FlashAdress;
	
		len = sizeof sPara;
		bTempCRC = CalculateCRC((uint8*)&sPara, len);
		IFsh1_EnableEvent();		//Enable flash process events
		FlashAdress = FLASHBLEADDRESS;
		// wait until writting done
		while(IFsh1_SetBlockFlash((IFsh1_TDataAddress) &sPara, FlashAdress, len)!= ERR_OK); // let watch dog reset it if something is wrong
		// write CRC at the end
		FlashAdress += len;
		while(IFsh1_SetBlockFlash(&bTempCRC, FlashAdress, 1) !=  ERR_OK); 
		// write written flag at the end
		FlashAdress += 1;
		while(IFsh1_SetBlockFlash(&bTempFlag, FlashAdress, 1) !=  ERR_OK); 	
		IFsh1_DisableEvent();
}

/*
** =========================================================================================================================
**     Method      :void SetDefaultOpParas(void)
**     Description :
**         This method update operational parameters based on SKU. and save the data to flash. 
**     Parameters  : Nothing
**     Returns     : Nothing
** =========================================================================================================================
*/
void SetDefaultOpParas(void){
	
	//NVsBLEPara = LoadDefaultBLEPara();
	switch (NVsBLEPara.bSKUIndex){
	// reqular closet
		case 0:
		case 1:	
		case 3:
		case 4:
		case 5:
		case 20:
			NVsOpPara.ArmTM = 16;		//default for closet
			NVsOpPara.OpenTM = 28;
			NVsOpPara.ONDelayTM = 2;
		break;
	// 1.1 gpf closet
		case 2:
		case 6:
			NVsOpPara.ArmTM = 16;		// default for closet
			NVsOpPara.OpenTM = 20;
			NVsOpPara.ONDelayTM = 2;
		break;
	// dual flushes	
		case 7:
			NVsOpPara.ArmTM = 16;		// default for closet
			NVsOpPara.OpenTM = 28;
			NVsOpPara.ONDelayTM = 2;
		break;
		case 8:
			NVsOpPara.ArmTM = 16;		// default for closet
			NVsOpPara.OpenTM = 28;
			NVsOpPara.ONDelayTM = 2;
		break;
	// 0.125 urinal	
		case 9:
		case 13:
			NVsOpPara.ArmTM = 8;		// default for urinal
			NVsOpPara.OpenTM = 12;
			NVsOpPara.ONDelayTM = 1;
		break;
	// regular urinal		
		case 10:
		case 11:
		case 12:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
			NVsOpPara.ArmTM = 8;		// default for urinal
			NVsOpPara.OpenTM = 22;
			NVsOpPara.ONDelayTM = 1;
		//	NVsOpPara.WaitDelayTM = 1;	
		break;
		default:
		break;
	}
	NVsOpPara.SentinalTM = 0;   					// disable sentinal
	SaveParaToFlash(NVsOpPara);						// Sensor operational parameters
	strcpy(NVsBLEPara.sSensorRange,"3");			// default sensing range
	SaveBLEParaToFlash(NVsBLEPara);					// BLE related parameters
	GVsTurnOnTM = SetValveOnTime(NVsOpPara.OpenTM);	// update Flush on time
}

/*
** =========================================================================================================================
**     Method      :BLEDongleParaType LoadDefaultBLEPara(void){
**     Description :
**         This method get BLE parametrs from board info in flash. if data in flash is corrupted (or blank), use default instead.
**     Parameters  : Nothing
**     Returns     : Parameter struck
** =========================================================================================================================
*/
BLEDongleParaType LoadDefaultBLEPara(void){
	
	uint8 i;
	uint8 temp;
	char sBoardInfo[BOARDINFOLEN + 1];		// Board info string
	char sBoardSN[11];						// serial Number
	char sBoardDATE[7];						// build data
	char sBoardREV[5];						// Board revision
	char sBoardSKU[3];						// SKU
	uint8 bSKUIndex;						// SKU Index
	BLEDongleParaType sPara;
			IFsh1_EnableEvent();																						// Enable flash process events
			while(IFsh1_GetBlockFlash(FLASHBOARDADDRESS, (IFsh1_TDataAddress) sBoardInfo, BOARDINFOLEN)!= ERR_OK); 		// wait until load parameters from flash done
			IFsh1_DisableEvent();
			
			if(!IsBoardInfoValid((uint8 *)sBoardInfo)){   			// data crupted or never written
			#ifdef WITHBLE
				#ifdef WITHANT  /* MVP add begin */
					switch(SKU){  // ANT SN  remap sku example BLE sku 1 = Ant SKU 30+1 = 31
						case 1:
							strcpy(sBoardInfo,"I3100000842106080001");	// default  “AKKSSSSSSSYYMMDDVVVV” -I for production -B for india testing  -A for Andover -M for Minhthu
						break;
						case 2:
							strcpy(sBoardInfo,"I3200000002106300001");	// default
						break;
						case 3:
							strcpy(sBoardInfo,"I3300000000000000001");	// default
						break;
						case 8:
							strcpy(sBoardInfo,"I3800000002011040001");	// default
						break;
						case 10:
							strcpy(sBoardInfo,"I4000000000000000001");	// default
						break;
						case 11:
							strcpy(sBoardInfo,"I4100000010000000001");	// default
						break;
						case 12:
							strcpy(sBoardInfo,"I4200000002011040001");	// default
						break;					
						case 13:
							strcpy(sBoardInfo,"I4300000000000000001");	// default
						break;
						case 21:
							strcpy(sBoardInfo,"I3000000000000000001");	// default
						break;
						default:
							strcpy(sBoardInfo,"I3100000000000000001");	// default
						break;
						
					}
				#else  /* MVP add end */
					switch(SKU){ //BLE SN
						case 1:
							strcpy(sBoardInfo,"A0100000000000000001");	// default
						break;
						case 2:
							strcpy(sBoardInfo,"A0200000000000000001");	// default
						break;
						case 3:
							strcpy(sBoardInfo,"A0300000000000000001");	// default
						break;
						case 8:
							strcpy(sBoardInfo,"A0800000000000000001");	// default
						break;
						case 10:
							strcpy(sBoardInfo,"A1000000000000000001");	// default
						break;
						case 11:
							strcpy(sBoardInfo,"A1100000000000000001");	// default
						break;
						case 12:
							strcpy(sBoardInfo,"A1200000000000000001");	// default
						break;					
						case 13:
							strcpy(sBoardInfo,"A1300000000000000001");	// default
						break;
						
						default:
							strcpy(sBoardInfo,"A0100000000000000001");	// default
						break;
						
					}
				#endif  // MVP add
			#else
				switch(SKU){
							case 1:		// Closet
								strcpy(sBoardInfo,"S0100000000000000001");	// default
							break;
							
							case 2:		// 1.1 GPF Closet
								strcpy(sBoardInfo,"S0200000000000000001");	// default
							break;
							
							case 3:		// Urinal
								strcpy(sBoardInfo,"S0300000000000000001");	// default
							break;
							
							case 4:		// 0.125 Urinal
								strcpy(sBoardInfo,"S0400000000000000001");	// default
							break;
							
							case 5: // Dual Buttons
								strcpy(sBoardInfo,"S0500000000000000001");	// default
							break;
							
							case 6:  	// Conceal Closet 1.6 GPF
								strcpy(sBoardInfo,"S0600000000000000001");	// default
							break;
							
							case 7:  	// Conceal Closet 1.28 GPF
								strcpy(sBoardInfo,"S0700000000000000001");	// default
							break;
							
							case 8:  	// Conceal Urinal 1.0 GPF
								strcpy(sBoardInfo,"S0800000000000000001");	// default
							break;
							
							case 9:  	// Conceal Urinal 0.5 GPF
								strcpy(sBoardInfo,"S0900000000000000001");	// default
							break;
							
							default: // Closet mode
								strcpy(sBoardInfo,"S0100000000000000001");	// default
							break;
				}
		#endif
				
				temp = CalculateCRC((uint8*)sBoardInfo, 20);		// calculate crc of board infor
				sBoardInfo[20] = temp;
									  
				SaveBoardInfoToFlash((uint8*)sBoardInfo, 21);		// Data including CRC

			}
			
			
			sBoardInfo[BOARDINFOLEN - 2] = '\0';
			// set board serial string
			for(i = 0; i < 10; i++){
				sBoardSN[i]= sBoardInfo[i];
			}
			sBoardSN[10] = '\0';
			
			// set board Date string
			for(i = 0; i < 6; i++){
				sBoardDATE[i]= sBoardInfo[i+10];
			}
			sBoardDATE[6] = '\0';
			
			// set board revision string
			for(i = 0; i < 4; i++){
				sBoardREV[i] = sBoardInfo[i+16];
			}
			sBoardREV[4] = '\0';
			
			// set board SKU string
			for(i = 0; i < 2; i++){
				sBoardSKU[i] = sBoardInfo[i+1];
			}
			sBoardSKU[2] = '\0';
			
		#ifdef WITHANT  /* MVP add begin */
			// set SKU index
			if(!strcmp(sBoardSKU,"31")) bSKUIndex = 0;
			if(!strcmp(sBoardSKU,"32")) bSKUIndex = 1;
			if(!strcmp(sBoardSKU,"33")) bSKUIndex = 2;
			if(!strcmp(sBoardSKU,"34")) bSKUIndex = 3;
			if(!strcmp(sBoardSKU,"35")) bSKUIndex = 4;
			if(!strcmp(sBoardSKU,"36")) bSKUIndex = 5;
			if(!strcmp(sBoardSKU,"37")) bSKUIndex = 6;
			if(!strcmp(sBoardSKU,"38")) bSKUIndex = 7;
			if(!strcmp(sBoardSKU,"39")) bSKUIndex = 8;
			if(!strcmp(sBoardSKU,"40")) bSKUIndex = 9;
			if(!strcmp(sBoardSKU,"41")) bSKUIndex = 10;
			if(!strcmp(sBoardSKU,"42")) bSKUIndex = 11;
			if(!strcmp(sBoardSKU,"43")) bSKUIndex = 12;
			if(!strcmp(sBoardSKU,"44")) bSKUIndex = 13;
			if(!strcmp(sBoardSKU,"45")) bSKUIndex = 14;
			if(!strcmp(sBoardSKU,"46")) bSKUIndex = 15;
			if(!strcmp(sBoardSKU,"47")) bSKUIndex = 16;
			if(!strcmp(sBoardSKU,"48")) bSKUIndex = 17;
			if(!strcmp(sBoardSKU,"49")) bSKUIndex = 18;
			if(!strcmp(sBoardSKU,"50")) bSKUIndex = 19;
			if(!strcmp(sBoardSKU,"30")) bSKUIndex = 20;
		#else
			// set SKU index
			if(!strcmp(sBoardSKU,"01")) bSKUIndex = 0;
			if(!strcmp(sBoardSKU,"02")) bSKUIndex = 1;
			if(!strcmp(sBoardSKU,"03")) bSKUIndex = 2;
			if(!strcmp(sBoardSKU,"04")) bSKUIndex = 3;
			if(!strcmp(sBoardSKU,"05")) bSKUIndex = 4;
			if(!strcmp(sBoardSKU,"06")) bSKUIndex = 5;
			if(!strcmp(sBoardSKU,"07")) bSKUIndex = 6;
			if(!strcmp(sBoardSKU,"08")) bSKUIndex = 7;
			if(!strcmp(sBoardSKU,"09")) bSKUIndex = 8;
			if(!strcmp(sBoardSKU,"10")) bSKUIndex = 9;
			if(!strcmp(sBoardSKU,"11")) bSKUIndex = 10;
			if(!strcmp(sBoardSKU,"12")) bSKUIndex = 11;
			if(!strcmp(sBoardSKU,"13")) bSKUIndex = 12;
			if(!strcmp(sBoardSKU,"14")) bSKUIndex = 13;
			if(!strcmp(sBoardSKU,"15")) bSKUIndex = 14;
			if(!strcmp(sBoardSKU,"16")) bSKUIndex = 15;
			if(!strcmp(sBoardSKU,"17")) bSKUIndex = 16;
			if(!strcmp(sBoardSKU,"18")) bSKUIndex = 17;
			if(!strcmp(sBoardSKU,"19")) bSKUIndex = 18;
			if(!strcmp(sBoardSKU,"20")) bSKUIndex = 19;
		#endif
			
			strcpy(sPara.sBoardSN,sBoardSN);
			strcpy(sPara.sBoardDATE,sBoardDATE);
			strcpy(sPara.sBoardREV,sBoardREV);
			strcpy(sPara.sBoardSKU,sBoardSKU);
			strcpy(sPara.sSensorRange,"3");
			sPara.bSKUIndex = bSKUIndex;
			strcpy(sPara.sGPF,sGPFSTring[bSKUIndex]);
			sPara.SentinalActivation = 0 ;
			sPara.ReduceFlushActivation = 0 ;	
			SaveBLEParaToFlash(sPara);  // save it to flash
			
		//	SetDefaultOpParas();		// update operational parameters
		return sPara;
}

/*
** =========================================================================================================================
**     Method      :BLEDongleParaType LoadBLEParaFromFlash(void){
**     Description :
**         This method get parameters from flash. if data in flash is corrupted (or blank), use default instead.
**     Parameters  : Nothing
**     Returns     : Parameter struck
** =========================================================================================================================
*/
BLEDongleParaType LoadBLEParaFromFlash(void){
	
	uint16 len;
	uint8 bTempCRC;

	BLEDongleParaType sPara;
	uint8 bTempFlag;
	IFsh1_TAddress FlashAdress;

		IFsh1_EnableEvent();																// Enable flash process events
		len = sizeof sPara;
		FlashAdress = FLASHBLEADDRESS;
		while(IFsh1_GetBlockFlash(FlashAdress, (IFsh1_TDataAddress) &sPara, len)!= ERR_OK); // wait until load parameters from flash done
		FlashAdress += len;
		while(IFsh1_GetBlockFlash(FlashAdress, &bTempCRC, 1)!= ERR_OK); 					// get stored CRC.
		FlashAdress += 1;
		while(IFsh1_GetBlockFlash(FlashAdress, &bTempFlag, 1)!= ERR_OK); 					// get written flag.
		IFsh1_DisableEvent();                                                               // complete flash operation
		
		if(((CalculateCRC((uint8*)&sPara, len)) == bTempCRC) && (bTempFlag == WRITTENFLAG)){// data in flash is good																	// using the data from flash
			return sPara;
		}
		else{// data in flash corrupted or blank
			sPara = LoadDefaultBLEPara();	// load default based on board infor
		}
		
		return sPara;
}

/*
** ===================================================================================================================================================================
**     Method      :BLEReadOnlyType GetEngineeringReadOnlyData(void)
**     Description :
**         This method assign enginerring data 1.
**     Parameters  : nothing
**     Returns     : Nothing
** ===================================================================================================================================================================
*/
BLEReadOnlyType GetEngineeringReadOnlyData(void){

	BLEReadOnlyType sTemp;
		sTemp.iEngReadOnly[0]	= 	NVsOpPara.CalibrationEcho;
		sTemp.iEngReadOnly[1]	=	NVsOpPara.CleanBackground;
		sTemp.iEngReadOnly[2]	=	NVsOpPara.BuildY;
		sTemp.iEngReadOnly[3]	=	NVsOpPara.ConfirmTimeTH;
		sTemp.iEngReadOnly[4]	=	NVsOpPara.MaxIRTH;
		sTemp.bEngReadOnly[0]	=	NVsOpPara.BuildD;
		sTemp.bEngReadOnly[1]	=	NVsOpPara.ArmTM;
		sTemp.bEngReadOnly[2]	=	NVsOpPara.BVolt;
		sTemp.bEngReadOnly[3]	=	NVsOpPara.ONDelayTM;
		sTemp.bEngReadOnly[4]	=	NVsOpPara.OpenTM;
		sTemp.bEngReadOnly[5]	=	NVsOpPara.WaitDelayTM;
		sTemp.bEngReadOnly[6]	=	NVsOpPara.AdjudtedFailCT;
		sTemp.bEngReadOnly[7]	=	NVsOpPara.CalibrationFlag;
		sTemp.bEngReadOnly[8]	=	NVsOpPara.VerMajor;
		sTemp.bEngReadOnly[9]	=	NVsOpPara.VerMinor;
	return sTemp;
}

/*
** ===================================================================================================================================================================
**     Method      :BLEReadWriteType GetEngineeringReadWriteData(void)
**     Description :
**         This method assign enginerring data 2.
**     Parameters  : Nothing
**     Returns     : Nothing
** ===================================================================================================================================================================
*/
BLEReadWriteType GetEngineeringReadWriteData(void){
	
	BLEReadWriteType sTemp;
	
		sTemp.iEngReadWrite[0] 	=	NVsOpPara.MinUserTH;
		sTemp.iEngReadWrite[1] 	=	NVsOpPara.TouchTH;
		sTemp.iEngReadWrite[2] 	=	NVsOpPara.MaxBackground;
		sTemp.bEngReadWrite[0] 	= 	NVsOpPara.Mode;
		sTemp.bEngReadWrite[1] 	=	NVsOpPara.ONDelayTM;
		sTemp.bEngReadWrite[2] 	=	NVsOpPara.OpenTM;
		sTemp.bEngReadWrite[3] 	=	NVsOpPara.ArmTM;
	return sTemp;
}

/*
** ===================================================================================================================================================================
**     Method      :void SetEngineeringWriteData(void)
**     Description :
**         This method copy the enginerring data to operation parameters
**     Parameters  : nothing
**     Returns     : Nothing
**     		
** ===================================================================================================================================================================
*/
void SetEngineeringWriteData(BLEReadWriteType sTemp){
	
	uint8 bChanged;
	
		bChanged  = FALSE;
		if((sTemp.iEngReadWrite[0] >= 4000) && (sTemp.iEngReadWrite[0] <= 6000)){ // verify setting is in limits
			NVsOpPara.MinUserTH = sTemp.iEngReadWrite[0];
			bChanged  = TRUE;
		}
		if((sTemp.iEngReadWrite[1] >= 30) && (sTemp.iEngReadWrite[1] <= 100)){ // verify setting is in limits
			NVsOpPara.TouchTH =	sTemp.iEngReadWrite[1];
			bChanged  = TRUE;
		}
		if((sTemp.iEngReadWrite[2] >= 12000) && (sTemp.iEngReadWrite[2] <= 15000)){ // verify setting is in limits
			NVsOpPara.MaxBackground =	sTemp.iEngReadWrite[2];
			bChanged  = TRUE;
		}
		if((sTemp.bEngReadWrite[0] >= 0) && (sTemp.bEngReadWrite[0] <= 3)){ // verify setting is in limits
			NVsOpPara.Mode =	sTemp.bEngReadWrite[0];
			bChanged  = TRUE;
		}
		if((sTemp.bEngReadWrite[1] >= 2) && (sTemp.bEngReadWrite[1] <= 60)){ // verify setting is in limits
			NVsOpPara.ONDelayTM =	sTemp.bEngReadWrite[1];
			bChanged  = TRUE;
		}
		if((sTemp.bEngReadWrite[2] >= 5) && (sTemp.bEngReadWrite[2] <= 30)){ // verify setting is in limits
			NVsOpPara.OpenTM =	sTemp.bEngReadWrite[2];
			bChanged  = TRUE;
		}
		if((sTemp.bEngReadWrite[3] >= 2) && (sTemp.bEngReadWrite[3] <= 60)){ // verify setting is in limits
			NVsOpPara.ArmTM =	sTemp.bEngReadWrite[3];
			bChanged  = TRUE;
		}
		if(bChanged  == TRUE){
			SaveParaToFlash(NVsOpPara); // store to flash
		}
}

/*
** =================================================================================================================================
**     Method      :  uint8 ExcuteBLECommandID(uint16 iID, char* sInData, char* sResponce)
**     Description :
**         This method excute single command from BLE
**     Parameters  : command ID
**     			   : pointer to data from BLE
**     			   : pointer to data return to BLE
**     Returns     : TRUE if a flush activation is requested
** =================================================================================================================================
*/

uint8 ExcuteBLECommandID(uint16 iID, char* sInData, char* sResponce){
	
uint8 bTemp, bTempDay,i;  
uint16 iTemp, ii;  //6/14/21 fix from ii being uint8, uint8 ii create an loop create reset with watchdog
uint32 lTemp;
uint8 bFlushRequest;

				bFlushRequest = FALSE;   // default to no flush

				switch(iID){
		
				// Product Information
					case 1:	// Serial number RO
						strcpy(sResponce,NVsBLEPara.sBoardSN);
					break;
					
					case 2:	// mfg date RO
						strcpy(sResponce,NVsBLEPara.sBoardDATE);
					break;
					
					case 3:	// hardware version RO
						strcpy(sResponce,NVsBLEPara.sBoardREV);
					break;
					
					case 4:	// firmware version RO
						strcpy(sResponce,VERISION_MAJOR);
						strcat(sResponce,VERISION_MINOR);
					break;
					
					case 5:	//sku RO
						strcpy(sResponce,NVsBLEPara.sBoardSKU);	
					break;
						
				// Settings
					case 6:	// factory reset RW
						if(*sInData == '1'){			// go to factory reset
							SetDefaultOpParas();
						}
						strcpy(sResponce,sInData);
					break;
					
					case 7:	// sensor range  RW
						if(*sInData == '\0'){	//just to read
							strcpy(sResponce,NVsBLEPara.sSensorRange);
						}
						else{ 				// set new value
							*(sInData+1) = '\0';    // make sure it will not overflow
							strcpy(NVsBLEPara.sSensorRange,sInData);
							strcpy(sResponce,sInData);
							SetSensingRange(NVsBLEPara.sSensorRange);
							SaveBLEParaToFlash(NVsBLEPara);		//	NVsBLEPara changed, save it to flash
							SaveParaToFlash(NVsOpPara);			//	NVsOpPara changed, store it to flash
						}	
					break;
		
					case 8:	// Arm Time (Activation Time) RW
						if(*sInData == '\0'){				//just to read
							bTemp = NVsOpPara.ArmTM;
							UTIL1_Num8uToStr((uint8*)sResponce,5,bTemp);  // set responce data
						}
						else{ 								// set new value
							bTemp = (uint8)atoi(sInData);
							if(bTemp > 28) bTemp = 28;		// maxium 28
							if(bTemp < 8)  bTemp = 8; 		// minium
							NVsOpPara.ArmTM = bTemp;
							SaveParaToFlash(NVsOpPara);		// store to flash
							strcpy(sResponce,sInData);		// set responce data
						}
					break;
					
					case 9: // Line Flush Time (Sentinal Time) RW
						if(*sInData == '\0'){				//just to read
							bTemp = NVsOpPara.SentinalTM;   // the hours
							bTempDay = 0;
							while(bTemp >= 24){ 			// covert hours to days
								bTemp = bTemp - 24;
								bTempDay++;
							}
							UTIL1_Num8uToStr((uint8*)sResponce,5,bTempDay); // set responce data
						}
						else{ 								// set New value
							bTempDay = (uint8)atoi(sInData);
							bTemp = 0 ;
							while(bTempDay > 0){
								bTemp += 24;  				// convert days to hours
								bTempDay--;
							}
							if(bTemp > 168){
								bTemp = 168;  				// max 7 days
							}
							NVsOpPara.SentinalTM = bTemp;
							GVbSentinelFlush = CheckSentinelSetting();			// reset sentinel flush flag
							SaveParaToFlash(NVsOpPara);		// store to flash
							strcpy(sResponce,sInData);		// set responce data
						}
					break;
					
					case 10: // Flush Volume RO
							
							strcpy(sResponce,NVsBLEPara.sGPF);
							
						break;
					
					// Diagnosis
					
					case 11:	// activate solenoid
						if(*sInData == '1'){ // to turn solenoid on
							bFlushRequest = TRUE;						// request to activate solenoid
							strcpy(sResponce,"1");						// Set responce data
						}
						else{				// just read 
							strcpy(sResponce,"0");						// set responce data
						}
					break;
					
					case 12: // Start diagnosis
						if(*sInData == '1'){ // to start diagnosis
							GVbInDiag = TRUE;							// set flag of unit is in diagnosis
							strcpy(sResponce,"1");						// set responce data
						}
						else{											// just read
							GVbInDiag = FALSE;							// clear flag to let unit work in normal
							strcpy(sResponce,"0"); 						// set responce data
						}	
					break;
					
					case 13:	// BLE Enable/disable
						strcpy(sResponce,sInData);
					break;
					
					case 14:	// Sensor Status					
						if(*sInData == '\0'){     
							strcpy(sResponce,"1");
						}
						else{
							strcpy(sResponce,sInData);
						}
					break;
										
					case 15:	// Solenoid Status					
						if(*sInData == '\0'){
							strcpy(sResponce,"1");
						}
						else{
							strcpy(sResponce,sInData);
						}
					break;
															
					case 16: // battery level (Hex)
						iTemp = NVsOpPara.BVolt;						// get data
						if(iTemp >= FULLBATTERY){						// set limits
							bTemp = 100;
						}
						else{
							if(iTemp <= ENDBATTERY){
								bTemp = 1;
							}
							else{
								bTemp = (iTemp - ENDBATTERY) / STEPS;   // convert to %
							}
						}
						ConvertByteToHexStr(bTemp,sResponce);  // set responce data
					break;
		
				// Enginering Data
					case 17: // Engineering Data 1 (40 byte) RO
						sBLEEngDataROnly = GetEngineeringReadOnlyData();	// BLE engineering data		
						ConvertBlockToHexStr( (uint8*)&sBLEEngDataROnly, sizeof sBLEEngDataROnly, sResponce); 		// set responce data
					break;
					
					case 18: // Engineering Data 2 (20 byte) RW
						if(*sInData == '\0'){ 	// read 
							sBLEEngDataRWrite = GetEngineeringReadWriteData();// BLE related enginerring data
							ConvertBlockToHexStr( (uint8*)&sBLEEngDataRWrite, sizeof sBLEEngDataRWrite, sResponce); // set responce data
						}
						else{ 					// write
							strcpy(sResponce,sInData); //return whatever received
							if(strlen(sInData) == sizeof sBLEEngDataRWrite){	// write
								HexStrToBlock(sInData, (uint8*)&sBLEEngDataRWrite, sizeof sBLEEngDataRWrite);       // convert ascii to data
								SetEngineeringWriteData(sBLEEngDataRWrite);																// set the data active
							}	
						}	
						
					break;
					
					// Statistic Information
					case 19: // Activations in Low battery (HEX 4 byte) RO
						iTemp = NVsOpPara.LBActivationCT;                      	// get the data
						Convert16bitsToHexStr(iTemp, sResponce);				// set the responce
					break;	
										
					case 20: // Total working hours (HEX 8 byte) RO
						// calculate total days
						iTemp = NVsOpPara.RDay;       				// days recorded
						for(i = 0; i< NVsOpPara.RYear;i++){			// add yraes
							iTemp += 365;							// days per year
						}
						// calculate total hours
						lTemp = NVsOpPara.RHour;					// hours
						for(ii=0; ii< iTemp;ii++){					// add days
							lTemp += 24;							// hours per day
						}
						Convert32bitsToHexStr(lTemp, sResponce);	// set responce data
					break;
					
					case 21: //Total Line Flush Activations (HEX 8 byte) RO				
						lTemp = NVsBLEPara.SentinalActivation;       // retrive data
						Convert32bitsToHexStr(lTemp, sResponce);	// set responce data
												
					break;
										
					case 22: //Total Activations (HEX 8 byte) RO
						
						lTemp = NVsOpPara.TotalActivation;			// retrive data
						Convert32bitsToHexStr(lTemp, sResponce);	// set responce data							
					break;
					
					case 23: //Total Reduced Activations (HEX 8 byte) RO
											
						lTemp = NVsBLEPara.ReduceFlushActivation;	// retrive data
						Convert32bitsToHexStr(lTemp, sResponce); 	// set responce data							
					break;

					default:
						strcpy(sResponce,sInData);
					break;
				}
				
				return bFlushRequest;	
}

/*
** =================================================================================================================================
**     Method      : void TestBLECommand(void)
**     Description :
**         This method test excuteing BLE command without BLE installed
**         Note: use this only after BLE has beeen waked up and uart has been set
**     Parameters  : Nothing
**     Returns     : TRUE after "c 15 1 \cr" has been received. It is the last command from BLE at the connection
** =================================================================================================================================
*/
void TestBLECommand(void){	

	char sCmdID[3];		 	// command ID string
	char sData[140];		// incoming data string	
	char sOutData[121];		// response data string
	char sSendout[140];		// whole response string
	uint16 iCmd;
	
		sData[0] = '\0';											
			for(iCmd =0; iCmd <25; iCmd++){
				UTIL1_Num8uToStr((uint8*)sCmdID,5,iCmd);
				switch(iCmd){
		
				// Product Information
					case 6:		// factory reset	
					case 11:	// activate solenoid		
						strcpy(sData,"1");
					break;
					
					case 18:	// enginerring data 2
						strcpy(sData,"12345678901234567890");
					break;
					
					default:
						sData[0] = '\0';	
					break;
				}
				sOutData[0]='\0'; // clear output string 
				ExcuteBLECommandID(iCmd, sData, sOutData);
			// respond to BLE
				sSendout[0]='\0';					// Clear out string
				strcpy((char*)sSendout, "c ");		// first two characters "c "
				strcat((char*)sSendout, sCmdID);	// command ID
				strcat((char*)sSendout, " ");		// Space
				strcat((char*)sSendout, sOutData);	// data
				strcat((char*)sSendout, " \r");		// last two charcters " \r"
				UARTOutStr(sSendout);				// send out
				UARTOutStr("\r\n");
	} 	
}

/*
** ========================================================================================================================================================
**     Method      :  uint8 ProcessBLECommand(void)
**     Description :
**         This method receving and processing a single command from BLE
**         Note: use this only after BLE has beeen waked up and uart has been set.
**         The flag (GVwBLEDone) will be set when "c 13 0 \r" received. it will lead to terminate BLE operation.
**         Once the 'c' is received, it will stay in this function until end of command '0x0d' is received.
**         it exit on time out or received string is out off maximum command length
**     Parameters  : Nothing
**     Returns     : TRUE after "c 13 1 \cr" has been received. It is the last command from BLE after it is waked up and from disable to enabled.
** =========================================================================================================================================================
*/
uint8 ProcessBLECommand(void){	

	char sReceicved[140]; 	// buffer to put incoming command from BLE
	char sCmdID[3];		 	// command ID string
	char sData[140];		// incoming data string	
	char sOutData[121];		// response data string
	char sSendout[140];		// whole response string

	uint8 bBLEInitDown;		// Flag of BLE initialization connection complete
	uint8 bIndex;			// index of receiving char
	uint8 bTemp,bCmdLn,bDataLn;
	uint16 iCmd;			// command ID
	uint8 bFlush;			// flag of requesting activate solenoid
	
	bFlush = FALSE;			// no activation requested
	bBLEInitDown = FALSE;   // default
	iCmd = 0;				// default nothing
	bIndex = 0;				// initilize incoming buffer
	WDReset();				// kick dog

	if(AS1_RecvChar(&bTemp) == ERR_OK){					// Received something 
		
		GVwBLENoActionTimer = 0; 						// reset time out 
		
		if(bTemp == 0x63){								// start with c. It is the beginning of a command
			sReceicved[0] = bTemp;						// move to incoming command buffer
			bIndex = 1;									// next position
			WDReset();									// Kick dog
			wTimerTick2 = 0;							// start to count time
			while(bTemp != 0x0D){						// not the end of command
				WDReset();	
				if(wTimerTick2 >= COMMUNICATIONLIMIT){	// check if time run out
					break; 								// can't receive whole package. break
				}
				if(AS1_RecvChar(&bTemp) == ERR_OK){		// Received something 
					sReceicved[bIndex] = bTemp;			// move to command buffer
					bIndex++;							// next position
					if(bIndex > 130){					// exceed the maxium length 
						break;
					}
				}
			}
			// command format: "c ID DATA 0x0d" DATA may be empity
			if(bIndex >= 5){ // make sure receveing string length is greater than minimum command length
				if((sReceicved[bIndex-1] == 0x0D) && (sReceicved[1] == 0x20) && (sReceicved[bIndex-2] == 0x20)){	// valid format command received
					
					sCmdID[0] = sReceicved[2];				// first character of command
					if(sReceicved[3] == 0x20){				// decode command ID
						bCmdLn = 1;							// one charater command
						sCmdID[1] = '\0';					// add string terminator
					}
					else{	// not the space. it is two character command
						sCmdID[1] = sReceicved[3];			// second caracter of command
						bCmdLn = 2;							// two caracter command
						sCmdID[2] = '\0';					// add string terminator
					}
					
					if(sReceicved[bCmdLn + 3] == 0x0D){		// decode data 
						bDataLn = 0;
						sData[0] = '\0';					// add string terminator
					}
					else{
						bDataLn = bIndex - bCmdLn - 5;		// determine data length. 5 = 'c' + 3 ' ' + 0x0d
						for(bTemp = 0; bTemp < bDataLn; bTemp++){ // setup data string
							sData[bTemp] = sReceicved[bTemp + bCmdLn + 3];
						}
						sData[bDataLn] = '\0';				// add string terminator
					}
					
					iCmd = atoi(sCmdID); 					// Convert from ascii to number
	
					sOutData[0]='\0'; 						// clear output string 

					bFlush = ExcuteBLECommandID(iCmd, sData, sOutData); // excute command
					// respond to BLE
					sSendout[0]='\0';						// Clear out string
					strcpy((char*)sSendout, "c ");			// first two characters "c "
					strcat((char*)sSendout, sCmdID);		// command ID
					strcat((char*)sSendout, " ");			// Space
					strcat((char*)sSendout, sOutData);		// data
					strcat((char*)sSendout, " \r");			// last two charcters " \r"
					UARTOutStr(sSendout);					// send out
					if(bFlush){                         	// Activate solenoid after responcing, otherwise BLE is waiting for
						Flush100ms(GVsTurnOnTM.bStandardFull);				// Turn on solenoid
					}
					// End of BLE operation or complete of BLE initilization
					if(iCmd == 13){					// specical commandID relating BLE itself	
						if(sData[0] == '1'){ 		// BLE initialisation done. It is enabled now
							bBLEInitDown = TRUE;	// BLE would send "c13 1 CR" to indicate initialization completed.
							GVbBLEEnabled = TRUE;	// Only enable once. 
						}				
						if(sData[0] == '0'){ 		// BLE would send "c13 0 CR" to terminate BLE operation
							GVwBLEDone = TRUE;
						}				
					}
				}// end of right command being received 
			}// end of receiving string length is greater than 5
		} // end of receiving 'c'

		AS1_ClearRxBuf();
		AS1_ClearTxBuf();
	} // end of receiving any thing
	return bBLEInitDown;		
}

/*
** ====================================================================================================================================================================
**     Method      :  uint8 BLEInit(void)
**     Description :
**         This method initilize the BLE dongle. It including wake up BLE and responce all the commands from BLE untill BLE is enabled command
**         Note: after BLE donegle waked up , it start to get all the parameters and send BLE enabled at the end of this process.
**         		Once BLE donggle waked up, it may send out request right away. so make sure UART is ready before call this
**     Parameters  : Nothing
**     Returns     : True if intilization sucessful, false otherwise.
** ======================================================================================================================================================================
*/
uint8 BLEInit(void){	

	    UartWake_ClrVal();				// pull down wakeup pin
		//BleWake_ClrVal();				// pull down wakeup pin	
	    
		BleXres_ClrVal();				// 
		DelayMS(10);					// pull dowm 10 ms to wake up BLE.
		
		UartWake_SetVal();				// pull up
		//BleWake_SetVal();				// pull up
		
		BleXres_SetVal();
		wTimerTick = 0;					// reset timer
		TI1_Enable();					// use the timer to prevent endless loop
		while(GVbBLEEnabled != TRUE){ 	// untill BLE enabled command received
			ProcessBLECommand();
			if(wTimerTick >= 5000){		// check if time run out
				TI1_Disable();			// stop the timer
				return FALSE; 			// initialization failed
			}
			WDReset();					// kick dog
		}
		TI1_Disable();					// stop the timer
		return TRUE;
}

/*
** ====================================================================================================================================================================
**     Method      :  uint8 IsBLEInstalled(void)
**     Description :
**         This method check if the BLE dongle installed. 
**         Note: after BLE donegle waked up , it start to get all the parameters and send BLE enabled at the end of this process.
**         		Once BLE donggle waked up, it may send out request right away. so make sure UART is ready before call this
**     Parameters  : Nothing
**     Returns     : True if dongle installed, false otherwise.
** ======================================================================================================================================================================
*/
uint8 IsBLEInstalled(void){	
	uint8 bResult;
	char sReceicved[10];
	uint16 ln;
	
	    UartWake_ClrVal();				// pull down wakeup pin 
		//BleWake_ClrVal();				// pull down wakeup pin
		
		BleXres_ClrVal();				// 
		DelayMS(10);					// pull dowm 10 ms to wake up BLE.
		
		UartWake_SetVal();				// pull up
		//BleWake_SetVal();				// pull up
		
		BleXres_SetVal();
		wTimerTick = 0;					// reset timer
		bResult =  FALSE;
		TI1_Enable();					// use the timer to prevent endless loop
		while(AS1_GetCharsInRxBuf() < 5){   	// wait for a command
			WDReset();							// kick dog
			if(wTimerTick >= 1000){			// more than a second
				TI1_Disable();	//
				return FALSE;
			}
		}
		AS1_RecvBlock((uint8*)sReceicved, 5, &ln);
		if((sReceicved[0] == 0x63) && (sReceicved[1] == 0x20)){ // right first two chars from BLE
			bResult = TRUE;
		}
		TI1_Disable();					// stop the timer
		return bResult;
}

/*
** =================================================================================================================================
**     Method      :  void BLEProcess(void)
**     Description :
**         This method process commands from BLE.
**         Note: BLE would go to advitising after collectting all data at the moment being waked up.
**         
**     Parameters  : Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/
void BLEProcess(void){	
	
	if(GVbBLEEnabled == FALSE){			// not enabled yet. It is the first time 
		if(BLEInit() == FALSE){			// Initialize BLE,terminate if initilization failed
			GVwBLEDone = TRUE;			// terminate BLE operation
		}
	}
	else{ //BLE already enabled
		ProcessBLECommand(); 			// Check and process if any command comes from BLE. 
	}				
}
