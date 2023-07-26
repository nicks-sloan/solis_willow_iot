/*
 * Ant.c
 *
 *  Created on: Mar 21, 2023
 *      Author: WANGS1
 */
#include "Ant.h"
#include "UART.h"
#include "Operation.h"
#include "PowerSupply.h"
#include "Timing.h"
#include "TI1.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


uint8 GVbMultiPDUPart;  	// multiple PDU command part 1 or 2
uint8 GVbMultiPDULocCnt; 	//  Store Part two array location 
uint8 GVbBatLevelStore; 	//  Save battery level for compare
uint8 GVbBatLevelChkCnt; 	//  number of battery level compare 10/28/20
uint8 GVbBatLowChkCnt; 		//  number of battery low compare  7/11/23
uint16 GVbActRptThold; 		//  store activation reporting threshold
uint16 GVbActRptTholdCnt; 	//  store activation reporting threshold count number

uint8 GVbCmdListArr[41] = {SERIALNUM, ANTSKU, SENSORRANGE, MRTOTIMER, FLUSHACT, LPMGPF, SENSORSTAT, VALVESTAT, BATLEVEL, OPHRSINCEINSTALL,
		ACTSINCEINSTALL, SRSTATUS, MULTIPDU, FLUSHACTREQUEST, ACTRPTTHOLD, OCCUPANCY, FEATUREENABLE, MFGDATE, HWREVISION, FMWREVISION, FACTORYRESET, MODESELECTION,
		FLUSHSENTINTIME, DIAGNOSISINIT, BLEENABLE, TURBINESTAT, SOLARPANELSTAT, ARMTIMER, SITTINGUSRTHOLD, BGCHANGETHOLD,
		IRTGTTHOLD, IRUPDATETHOLD, MAXIRTHOLD, TOFDISTCALFACTOR, TOFOFFSETCALFACTOR, DISTADJCNT, DISTADJFAILCNT,
		SALKDNIRRECCAL, SALKDNIRMFGCAL, SALKDNBGCURUSED, DNTLSTFMWUPDATE};
uint8 GVbUartCmdStateNerrFlg;  	// 0 - Unused, 1 - SR Request time out, 2 - SR Request payload exceed, 3 - SR Request invalid checksum, 
								// 4 - SR Request invalid cmd, 5 - Cmd multipdu received, 6 - SR Response ACK time out,
								// 7 - SR Response ACK payload exceed, 8 - SR Response Ack invalid cmd, 9 - SR Response ACK retry twice fail
								// 10 - SR Response ACK received
uint8 GVbUARTSensorCMDNerrFlg; 	// 0 - Unused, 1 - SR Reponse ACK time out, 2 - SR Reponse ACK payload exceed, 3 - Unused 
								// 4 - SR Response ACK invalid cmd, 5 - SR Response ACK retry twice fail
char GVbUARTInCMDList[97];  // store cmds for Multi PDU cmd 
//uint8 GVbGetSRStatFlag;					// get SR status flag //remove 4/7/23
uint8 GVbFactoryRestFlg; 	// Factory reset flag


/*
** =================================================================================================================================
**     Method      :uint8 isCmdInCmdlist(uint8 cmd)
**     Description :
**         This method check if cmd is in in cmd list
**     Parameters  : cmd - command
**     Returns     : 1 for True, 0 for false
** =================================================================================================================================
*/
uint8 isCmdInCmdlist(uint8 cmd)
{
   int i;
   for (i=0; i< sizeof(GVbCmdListArr); i++)
   {
	 if (GVbCmdListArr[i] == cmd)
	 {
	    return 1;  /* it was found */
	 }
   }
   return 0;  /* if it was not found */
}
	
/*
** =================================================================================================================================
**     Method      :uint8 batConvert16to8(void)
**     Description :
**         This method convert 16 bit to 8 bit battery level
**     Parameters  : None
**     Returns     : 1 byte battery level
** =================================================================================================================================
*/
uint8 batConvert16to8(void){
	uint8 bTemp;
	uint16 iTemp = NVsOpPara.BVolt;
	if(iTemp >= FULLBATTERY){
		bTemp = 100;
	}
	else{
		if(iTemp <= ENDBATTERY){
			bTemp = 1;
		}
		else{
			bTemp = (iTemp - ENDBATTERY) / STEPS;
		}
	}
	return bTemp;
}
	

/*
** =================================================================================================================================
**     Method      :calculateChecksum(char *arr, int StartIndex, int Count)
**     Description :
**         This method calculate 1-byte check-sum of incoming message.
**     Parameters  : point to the string
**     			   : start index
**     			   : end count
**     Returns     : sum of checksum
** =================================================================================================================================
*/

uint8 calculateChecksum(char *arr, int StartIndex, int Count)
{
	uint8 sum = 0;
	uint8 i = 0;

	for(i = StartIndex; i < Count; i++)
	{
		sum += arr[i];
		sum &= 0xFF;
	}
	
	sum = ~ sum;
	sum &= 0xFF;
	sum ++;
	sum &= 0xFF;
	
	return sum;
}

/*
** =================================================================================================================================
**     Method      :UARTOutStrPlusNull(uint8* src, int buflen)
**     Description :
**         This method sends out input string and Null (0x00)
**     Parameters  : point to the string
**     				: length of string
**     Returns     : Nothing
** =================================================================================================================================
*/
void UARTOutStrPlusNull(char* src, int buflen){
	uint16 len;	// length of string
	uint16 sd; 	//number char sent
		WDReset();								// Kick dog
		//len = strlen(src);  // come out cause 0x00 problem
		//len = sizeof(src);  // solve problem when transmit 0x00  // stop after 0x00
		len = buflen;
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
**     Method      :  uint8 SensorRequestUARTHandler(uint8 CmdID,  int payload, char *sOutPayloadData)
**     Description :
**         This method send and process a single request or response command from UART to RF Dongle
**         Note: use this only after RF Dongle and UART been set
**     Parameters  : CmdID, payload, sOutPayloadData
**     Returns     : TRUE if ACK received from RF Dongle
** =================================================================================================================================
*/

uint8 SensorRequestUARTHandler(uint8 CmdID,  int payload, char *sOutPayloadData){	 // Sensor -> SR SIM Board	
	bool IsACKReceived = FALSE;
	bool chkACKResponse = FALSE;
	bool GetSRresponse = FALSE;	
	bool EndByteReceived = FALSE;  // version 3 to check for F8 F8 checksum and end byte
	bool reTryAck = TRUE;
	uint8 reTryCnt = 0;
	
	uint8 bTemp;
	uint8 bIndex;
	
	char sInUARTArrCmd[20];
	char sOutUARTRqCmd[97];		// whole request string
	
	char sOutCmdID[2];
	char sOutMsgLength[2];		// 1 byte message length out

	uint8 bOutPayloadLength;	// 1 byte length of payload (calculated)
	uint8 bOutCheckSumLengthResponse; 	// 1 byte payload length + msg length + command ID 
	uint8 i;
	
	bOutPayloadLength = payload;
	
	// request to RF Dongle begin
	//bOutPayloadLength is known 
	//boutPayloadlength = 1 ,2 
	sOutCmdID[0] = CmdID;
	sOutMsgLength[0]= bOutPayloadLength + 1;     //2 , 3
	bOutCheckSumLengthResponse = 3 + bOutPayloadLength; // 4, 5
					
	sOutUARTRqCmd[0]='\0';					// Clear out string
	sOutUARTRqCmd[0] = STARTBYTE;
	sOutUARTRqCmd[1] = sOutMsgLength[0];  //2, 3
	sOutUARTRqCmd[2] = sOutCmdID[0];     // 3A

	// put out payload data
	for(i = 0; i < bOutPayloadLength; i++){
		sOutUARTRqCmd[3+i] = sOutPayloadData[i];   //[3]; [3],[4]
	} 
	sOutUARTRqCmd[bOutPayloadLength+3] = calculateChecksum(sOutUARTRqCmd,1,bOutCheckSumLengthResponse);  //[4] 4; [5], 5
	sOutUARTRqCmd[bOutPayloadLength+4] = ENDBYTE;   //[5]; [6]
	
	while (reTryAck){

		UartWakeInt_Disable(); // disable interrupt
		
		/* Check Wake up pin high before send 4/4/23 --mod 4/10/23*/
		if(UartWake_GetVal()){
			UartWake_SetOutput(); //set UartWake to output (pin 27)
						
			//begin 50ms pulse high to low turn on SR RF 
			UartWake_ClrVal();
			DelayMS(50);			// pull up 50 ms to wake up BLE
			UartWake_SetVal();   // pull up
			DelayMS(50);	
			UartWake_ClrVal(); // Pull low
			DelayMS(50);

			UARTOutStrPlusNull(sOutUARTRqCmd,bOutCheckSumLengthResponse+2);  // 6; 7   // sent request
			
			UartWake_SetVal();   // pull up 
			UartWake_SetInput(); // reset UartWake as input (pin 27) 

		} else
		{
			UARTOutStrPlusNull(sOutUARTRqCmd,bOutCheckSumLengthResponse+2);  // 6; 7   // sent request
		}
		//--
		
		UartWakeInt_Enable(); // re enable interrupt
		
//		//--org code --mod 4/10/23
//		UartWakeInt_Disable(); // disable interrupt
//		UartWake_SetOutput(); //set UartWake to output (pin 27)
//					
//		//begin 50ms pulse high to low turn on SR RF 
//		UartWake_ClrVal();
//		DelayMS(50);			// pull up 50 ms to wake up BLE
//		UartWake_SetVal();   // pull up
//		DelayMS(50);	
//		UartWake_ClrVal(); // Pull low
//		DelayMS(50);
//
//		UARTOutStrPlusNull(sOutUARTRqCmd,bOutCheckSumLengthResponse+2);  // 6; 7   // sent request
//		
//		UartWake_SetVal();   // pull up 
//		UartWake_SetInput(); // reset UartWake as input (pin 27) 
//		UartWakeInt_Enable(); // re enable interrupt
//		//-- org code end
		
		//if (SendComplete == 0 ){
			chkACKResponse = TRUE;
			bIndex = 0;				// initilize
			WDReset();				// Kick dog
			wTimerTick = 0;			// reset timer
			TI1_Enable();
		//}
		
		
		while (chkACKResponse){  // get ACK from Sensor Request - SR Sim Board  ->  Sensor
			WDReset();
			if(AS1_RecvChar(&bTemp) == ERR_OK){					// Received something 
				if(bTemp == STARTBYTE){							// start with 0xF7, start byte of message(sheng) 0x63 pre
					sInUARTArrCmd[bIndex] = bTemp;				// move to incoming command buffer
					bIndex++;									// next position
					WDReset();									// Kick dog
					wTimerTick = 0;								// reset timer
					//EndByteReceived = FALSE;					// 9/8/20 End byte received false
					TI1_Enable();								// Enable timer to terminate if something wrong
	
					//while((bTemp != ENDBYTE) && !EndByteReceived){					// not the end of command, 0xF8 end byte(sheng)0x0D pre
					while(bTemp != ENDBYTE){					// not the end of command, 0xF8 end byte(sheng)0x0D pre
						WDReset();
						if(wTimerTick >= TIMERWAITLIMIT){		// check if time run out (300ms)
							GVbUARTSensorCMDNerrFlg = 1;  		//SR Response ACK time out
							break; 								// can't receive whole package. break
						}
						if(AS1_RecvChar(&bTemp) == ERR_OK){		// Received something 
							sInUARTArrCmd[bIndex] = bTemp;		// move to command buffer
							bIndex++;							// next position
							
						/*	// added 9/8/20 version 3  to fix F8 F8 when checksum same as F8 of endbyte
							if((bTemp == ENDBYTE) && ((sInUARTArrCmd[1] + 4) == bIndex))			
							{
								EndByteReceived = TRUE;
							}*/
							
							if(bIndex > 19){					// exceed the maxium length 
								GVbUARTSensorCMDNerrFlg = 2; 	//SR Response ACK payload exceed,
								break;
							}
						}
					}											// end of command or time out
						
					TI1_Disable();								// stop timer to save power and 
							
					if((sInUARTArrCmd[0] == STARTBYTE) && (sInUARTArrCmd[bIndex-1] == ENDBYTE) && (sInUARTArrCmd[1] == sOutUARTRqCmd[2])){ // got ACK Response
						
						if (sInUARTArrCmd[1] == SRSTATUS) // sr dongle status
						{
							// rf dongle response  - SR Sim Board -> sensor
							GetSRresponse = TRUE;
							sOutCmdID[0] = CmdID + 0x80;
							bIndex = 0;				// initilize
							WDReset();									// Kick dog
							wTimerTick = 0;								// reset timer
							TI1_Enable();
							
							while (GetSRresponse){
								WDReset();	
								if(AS1_RecvChar(&bTemp) == ERR_OK){					// Received something 
									if(bTemp == STARTBYTE){								// start with 0xF7, start byte of message(sheng) 0x63 pre
										sInUARTArrCmd[bIndex] = bTemp;					// move to incoming command buffer
										bIndex++;									// next position
										WDReset();									// Kick dog
										wTimerTick = 0;								// reset timer
										EndByteReceived = FALSE;					// 9/8/20 End byte received false
										TI1_Enable();								// Enable timer to terminate if something wrong
	
										while((bTemp != ENDBYTE) && !EndByteReceived){						// not the end of command, 0xF8 end byte(sheng)0x0D pre
											WDReset();	
											if(wTimerTick >= TIMERWAITLIMIT){					// check if time run out (300ms)
												//chkACKResponse = FALSE;
												//IsACKReceived = FALSE;
												break; 								// can't receive whole package. break
											}
											if(AS1_RecvChar(&bTemp) == ERR_OK){		// Received something 
												sInUARTArrCmd[bIndex] = bTemp;			// move to command buffer
												bIndex++;							// next position
												
												// added 9/8/20 version 3  to fix F8 F8 when checksum same as F8 of endbyte
												if((bTemp == ENDBYTE) && ((sInUARTArrCmd[1] + 4) == bIndex))			
												{
													EndByteReceived = TRUE;
												}
												
												if(bIndex > 19){					// exceed the maxium length 
													//chkACKResponse = FALSE;
													//IsACKReceived = FALSE;
													break;
												}
											}
										} // end while(bTemp											
												
										TI1_Disable();								// stop timer to save power and 
													
										if((sInUARTArrCmd[0] == STARTBYTE) && (sInUARTArrCmd[bIndex-1] == ENDBYTE) && (sInUARTArrCmd[2] == sOutCmdID[0])){ // got Response
											GVbSRModuleStatus = sInUARTArrCmd[4]+0;
											
											// sent Response ACK  - sensor -> SR Sim Board  
											sOutUARTRqCmd[0]='\0';
											sOutUARTRqCmd[0] = STARTBYTE;								
											sOutUARTRqCmd[1] = sOutCmdID[0];
											sOutUARTRqCmd[2] = ENDBYTE;
											
											UartWakeInt_Disable(); // disable interrupt -- add 4/10/23
											
											/* Check Wake up pin high before send 4/4/23 -mod 4/10/23*/
											if(UartWake_GetVal()){
												//begin 50ms pulse high to low turn on SR RF 
												UartWake_ClrVal();
												DelayMS(50);			// pull up 50 ms to wake up BLE
												UartWake_SetVal();   // pull up
												DelayMS(50);	
												UartWake_ClrVal(); // Pull low
												DelayMS(50);
	
												UARTOutStrPlusNull(sOutUARTRqCmd,3);
												
												UartWake_SetVal();   // pull up 
												UartWake_SetInput(); // reset UartWake as input (pin 27) 
											}else
											{
												UARTOutStrPlusNull(sOutUARTRqCmd,3);
											}
											//--
											
											//org code -mod 4/10/23
//											//begin 50ms pulse high to low turn on SR RF 
//											UartWake_ClrVal();
//											DelayMS(50);			// pull up 50 ms to wake up BLE
//											UartWake_SetVal();   // pull up
//											DelayMS(50);	
//											UartWake_ClrVal(); // Pull low
//											DelayMS(50);
//
//											UARTOutStrPlusNull(sOutUARTRqCmd,3);
//											
//											UartWake_SetVal();   // pull up 
//											UartWake_SetInput(); // reset UartWake as input (pin 27) 
											//org code end
											
											UartWakeInt_Enable(); // re enable interrupt
											
											GetSRresponse = FALSE;
											IsACKReceived = TRUE;
											chkACKResponse = FALSE;
											reTryAck = FALSE;
										}else
										{
											GetSRresponse = FALSE;
											IsACKReceived = FALSE;
											chkACKResponse = FALSE;
											reTryAck = FALSE;
										}
									}
								}else
								{
									if(wTimerTick >= TIMERWAITLIMIT){
										GetSRresponse = FALSE;
										IsACKReceived = FALSE;
										chkACKResponse = FALSE;
										reTryAck = FALSE;
										break; 								// can't receive whole package. break
									}
								}	
							} // end while (GetSRreponse)	
						}else
						{
							IsACKReceived = TRUE;
							chkACKResponse = FALSE;
							reTryAck = FALSE;
						}
						
						//IsACKReceived = TRUE;
						//chkACKResponse = FALSE;
						//reTryAck = FALSE;
					} else //if((sInUARTArrCmd[0] == STARTBYTE)
					{
						GVbUARTSensorCMDNerrFlg = 4; 	// SR Reponse ACK invalid cmd
						//IsACKReceived = FALSE;
						chkACKResponse = FALSE;
						reTryCnt++;
					}
				}		
			} else //if(AS1_RecvChar(&bTemp)
			{
				if(wTimerTick >= TIMERWAITLIMIT){
					GVbUARTSensorCMDNerrFlg = 1; 	// SR Reponse ACK time out
					//IsACKReceived = FALSE;
					chkACKResponse = FALSE;
					reTryCnt++;
					break; 							// can't receive whole package. break
				}
			}	
				 
		} // end while (chkACK
		
		if(reTryCnt > 1)
		{
			IsACKReceived = FALSE;
			GVbUARTSensorCMDNerrFlg = 5; 	// SR Response ACK retry twice fail
			reTryAck = FALSE;
		}
	} // end while (reTryACK)
	
	return IsACKReceived;	
}

/*
** =================================================================================================================================
**     Method      :  void UARTProcessSendRequest(char* CmdID, int outData){	
**     Description :
**         This method process commands from UART
**         Note: BLE would go to advitising after collectting all data at the moment being waked up.
**         
**     Parameters  : cmdId - command Id,  OutData - 1 byte, 1 or 0 or battery level
**     Returns     : Nothing
** =================================================================================================================================
*/

void UARTProcessSendRequest(uint8  CmdID){	//Sensor -> SR SIM board
 
	char sOutPayloadData[97];		// response data string  ( max 96 bytes, 1 byte to null)
	uint32 lTemp;
	
	uint8 bTemp;
	char cTemp[11], cTempDate[11];
	char arr[4];
	int k;			
	uint8 i,j,l;
	uint16 iTemp,ii;
	
	bool isValidCmd = TRUE;
	uint8 bOutPayloadLength;
	
	
	switch(CmdID){
		
		case MULTIPDU:	// all command Read Write Request (max command 8)
			
			i = 0; // main counter for sOutPayloadData
			
			uint8 PduCmdID;
			uint8 CmdNum;
			uint8 CmdCnt;
			uint8 c, t, n;
			bool isReadCmd = FALSE;
			bool validCmd = TRUE;
			
			 //get the number of command to be sent
			 if (GVbMultiPDUPart == 1)
			 {		 
				 if (GVbUARTInCMDList[0] <= MULTIPDUCMDLP) //5) 4/11/23 increase to allowable 8 cmd
				 {
					sOutPayloadData[i] = GVbUARTInCMDList[0];  // num of comands
				 } else
				 {
					sOutPayloadData[i] = MULTIPDUHEXCMD;  //0x05; 4/11/23 increase to 8 cmd
					
				 }	 
			 }else
			 {
				 if (GVbMultiPDUPart == 2)
				 {
					 if(GVbUARTInCMDList[0] > 10)
					 {
						sOutPayloadData[i] = MULTIPDUHEXCMD;  //0x05; 4/11/23 increase to 8 cmd 
					 } else
					 {
						 sOutPayloadData[i] = GVbUARTInCMDList[0] - MULTIPDUHEXCMD;  //0x05; 4/11/23 increase to 8 cmd 
					 }
					 
				 }
			 }
			 
			 CmdNum = sOutPayloadData[0];  // temp store number of cmds
			 CmdCnt = 0; // keep track counter for cmd actual read
			 
			 if (GVbMultiPDUPart == 2)
			 {
				 n = GVbMultiPDULocCnt;
			 }else
			 {
				 n = 1;	//counter for stepping thru  GVbUARTInCMDList 
			 }
			 
			 for(c = 0; c < CmdNum; c++){  // loop for number of cmd needed
				 
				 for(t = 0; t < 2; t++) // loop thru 2 placement in GVbUARTInCMDList array
				 {
					 if (t == 0)
					 {
						 if(GVbUARTInCMDList[n] == 0x01){  // check for read (0x01)
							isReadCmd = TRUE; 
						 }
						 n++;						         
					 }else
					 {
						 if(isReadCmd)  // if read enable create cmd id structure
						 {
							 PduCmdID = GVbUARTInCMDList[n];
							 switch(PduCmdID){				 													
								case SERIALNUM:	// Serial number 1 (0x01) **
									cTemp[0]= '\0';
									strcpy(cTemp, NVsBLEPara.sBoardSN); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = SERIALNUM;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case MFGDATE:	// Manufacturing Date 	
									cTemp[0]= '\0';
									strcpy(cTemp, NVsBLEPara.sBoardDATE); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = MFGDATE;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
									
								case HWREVISION:	// Hardware Revision 	
									cTemp[0]= '\0';
									strcpy(cTemp, NVsBLEPara.sBoardREV); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = HWREVISION;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case FMWREVISION:	// Firmware Revision
									cTemp[0]= '\0';
									strcpy(cTemp,VERISION_MAJOR);
									strcat(cTemp,VERISION_MINOR);
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = FMWREVISION;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;			
								case ANTSKU:	// SkU (0x05)**
									cTemp[0]= '\0';
									//bTemp = (uint8)atoi(NVsBLEPara.sBoardSKU) + 30;
									bTemp = (uint8)atoi(NVsBLEPara.sBoardSKU);
									UTIL1_Num8uToStr((unsigned char *)cTemp, 3, bTemp ); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = ANTSKU;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
							
									break;
								
								/*
								// Delphian take care
								#define RFSERIALNUM	    6 		// RF Dongle Serial Number        --Don't need to implement on device
								#define RFMFGDATE		7 		// RF Dongle Manufacture Date     --Don't need to implement on device
								#define RFHWREVISION	8 		// RF Dongle hardware version     --Don't need to implement on device
								#define RFFMWREVISION	9 		// RF dongle firmware version     --Don't need to implement on device
								 */									
																
								case FACTORYRESET:	// Factory Reset  	
									sOutPayloadData[++i] = 0x02;  //length not include length byte
									sOutPayloadData[++i] = FACTORYRESET;  //command
									sOutPayloadData[++i] = 0x30;
									
									break;
									
								case SENSORRANGE:	// SENSOR RANGE (0x0B)**
									
									cTemp[0]= '\0';
									strcpy(cTemp, NVsBLEPara.sSensorRange); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = SENSORRANGE;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case MODESELECTION:	// Solis "0": closet; "1": urinal; "2": urinal with ballpark  	
										
									cTemp[0]= '\0';  
									UTIL1_Num8uToStr((unsigned char *)cTemp, 2, NVsOpPara.Mode );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = MODESELECTION;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case MRTOTIMER:	// Open Timer (flushometer) (0x0D) **
									bTemp = NVsOpPara.OpenTM;
									arr[0]= '\0';
									for(k = 0; k < 3; k++)
									{
										if (k != 1)
										{ 
											arr[2-k] = (bTemp% 10) + 48;
											bTemp /= 10;
										} else
										{
											arr[k] = 0x2E;
										}
									}
									arr[k]= '\0';
									sOutPayloadData[++i] = 0x04;  //length not include length byte
									sOutPayloadData[++i] = MRTOTIMER;  //command
									cTemp[0]= '\0';
									strcpy(cTemp, arr);				
									for(j = 0; j < 3; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								/*	case FLUSHACT: // Flush Activate (66 POC1)
										bOutPayloadLength = 0;
										if( sInUARTArrCmd[3] == 0x31)
										{
											//LED_SetVal();
											DelayMS(200); // add to get rid of double flush after flush most important
											//LED_ClrVal();
											
											GVbFlushActivate = 1;
										}
										break;
								*/								
								case FLUSHSENTINTIME:	// Sentinel Time    	
									cTemp[0]= '\0';  
									UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.SentinalTM );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = FLUSHSENTINTIME;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;	
								case LPMGPF:	// GPF Value (flushometer) (0x10)**
									
									cTemp[0]= '\0';
									strcpy(cTemp, NVsBLEPara.sGPF); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = LPMGPF;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									
									break;
								case DIAGNOSISINIT:	// Diagnosis Init  	
									cTemp[0]= '\0';  
									UTIL1_Num8uToStr((unsigned char *)cTemp, 2, GVbInDiag );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = DIAGNOSISINIT;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									
									break;
			/*	Write only		case BLEENABLE:	// BLE Enable 	
									bOutPayloadLength = 4;
									
									break;	
			*/					
								case SENSORSTAT:	// Sensor Status (0x13)**
									sOutPayloadData[++i] = 0x02;  //length not include length byte
									sOutPayloadData[++i] = SENSORSTAT;  //command
									//if (GVeUserSts == NOTPRESENT)
									//{
									//	sOutPayloadData[++i] = 0x30;
									//}
								//	else{
										sOutPayloadData[++i] = 0x31;
										//sOutPayloadData[++i] = 0x30; //testing
									//}
									break;
								case VALVESTAT:	// Valve Status (0x14)**
									sOutPayloadData[++i] = 0x02;  //length not include length byte
									sOutPayloadData[++i] = VALVESTAT;  //command
									//if (GVbSolenoidSts == SOLENOIDON)
									//{
										sOutPayloadData[++i] = 0x31;
										//sOutPayloadData[++i] = 0x30; //testing
									//}
									//else{
									//	sOutPayloadData[++i] = 0x30;
									//}												
									break;
								/*case TURBINESTAT:	// Turbine Status   	
									sOutPayloadData[++i] = 0x02;  //length not include length byte
									sOutPayloadData[++i] = TURBINESTAT;  //command
									sOutPayloadData[++i] = 0x30;
									
									break;*/
								case SOLARPANELSTAT:	// Solar Panel Status 	
									sOutPayloadData[++i] = 0x02;  //length not include length byte
									sOutPayloadData[++i] = SOLARPANELSTAT;  //command
									sOutPayloadData[++i] = 0x31;
									break;		
								case BATLEVEL:	// Battery Level (0x17)**
									bTemp = batConvert16to8();
									GVbBatLevelStore = bTemp;
									arr[0]= '\0';
									if(bTemp >= 100)
									{
										k = 3;
										arr[2] = 0x30;
										arr[1] = 0x30;
										arr[0] = 0x31;
									}else 
									{
										UTIL1_Num8uToStr((unsigned char *)arr, 3, bTemp );
										k = strlen(arr);					
									}
										
									sOutPayloadData[++i] = ++k;  //length not include length byte
									sOutPayloadData[++i] = BATLEVEL;  //command
									cTemp[0]= '\0';
									strcpy(cTemp, arr);				
									for(j = 0; j < (k-1); j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case OPHRSINCEINSTALL:	// Operation Hours since install (0x18)**
									// total days
									iTemp = NVsOpPara.RDay;
									for(k=0; k < NVsOpPara.RYear;k++){
										iTemp += 365;
									}
									
									lTemp = NVsOpPara.RHour;
									for(ii=0; ii< iTemp;ii++){
										lTemp += 24;
									}
									cTemp[0]= '\0';
									UTIL1_Num32uToStr((unsigned char *)cTemp, 9, lTemp );
									l = strlen(cTemp);
									sOutPayloadData[++i] = ++l; //l +1;  //length not include length byte
									sOutPayloadData[++i] = OPHRSINCEINSTALL;  //command
									for(j = 0; j < (l-1); j++){
										sOutPayloadData[++i] = cTemp[j];  // store into output array
									}																
									break;
								case ACTSINCEINSTALL:	// Activation since install (0x19)**
									lTemp = NVsOpPara.TotalActivation;
									cTemp[0]= '\0';
									UTIL1_Num32uToStr((unsigned char *)cTemp, 9, lTemp );
									l = strlen(cTemp);
									sOutPayloadData[++i] = ++l; // l +1;  //length not include length byte
									sOutPayloadData[++i] = ACTSINCEINSTALL;  //command
									for(j = 0; j < (l-1); j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}				
									break;
									
								/*
								//Handle on Argos
								#define DNTLSTFACTRESET	26 		// Date & time of last factory reset
								#define DNTLSTRANGECHG	27 		// Date & time of last range change
								#define DNTLSTMODECHG	28 		// Date & time of last mode change
								#define DNTLSTSENTINALCHG 29 	// Date & time of last Sentinel change
								#define DNTLSTDIAG		30 		// Date & time of last diagnostic
								*/
								
								/* Don't need
								 * case SRSTATUS: // RF Dongle Status Update
									bOutPayloadLength = 0;
									GVbSRModuleStatus = sInUARTArrCmd[3]+0;
									break;		
								
								case MULTIPDU: // All command w request
									bOutPayloadLength = 0;
									DelayMS(200);
									GVbSRRefreshCmd = 1;
									break;
								case FLUSHACTREQUEST: // Flush Activate Request
									bOutPayloadLength = 1;
									sOutPayloadData[0] = 0x31;
									break;
								case ACTRPTTHOLD: // RF Dongle Status Update
									bOutPayloadLength = 0;
									GVbActRptThold = atoi(sInPayloadData);
									break; 
								*/
								case OCCUPANCY:
									sOutPayloadData[++i] = 0x02;  //length not include length byte
									sOutPayloadData[++i] = OCCUPANCY;  //command
									if(GVbOccupState == 1)
									{
										sOutPayloadData[++i] = 0x31;  // enter zone
									}else {
										sOutPayloadData[++i] = 0x32; // vacant
									}
									break;
								case FEATUREENABLE:
									sOutPayloadData[++i] = 0x03;  //length not include length byte
									sOutPayloadData[++i] = FEATUREENABLE;  //command
									if(NVsANTPara.iFeatureOccupEnDisable == 1)
									{
										// 11 = occupancy on
										sOutPayloadData[++i] = 0x31;  
										sOutPayloadData[++i] = 0x31; 
									}else {
										// 01 occupancy 0ff
										sOutPayloadData[++i] = 0x30;  
										sOutPayloadData[++i] = 0x31;
									}
									break;		
								case ARMTIMER:	// Arm Timer   	RW
									cTemp[0]= '\0';  
									UTIL1_Num8uToStr((unsigned char *)cTemp, 3, NVsOpPara.ArmTM );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = ARMTIMER;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
							/*	Not for MVP1 but coded
							 	case PORRESETCNT:	// POR Reset Counter	RO
									cTemp[0]= '\0';
									UTIL1_Num16uToStr((unsigned char *)cTemp, 3, NVsOpPara.PORResetCT );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = PORRESETCNT;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
									/////pick up here
								case UIRCURRATIO:			// Urinal IR Current Ratio	RO
									cTemp[0]= '\0';   
									UTIL1_Num8uToStr((unsigned char *)cTemp, 2, GVsTempPara.UrinalIR );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = UIRCURRATIO;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;  
								case NOISEFLOOR:			// Noise Floor	RW

									cTemp[0]= '\0';
									strcpy(cTemp, dvarNOISEFLOOR); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = NOISEFLOOR;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									
									break;
								case ECHODIFFERENCE:		// Echo difference	 RW	
									cTemp[0]= '\0';
									strcpy(cTemp, dvarECHODIFFERENCE); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = ECHODIFFERENCE;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									
									break;
								case TGTINTHOLD:			// Target in threshold	RW
									cTemp[0]= '\0';
									strcpy(cTemp, dvarTGTINTHOLD); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = TGTINTHOLD;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case TGTOUTTHOLD:			// Target out threshold	RW
									cTemp[0]= '\0';
									strcpy(cTemp, dvarTGTOUTTHOLD); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = TGTOUTTHOLD;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case TGTSTAYINTHOLD:		// Target Stay in threshold	RW	
									cTemp[0]= '\0';
									strcpy(cTemp, dvarTGTSTAYINTHOLD); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = TGTSTAYINTHOLD;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case TGTSTAYOUTTHOLD:		// Target stay out threshold RW		
									cTemp[0]= '\0';
									strcpy(cTemp, dvarTGTSTAYOUTTHOLD); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = TGTSTAYOUTTHOLD;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case TGTSTAYOUTTHOLD1:	// Target stay out threshold RW		
									cTemp[0]= '\0';
									strcpy(cTemp, dvarTGTSTAYOUTTHOLD1); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = TGTSTAYOUTTHOLD1;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case TGTBACKTHOLD:		// Target back threshold RW		
									cTemp[0]= '\0';
									strcpy(cTemp, dvarTGTBACKTHOLD); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = TGTBACKTHOLD;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case TGTSTAYTHOLD:		// Target stay threshold RW	
									cTemp[0]= '\0';
									strcpy(cTemp, dvarTGTSTAYTHOLD); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = TGTSTAYTHOLD;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case BACKTOIDLETHOLD:		// Back to Idle threshold RW	
									cTemp[0]= '\0';
									strcpy(cTemp, dvarBACKTOIDLETHOLD); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = BACKTOIDLETHOLD;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								*/
									
								case SITTINGUSRTHOLD:		// Sitting user threshold; Solis = NVsOpPara.MinUserTH RW	
									cTemp[0]= '\0';
									//strcpy(cTemp, dvarSITTINGUSRTHOLD); 
									UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.MinUserTH );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = SITTINGUSRTHOLD;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case BGCHANGETHOLD:		// Background change threshold; Solis = NVsOpPara.TouchTH RW	
									cTemp[0]= '\0';
									//strcpy(cTemp, dvarBGCHANGETHOLD); 
									UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.TouchTH );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = BGCHANGETHOLD;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case IRTGTTHOLD:			// IR Target Threshold; Solis = NVsOpPara.MaxBackground RW
									cTemp[0]= '\0';
									//strcpy(cTemp, dvarIRTGTTHOLD); 
									UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.MaxBackground );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = IRTGTTHOLD;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case IRUPDATETHOLD:		// IR update threshold; Solis = NVsOpPara.ONDelayTM RW	
									cTemp[0]= '\0';
									//strcpy(cTemp, dvarIRUPDATETHOLD); 
									UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.ONDelayTM );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = IRUPDATETHOLD;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
							/* Not for MVP1 but coded		
								case IRCHANGETHOLD:		// IR change Threshold RW		
									cTemp[0]= '\0';
									strcpy(cTemp, dvarIRCHANGETHOLD); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = IRCHANGETHOLD;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
							 */
								case MAXIRTHOLD:			// Maximum IR Threshold	R
									cTemp[0]= '\0';
									UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.MaxIRTH ); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = MAXIRTHOLD;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
							/* not for MVP1 but coded		
								case MINIRTHOLD:			// Minimum IR Threshold	R
									cTemp[0]= '\0';
									UTIL1_Num16uToStr((unsigned char *)cTemp, 5, NVsOpPara.MinIRTH ); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = MINIRTHOLD;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case MINBG:				// Minimum background RW
									cTemp[0]= '\0';
									strcpy(cTemp, dvarMINBG); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = MINBG;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}					
									break;
							*/
								case TOFDISTCALFACTOR:	// Time of flight distance calibration factor; Solis: Calibration flag R			
									cTemp[0]= '\0';
									UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.CalibrationFlag );								
									//strcpy(cTemp, dvarTOFDISTCALFACTOR); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = TOFDISTCALFACTOR;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case TOFOFFSETCALFACTOR:	// Time of flight offset calibration factor; Solis = NVsOpPara.CalibrationEcho	R	
									cTemp[0]= '\0';
									//strcpy(cTemp, dvarTOFOFFSETCALFACTOR); 
									UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.CalibrationEcho );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = TOFOFFSETCALFACTOR;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case DISTADJCNT:			// Distance adjust counter; Solis = NVsOpPara.ConfirmTimeTH	R
									cTemp[0]= '\0';
									//strcpy(cTemp, dvarDISTADJCNT); 
									UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.ConfirmTimeTH );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = DISTADJCNT;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case DISTADJFAILCNT:		// Distance adjust fail counter; Solis = NVsOpPara.AdjudtedFailCT	R	
									cTemp[0]= '\0';
									UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.AdjudtedFailCT );
									//strcpy(cTemp, dvarDISTADJFAILCNT); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = DISTADJFAILCNT;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case SALKDNIRRECCAL:		// Sensor A (Look down) IR current from recently calibration or default; Solis = NVsOpPara.BVolt R	
									cTemp[0]= '\0';
									//strcpy(cTemp, dvarSALKDNIRRECCAL); 
									UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.BVolt );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = SALKDNIRRECCAL;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
							/* not MVP1 but coded	
								case SBLKUPIRRECCAL:		// Sensor B (Look up) IR current from recently calibration or default; Solis = NVsOpPara.ONDelayTM R	
									cTemp[0]= '\0';
									//strcpy(cTemp, dvarSBLKUPIRRECCAL); 
									UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.ONDelayTM );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = SBLKUPIRRECCAL;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
									*/
								case SALKDNIRMFGCAL:		// Sensor A (Look down) IR current from manufacture calibration or default; Solis = NVsOpPara.WaitDelayTM R	
									cTemp[0]= '\0';
									//strcpy(cTemp, dvarSALKDNIRMFGCAL); 
									UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.WaitDelayTM );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = SALKDNIRMFGCAL;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
							/* not MVP1 but coded		
								case SALKUPIRMFGCAL:		// Sensor B (Look up) IR current from manufacture calibration or default R		
									cTemp[0]= '\0';
									strcpy(cTemp, dvarSALKUPIRMFGCAL); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = SALKUPIRMFGCAL;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case SALKDNMAXNLVL:		// Sensor A (Look down) Maximum noise level	R
									cTemp[0]= '\0';
									strcpy(cTemp, dvarSALKDNMAXNLVL); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = SALKDNMAXNLVL;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case SBLKUPMAXNLVL:		// Sensor B (Look up) Maximum noise level R	
									cTemp[0]= '\0';
									strcpy(cTemp, dvarSBLKUPMAXNLVL); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = SBLKUPMAXNLVL;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
							*/
								case SALKDNBGCURUSED:		// Sensor A (Look down) background for current used; Solis = NVsOpPara.CleanBackground	R	
									cTemp[0]= '\0';
									//strcpy(cTemp, dvarSALKDNBGCURUSED); 
									UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.CleanBackground );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = SALKDNBGCURUSED;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
							/* not MVP1 but coded		
								case SBLKUPBGCURUSED:		// Sensor B (Look up) background for current used R	
									cTemp[0]= '\0';
									strcpy(cTemp, dvarSBLKUPBGCURUSED); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = SBLKUPBGCURUSED;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case TOTACTCNTLOWBAT:		// Total activation count in low battery R	
									cTemp[0]= '\0';
									UTIL1_Num16uToStr((unsigned char *)cTemp, 5, NVsOpPara.LBActivationCT );
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = TOTACTCNTLOWBAT;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								case MANUALACTTIMES:		// Manual Activation times	R	
									cTemp[0]= '\0';
									strcpy(cTemp, dvarMANUALACTTIMES); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = MANUALACTTIMES;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;
								*/	
								case DNTLSTFMWUPDATE:		// Date & time of last firmware update	RW
									cTemp[0]= '\0';
									cTempDate[0]= '\0';
									UTIL1_Num16uToStr((unsigned char *)cTempDate, 5, NVsOpPara.UpdateY );
									cTemp[0] = cTempDate[2];
									cTemp[1] = cTempDate[3];
									//strcpy(cTemp, cTempDate);
									cTempDate[0]= '\0';
									UTIL1_Num8uToStr((unsigned char *)cTempDate, 3, NVsOpPara.UpdateM );
									if ( NVsOpPara.UpdateM >= 10)
									{
										cTemp[2] = cTempDate[0];
										cTemp[3] = cTempDate[1];
									}else
									{
										cTemp[2] = 0x30;
										cTemp[3] = cTempDate[0];
									}
									//strcat(cTemp,cTempDate);
									cTempDate[0]= '\0';
									UTIL1_Num8uToStr((unsigned char *)cTempDate, 3, NVsOpPara.UpdateD );
									if (NVsOpPara.UpdateD >= 10)
									{
										cTemp[4] = cTempDate[0];
										cTemp[5] = cTempDate[1];
									}else
									{
										cTemp[4] = 0x30;
										cTemp[5] = cTempDate[0];
									}
									cTemp[6] = '\0';
									//strcat(cTemp,cTempDate); 
									l = strlen(cTemp);
									sOutPayloadData[++i] = l+1;  //length not include length byte
									sOutPayloadData[++i] = DNTLSTFMWUPDATE;  //command
									for(j = 0; j < l; j++){
										sOutPayloadData[++i] = cTemp[j];  // store SN into output array
									}
									break;		
								default:
									validCmd = FALSE; // not a valid cmd
									break;
									
							 }
							 
							 if(validCmd){
								 CmdCnt++;  // keep track number of cmd id done
							 } else
							 {
								validCmd = TRUE; 
							 }							 
							 isReadCmd = FALSE;
						 }
						 
						 n++;
						 
					 }
				 }
			 }
			 
			 if (sOutPayloadData[0] != CmdCnt)
			 {
				 sOutPayloadData[0] = CmdCnt;
			 }
			 
			 // bOutPayloadLength = i+1;  // [Total # cmd length] + num cmd byte --move down 4/12/23 
			 
			 //add check on max payload size must be less then 96 bytes 4/12/23
			 if (i > MAXDATASIZE){  // check payload size --was 94
				 bOutPayloadLength = MAXDATASIZE;  // --was 94
			 }else
			 {
				bOutPayloadLength = i+1;  // [Total # cmd length] + num cmd byte  
			 }
			 //--
			 
			 if (GVbUARTInCMDList[0] > MULTIPDUCMDLP)  //5)  4/11/23 
			 {
				 GVbMultiPDUPart = 2;
				 GVbMultiPDULocCnt = n;
			 }
			 
			 //check if there is a valid cmd id
			 if(sOutPayloadData[0] == 0)
			 {
				 isValidCmd = FALSE; 
			 }
			 
			break;
		case SERIALNUM:	// Serial number  - send as response **
			bOutPayloadLength = 10;
			strcpy(sOutPayloadData,NVsBLEPara.sBoardSN);
			break;
		case MFGDATE:	// Manufacturing Date 	
			bOutPayloadLength = 6;
			strcpy(sOutPayloadData, NVsBLEPara.sBoardDATE); 
			break;
		case HWREVISION:	// Hardware Revision 	
			bOutPayloadLength = 4;
			strcpy(sOutPayloadData, NVsBLEPara.sBoardREV);
			break;
		case FMWREVISION:	// Firmware Revision 	
			bOutPayloadLength = 4;
			strcpy(sOutPayloadData,VERISION_MAJOR);
			strcat(sOutPayloadData,VERISION_MINOR);
			break;						
		case ANTSKU:	// Device SKU **
			bOutPayloadLength = 2; // Device  2 byte  = "30"  i.e. 3 byte  ="159"
			arr[0]= '\0';
			//bTemp = (uint8)atoi(NVsBLEPara.sBoardSKU) + 30;
			bTemp = (uint8)atoi(NVsBLEPara.sBoardSKU);
			UTIL1_Num8uToStr((unsigned char *)arr, 3, bTemp );
			strcpy(sOutPayloadData, arr);
			break;
/*
// Delphian take care
#define RFSERIALNUM	    6 		// RF Dongle Serial Number        --Don't need to implement on device
#define RFMFGDATE		7 		// RF Dongle Manufacture Date     --Don't need to implement on device
#define RFHWREVISION	8 		// RF Dongle hardware version     --Don't need to implement on device
#define RFFMWREVISION	9 		// RF dongle firmware version     --Don't need to implement on device
*/
	/*	case FACTORYRESET:	// Factory Reset  	
			bOutPayloadLength = 0;
			
			break;
			*/			
		case SENSORRANGE:	// Sensor Range **
			if((uint8)atoi(NVsBLEPara.sSensorRange) >= 10)
			{
				bOutPayloadLength = 2;
				
			}else
			{
				bOutPayloadLength = 1;
			}
			
			strcpy(sOutPayloadData, NVsBLEPara.sSensorRange); 
			break;		
			
		case MODESELECTION:	// Solis "0": closet; "1": urinal; "2": urinal with ballpark  	
				
			bOutPayloadLength = 1;
			arr[0]= '\0';
			bTemp = NVsOpPara.Mode;   // the hours
			UTIL1_Num8uToStr((unsigned char *)arr, 2, bTemp );
			strcpy(sOutPayloadData, arr);
				
			break;
			
		case MRTOTIMER:	// Open Timer **
			
			arr[0]= '\0';
					
			bTemp = NVsOpPara.OpenTM;
		
			// convert dec i.e 28 to [2] [8] to 2.8 and send hex 0x32 0x38
			for(k = 0; k < 3; k++)
			{
				if (k != 1)
				{ 
					arr[2-k] = (bTemp% 10) + 48;
					bTemp /= 10;
				} else
				{
					arr[k] = 0x2E;
				}
			}
			arr[k] = '\0';
			
			bOutPayloadLength = 3;
			strcpy(sOutPayloadData, arr);
			break;	  
/* 
 * dont need
		 case FLUSHACT:	// Flush Activate
			bOutPayloadLength = 0;
			//reqOrResponse = 0;
			break;
*/		
			
		case FLUSHSENTINTIME:	// Sentinel Time 	
			arr[0]= '\0';
			bTemp = NVsOpPara.SentinalTM;   // the hours
			UTIL1_Num8uToStr((unsigned char *)arr, 4, bTemp );
			bOutPayloadLength = strlen(arr);
			strcpy(sOutPayloadData, arr);
			break;
		case LPMGPF:	// GPF  **
			bOutPayloadLength = 4;
			strcpy(sOutPayloadData,NVsBLEPara.sGPF);
			break;
		case DIAGNOSISINIT:	// Diagnosis Init  		
			bOutPayloadLength = 1;
			arr[0]= '\0';
			bTemp = GVbInDiag;
			UTIL1_Num8uToStr((unsigned char *)arr, 2, bTemp );
			strcpy(sOutPayloadData, arr);
			break;
		case BLEENABLE:	// BLE Enable 	
			bOutPayloadLength = 1;
			/*
 	 	 	   0x32 = “2” = is for BLE Enable up to receive disable command
			   0x31 = “1” = is for BLE Enable up to 180 seconds
			   0x30 = “0” = is for BLE Disable
			 */
			sOutPayloadData[0] = 0x30;
			break;				
		case SENSORSTAT:	// Sensor Status change Event **
			bOutPayloadLength = 1;
			cTemp[0]= '\0';
			UTIL1_Num8uToStr((unsigned char *)cTemp, 2, GVbDiagSensorStat );
			strcpy(sOutPayloadData, cTemp);	
			break;	
		case VALVESTAT:	// Valve Status change Event **
			bOutPayloadLength = 1;
			cTemp[0]= '\0';
			UTIL1_Num8uToStr((unsigned char *)cTemp, 2, GVbDiagValveStat );
			strcpy(sOutPayloadData, cTemp);
			break;	
		/*case TURBINESTAT:	// Turbine Status   	
			bOutPayloadLength = 1;
			sOutPayloadData[0] = 0x30;
			break;*/
		case SOLARPANELSTAT:	// Solar Panel Status 	
			bOutPayloadLength = 1;
			
			if (GVbDiagSolarStat == 0)
			{
				GVbDiagSolarStat = IsSolarCellgetPower();
			}
			
			cTemp[0]= '\0';
			UTIL1_Num8uToStr((unsigned char *)cTemp, 2, GVbDiagSolarStat );
			strcpy(sOutPayloadData, cTemp);
			break;		
		case BATLEVEL:	// Battery Level Change Event**
			
			bTemp = batConvert16to8();
			GVbBatLevelStore = bTemp;
			
			if(bTemp >= 100)
			{
				bOutPayloadLength = 3;
				sOutPayloadData[2] = 0x30;
				sOutPayloadData[1] = 0x30;
				sOutPayloadData[0] = 0x31;
			}else
			{
				arr[0]= '\0';
				UTIL1_Num8uToStr((unsigned char *)arr, 3, bTemp );
				bOutPayloadLength = strlen(arr);
				strcpy(sOutPayloadData, arr);
			}	
			break;	
		case OPHRSINCEINSTALL:	// Operation Hours since install **
								
			// total days
			iTemp = NVsOpPara.RDay;
			for(i=0; i< NVsOpPara.RYear;i++){
				iTemp += 365;
			}
			
			lTemp = NVsOpPara.RHour;
			for(ii=0; ii< iTemp;ii++){
				lTemp += 24;
			}
			cTemp[0]= '\0';
			UTIL1_Num32uToStr((unsigned char *)cTemp, 9, lTemp );
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			
			break;
		case ACTSINCEINSTALL:	// total activation  - set as response **
			lTemp = NVsOpPara.TotalActivation;
			cTemp[0]= '\0';
			UTIL1_Num32uToStr((unsigned char *)cTemp, 9, lTemp );
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;	
/*
//Handle on Argos
#define DNTLSTFACTRESET	26 		// Date & time of last factory reset
#define DNTLSTRANGECHG	27 		// Date & time of last range change
#define DNTLSTMODECHG	28 		// Date & time of last mode change
#define DNTLSTSENTINALCHG 29 	// Date & time of last Sentinel change
#define DNTLSTDIAG		30 		// Date & time of last diagnostic
*/			
		case SRSTATUS:	// RF Dongle Status Update **
			bOutPayloadLength = 0;
		//	reqOrResponse = 0;
			break;	
		
		
		case FLUSHACTREQUEST:	// Flush Request (0x65 PoC 1) **
			bOutPayloadLength = 1;
			sOutPayloadData[0] = 0x31;
			break;
// dont need --Test only
 		case ACTRPTTHOLD: // Activation reporting RO
 			iTemp = NVsANTPara.iActivationRptTh;
			cTemp[0]= '\0';
			UTIL1_Num16uToStr((unsigned char *)cTemp, 3, iTemp );
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
//
		/*case DONGLERFENDISABLE: // RF enable disable RW --remove 6/23/23
			bOutPayloadLength = 1;
			
			//if(GVbRFEnDisabState == 1)
			if(NVbRFEnDisableModeIn == 4) //add 3/30/23
			{
				sOutPayloadData[0] = 0x31;			//RF disable
				NVsANTPara.iRFDongleEnDisable = 1;  // RF disable 3/30/23 add
				NVbRFEnDisableModeIn = 0;
			}else {
				sOutPayloadData[0] = 0x30;			//RF enable
				NVsANTPara.iRFDongleEnDisable = 0;  //RF enable 3/30/23 add
				NVbRFEnDisableModeIn = 0;
			}
			
			SaveANTParaToFlash(NVsANTPara);  // save it to flash 3/30/23 add
			break;*/
					
		case SHIPMODEDEEPSLP:	//RF ship mode Enable Disable RW
			bOutPayloadLength = 1;		
			
			if(NVbDongleDeepSlpModeIn == 1)
			{
				sOutPayloadData[0] = 0x31;          //Shipping Mode Activate.
				NVbDongleDeepSlpModeIn = 2;
				NVsANTPara.iShipModeDeepSleep = 1;
				
			}else{
				sOutPayloadData[0] = 0x30;			//Shipping Mode deactivate
				NVbDongleDeepSlpModeIn = 0;
				NVsANTPara.iShipModeDeepSleep = 0;
			}
			
			//SaveANTParaToFlash(NVsANTPara);  // save it to flash  --remove 7/11/23 corrupt to flash 
			break;	
		case OCCUPANCY:  // Occupancy RO
			bOutPayloadLength = 1;
			
			if(GVbOccupState == 1)
			{
				sOutPayloadData[0] = 0x31;  // enter zone
			}else {
				sOutPayloadData[0] = 0x32; // vacant
			}
			break;
		case FEATUREENABLE: //Feature enable for Occupancy RW
			bOutPayloadLength = 2;
			
			if(NVsANTPara.iFeatureOccupEnDisable == 1)
			{
				// 11 = occupancy on
				sOutPayloadData[0] = 0x31;  
				sOutPayloadData[1] = 0x31; 
			}else {
				// 01 occupancy 0ff
				sOutPayloadData[0] = 0x30;  
				sOutPayloadData[1] = 0x31;
			}
			break;
		case ARMTIMER:	// Arm Timer   	RW	
			arr[0]= '\0';
			bTemp = NVsOpPara.ArmTM;   
			UTIL1_Num8uToStr((unsigned char *)arr, 3, bTemp );
			bOutPayloadLength = strlen(arr);
			strcpy(sOutPayloadData, arr);
			
			break;
	/* not MVP1 but coded		
		case PORRESETCNT:	// POR Reset Counter	RO
			iTemp = NVsOpPara.PORResetCT;
			cTemp[0]= '\0';
			UTIL1_Num16uToStr((unsigned char *)cTemp, 3, iTemp );
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
		case UIRCURRATIO:			// Urinal IR Current Ratio	RO
			bOutPayloadLength = 1;
			arr[0]= '\0';
			bTemp = GVsTempPara.UrinalIR;   
			UTIL1_Num8uToStr((unsigned char *)arr, 2, bTemp );
			//bOutPayloadLength = strlen(arr);
			strcpy(sOutPayloadData, arr);
			break;  
		case NOISEFLOOR:			// Noise Floor	RW

			bOutPayloadLength = strlen(dvarNOISEFLOOR);
			strcpy(sOutPayloadData, dvarNOISEFLOOR);
			
			break;
		case ECHODIFFERENCE:		// Echo difference	 RW	
			
			bOutPayloadLength = strlen(dvarECHODIFFERENCE);
			strcpy(sOutPayloadData, dvarECHODIFFERENCE);
			
			break;
		case TGTINTHOLD:			// Target in threshold	RW
			
			bOutPayloadLength = strlen(dvarTGTINTHOLD);
			strcpy(sOutPayloadData, dvarTGTINTHOLD);
			
			break;
		case TGTOUTTHOLD:			// Target out threshold	RW
			
			bOutPayloadLength = strlen(dvarTGTOUTTHOLD);
			strcpy(sOutPayloadData, dvarTGTOUTTHOLD);
			
			break;
		case TGTSTAYINTHOLD:		// Target Stay in threshold	RW	
			
			bOutPayloadLength = strlen(dvarTGTSTAYINTHOLD);
			strcpy(sOutPayloadData, dvarTGTSTAYINTHOLD);
			
			break;
		case TGTSTAYOUTTHOLD:		// Target stay out threshold RW		
			
			bOutPayloadLength = strlen(dvarTGTSTAYOUTTHOLD);
			strcpy(sOutPayloadData, dvarTGTSTAYOUTTHOLD);
			
			break;
		case TGTSTAYOUTTHOLD1:	// Target stay out threshold RW		
			
			bOutPayloadLength = strlen(dvarTGTSTAYOUTTHOLD1);
			strcpy(sOutPayloadData, dvarTGTSTAYOUTTHOLD1);
			
			break;
		case TGTBACKTHOLD:		// Target back threshold RW		
			
			bOutPayloadLength = strlen(dvarTGTBACKTHOLD);
			strcpy(sOutPayloadData, dvarTGTBACKTHOLD);
			
			break;
		case TGTSTAYTHOLD:		// Target stay threshold RW	
			
			bOutPayloadLength = strlen(dvarTGTSTAYTHOLD);
			strcpy(sOutPayloadData, dvarTGTSTAYTHOLD);
			
			break;
		case BACKTOIDLETHOLD:		// Back to Idle threshold RW	
			
			bOutPayloadLength = strlen(dvarBACKTOIDLETHOLD);
			strcpy(sOutPayloadData, dvarBACKTOIDLETHOLD);
			
			break;
	*/	
		case SITTINGUSRTHOLD:		// Sitting user threshold; Solis = NVsOpPara.MinUserTH RW	
			
			cTemp[0]= '\0';
			UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.MinUserTH );								 
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
		case BGCHANGETHOLD:		// Background change threshold; Solis = NVsOpPara.TouchTH RW	
			
			cTemp[0]= '\0';
			UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.TouchTH );							 
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
		case IRTGTTHOLD:			// IR Target Threshold; Solis = NVsOpPara.MaxBackground RW
			cTemp[0]= '\0';
			UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.MaxBackground );							 
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
		case IRUPDATETHOLD:		// IR update threshold; Solis = NVsOpPara.ONDelayTM RW	;
			cTemp[0]= '\0';
			UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.ONDelayTM );								 
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
	/* not MVP1 but coded			
		case IRCHANGETHOLD:		// IR change Threshold RW		
			
			bOutPayloadLength = strlen(dvarIRCHANGETHOLD);
			strcpy(sOutPayloadData, dvarIRCHANGETHOLD);
			
			break;
	*/
		case MAXIRTHOLD:			// Maximum IR Threshold	R
			iTemp = NVsOpPara.MaxIRTH;
			cTemp[0]= '\0';
			UTIL1_Num16uToStr((unsigned char *)cTemp, 6, iTemp );
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
	/* not MVP1 but coded		
		case MINIRTHOLD:			// Minimum IR Threshold	R
			iTemp = NVsOpPara.MinIRTH;
			cTemp[0]= '\0';
			UTIL1_Num16uToStr((unsigned char *)cTemp, 5, iTemp );
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
		case MINBG:				// Minimum background RW
			
			bOutPayloadLength = strlen(dvarMINBG);
			strcpy(sOutPayloadData, dvarMINBG);
								
			break;
	*/	
		case TOFDISTCALFACTOR:	// Time of flight distance calibration factor; Solis: calibration flag R			
			cTemp[0]= '\0';
			UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.CalibrationFlag );								 
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
		case TOFOFFSETCALFACTOR:	// Time of flight offset calibration factor; Solis = NVsOpPara.CalibrationEcho	R	
			cTemp[0]= '\0';
			UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.CalibrationEcho );								 
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
		case DISTADJCNT:			// Distance adjust counter; Solis = NVsOpPara.ConfirmTimeTH	R
			cTemp[0]= '\0';
			UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.ConfirmTimeTH );								 
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
		case DISTADJFAILCNT:		// Distance adjust fail counter; Solis = NVsOpPara.AdjudtedFailCT	R	
			cTemp[0]= '\0';
			UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.AdjudtedFailCT );							 
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
		case SALKDNIRRECCAL:		// Sensor A (Look down) IR current from recently calibration or default; Solis = NVsOpPara.BVolt	R	
			cTemp[0]= '\0';
			UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.BVolt );							 
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
	/* not MVP1 but coded		
		case SBLKUPIRRECCAL:		// Sensor B (Look up) IR current from recently calibration or default; Solis = NVsOpPara.ONDelayTM R	
			//bOutPayloadLength = 4;
			//strcpy(sOutPayloadData, dvarSBLKUPIRRECCAL);
			cTemp[0]= '\0';
			UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.ONDelayTM );						 
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
	*/		
		case SALKDNIRMFGCAL:		// Sensor A (Look down) IR current from manufacture calibration or default; Solis = NVsOpPara.WaitDelayTM R	
			cTemp[0]= '\0';
			UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.WaitDelayTM );					 
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
	/* not MVP1 but coded		
		case SALKUPIRMFGCAL:		// Sensor B (Look up) IR current from manufacture calibration or default R		
			bOutPayloadLength = 4;
			strcpy(sOutPayloadData, dvarSALKUPIRMFGCAL);
			break;
		case SALKDNMAXNLVL:		// Sensor A (Look down) Maximum noise level	R
			bOutPayloadLength = 4;
			strcpy(sOutPayloadData, dvarSALKDNMAXNLVL);
			break;
		case SBLKUPMAXNLVL:		// Sensor B (Look up) Maximum noise level R	
			bOutPayloadLength = 4;
			strcpy(sOutPayloadData, dvarSBLKUPMAXNLVL);
			break;
	*/		
		case SALKDNBGCURUSED:		// Sensor A (Look down) background for current used; Solis = NVsOpPara.CleanBackground	R	
			//bOutPayloadLength = 4;
			//strcpy(sOutPayloadData, dvarSALKDNBGCURUSED);
			cTemp[0]= '\0';
			UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.CleanBackground );							 
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
	/* not MVP1 but coded			
		case SBLKUPBGCURUSED:		// Sensor B (Look up) background for current used R	
			bOutPayloadLength = 4;
			strcpy(sOutPayloadData, dvarSBLKUPBGCURUSED);
			break;
		case TOTACTCNTLOWBAT:		// Total activation count in low battery R	

			iTemp = NVsOpPara.LBActivationCT;
			cTemp[0]= '\0';
			UTIL1_Num16uToStr((unsigned char *)cTemp, 5, iTemp );
			bOutPayloadLength = strlen(cTemp);
			strcpy(sOutPayloadData, cTemp);
			break;
		case MANUALACTTIMES:		// Manual Activation times	R	
			bOutPayloadLength = 4;
			strcpy(sOutPayloadData, dvarMANUALACTTIMES);
			break;
	*/		
		case DNTLSTFMWUPDATE:		// Date & time of last firmware update	RW
			cTemp[0]= '\0';
			cTempDate[0]= '\0';
			UTIL1_Num16uToStr((unsigned char *)cTempDate, 5, NVsOpPara.UpdateY );
			cTemp[0] = cTempDate[2];
			cTemp[1] = cTempDate[3];
			cTempDate[0]= '\0';
			UTIL1_Num8uToStr((unsigned char *)cTempDate, 3, NVsOpPara.UpdateM );
			if ( NVsOpPara.UpdateM >= 10)
			{
				cTemp[2] = cTempDate[0];
				cTemp[3] = cTempDate[1];
			}else
			{
				cTemp[2] = 0x30;
				cTemp[3] = cTempDate[0];
			}
			cTempDate[0]= '\0';
			UTIL1_Num8uToStr((unsigned char *)cTempDate, 3, NVsOpPara.UpdateD );
			if (NVsOpPara.UpdateD >= 10)
			{
				cTemp[4] = cTempDate[0];
				cTemp[5] = cTempDate[1];
			}else
			{
				cTemp[4] = 0x30;
				cTemp[5] = cTempDate[0];
			}
			cTemp[6] = '\0';
			strcpy(sOutPayloadData, cTemp);
			bOutPayloadLength = 6;
			break;						
		default:
			isValidCmd = FALSE;
			break;
															
	} // end switch(icmd)	
	
				
	if (isValidCmd)		
	{
		//SensorRequestUARTHandler(CmdID, bOutPayloadLength, reqOrResponse, sOutPayloadData);
		SensorRequestUARTHandler(CmdID, bOutPayloadLength, sOutPayloadData);
	
	}			
}

/*
** =================================================================================================================================
**     Method      :  uint8 ProcessingSingleUARTCommand(void)
**     Description :
**         This method receving and processing a single command from BLE
**         Note: use this only after BLE has beeen waked up and uart has been set
**     Parameters  : Nothing
**     Returns     : TRUE after "c 15 1 \cr" has been received. It is the last command from BLE at the connection
** =================================================================================================================================
*/

uint8 ProcessingSingleUARTCommand(void){	//SR sim board -> sensor
	// set control flags 
	bool isRequestComplete = FALSE;  // request state 
	bool isCmdNotFound = FALSE;    // command found or not
	bool chkACKResponse = FALSE;  // check ACK Response 
	bool EndByteReceived = FALSE;  // version 3 to check for F8 F8 checksum and end byte
	bool retryTwice = TRUE;  	// retry send response if no ACK
	uint8 retryCnt = 0;
	
	// error log flag
	//uint8 bErrFlg;  // 1 - Request time out, 2 - Request payload exceed, 3 - Request invalid checksum , 4 -Request invalid cmd , 5 - Request Ack Send
					// 6 - Response ACK time out, 7 - Response ACK payload exceed,  8 - Response Ack Send 
	
	// incoming side variables
	char sInUARTArrCmd[97]; 		// max 96 byte buffer to put incoming command array from UART, end byte \0
	char sInMsgLength[2]; 		// 1 byte message length
	char sCmdID[2];		 		// 1 byte command ID string
	char sInPayloadLength[2];	// 1 byte length of payload (calculated)
	char sInCalChecksum[2];   	// Incoming calculate checksum compare value
	char sInPayloadData[11];				// incoming data string
	uint8 bInCheckSumLength; 	// 1 byte payload length + msg length + command ID + status
	
	// outgoing side vriables
	char sOutMsgLength[2];		// 1 byte message length out
	char sOutPayloadData[11];		// response data string  
	char sOutUARTArrCmd[97];		// whole response string - max 96 byte buffer, end byte reserve \0 
	
	//char cTemp[4]; // temporary char storage
	char cTemp[11]; // temporary char storage
	char cTempDate[11];
	char arr[4];
	uint8 bOutPayloadLength;	// 1 byte length of payload (calculated)
	uint8 bOutCheckSumLengthResponse; 	// 1 byte payload length + msg length + command ID + status

	// counter variables
	uint8 bIndex;
	uint8 i,j;
	
	// temp variables
	uint8 bTemp;
	//uint8 sBytes32[4];		// temporary memory to hold data for conversion between data and ascii
	uint16 iTemp,ii;
	uint32 lTemp;
	
	// dummy var RW
	/* not in MVP1
	static char dvarNOISEFLOOR[5] =	"7388";
	static char dvarECHODIFFERENCE[5] = "7488";
	static char dvarTGTINTHOLD[3] = "75";
	static char dvarTGTOUTTHOLD[3]	= "76";
	static char dvarTGTSTAYINTHOLD[3] = "77";
	static char dvarTGTSTAYOUTTHOLD[3] = "78";
	static char dvarTGTSTAYOUTTHOLD1[3] = "79";
	static char dvarTGTBACKTHOLD[3] = "80";
	static char dvarTGTSTAYTHOLD[3] = "81";
	static char dvarBACKTOIDLETHOLD[3] = "82";
	//static char dvarSITTINGUSRTHOLD[3] = "83";
	//static char dvarBGCHANGETHOLD[3] = "84";
	//static char dvarIRTGTTHOLD[5] = "8588";
	//static char dvarIRUPDATETHOLD[3] =	"86";
	static char dvarIRCHANGETHOLD[2] =	"7";
	static char dvarMINBG[5] =	"9088";
	// dummy var RO 
	//static char dvarTOFDISTCALFACTOR[3] = "918";
	//static char dvarTOFOFFSETCALFACTOR[5] = "9288";
	//static char dvarDISTADJCNT[3] =	"93";
	//static char dvarDISTADJFAILCNT[3] =	"94";
	//static char dvarSALKDNIRRECCAL[5] = "9588";
	//static char dvarSBLKUPIRRECCAL[5] = "9688";
	//static char dvarSALKDNIRMFGCAL[5] = "9788";
	static char dvarSALKUPIRMFGCAL[5] = "9888";
	static char dvarSALKDNMAXNLVL[5] = "9988";
	static char dvarSBLKUPMAXNLVL[5] =	"1008";
	//static char dvarSALKDNBGCURUSED[5] = "1018";
	static char dvarSBLKUPBGCURUSED[5] = "1028";
	static char dvarMANUALACTTIMES[5] =	"1048";
	*/
	
	//Init variables
	bIndex = 0;				// initilize
	WDReset();				// kick dog

	/* Request - SR sim board -> sensor */
	if(AS1_RecvChar(&bTemp) == ERR_OK){					// Received something 
		
		if(bTemp == STARTBYTE){								// start with 0xF7, start byte of message(sheng) 0x63 pre
			sInUARTArrCmd[bIndex] = bTemp;					// move to incoming command buffer
			bIndex++;									// next position
			WDReset();									// Kick dog
			wTimerTick = 0;								// reset timer
			EndByteReceived = FALSE;					// 9/8/20 End byte received false
			TI1_Enable();								// Enable timer to terminate if something wrong

			while((bTemp != ENDBYTE) && !EndByteReceived){						// not the end of command, 0xF8 end byte(sheng)0x0D pre
				WDReset();	
				//if(wTimerTick >= 3000){					// check if time run out (3sec)
				if(wTimerTick >= TIMERWAITLIMIT){		// check if time run out (300msec)
					GVbUartCmdStateNerrFlg = 1;		    // SR Request time out
					break; 								// can't receive whole package. break
				}
				if(AS1_RecvChar(&bTemp) == ERR_OK){		// Received something 
					sInUARTArrCmd[bIndex] = bTemp;		// move to command buffer
					bIndex++;							// next position
					
					// added 9/8/20 version 3  to fix F8 F8 when checksum same as F8 of endbyte
					if((bTemp == ENDBYTE) && ((sInUARTArrCmd[1] + 4) == bIndex))			
					{
						EndByteReceived = TRUE;
					}
					
					if(bIndex > 96){					// exceed the max length 
						GVbUartCmdStateNerrFlg = 2;		// SR Request payload exceed
						break;
					}
				}
			}  //end while(bTemp 											// end of command or time out
			
			TI1_Disable();								// stop timer to save power and 
			
			/* check if we have start byte and end byte	 */										
			if((sInUARTArrCmd[0] == STARTBYTE) && (sInUARTArrCmd[bIndex-1] == ENDBYTE) && (isCmdInCmdlist(sInUARTArrCmd[2]) == 1)){
				
				sInMsgLength[0] = sInUARTArrCmd[1];
				sCmdID[0] = sInUARTArrCmd[2];
				sCmdID[1] = '\0';
				
				sInPayloadLength[0] = sInMsgLength[0] - 1;  // n-payload = msg length - "1" for cmd ID
				sInPayloadLength[1] =  '\0';
								
				bInCheckSumLength = 3 + sInPayloadLength[0];  // checksum length to be parse, [0],[1],[2] = 3 count
				sInCalChecksum[0] = calculateChecksum(sInUARTArrCmd,1,bInCheckSumLength); // get check sum 1 byte
				
				/* Send Request ACK if checksum calculate = checksum received */			
				if(sInCalChecksum[0] == sInUARTArrCmd[bIndex-2]){    
				
					/** Request ACK  - Sensor -> SR Sim board **/
					sOutUARTArrCmd[0]='\0';
					sOutUARTArrCmd[0] = STARTBYTE;								
					sOutUARTArrCmd[1] = sCmdID[0];
					sOutUARTArrCmd[2] = ENDBYTE;
				//	UARTOutStrPlusNull(sOutUARTArrCmd,3);
					
					UartWakeInt_Disable(); // disable interrupt
					
					/* Check Wake up pin high before send 4/4/23 -mod 4/10/23*/
					if(UartWake_GetVal()){
						UartWake_SetOutput(); //set UartWake to output (pin 27)
									
						//begin 50ms pulse high to low turn on SR RF 
						UartWake_ClrVal();
						DelayMS(50);			// pull up 50 ms to wake up BLE
						UartWake_SetVal();   // pull up
						DelayMS(50);	
						UartWake_ClrVal(); // Pull low
						DelayMS(50);
	
						UARTOutStrPlusNull(sOutUARTArrCmd,3);
						
						UartWake_SetVal();   // pull up /*PoC 2 */
						UartWake_SetInput(); // reset UartWake as input (pin 27) 
					}else
					{
						UARTOutStrPlusNull(sOutUARTArrCmd,3);
					}
					//--
					
					UartWakeInt_Enable(); // re enable interrupt
					
					//--org code
//					UartWakeInt_Disable(); // disable interrupt
//					UartWake_SetOutput(); //set UartWake to output (pin 27)
//								
//					//begin 50ms pulse high to low turn on SR RF 
//					UartWake_ClrVal();
//					DelayMS(50);			// pull up 50 ms to wake up BLE
//					UartWake_SetVal();   // pull up
//					DelayMS(50);	
//					UartWake_ClrVal(); // Pull low
//					DelayMS(50);
//
//					UARTOutStrPlusNull(sOutUARTArrCmd,3);
//					
//					UartWake_SetVal();   // pull up /*PoC 2 */
//					UartWake_SetInput(); // reset UartWake as input (pin 27) 
//					UartWakeInt_Enable(); // re enable interrupt
					//--org end
					
					DelayMS(20);  // delay 21ms between ACK and response data 
					
					if (sCmdID[0] == MULTIPDU)
					{
						GVbUARTInCMDList[0] = '\0';
						// get incoming payload data
						for(i = 0; i < sInPayloadLength[0]; i++){
							GVbUARTInCMDList[i] = sInUARTArrCmd[3+i];
						}
						GVbUARTInCMDList[i] = '\0';
						GVbSRRefreshCmd = 1;  // refresh command received 69 (0x45)
						GVbUartCmdStateNerrFlg = 5;  // Cmd multipdu 
						isRequestComplete = TRUE;
							
					} else 
					{
						/* Send Response Method*/
									
						//iCmd = atoi(sCmdID); 	// Convert from ascii to number
						sOutPayloadData[0]='\0'; 		// clear output string 
					
						// get incoming payload data
						for(i = 0; i < sInPayloadLength[0]; i++){
							sInPayloadData[i] = sInUARTArrCmd[3+i];
						}
						sInPayloadData[i] = '\0';
						
						switch(sCmdID[0]){
// MVP Need to implement--------------------													
/**MARKMVP*/				case SERIALNUM:	// Serial number 	
								bOutPayloadLength = 10;
								/*POC2
								//strcpy(sOutPayloadData,"1234567890");
								strcpy(sOutPayloadData, SERIALNUMBER);
								*/
								strcpy(sOutPayloadData,NVsBLEPara.sBoardSN);
								break;
							case MFGDATE:	// Manufacturing Date 	
								bOutPayloadLength = 6;
								strcpy(sOutPayloadData, NVsBLEPara.sBoardDATE); 
								break;
							case HWREVISION:	// Hardware Revision 	
								bOutPayloadLength = 4;
								strcpy(sOutPayloadData, NVsBLEPara.sBoardREV);
								break;
							case FMWREVISION:	// Firmware Revision 	
								bOutPayloadLength = 4;
								strcpy(sOutPayloadData,VERISION_MAJOR);
								strcat(sOutPayloadData,VERISION_MINOR);
								break;								
							case ANTSKU:	// Device SKU 	
								bOutPayloadLength = 2;  // Device  2 byte  = "30"  i.e. 3 byte  ="159"
								arr[0]= '\0';
								//bTemp = (uint8)atoi(NVsBLEPara.sBoardSKU) + 30;
								bTemp = (uint8)atoi(NVsBLEPara.sBoardSKU);
								UTIL1_Num8uToStr((unsigned char *)arr, 3, bTemp );
								strcpy(sOutPayloadData, arr);
								break;
// MVP --------------------------------	

/*
// Delphian take care
#define RFSERIALNUM	    6 		// RF Dongle Serial Number        --Don't need to implement on device
#define RFMFGDATE		7 		// RF Dongle Manufacture Date     --Don't need to implement on device
#define RFHWREVISION	8 		// RF Dongle hardware version     --Don't need to implement on device
#define RFFMWREVISION	9 		// RF dongle firmware version     --Don't need to implement on device
 */								

// MVP Need to implement--------------------
							case FACTORYRESET:	// Factory Reset 
								
								if(sInPayloadLength[0] == 0){		//	read value
									bOutPayloadLength = 1;
									sOutPayloadData[0] = 0x30;
								}else{								 
									bOutPayloadLength = 0;
									if( sInUARTArrCmd[3] == 0x31)
									{
										SetDefaultOpParas();   // reset function
										GVbFactoryRestFlg = TRUE;
									}else
									{
										GVbFactoryRestFlg = FALSE;
									}
								}
								break;
							case SENSORRANGE:	// Sensor Range 	
								
								if(sInPayloadLength[0] == 0){		//	read value
									
									//if((uint8)atoi(GVbSrange) >= 10) // PoC2
									if((uint8)atoi(NVsBLEPara.sSensorRange) >= 10)
									{
										bOutPayloadLength = 2;
										
									}else
									{
										bOutPayloadLength = 1;
									}
									// strcpy(sOutPayloadData, GVbSrange);  // PoC2
									strcpy(sOutPayloadData, NVsBLEPara.sSensorRange); 
									
								}else{ 								// write new value
									bOutPayloadLength = 0;
									if((uint8)atoi(sInPayloadData) <= 10)
									{
										//strcpy(GVbSrange,sInPayloadData);	//Poc2
										strcpy(NVsBLEPara.sSensorRange,sInPayloadData);
										SetSensingRange(NVsBLEPara.sSensorRange);
										SaveBLEParaToFlash(NVsBLEPara);		//	NVsBLEPara changed, save it to flash
										SaveParaToFlash(NVsOpPara);			//	NVsOpPara changed, store it to flash
										
									}
								}
								
								break;
								
							case MODESELECTION:	// Solis "0": closet; "1": urinal; "2": urinal with ballpark  	
								
								if(sInPayloadLength[0] == 0){		//	read value
									bOutPayloadLength = 1;
									arr[0]= '\0';
									bTemp = NVsOpPara.Mode;   // the hours
									UTIL1_Num8uToStr((unsigned char *)arr, 2, bTemp );
									//bOutPayloadLength = strlen(arr);
									strcpy(sOutPayloadData, arr);
									
									//strcpy(sOutPayloadData, NVsOpPara.Mode); 
									
								}else{ 								// write new value
									bOutPayloadLength = 0;
									bTemp = (uint8)atoi(sInPayloadData);
									NVsOpPara.Mode = bTemp;
									//strcpy(NVsOpPara.Mode,sInPayloadData);
									//SaveBLEParaToFlash(NVsBLEPara);		//	NVsBLEPara changed, save it to flash
									SaveParaToFlash(NVsOpPara);			//	NVsOpPara changed, store it to flash
								}
								break;
								
							case MRTOTIMER: // Open Timer 
							
								if(sInPayloadLength[0] == 0){		//	read value
									
									int k;	
									arr[0]= '\0';
									
									bTemp = NVsOpPara.OpenTM;
								
									// convert dec i.e 28 to [2] [8] to 2.8 and send hex 0x32 0x38
									for(k = 0; k < 3; k++)
									{
										if (k != 1)
										{ 
											arr[2-k] = (bTemp% 10) + 48;
											bTemp /= 10;
										} else
										{
											arr[k] = 0x2E;
										}
									}
									arr[k] = '\0';
									
									bOutPayloadLength = 3;
									strcpy(sOutPayloadData, arr);
									
								}else{ 								// write new value
									bOutPayloadLength = 0;
									j = 0;
									char oTemp[3];
									oTemp[0] = '\0';
									bool sValue = FALSE;
									
									for(i = 0; i < sInPayloadLength[0]; i++){
										if (sInPayloadData[i] != 0x2E)
										{
											oTemp[j] = sInPayloadData[i];
											j++;
										} else 
										{
											sValue = TRUE;
										}
									}
									oTemp[2] = '\0';
									
									if (sValue)
									{	
										NVsOpPara.OpenTM = atoi(oTemp);  	//replace ArmTM with OpenTM (sheng)
										SetValveOnTime(NVsOpPara.OpenTM);				// set valve turn on time for all types of flush. There are 4 global variables.	
										SaveParaToFlash(NVsOpPara);	// store to flash
									}
									
								}
															
								break;
// MVP --------------------------------	
								
/** POC **/					case FLUSHACT: // Flush Activate (66 POC1)
								bOutPayloadLength = 0;
								if( sInUARTArrCmd[3] == 0x31)
								{
									//LED_SetVal();
									DelayMS(200); // add to get rid of double flush after flush most important
									//LED_ClrVal();
									
									GVbFlushActivate = 1;
								}
								break;
								
// MVP Need to implement--------------------
							case FLUSHSENTINTIME:	// Sentinel Time    	
								//bOutPayloadLength = 1;
								//strcpy(sOutPayloadData, SERIALNUMBER);
								
								if(sInPayloadLength[0] == 0){		//	read value
									arr[0]= '\0';
									bTemp = NVsOpPara.SentinalTM;   // the hours
									UTIL1_Num8uToStr((unsigned char *)arr, 4, bTemp );
									bOutPayloadLength = strlen(arr);
									strcpy(sOutPayloadData, arr);
								}
								else{ 								// set New value
									
									bOutPayloadLength = 0;
									bTemp = (uint8)atoi(sInPayloadData);
									NVsOpPara.SentinalTM = bTemp;
									GVbSentinelFlush = CheckSentinelSetting();			// reset sentinel flush flag
									SaveParaToFlash(NVsOpPara);		// store to flash
								}
								break;
// MVP --------------------------------	
								
							case LPMGPF: // GPF
								bOutPayloadLength = 4;
								strcpy(sOutPayloadData,NVsBLEPara.sGPF);
								//strcpy(sOutPayloadData, GPFVALUE); // Poc2
								break;
								
// MVP Need to implement--------------------
							case DIAGNOSISINIT:	// Diagnosis Init  	
								
								if(sInPayloadLength[0] == 0){		//	read value
									bOutPayloadLength = 1;
									arr[0]= '\0';
									bTemp = GVbInDiag;
									UTIL1_Num8uToStr((unsigned char *)arr, 2, bTemp );
									strcpy(sOutPayloadData, arr);
									
								}else{ 								// set New value
									bOutPayloadLength = 0;
									if( sInUARTArrCmd[3] == 0x31) // to start diagnosis
									{ 
										GVbInDiag = TRUE;							// set flag of unit is in diagnosis
										GVbDIAGLEDFLG = TRUE;
										GVbDIAGVALVEFLG = TRUE;
												
										GVbDiagSensorStat = 0;
										GVbDiagValveStat = 0;
										GVbDiagSolarStat = 0;
										GVbDiagSolarStat = IsSolarCellgetPower();
										
									}
									else{											// just read
										GVbInDiag = FALSE;							// clear flag to let unit work in normal
										//GVbDIAGLEDFLG = FALSE;
										DelayMS(200); 
										GVbFlushActivate = 1;
										
									}	
								}
								break;
/*	Write only				case BLEENABLE:	// BLE Enable 	
								bOutPayloadLength = 4;
								strcpy(sOutPayloadData, SERIALNUMBER);
								break;	
*/
// MVP --------------------------------					
								
/** POC **/					case SENSORSTAT:	// Sensor Status
								bOutPayloadLength = 1;
								 sOutPayloadData[0] = 0x31;
								//sOutPayloadData[0] = 0x30;  //testing
								/*
								if (GVeUserSts == NOTPRESENT)
								{
									sOutPayloadData[0] = 0x30;
								}else{
									sOutPayloadData[0] = 0x31;
								}
								sOutPayloadData[0] = 0x31;
							*/
								break;
																				
/** POC **/					case VALVESTAT:	// Valve Status	
								bOutPayloadLength = 1;
								sOutPayloadData[0] = 0x31;
								//sOutPayloadData[0] = 0x30;  //testing
								/*
								if (GVbSolenoidSts == SOLENOIDON)
								{
									sOutPayloadData[0] = 0x31;
								}else{
									sOutPayloadData[0] = 0x30;
								}
								*/
								break;
								
// MVP Need to implement--------------------
						/*	case TURBINESTAT:	// Turbine Status   	
								bOutPayloadLength = 1;
								sOutPayloadData[0] = 0x30;
								break;*/
							case SOLARPANELSTAT:	// Solar Panel Status 	
								bOutPayloadLength = 1;
								sOutPayloadData[0] = 0x31;
								break;	
// MVP --------------------------------	
 
/** POC **/					case BATLEVEL: // battery level (Hex)	
								
								bTemp = batConvert16to8();
								GVbBatLevelStore = bTemp;
								
								if(bTemp >= 100)
								{
									bOutPayloadLength = 3;
									sOutPayloadData[2] = 0x30;
									sOutPayloadData[1] = 0x30;
									sOutPayloadData[0] = 0x31;
								}else 
								{
									arr[0]= '\0';
									UTIL1_Num8uToStr((unsigned char *)arr, 3, bTemp );
									bOutPayloadLength = strlen(arr);
									strcpy(sOutPayloadData, arr);
								}	
																	
								break;
								
							case OPHRSINCEINSTALL:	// Operation hours since install 	
								
								// total days
								iTemp = NVsOpPara.RDay;
								for(i=0; i< NVsOpPara.RYear;i++){
									iTemp += 365;
								}
								
								lTemp = NVsOpPara.RHour;
								for(ii=0; ii< iTemp;ii++){
									lTemp += 24;
								}
								
								cTemp[0]= '\0';
								UTIL1_Num32uToStr((unsigned char *)cTemp, 9, lTemp );
								bOutPayloadLength = strlen(cTemp);
								strcpy(sOutPayloadData, cTemp);
								break;

/** POC **/					case ACTSINCEINSTALL: //Total Activations (HEX 8 byte) 								
								lTemp = NVsOpPara.TotalActivation;
								cTemp[0]= '\0';
								UTIL1_Num32uToStr((unsigned char *)cTemp, 9, lTemp );
								bOutPayloadLength = strlen(cTemp);
								strcpy(sOutPayloadData, cTemp);
								
								break;
/*
//Handle on Argos
#define DNTLSTFACTRESET	26 		// Date & time of last factory reset
#define DNTLSTRANGECHG	27 		// Date & time of last range change
#define DNTLSTMODECHG	28 		// Date & time of last mode change
#define DNTLSTSENTINALCHG 29 	// Date & time of last Sentinel change
#define DNTLSTDIAG		30 		// Date & time of last diagnostic
*/
								
							case SRSTATUS: // RF Dongle Status Update
								bOutPayloadLength = 0;
								GVbSRModuleStatus = sInUARTArrCmd[3]+0;
								break;		
							
/** may not need here **//* case MULTIPDU: // All command w request
								bOutPayloadLength = 0;
								DelayMS(200);
								GVbSRRefreshCmd = 1;
								break;*/
/** may not need here **/	//case FLUSHACTREQUEST: // Flush Activate Request
								// bOutPayloadLength = 2;
								//	break;								
/** POC **/					case ACTRPTTHOLD: // RF Dongle Status Update
								if(sInPayloadLength[0] == 0){		//	read value
									iTemp = NVsANTPara.iActivationRptTh;
									cTemp[0]= '\0';
									UTIL1_Num16uToStr((unsigned char *)cTemp, 3, iTemp );
									bOutPayloadLength = strlen(cTemp);
									strcpy(sOutPayloadData, cTemp);
								}else{ 	
									bOutPayloadLength = 0;
									GVbActRptThold = (uint16)atoi(sInPayloadData);
									NVsANTPara.iActivationRptTh = GVbActRptThold;
									SaveANTParaToFlash(NVsANTPara);  // save it to flash
								}
								break;
/*	May not need			case DONGLERFENDISABLE: // RF enable disable RW
								bOutPayloadLength = 1;
								
								if(GVbRFEnDisabState == 1)
								{
									sOutPayloadData[0] = 0x31;
									
								}else {
									sOutPayloadData[0] = 0x30;
									
								}

								break;
										
							case SHIPMODEDEEPSLP:	//RF ship mode Enable Disable RW
								bOutPayloadLength = 1;
								//reqOrResponse = 0;
								
								if(NVbDongleDeepSlpModeIn == 1)
								{
									sOutPayloadData[0] = 0x31;
									NVbDongleDeepSlpModeIn = 2;
									
								}else{
									sOutPayloadData[0] = 0x30;
									NVbDongleDeepSlpModeIn = 0;
								}
								break;	
*/								
							case OCCUPANCY:  // Occupancy RO
								bOutPayloadLength = 1;
								
								if(GVbOccupState == 1)
								{
									sOutPayloadData[0] = 0x31;  // enter zone
								}else {
									sOutPayloadData[0] = 0x32; // vacant
								}
								break;
							case FEATUREENABLE: //Feature enable for Occupancy RW
								
								if(sInPayloadLength[0] == 0){		//	read value
									bOutPayloadLength = 2;
									
									if(NVsANTPara.iFeatureOccupEnDisable == 1)
									{
										// 11 = occupancy on
										sOutPayloadData[0] = 0x31;  
										sOutPayloadData[1] = 0x31; 
									}else {
										// 01 occupancy 0ff
										sOutPayloadData[0] = 0x30;  
										sOutPayloadData[1] = 0x31;
									}
								}else{
									bOutPayloadLength = 0;
									if (sInPayloadData[1] == 0x31){
										if(sInPayloadData[0] == 0x31){
											NVsANTPara.iFeatureOccupEnDisable  = 1; // enable
											
										}else {
											NVsANTPara.iFeatureOccupEnDisable  = 0; // disable
										}
										
										SaveANTParaToFlash(NVsANTPara);  // save it to flash
									}
								}
								
								break;
								
// MVP Need to implement--------------------
							case ARMTIMER:	// Arm Timer   	RW
								
								if(sInPayloadLength[0] == 0){		//	read value
									arr[0]= '\0';
									bTemp = NVsOpPara.ArmTM;   
									UTIL1_Num8uToStr((unsigned char *)arr, 3, bTemp );
									bOutPayloadLength = strlen(arr);
									strcpy(sOutPayloadData, arr);
								}else{ 								// write value
									bOutPayloadLength = 0;
									bTemp = (uint8)atoi(sInPayloadData);
									if(bTemp > 28) bTemp = 28;		// maxium 28
									if(bTemp < 4)  bTemp = 4; 		// minium
									NVsOpPara.ArmTM = bTemp;
									SaveParaToFlash(NVsOpPara);		// store to flash
								}
								break;
						/* not MVP1 but coded		
							case PORRESETCNT:	// POR Reset Counter	RO
								iTemp = NVsOpPara.PORResetCT;
								cTemp[0]= '\0';
								UTIL1_Num16uToStr((unsigned char *)cTemp, 3, iTemp );
								bOutPayloadLength = strlen(cTemp);
								strcpy(sOutPayloadData, cTemp);
								break;
							case UIRCURRATIO:			// Urinal IR Current Ratio	RO
								bOutPayloadLength = 1;
								arr[0]= '\0';
								bTemp = GVsTempPara.UrinalIR;   
								UTIL1_Num8uToStr((unsigned char *)arr, 2, bTemp );
								//bOutPayloadLength = strlen(arr);
								strcpy(sOutPayloadData, arr);
								break;  
							case NOISEFLOOR:			// Noise Floor	RW

								if(sInPayloadLength[0] == 0){		//	read value
									//bOutPayloadLength = 4;
									bOutPayloadLength = strlen(dvarNOISEFLOOR);
									strcpy(sOutPayloadData, dvarNOISEFLOOR);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									strcpy(dvarNOISEFLOOR, sInPayloadData);
								}
								break;
							case ECHODIFFERENCE:		// Echo difference	 RW	
								
								if(sInPayloadLength[0] == 0){		//	read value
									//bOutPayloadLength = 4;
									bOutPayloadLength = strlen(dvarECHODIFFERENCE);
									strcpy(sOutPayloadData, dvarECHODIFFERENCE);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									strcpy(dvarECHODIFFERENCE, sInPayloadData);
								}
								break;
							case TGTINTHOLD:			// Target in threshold	RW
								if(sInPayloadLength[0] == 0){		//	read value
									//bOutPayloadLength = 2;
									bOutPayloadLength = strlen(dvarTGTINTHOLD);
									strcpy(sOutPayloadData, dvarTGTINTHOLD);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									strcpy(dvarTGTINTHOLD, sInPayloadData);
								}
								break;
							case TGTOUTTHOLD:			// Target out threshold	RW
								if(sInPayloadLength[0] == 0){		//	read value
									//bOutPayloadLength = 2;
									bOutPayloadLength = strlen(dvarTGTOUTTHOLD);
									strcpy(sOutPayloadData, dvarTGTOUTTHOLD);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									strcpy(dvarTGTOUTTHOLD, sInPayloadData);
								}
								break;
							case TGTSTAYINTHOLD:		// Target Stay in threshold	RW	
								if(sInPayloadLength[0] == 0){		//	read value
									//bOutPayloadLength = 2;
									bOutPayloadLength = strlen(dvarTGTSTAYINTHOLD);
									strcpy(sOutPayloadData, dvarTGTSTAYINTHOLD);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									strcpy(dvarTGTSTAYINTHOLD, sInPayloadData);
								}
								break;
							case TGTSTAYOUTTHOLD:		// Target stay out threshold RW		
								if(sInPayloadLength[0] == 0){		//	read value
									//bOutPayloadLength = 2;
									bOutPayloadLength = strlen(dvarTGTSTAYOUTTHOLD);
									strcpy(sOutPayloadData, dvarTGTSTAYOUTTHOLD);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									strcpy(dvarTGTSTAYOUTTHOLD, sInPayloadData);
								}
								break;
							case TGTSTAYOUTTHOLD1:	// Target stay out threshold RW		
								if(sInPayloadLength[0] == 0){		//	read value
									//bOutPayloadLength = 2;
									bOutPayloadLength = strlen(dvarTGTSTAYOUTTHOLD1);
									strcpy(sOutPayloadData, dvarTGTSTAYOUTTHOLD1);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									strcpy(dvarTGTSTAYOUTTHOLD1, sInPayloadData);
								}
								break;
							case TGTBACKTHOLD:		// Target back threshold RW		
								if(sInPayloadLength[0] == 0){		//	read value
									//bOutPayloadLength = 2;
									bOutPayloadLength = strlen(dvarTGTBACKTHOLD);
									strcpy(sOutPayloadData, dvarTGTBACKTHOLD);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									strcpy(dvarTGTBACKTHOLD, sInPayloadData);
								}
								break;
							case TGTSTAYTHOLD:		// Target stay threshold RW	
								if(sInPayloadLength[0] == 0){		//	read value
									//bOutPayloadLength = 2;
									bOutPayloadLength = strlen(dvarTGTSTAYTHOLD);
									strcpy(sOutPayloadData, dvarTGTSTAYTHOLD);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									strcpy(dvarTGTSTAYTHOLD, sInPayloadData);
								}
								break;
							case BACKTOIDLETHOLD:		// Back to Idle threshold RW	
								if(sInPayloadLength[0] == 0){		//	read value
									//bOutPayloadLength = 2;
									bOutPayloadLength = strlen(dvarBACKTOIDLETHOLD);
									strcpy(sOutPayloadData, dvarBACKTOIDLETHOLD);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									strcpy(dvarBACKTOIDLETHOLD, sInPayloadData);
								}
								break;
						*/		
							case SITTINGUSRTHOLD:		// Sitting user threshold; Solis = NVsOpPara.MinUserTH RW	
								if(sInPayloadLength[0] == 0){		//	read value
									//bOutPayloadLength = strlen(dvarSITTINGUSRTHOLD);
									//strcpy(sOutPayloadData, dvarSITTINGUSRTHOLD);
									cTemp[0]= '\0';
									UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.MinUserTH );								 
									bOutPayloadLength = strlen(cTemp);
									strcpy(sOutPayloadData, cTemp);
									
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									//strcpy(dvarSITTINGUSRTHOLD, sInPayloadData);
									NVsOpPara.MinUserTH = (uint16)atoi(sInPayloadData);
									SaveParaToFlash(NVsOpPara); // store to flash
								}
								break;
							case BGCHANGETHOLD:		// Background change threshold; Solis = NVsOpPara.TouchTH RW	
								if(sInPayloadLength[0] == 0){		//	read value
								
									//bOutPayloadLength = strlen(dvarBGCHANGETHOLD);
									//strcpy(sOutPayloadData, dvarBGCHANGETHOLD);
									cTemp[0]= '\0';
									UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.TouchTH );							 
									bOutPayloadLength = strlen(cTemp);
									strcpy(sOutPayloadData, cTemp);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									//strcpy(dvarBGCHANGETHOLD, sInPayloadData);
									NVsOpPara.TouchTH = (uint16)atoi(sInPayloadData);
									SaveParaToFlash(NVsOpPara); // store to flash
									
								}
								break;
							case IRTGTTHOLD:			// IR Target Threshold; Solis = NVsOpPara.MaxBackground RW
								if(sInPayloadLength[0] == 0){		//	read value
									
									//bOutPayloadLength = strlen(dvarIRTGTTHOLD);
									//strcpy(sOutPayloadData, dvarIRTGTTHOLD);
									cTemp[0]= '\0';
									UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.MaxBackground );							 
									bOutPayloadLength = strlen(cTemp);
									strcpy(sOutPayloadData, cTemp);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									//strcpy(dvarIRTGTTHOLD, sInPayloadData);
									NVsOpPara.MaxBackground = (uint16)atoi(sInPayloadData);
									SaveParaToFlash(NVsOpPara); // store to flash
								}
								break;
							case IRUPDATETHOLD:		// IR update threshold; Solis = NVsOpPara.ONDelayTM RW	
								if(sInPayloadLength[0] == 0){		//	read value
									
									//bOutPayloadLength = strlen(dvarIRUPDATETHOLD);
									//strcpy(sOutPayloadData, dvarIRUPDATETHOLD);
									cTemp[0]= '\0';
									UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.ONDelayTM );								 
									bOutPayloadLength = strlen(cTemp);
									strcpy(sOutPayloadData, cTemp);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									//strcpy(dvarIRUPDATETHOLD, sInPayloadData);
									NVsOpPara.ONDelayTM = (uint8)atoi(sInPayloadData);
									SaveParaToFlash(NVsOpPara); // store to flash
								}
								break;
						/* not MVP1 but coded					
							case IRCHANGETHOLD:		// IR change Threshold RW		
								if(sInPayloadLength[0] == 0){		//	read value
									//bOutPayloadLength = 1;
									bOutPayloadLength = strlen(dvarIRCHANGETHOLD);
									strcpy(sOutPayloadData, dvarIRCHANGETHOLD);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									strcpy(dvarIRCHANGETHOLD, sInPayloadData);
								}
								break;
						*/		
							case MAXIRTHOLD:			// Maximum IR Threshold	R
								//bOutPayloadLength = 4;
								iTemp = NVsOpPara.MaxIRTH;
								cTemp[0]= '\0';
								UTIL1_Num16uToStr((unsigned char *)cTemp, 6, iTemp );
								bOutPayloadLength = strlen(cTemp);
								strcpy(sOutPayloadData, cTemp);
								break;
						/* not MVP1 but coded		
							case MINIRTHOLD:			// Minimum IR Threshold	R
								//bOutPayloadLength = 4;
								iTemp = NVsOpPara.MinIRTH;
								cTemp[0]= '\0';
								UTIL1_Num16uToStr((unsigned char *)cTemp, 5, iTemp );
								bOutPayloadLength = strlen(cTemp);
								strcpy(sOutPayloadData, cTemp);
								break;
							case MINBG:				// Minimum background RW
								if(sInPayloadLength[0] == 0){		//	read value
									//bOutPayloadLength = 4;
									bOutPayloadLength = strlen(dvarMINBG);
									strcpy(sOutPayloadData, dvarMINBG);
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
									strcpy(dvarMINBG, sInPayloadData);
								}						
								break;
						*/		
							case TOFDISTCALFACTOR:	// Time of flight distance calibration factor; solis calibration flag R			
								//bOutPayloadLength = 3;
								//strcpy(sOutPayloadData, dvarTOFDISTCALFACTOR);
								cTemp[0]= '\0';
								UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.CalibrationFlag );								 
								bOutPayloadLength = strlen(cTemp);
								strcpy(sOutPayloadData, cTemp);
								break;
							case TOFOFFSETCALFACTOR:	// Time of flight offset calibration factor; Solis = NVsOpPara.CalibrationEcho	R	
								//bOutPayloadLength = 4;
								//strcpy(sOutPayloadData, dvarTOFOFFSETCALFACTOR);
								cTemp[0]= '\0';
								UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.CalibrationEcho );								 
								bOutPayloadLength = strlen(cTemp);
								strcpy(sOutPayloadData, cTemp);
								break;
							case DISTADJCNT:			// Distance adjust counter; Solis = NVsOpPara.ConfirmTimeTH	R
								//bOutPayloadLength = 2;
								//strcpy(sOutPayloadData, dvarDISTADJCNT);
								cTemp[0]= '\0';
								UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.ConfirmTimeTH );								 
								bOutPayloadLength = strlen(cTemp);
								strcpy(sOutPayloadData, cTemp);
								break;
							case DISTADJFAILCNT:		// Distance adjust fail counter; Solis = NVsOpPara.AdjudtedFailCT	R	
								//bOutPayloadLength = 2;
								//strcpy(sOutPayloadData, dvarDISTADJFAILCNT);
								cTemp[0]= '\0';
								UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.AdjudtedFailCT );							 
								bOutPayloadLength = strlen(cTemp);
								strcpy(sOutPayloadData, cTemp);
								break;
							case SALKDNIRRECCAL:		// Sensor A (Look down) IR current from recently calibration or default; Solis = NVsOpPara.BVolt	R	
								//bOutPayloadLength = 4;
								//strcpy(sOutPayloadData, dvarSALKDNIRRECCAL);
								cTemp[0]= '\0';
								UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.BVolt );							 
								bOutPayloadLength = strlen(cTemp);
								strcpy(sOutPayloadData, cTemp);
								break;
					/* not MVP1 but coded				
							case SBLKUPIRRECCAL:		// Sensor B (Look up) IR current from recently calibration or default; Solis = NVsOpPara.ONDelayTM R	
								//bOutPayloadLength = 4;
								//strcpy(sOutPayloadData, dvarSBLKUPIRRECCAL);
								cTemp[0]= '\0';
								UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.ONDelayTM );						 
								bOutPayloadLength = strlen(cTemp);
								strcpy(sOutPayloadData, cTemp);
								break;
						*/
							case SALKDNIRMFGCAL:		// Sensor A (Look down) IR current from manufacture calibration or default; Solis = NVsOpPara.WaitDelayTM R	
								//bOutPayloadLength = 4;
								//strcpy(sOutPayloadData, dvarSALKDNIRMFGCAL);
								cTemp[0]= '\0';
								UTIL1_Num8uToStr((unsigned char *)cTemp, 4, NVsOpPara.WaitDelayTM );					 
								bOutPayloadLength = strlen(cTemp);
								strcpy(sOutPayloadData, cTemp);
								break;
					/* not MVP1 but coded			
							case SALKUPIRMFGCAL:		// Sensor B (Look up) IR current from manufacture calibration or default R		
								bOutPayloadLength = 4;
								strcpy(sOutPayloadData, dvarSALKUPIRMFGCAL);
								break;
							case SALKDNMAXNLVL:		// Sensor A (Look down) Maximum noise level	R
								bOutPayloadLength = 4;
								strcpy(sOutPayloadData, dvarSALKDNMAXNLVL);
								break;
							case SBLKUPMAXNLVL:		// Sensor B (Look up) Maximum noise level R	
								bOutPayloadLength = 4;
								strcpy(sOutPayloadData, dvarSBLKUPMAXNLVL);
								break;
						*/		
							case SALKDNBGCURUSED:		// Sensor A (Look down) background for current used; Solis = NVsOpPara.CleanBackground	R	
								//bOutPayloadLength = 4;
								//strcpy(sOutPayloadData, dvarSALKDNBGCURUSED);
								cTemp[0]= '\0';
								UTIL1_Num16uToStr((unsigned char *)cTemp, 6, NVsOpPara.CleanBackground );							 
								bOutPayloadLength = strlen(cTemp);
								strcpy(sOutPayloadData, cTemp);
								break;
						/* not MVP1 but coded		
							case SBLKUPBGCURUSED:		// Sensor B (Look up) background for current used R	
								bOutPayloadLength = 4;
								strcpy(sOutPayloadData, dvarSBLKUPBGCURUSED);
								break;
							case TOTACTCNTLOWBAT:		// Total activation count in low battery R	
								//bOutPayloadLength = 4;
								iTemp = NVsOpPara.LBActivationCT;
								cTemp[0]= '\0';
								UTIL1_Num16uToStr((unsigned char *)cTemp, 5, iTemp );
								bOutPayloadLength = strlen(cTemp);
								strcpy(sOutPayloadData, cTemp);
								break;
							case MANUALACTTIMES:		// Manual Activation times	R	
								bOutPayloadLength = 4;
								strcpy(sOutPayloadData, dvarMANUALACTTIMES);
								break;
						*/		
							case DNTLSTFMWUPDATE:		// Date & time of last firmware update	RW
								
								if(sInPayloadLength[0] == 0){		//	read value
									//bOutPayloadLength = 6;
									//UpdateM,UpdateD,UpdateY
									//strcpy(sOutPayloadData, "8888");
									
									cTemp[0]= '\0';
									cTempDate[0]= '\0';
									UTIL1_Num16uToStr((unsigned char *)cTempDate, 5, NVsOpPara.UpdateY );
									cTemp[0] = cTempDate[2];
									cTemp[1] = cTempDate[3];
									cTempDate[0]= '\0';
									UTIL1_Num8uToStr((unsigned char *)cTempDate, 3, NVsOpPara.UpdateM );
									if ( NVsOpPara.UpdateM >= 10)
									{
										cTemp[2] = cTempDate[0];
										cTemp[3] = cTempDate[1];
									}else
									{
										cTemp[2] = 0x30;
										cTemp[3] = cTempDate[0];
									}
									cTempDate[0]= '\0';
									UTIL1_Num8uToStr((unsigned char *)cTempDate, 3, NVsOpPara.UpdateD );
									if (NVsOpPara.UpdateD >= 10)
									{
										cTemp[4] = cTempDate[0];
										cTemp[5] = cTempDate[1];
									}else
									{
										cTemp[4] = 0x30;
										cTemp[5] = cTempDate[0];
									}
									cTemp[6] = '\0'; 
									strcpy(sOutPayloadData, cTemp);
									bOutPayloadLength = 6;
									
									/*cTemp[0]= '\0';
									iTemp = NVsOpPara.UpdateY;                      	// get the data
									UTIL1_Num16uToStr((unsigned char *)cTemp, 3, iTemp );
									strcpy(sOutPayloadData, cTemp);
									cTemp[0]= '\0';
									iTemp = NVsOpPara.UpdateM;                      	// get the data
									UTIL1_Num16uToStr((unsigned char *)cTemp, 3, iTemp );
									strcat(sOutPayloadData,cTemp);
									cTemp[0]= '\0';
									iTemp = NVsOpPara.UpdateD;                      	// get the data
									UTIL1_Num16uToStr((unsigned char *)cTemp, 3, iTemp );
									strcat(sOutPayloadData,cTemp);
									bOutPayloadLength = strlen(sOutPayloadData);*/
									
								}else{ 								// write value
									bOutPayloadLength = 0;
									//write to var
								}	
								
								break;									
// MVP --------------------------------	
								
							default:
								isCmdNotFound = TRUE;			// cmd not in list
							//	GVbUartCmdStateNerrFlg = 4;		// SR Request invalid cmd
								break;
																						
						} // end switch(icmd)
					
					
						/* Send Response if command exist  - Sensor -> SR Sim board */
						if (!isCmdNotFound){
							
							// respond to RF Dongle begin
							//bOutPayloadLength is known 
							sOutMsgLength[0]= bOutPayloadLength + 2;
							bOutCheckSumLengthResponse = 4 + bOutPayloadLength;
					
							sOutUARTArrCmd[0]='\0';					// Clear out string
							sOutUARTArrCmd[0] = STARTBYTE;
							sOutUARTArrCmd[1] = sOutMsgLength[0];
							sOutUARTArrCmd[2] = sCmdID[0]+0x80;
					
							sOutUARTArrCmd[3] = ECSUCCESS;
							// put out payload data
							for(i = 0; i < bOutPayloadLength; i++){
								sOutUARTArrCmd[4+i] = sOutPayloadData[i];
							} 
							sOutUARTArrCmd[bOutPayloadLength+4] = calculateChecksum(sOutUARTArrCmd,1,bOutCheckSumLengthResponse);
							sOutUARTArrCmd[bOutPayloadLength+5] = ENDBYTE;
							retryCnt = 0;
							
							while (retryTwice){
								
								//UARTOutStrPlusNull(sOutUARTArrCmd,bOutCheckSumLengthResponse+2);
								UartWakeInt_Disable(); // disable interrupt
								
								/* Check Wake up pin high before send 4/4/23 --mod 4/10/23*/
								if(UartWake_GetVal()){
									UartWake_SetOutput(); //set UartWake to output (pin 27)
																	
									//begin 50ms pulse high to low turn on SR RF 
									UartWake_ClrVal();
									DelayMS(50);			// pull up 50 ms to wake up BLE
									UartWake_SetVal();   // pull up
									DelayMS(50);	
									UartWake_ClrVal(); // Pull low
									DelayMS(50);
	
									UARTOutStrPlusNull(sOutUARTArrCmd,bOutCheckSumLengthResponse+2);
									
									UartWake_SetVal();   // pull up 
									UartWake_SetInput(); // reset UartWake as input (pin 27) 
									
								}else
								{
									UARTOutStrPlusNull(sOutUARTArrCmd,bOutCheckSumLengthResponse+2);
								}
								//--
								
								UartWakeInt_Enable(); // re enable interrupt
								
								//--- orginal code 
//								UartWakeInt_Disable(); // disable interrupt
//								UartWake_SetOutput(); //set UartWake to output (pin 27)
//																
//								//begin 50ms pulse high to low turn on SR RF 
//								UartWake_ClrVal();
//								DelayMS(50);			// pull up 50 ms to wake up BLE
//								UartWake_SetVal();   // pull up
//								DelayMS(50);	
//								UartWake_ClrVal(); // Pull low
//								DelayMS(50);
//
//								UARTOutStrPlusNull(sOutUARTArrCmd,bOutCheckSumLengthResponse+2);
//								
//								UartWake_SetVal();   // pull up 
//								UartWake_SetInput(); // reset UartWake as input (pin 27) 
//								UartWakeInt_Enable(); // re enable interrupt
								//--- orginal code end
					
								chkACKResponse = TRUE;
							
								// check Response ACK begin 
								bIndex = 0;				// initilize
								WDReset();				// kick dog
								wTimerTick = 0;								// reset timer
								TI1_Enable();
						
								
								/* Get Response ACK  - SR Sim board -> Sensor */	
								while (chkACKResponse){
									WDReset();				// kick dog
									
									if(AS1_RecvChar(&bTemp) == ERR_OK){					// Received something 
														
										if(bTemp == STARTBYTE){								// start with 0xF7, start byte of message(sheng) 0x63 pre
											sInUARTArrCmd[bIndex] = bTemp;					// move to incoming command buffer
											bIndex++;									// next position
											WDReset();									// Kick dog
											wTimerTick = 0;								// reset timer
											//EndByteReceived = FALSE;					// 9/8/20 End byte received false
											TI1_Enable();								// Enable timer to terminate if something wrong
		
											//while((bTemp != ENDBYTE) && !EndByteReceived) {						// not the end of command, 0xF8 end byte(sheng)0x0D pre
											while(bTemp != ENDBYTE) {						// not the end of command, 0xF8 end byte(sheng)0x0D pre
												WDReset();
												if(wTimerTick >= TIMERWAITLIMIT){	 	// check if time run out (300msec)
												//if(wTimerTick >= 600){	 	// check if time run out (300msec)
													GVbUartCmdStateNerrFlg = 6;  		// SR Response ACK Time Out
													chkACKResponse = FALSE;
													break; 								// can't receive whole package. break
												}
									
												if(AS1_RecvChar(&bTemp) == ERR_OK){		// Received something 
													sInUARTArrCmd[bIndex] = bTemp;			// move to command buffer
													bIndex++;							// next position
													
												/*	// added 9/8/20 version 3  to fix F8 F8 when checksum same as F8 of endbyte
													if((bTemp == ENDBYTE) && ((sInUARTArrCmd[1] + 4) == bIndex))			
													{
														EndByteReceived = TRUE;
													}
													*/
													
													if(bIndex > 19){					// exceed the maxium length 
														chkACKResponse = FALSE;
														GVbUartCmdStateNerrFlg = 7; 	// SR Response ACK Payload exceed
														break;
													}
												}
											} //end while(bTemp							// end of command or time out
															
											TI1_Disable();								// stop timer to save power and 
																					
											// check if we have start byte and end byte											
											if((sInUARTArrCmd[0] == STARTBYTE) && (sInUARTArrCmd[bIndex-1] == ENDBYTE) && (sInUARTArrCmd[1] == sOutUARTArrCmd[2])){
									
												GVbStorePreviousCmd = sCmdID[0];  // store complete command
												isRequestComplete = TRUE;
												chkACKResponse = FALSE;
												retryTwice = FALSE;
												GVbUartCmdStateNerrFlg = 10; // SR Response ACK received
												break;	
											}else
											{
												//isRequestComplete = FALSE;
												GVbUartCmdStateNerrFlg = 8; // SR Response Ack invalid cmd
												chkACKResponse = FALSE;
												retryCnt++;
												break;
											}
										}
									}else // end if(AS1_RecvChar(&bTemp) == ERR_OK){	
									{
										if(wTimerTick >= TIMERWAITLIMIT){	 			// check if time run out (300ms)
											//LED_SetVal();
											//SleepMS(100);  //1000
											//LED_ClrVal();
								
											GVbUartCmdStateNerrFlg = 6;  		// SR Response ACK Time Out
											chkACKResponse = FALSE;
											retryCnt++;
											TI1_Disable();								// stop timer to save power
											break; 								// can't receive whole package. break
										}
									}
								} //end while(chkACKResponse
								
								if(retryCnt > 1)
								{
									isRequestComplete = TRUE;
									GVbUartCmdStateNerrFlg = 9;  		// SR Response ACK retry twice fail
									retryTwice = FALSE;
								}
							} // end while(retryTwice)
						} else //end if (!isCmdNotFound
						{
							isRequestComplete = TRUE;
							GVbUartCmdStateNerrFlg = 4; // SR Request invalid cmd
						}
					} //end if (sCmdID[0]
				} else // end if(sInCalChecksum[0]
				{
					isRequestComplete = TRUE;
					GVbUartCmdStateNerrFlg = 3;  // SR Request invalid checksum
				}
			} else // end if ((sInUARTArrCmd[0] ....	
			{
				isRequestComplete = TRUE;
				GVbUartCmdStateNerrFlg = 4; // SR Request invalid cmd
			}
		} // end if(bTemp

		AS1_ClearRxBuf();
		//AS1_ClearTxBuf();
	} // end of receiving any thing
	return isRequestComplete;		
}



/*
** =================================================================================================================================
**     Method      :  void UARTProcess(void)
**     Description :
**         This method process commands from UART
**         Note: BLE would go to advitising after collectting all data at the moment being waked up.
**         
**     Parameters  : Nothing
**     Returns     : Nothing
** =================================================================================================================================
*/

uint8 UARTProcess(void){		
	
	// Make sure UART is ready before wake up RF dongle, RF dongle may send right away
	//if(GVbBLEEnabled == FALSE){	// not enabled. It is the first time 
	bool isUartProcessTimeOut = FALSE;
	
		WDReset();	// kick dog		
		wTimerTick = 0;			// reset timer
		TI1_Enable();			// use the timer to prevent endless loop
		//GVbBLEEnabled = TRUE;	// Only enable once. 
			
		while((ProcessingSingleUARTCommand()) != TRUE){ // waiting for BLE initialize (collect all data)
			WDReset();	// kick dog
			//if(wTimerTick >= TIMERWAITLIMIT){  //not working
			if(wTimerTick >= MAXWAITTIME){		// needed to work
				//LED_SetVal();
				//SleepMS(1000);  //1000
			//	LED_ClrVal();
				
				isUartProcessTimeOut = TRUE;
				break; 								// can't receive whole package. break
			}
			
			// add logic for the uart cmd state
			/*if (GVbUartCmdStateNerrFlg == ??)
			{
				
			}*/
			
		}
		
		TI1_Disable();
	return isUartProcessTimeOut;	
}

/*
** =========================================================================================================================
**     Method      :ANTDongleParaType LoadANTParaFromFlash(void){
**     Description :
**         This method get parameters from flash. if data in flash is corrupted (or blank), use default instead.
**     Parameters  : Nothing
**     Returns     : Parameter struck
** =========================================================================================================================
*/
ANTDongleParaType LoadANTParaFromFlash(void){
	
	uint16 len;
	uint8 bTempCRC;

	ANTDongleParaType sPara;
	uint8 bTempFlag;
	IFsh1_TAddress FlashAdress;

		IFsh1_EnableEvent();																// Enable flash process events
		len = sizeof sPara;
		FlashAdress = FLASHANTADDRESS;
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
			sPara = LoadDefaultANTPara();	// load default 
		}
		
		return sPara;
}

/*
** =========================================================================================================================
**     Method      :ANTDongleParaType LoadDefaultANTPara(void){
**     Description :
**         This method use default instead.
**     Parameters  : Nothing
**     Returns     : Parameter struck
** =========================================================================================================================
*/
ANTDongleParaType LoadDefaultANTPara(void){
	
	ANTDongleParaType sPara;
	
	sPara.iActivationRptTh = 1;
	sPara.iShipModeDeepSleep = 0;
	sPara.iFeatureOccupEnDisable = 0;  	// default 0, changed to 0 on 6/22/2022
	sPara.iRFDongleEnDisable = 0;		// default 0 RF enable , 1 RF disable 3/30/23 add
	SaveANTParaToFlash(sPara);  		// save it to flash
		
	return sPara;
}

/*
** ===================================================================================================================================================================
**     Method      :void SaveANTParaToFlash(ANTDongleParaType sPara){
**     Description :
**         This method save ANT related parameters to flash. Also the data CRC is calculated and stored at the end of parameter.
**         A written flag is also stored at the end.
**     Parameters  : parameter
**     Returns     : Nothing
** ===================================================================================================================================================================
*/
void SaveANTParaToFlash(ANTDongleParaType sPara){
	uint16 len;
	uint8 bTempCRC;	
	uint8 bTempFlag = WRITTENFLAG;
	IFsh1_TAddress FlashAdress;
	
		len = sizeof sPara;
		bTempCRC = CalculateCRC((uint8*)&sPara, len);
		IFsh1_EnableEvent();		//Enable flash process events
		FlashAdress = FLASHANTADDRESS;
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

/* Sheng MVP Mod end */ 

