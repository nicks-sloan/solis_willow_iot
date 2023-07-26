/*
 * TouchSensor.c
 *
 *  	Created on: Feb 17, 2017
 *      Author: Scott Wang
 *      This module contains all touch sensing functions
 */


#include "TouchSensor.h"
#include "Operation.h"
#include "Timing.h"

// module variable, these would be cleared even for partial start
static uint16 MGVwBigButtonBaseline;  			// Big button baseline
static uint16 MGVwBigCapInstant;				// Big button instant reading
static uint16 MGVwSmallButtonBaseline;			// Small button baseline
static uint16 MGVwSmallCapInstant;				// Small button instant reading 

//static uint16 MGVwMinum0;						// Big button instant reading
//static uint16 MGVwMinum1;						// Small button baseline
//static uint16 MGVwMinum2;						// Small button instant reading 
static uint16 MGVwFiltered;						// Small button instant reading 

/*
** ========================================================================================================================================================
**     Method	: 
**     uint16 GetBigButtonBaselineValue(void);
**     uint16 GetBigButtonCapInstantValue(void);
**     uint16 GetSmallButtonBaselineValue(void);
**     uint16 GetSmallButtonCapInstantValue(void);
**     
**     Description :
**         These methods just export module data
**     Parameters  : None
**     Returns     : BigButtonBaselineValue
** ========================================================================================================================================================
*/
uint16 GetBigButtonBaselineValue(void){
	return MGVwBigButtonBaseline;
}

uint16 GetBigButtonCapInstantValue(void){
	return MGVwBigCapInstant;
}

uint16 GetSmallButtonBaselineValue(void){
	return MGVwSmallButtonBaseline;
}

uint16 GetSmallButtonCapInstantValue(void){
	return MGVwSmallCapInstant;
}

uint16 GetFilteredValue(void){
	return MGVwFiltered;
}

/*
** ========================================================================================================================================================
**     Method      :  void TButtonInit(void)
**     Description :
**         This method initialize TSI module hardware.
**     Parameters  : None
**     Returns     : Nothing
** ========================================================================================================================================================
*/
void TButtonInit(void){
	
	/* SIM_SCGC5: TSI=1 */
	  SIM_SCGC5 |= SIM_SCGC5_TSI_MASK;
	  /* TSI0_GENCS: OUTRGF=1,ESOR=0,MODE=0,REFCHRG=0,DVOLT=0,EXTCHRG=0,PS=0,NSCN=0,TSIIEN=0,STPE=1,STM=0,EOSF=1,CURSW=0 */
	  TSI0_GENCS = (uint32_t)((TSI0_GENCS & (uint32_t)~(uint32_t)(
	                TSI_GENCS_ESOR_MASK |
	                TSI_GENCS_MODE(0x0F) |
	                TSI_GENCS_REFCHRG(0x07) |
	                TSI_GENCS_DVOLT(0x03) |
	                TSI_GENCS_EXTCHRG(0x07) |
	                TSI_GENCS_PS(0x07) |
	                TSI_GENCS_NSCN(0x1F) |
	                TSI_GENCS_TSIIEN_MASK |
	                TSI_GENCS_STM_MASK |
	                TSI_GENCS_CURSW_MASK
	               )) | (uint32_t)(
	                TSI_GENCS_OUTRGF_MASK |
	                TSI_GENCS_STPE_MASK |
	                TSI_GENCS_EOSF_MASK
	               ));
	  /* TSI0_TSHD: THRESH=0,THRESL=0 */
	  TSI0_TSHD = (TSI_TSHD_THRESH(0x00) | TSI_TSHD_THRESL(0x00));
	  /* TSI0_DATA: TSICH=0,DMAEN=0 */
	  TSI0_DATA &= (uint32_t)~(uint32_t)(
	                TSI_DATA_TSICH(0x0F) |
	                TSI_DATA_DMAEN_MASK
	               );
	  /* TSI0_GENCS: OUTRGF=0,TSIEN=1,EOSF=0 */
	  TSI0_GENCS = (uint32_t)((TSI0_GENCS & (uint32_t)~(uint32_t)(
	                TSI_GENCS_OUTRGF_MASK |
	                TSI_GENCS_EOSF_MASK
	               )) | (uint32_t)(
	                TSI_GENCS_TSIEN_MASK
	               ));	  
}

/*
** ========================================================================================================================================================
**     Method      :  void TButtonStartScan(uint8 chan)
**     Description :
**         This method starts one TSI scan.
**     Parameters  : button channel
**     Returns     : Nothing
** ========================================================================================================================================================
*/
void TButtonStartScan(uint8 chan){
	
	switch(chan){
	
		case BIGBUTTON:	
			TSI0_DATA &= (uint32_t)~(uint32_t)(
		                TSI_DATA_TSICH(0x0F) |
		                TSI_DATA_DMAEN_MASK
		               );
		break;
		
		case SMALLBUTTON:
			TSI0_DATA = (uint32_t)((TSI0_DATA & (uint32_t)~(uint32_t)(
		              TSI_DATA_TSICH(0x01) |
		              TSI_DATA_DMAEN_MASK
		             )) | (uint32_t)(
		              TSI_DATA_TSICH(0x0E)
		             ));	
		break;
		
		default:
		break;
	}
	
	TSI0_DATA |= TSI_DATA_SWTS_MASK;  //start one scan
	
}

/*
** ========================================================================================================================================================
**     Method      :  TButtonClr(void)
**     Description :
**         This method reset scan completion flag.
**     Parameters  : None
**     Returns     : Nothing
** ========================================================================================================================================================
*/
void TButtonClr(void){
	TSI0_GENCS |= TSI_GENCS_EOSF_MASK;
}


/*
** ========================================================================================================================================================
**     Method      :  uint16 TButtonInitBaseline(uint8 chan)
**     Description :
**         This method initialize button baseline.
**         Note: it's noticed that the result data are differnt between scaning during sleep and scaning in active
**     Parameters  : channel
**     Returns     : Baseline
** ========================================================================================================================================================
*/
uint16 TButtonInitBaseline(uint8 chan){
	uint8 lp;
	uint16 wTempIns, wTempBase;	
	
	
		TButtonStartScan(chan);
		while((TSI0_GENCS & TSI_GENCS_EOSF_MASK) == 0);		// waiting for scan complete
		wTempBase = CAPDATA;								// First scan data
		TButtonClr();										// clear scan
		
		for(lp = 0; lp < 12; lp++){							// for 3 seconds
			wTempIns = CAPDATA;								// Get cap sensor data
			TButtonClr();
			wTempBase = GetDataMovingEverage(wTempIns,wTempBase);
			TButtonStartScan(chan);
			SleepMS(250);
		}
		return wTempBase;
}

/*
** ========================================================================================================================================================
**     Method      :  void ToutchButtonInit(void)
**     Description :
**         This method initialize touch button related data. This should be called before use buttons.
**     Parameters  : None
**     Returns     : Nothing
** ========================================================================================================================================================
*/
/*
void ToutchButtonInit(void){
		
		MGVwBigButtonBaseline = TButtonInitBaseline(BIGBUTTON);				// Initialize big button baseline
		MGVwBigCapInstant = MGVwBigButtonBaseline;							// instant data
		
		//#ifdef TWOBUTTON
		#ifdef BUILDDUAL
			MGVwSmallButtonBaseline = TButtonInitBaseline(SMALLBUTTON);		// Initialize small button baseline
			MGVwSmallCapInstant = MGVwSmallButtonBaseline;											// use default instead
		#endif
		
		//if(NVsOpPara.Mode == DUALMODE){										// for dual flush operation
		//	MGVwSmallButtonBaseline = TButtonInitBaseline(SMALLBUTTON);		// Initialize small button baseline
		//	MGVwSmallCapInstant = MGVwSmallButtonBaseline;
		//}
		
		//#endif
		
			MGVwMinum0 = 0;
			MGVwMinum1 = 0;			// Small button baseline
			MGVwMinum2 = 0;			// Small button instant reading 
			MGVwFiltered = 0;
			//bData = IsButtonTouched(BIGBUTTON);		// Get big button current states, scan started before sleep and it should be down during sleep.
		
}
*/

/*
void ToutchButtonInit(void){

	BlinkLED(1);
	MGVwBigButtonBaseline = TButtonInitBaseline(BIGBUTTON);				// Initialize big button baseline
	MGVwBigCapInstant = MGVwBigButtonBaseline;							// instant data
	BlinkLED(1);	
	//#ifdef TWOBUTTON
	#ifdef BUILDDUAL
		MGVwSmallButtonBaseline = TButtonInitBaseline(SMALLBUTTON);		// Initialize small button baseline
		MGVwSmallCapInstant = MGVwSmallButtonBaseline;					// Initilize instant data
	#endif
	#ifndef BUILDDUAL
		SleepMS(3000);													// match the time as the dual flush
	#endif
		//if(NVsOpPara.Mode == DUALMODE){										// for dual flush operation
		//	MGVwSmallButtonBaseline = TButtonInitBaseline(SMALLBUTTON);		// Initialize small button baseline
		//	MGVwSmallCapInstant = MGVwSmallButtonBaseline;
		//}
		
		//#endif
		
			MGVwMinum0 = 0;
			MGVwMinum1 = 0;			// Small button baseline
			MGVwMinum2 = 0;			// Small button instant reading 
			MGVwFiltered = 0;
			//bData = IsButtonTouched(BIGBUTTON);		// Get big button current states, scan started before sleep and it should be down during sleep.
		
}
*/
/*
** ========================================================================================================================================================
**     Method      :  void ToutchButtonInit(void)
**     Description :
**         This method initialize touch button related data. This should be called before use buttons.
**     Note: it's noticed that the result data are differnt between scaning during sleep and scaning in active. Bigbutton scaning is during sleep, small button is in active
**     Parameters  : None
**     Returns     : Nothing
** ========================================================================================================================================================
*/

void ToutchButtonInit(void){
uint8 lp;

	for(lp = 0; lp < 2; lp++){							// skip the first sample						
		TButtonClr();
		TButtonStartScan(BIGBUTTON);
		SleepMS(250);
	}
	MGVwBigCapInstant = CAPDATA;						// take the data
	MGVwBigButtonBaseline  = MGVwBigCapInstant;			// sset as baseline
	
	#ifdef BUILDDUAL									// for dual buttons
		for(lp = 0; lp < 2; lp++){							
			TButtonClr();
			TButtonStartScan(SMALLBUTTON);
			SleepMS(250);
		}
		MGVwSmallCapInstant = CAPDATA;
		MGVwSmallButtonBaseline  = MGVwSmallCapInstant;
	#endif
		
	GVsButtonSts.bBigButton = NOTOUCH;					// big Button not touched
	GVsButtonSts.bSmallButton = NOTOUCH;				// small Button Status
	GVsButtonSts.bCombination = BUSU;					// buttons Combination status	
				
}

/*
** ========================================================================================================================================================
**     Method      :  uint8 IsButtonTouched(uint8 chan)
**     Description :
**         This method determine button states based on scan data. It also maintains baseline and store current data.
**         Note, the scan has to be completed before call this method.
**     Parameters  : Button channel
**     Returns     : button status
** ========================================================================================================================================================
*/

uint8 IsButtonTouched(uint8 chan){

	uint8 bResult;
	uint16 wTempIns, wTempBack, wPreCapIns;
	uint16 * pwTempCapTns;					// point to store instant value
	uint16 * pwTempBaseline;				// point to store baseline
	static uint8 bCountBG = 0;              // count of signal drop for big button
	static uint8 bCountSM = 0;              // count of signal drop for small button
	uint8 * pCount;							// point to counter
	uint8 bUpdateBaseline;					// flag to update baseline
		
		switch(chan){						// pick up data set
			case BIGBUTTON:
				pwTempBaseline = &MGVwBigButtonBaseline;	// working with big button data
				wPreCapIns = MGVwBigCapInstant;				// save previous sample
				MGVwBigCapInstant = CAPDATA;				// Get cap sensor data
				pwTempCapTns = &MGVwBigCapInstant;			// update current data to global
				pCount = &bCountBG;							// pick counter for big button
			break;
			
			case SMALLBUTTON:
				pwTempBaseline = &MGVwSmallButtonBaseline;	// pick small button stored data
				wPreCapIns = MGVwSmallCapInstant;
				MGVwSmallCapInstant = CAPDATA;				// Get cap sensor data
				pwTempCapTns = &MGVwSmallCapInstant ;		// update current data to global
				pCount = &bCountSM;							// pick small button counter
			break;
			
			default:
			break;
		}
		
		TButtonClr();										// clear TSI for the next scan
		bResult = NOTOUCH;									// Default
		bUpdateBaseline = FALSE;							// defalt to not update
		
		if(*pwTempCapTns < *pwTempBaseline){				// new data less than baseline	
			if((*pwTempBaseline - *pwTempCapTns) < NVsOpPara.TouchTH){
				*pCount = 0;
				bUpdateBaseline = TRUE;  					// Need to update baseline
			}
			else{
				(*pCount)++;								// extream data
				if(*pCount >= 8 ){							// get rid of noise
			//if(*pCount >= 80 ){							// get rid of noise
					*pwTempBaseline = *pwTempCapTns;  		// trace new data
					*pCount = 0;
				}
			}
			bResult = NOTOUCH;
		}
		else{// New data is bigger than baseline
			
			*pCount = 0;
			
			if((*pwTempCapTns - *pwTempBaseline) >= NVsOpPara.TouchTH){ // signal is big enough
				bResult = TOUCHED;					// the only place to set this flag
			}
			else{
				bUpdateBaseline = TRUE;  			// Need to update baseline
			}
		}
		
		if(bUpdateBaseline){ // update baseline
			//wTempIns = *pwTempCapTns;			// current data
			wTempBack = *pwTempBaseline;		// previous baseline
			wTempIns = wPreCapIns;				// previous data
			if(((wTempIns - NVsOpPara.TouchTH) > wTempBack ) || ((wTempBack - NVsOpPara.TouchTH) > wTempIns)){
				wTempIns = wTempBack;
			}
			*pwTempBaseline  = GetDataMovingEverage(wTempIns,wTempBack); // update baseline. calculate new baseline
		}
		
		return bResult;			
}

/*
uint8 IsButtonTouched(uint8 chan){

	uint8 bResult;
	uint16 wTempIns, wTempBack;
	uint16 * pwTempCapTns;					// point to store instant value
	uint16 * pwTempBaseline;				// point to store baseline
	static uint8 bCountBG = 0;              // count of signal drop for big button
	static uint8 bCountSM = 0;              // count of signal drop for small button
	uint8 * pCount;							// point to counter
	uint16 wRawSignal;	
		switch(chan){						// pick up data set
			case BIGBUTTON:
				pwTempBaseline = &MGVwBigButtonBaseline;	// working with big button data
				MGVwBigCapInstant = CAPDATA;				// Get cap sensor data
				pwTempCapTns = &MGVwBigCapInstant;			// update current data to global
				pCount = &bCountBG;							// pick counter for big button
			break;
			
			case SMALLBUTTON:
				pwTempBaseline = &MGVwSmallButtonBaseline;	// pick small button stored data
				MGVwSmallCapInstant = CAPDATA;				// Get cap sensor data
				pwTempCapTns = &MGVwSmallCapInstant ;		// update current data to global
				pCount = &bCountSM;							// pick small button counter
			break;
			
			default:
			break;
		}
		
		TButtonClr();								// clear TSI for the next scan
		bResult = NOTOUCH;							// Default
		wRawSignal = 0;
		MGVwFiltered = 0;
		
		if(*pwTempCapTns < *pwTempBaseline){		// new data less than baseline	
			(*pCount)++;							// add debunce
		//	if(*pCount >= 8 ){						// get rid of noise
			if(*pCount >= 80 ){						// get rid of noise
				*pwTempBaseline = *pwTempCapTns;  	// trace new data
				*pCount = 0;
			}
			//bResult = NOTOUCH;
		}
		else{// New data is bigger than baseline
			
			*pCount = 0;
			
			if((*pwTempCapTns - *pwTempBaseline) < NVsOpPara.TouchTH){ // signal is big enough
			
				wTempIns = *pwTempCapTns;			// previous data
				wTempBack = *pwTempBaseline;		// previous baseline
				*pwTempBaseline  = GetDataMovingEverage(wTempIns,wTempBack); // update baseline. calculate new baseline
				//bResult = NOTOUCH;
			}
			
			wRawSignal = (*pwTempCapTns - *pwTempBaseline);
			MGVwFiltered = GetFilteredData(wRawSignal);
			if(MGVwFiltered > NVsOpPara.TouchTH){
				bResult = NOTOUCH;
			}
			
			
		}
		
		return bResult;			
}

*/
/*
** ========================================================================================================================================================
**     Method      :  uint8 TouchVerify(uint8 bChan, uint16 wBaseline)
**     Description :
**         This method scan the button instantly to verify if it is really touched.
**     Parameters  : button channel, button baseline 
**     Returns     : button status
** ========================================================================================================================================================
*/

uint8 TouchVerify(uint8 bChan, uint16 wBaseline){

	uint8 bResult;
	uint16 	wTempCapTns;

	bResult = NOTOUCH;								// default to no touch
	TButtonStartScan(bChan);
	if(bChan == BIGBUTTON){                         // big button, let TSI in sleep mode
		SleepMS(10);
	}
	else{											// Small button
		while((TSI0_GENCS & TSI_GENCS_EOSF_MASK) == 0);	// waiting for scan complete
	}
	wTempCapTns = CAPDATA;							// Get cap sensor data
	TButtonClr();									// reset for next scan
	if(wTempCapTns > wBaseline){					// make sure data is valid
		if((wTempCapTns - wBaseline) > NVsOpPara.TouchTH){ 	// signal is big enough
			bResult = TOUCHED;
		}
	}
	return bResult;			
}

/*
** ========================================================================================================================================================
**     Method      :  uint8 TwoButtonProcess(uint8 bSMNewStatus,uint8 bBGNewStatus))
**     Description :
**         This method process button related task for two buttons unit.
**         There are total 16 situations.
**         
**     Parameters  :
**     				uint8 bSMNewStatus:  small button new status
**     				uint8 bBGNewStatus:  big button new status
**     Returns     : Buttons new status
** ========================================================================================================================================================
*/
uint8 TwoButtonProcess(uint8 bSMNewStatus,uint8 bBGNewStatus){
	
	uint8 bNewButtonsSts;					// temp button status
	static uint8 bTick = 0;					// tick for counting a second
	static uint16 wBigTouchTime = 0;		// count of bigbutton touch
	static uint16 wSmTouchTime = 0;			// count of smallbutton touch
	static uint16 wBothTouchTime = 0;		// count of both button touch
	
		//determine new combination status
		if(bBGNewStatus == TOUCHED){ 		// Big button touched
			if(bSMNewStatus == TOUCHED){	// Small button touched
				bNewButtonsSts = BTST;		// combination status
			}
			else{							// Small button not touched
				bNewButtonsSts = BTSU;		// combination status
			}	
		}
		else{								// Big not touch
			if(bSMNewStatus == TOUCHED){	// Small button touched
				bNewButtonsSts = BUST;		// combination status
			}
			else{							// Small button not touched							
				bNewButtonsSts = BUSU;		// combination status
			}	
		}

		// process requesting based on combination status of two buttons
		switch(GVsButtonSts.bCombination){  // Look at previous buttons status
		// Old status 1, Both buttons were not touched
			case BUSU:
				switch (bNewButtonsSts){	// Comparing current button status
					case BUSU:	// Bigbutton: stay no touch. Small button: stay no touch.
								// both buttons keeping in not touch. do nothing
					break;
					case BUST:  // Bigbutton: stay no touch. Small button: just touched.
						bSMNewStatus = TouchVerify(SMALLBUTTON, MGVwSmallButtonBaseline); 	// one more scan to verify
							if(bSMNewStatus == TOUCHED){									// Verified, need liter flush
							#ifndef CONCEAL													// No button activation for concealed units
								if(NVsOpPara.FlushVolumeM == 1){ 							// Grand mode
									GVbFlushRequest = GRDLITEBT;							// set corresponding flush flag
								}
								else{														// standard mode
									GVbFlushRequest = STDLITEBT;							// set corresponding flush flag
								}
							
								GVeUserSts = NOTPRESENT;									// Reset IR status, Start a new cycle for IR scan
								bTick = 0;
								wBigTouchTime += 32;										// possible big button touched a bit later
								wSmTouchTime += 32;											// count flush action total time flush on time plus delay after (3 + 5)= 8 seconds
								wBothTouchTime += 32;										// assume both button touched
							#endif
								LED_ClrVal();												// Turn off LED in case it was turn on.
							
							}
							else{	//Verified fail
								bNewButtonsSts = BUSU;										// update status
							}	
					break;
					
					case BTSU: // Bigbutton: just touched. Small button: stay no touch.
						bBGNewStatus = TouchVerify(BIGBUTTON, MGVwBigButtonBaseline); 		// one more scan to verify
						if(bBGNewStatus == TOUCHED){// Verified, need Full flush
						#ifndef CONCEAL										// No button activation for concealed units
							if(NVsOpPara.FlushVolumeM == 1){ 								// Grand mode
								GVbFlushRequest = GRDFULLBT;								// set corresponding flush flag
							}
							else{
								GVbFlushRequest = STDFULLBT;								// set corresponding flush flag
							}
							GVeUserSts = NOTPRESENT;										// Reset IR status, Start a new cycle for IR scan
							bTick = 0;
							wBigTouchTime += 32;											// count flush action total time flush on time plus delay after (3 + 5)= 8 seconds
							wSmTouchTime += 32;												// may be touched a bit later
							wBothTouchTime += 32;											// assume both button touched
						#endif
							LED_ClrVal();													// Turn off LED in case it was turn on.
							
						}
						else{	//Verified fail
							bNewButtonsSts = BUSU;
						}	
					break;
					
					case BTST: // Bigbutton: just touched. Small button: just touched.
					#ifndef CONCEAL														// No button activation for concealed units
						if(NVsOpPara.FlushVolumeM == 1){ 									// Grand mode
							GVbFlushRequest = GRDLITEBT;
						}
						else{
							GVbFlushRequest = STDLITEBT;
						}
						GVeUserSts = NOTPRESENT;											// Reset IR status, Start a new cycle for IR scan
						bTick = 0;
						wBigTouchTime += 32;
						wSmTouchTime += 32;													// count flush action total time flush on time plus delay after (3 + 5)= 8 seconds
						wBothTouchTime += 32;
					#endif
						LED_ClrVal();														// Turn off LED in case it was turn on.
					break;
					
					default:
					break;
				
				}
			break; // end of BUSU
		
			//Old status 2, Big button was not touched. small button was touched
			case BUST:
				switch (bNewButtonsSts){
					case BUSU: // Bigbutton: stay no touch. Small button: just relesed touch
						// just reset all timers
						bTick = 0;
						wBigTouchTime = 0;
						wSmTouchTime = 0;
						wBothTouchTime = 0;
						LED_ClrVal();	// Turn off LED in case it was turn on.
					break;
					
					case BUST:	// Bigbutton: stay no touch. Small button: keep touched
						wSmTouchTime += 1;									// increse touch timer
						if(wSmTouchTime >= FORRESET){						// time to update baseline
							MGVwSmallButtonBaseline = MGVwSmallCapInstant; 	// update baseline	
							wSmTouchTime = 0;
						}
					break;
					
					case BTSU:		// Bigbutton: Just touched. Small button: just relesed touch
						bBGNewStatus = TouchVerify(BIGBUTTON, MGVwBigButtonBaseline); 	// one more scan to verify
							if(bBGNewStatus == TOUCHED){	// Verified, need Full flush
							#ifndef CONCEAL										// No button activation for concealed units
								if(NVsOpPara.FlushVolumeM == 1){ // Grand mode
									GVbFlushRequest = GRDFULLBT;
								}
								else{
									GVbFlushRequest = STDFULLBT;
								}
								GVeUserSts = NOTPRESENT;		// Reset IR status, Start a new cycle for IR scan
								// update all timers
								bTick = 0;
								wBigTouchTime += 32;			// count flush action total time flush on time plus delay after (3 + 5)= 8 seconds
								wSmTouchTime = 0;				
								wBothTouchTime = 0;
							#endif
								LED_ClrVal();	// Turn off LED in case it was turn on.
							}
							else{	//Verified fail
								bNewButtonsSts = BUSU;
								// update all timers
								bTick = 0;
								wBigTouchTime = 0;
								wSmTouchTime = 0;
								wBothTouchTime = 0;
								LED_ClrVal();	// Turn off LED in case it was turn on.
							}	
					break;
					
					case BTST: // Bigbutton: Just touched. Small button: keep touched. Don't flush because flush was gived due to small button touch before
						// Reset all timers
						bTick = 0;
						wBigTouchTime = 32;
						wSmTouchTime = 32;
						wBothTouchTime = 32;	// count flush action total time flush on time plus delay after (3 + 5)= 8 seconds
						LED_ClrVal();			// Turn off LED in case it was turn on.
					break;
					
					default:
					break;
				
				}
			break; // end of BUST
		
			// Old status 3, big was touched, small not touch
			case BTSU:									
				switch (bNewButtonsSts){				// check new button status
					case BUSU:							// Bigbutton: Just released. Small button: keep no touch
						// request to do current calibration or start power up mode
						if((wBigTouchTime >= CALLOW) && (wBigTouchTime <= CALHIGH)){
							if(NVbPowerupSts == NOTDONE){	// during power up mode, request calibration
								LED_ClrVal();				// Turn off LED in case it was turn on.
								GVeOperationSts = SetOperationState(CALIBRATION); // go to calibration operation
							}
							else{ 									// Out of power up mode, request getting in power up mode
								LED_ClrVal();						// Turn off LED in case it was turn on.
								NVbPowerupSts = NOTDONE; 			// restart power up period to enable LED feedback
								GVbDutyRate = POWERUPDUTYRATE;		// power up IR scan duty rate
							}
						}
											
						// Status changed, reset all timers
						bTick = 0;
						wBigTouchTime = 0;
						wSmTouchTime = 0;
						wBothTouchTime = 0;
						LED_ClrVal();	// Turn off LED in case it was turn on.
					break;
					
					case BUST:							// Bigbutton: Just released. Small button: just touched
						// request to do current calibration or start power up mode
						if((wBigTouchTime >= CALLOW) && (wBigTouchTime <= CALHIGH)){
							if(NVbPowerupSts == NOTDONE){	// during power up mode, request calibration
								LED_ClrVal();				// Turn off LED in case it was turn on.
								GVeOperationSts = SetOperationState(CALIBRATION); // go to calibration operation
							}
							else{ 							// Out of power up mode, request getting in power up mode
								LED_ClrVal();						// Turn off LED in case it was turn on.
								NVbPowerupSts = NOTDONE; 			// restart power up period to enable LED feedback
								GVbDutyRate = POWERUPDUTYRATE;		// power up IR scan duty rate
							}
						}
						// Status changed, reset all timers
						bTick = 0;
						wBigTouchTime = 0;
						wSmTouchTime = 0;
						wBothTouchTime = 0;
						LED_ClrVal();	// Turn off LED in case it was turn on.
					break;
					
					case BTSU:							// Bigbutton: keep touched. Small button: keep no touch
						wBigTouchTime += 1;				// increase touch time
						if((wBigTouchTime >= CALLOW) && (wBigTouchTime < CALHIGH)){	// always LED blink during this period time even after power up period 
							bTick += 1;
							if(bTick >= 4){				// every second
								bTick = 0;
								BlinkLED(1);			// Let user know it is the time period to enter distance calibration
							}
						}
						
						if(wBigTouchTime >= FORSHIP){  		// time to check if shipping mode requested
							if(IsSystemInShipCondition()){	// meet ship conditions 
								//Flush100ms(10);				// short solenoid action to tell get in shipping
								#ifdef WITHANT
									GVeOperationSts = SetOperationState(UPDATEANT);	// change system status
									NVbDongleDeepSlpModeIn = 1;	// shipping mode put dongle into deep sleep mode, go in to deepsleep
								#else
									Flush100ms(10);	
									GVeOperationSts = SetOperationState(SHIPPING);	// change system status
								#endif
								wBigTouchTime = 0;
							}
						}
									
					#ifdef WITHBLE
						#ifndef WITHANT //MVP add
							if((GVbBLEEnabled == FALSE) && (wBigTouchTime == 40)){
								GVbWakeBLE = TRUE;  // wake up BLE
							}
						#endif //MVP add
					#endif				
										
						if(wBigTouchTime >= FORRESET){		// time to reset baseline
							MGVwSmallButtonBaseline = MGVwSmallCapInstant; 	// reset baseline
							MGVwBigButtonBaseline = MGVwBigCapInstant;  	// reset baseline
							wBigTouchTime = 0;
						}
				
					break;
					
					case BTST:							// Bigbutton: keep touched. Small button: Just touched
						// update all timers
						bTick = 0;
						wBigTouchTime = 32;		
						wSmTouchTime = 32;		// Assume small button were touched during flush
						wBothTouchTime = 32;	// count flush action total time flush on time plus delay after (3 + 5)= 8 seconds
						LED_ClrVal();			// Turn off LED in case it was turn on.
					break;
					
					default:
					break;
				
				}
					
			break; // end of BTSU,
			
			// old status 4, two buttons were touched
			case BTST:
				switch (bNewButtonsSts){
					case BUSU:			// Bigbutton: just released. Small button: Just released
						#ifndef WITHBLE			// disable this function for BLE unit
						if((wBothTouchTime >= MODECHANGELOW) && (wBothTouchTime <= MODECHANGEHIGH)){	// check if it is in the change window 
							if(NVsOpPara.FlushVolumeM == 1){ 	// Grand mode
								NVsOpPara.FlushVolumeM = 0; 	// swap mode.
							}
							else{
								NVsOpPara.FlushVolumeM = 1; 	// swap mode.
							}
							SetValveOnTime(NVsOpPara.OpenTM);	// set valve turn on time for all types of flush
							SaveParaToFlash(NVsOpPara);			// store new cycle operation parameters to flash	
						}
						LED_ClrVal();			// Turn off LED in case it was turn on.
						bTick = 0;
						wBigTouchTime = 0;		// reset
						wSmTouchTime = 0;		// reset
						wBothTouchTime = 0;		// reset
						#endif
					break;
					
					case BUST:					// Bigbutton: just released. Small button: keep touch, it is possible that small button release not detected yet. treat it as both buttons released
						#ifndef WITHBLE			// disable this function for BLE unit
						if((wBothTouchTime >= MODECHANGELOW) && (wBothTouchTime <= MODECHANGEHIGH)){	// in mode change period 
							if(NVsOpPara.FlushVolumeM == 1){ 	// Grand mode
								NVsOpPara.FlushVolumeM = 0; 	// swap mode.
							}
							else{
								NVsOpPara.FlushVolumeM = 1; 	// swap mode.
							}
							SetValveOnTime(NVsOpPara.OpenTM);	// set valve turn on time for all types of flush
							SaveParaToFlash(NVsOpPara);		// store new cycle operation parameters to flash	
						}
						LED_ClrVal();				// Turn on LED in case it was turn on.
						bTick = 0;
						wBigTouchTime = 0;		// reset
						wSmTouchTime = 0;		// reset
						wBothTouchTime = 0;		// reset
						#endif
					break;
					
					case BTSU:					// Bigbutton: keep touched. Small button: keep released, it is possible that big button release not detected yet. treat it as both buttons released
						#ifndef WITHBLE			// disable this function for BLE unit
						if((wBothTouchTime >= MODECHANGELOW) && (wBothTouchTime <= MODECHANGEHIGH)){	// in mode change period 
							if(NVsOpPara.FlushVolumeM == 1){ 	// Grand mode
								NVsOpPara.FlushVolumeM = 0; 	// swap mode.
							}
							else{
								NVsOpPara.FlushVolumeM = 1; 	// swap mode.
							}
							SetValveOnTime(NVsOpPara.OpenTM);	// set valve turn on time for all types of flush
							SaveParaToFlash(NVsOpPara);		// store new cycle operation parameters to flash	
						}
						LED_ClrVal();				// Turn on LED in case it was turn on.
						bTick = 0;
						wBigTouchTime = 0;			// reset
						wSmTouchTime = 0;			// reset
						wBothTouchTime = 0;			// reset
						#endif
					break;
					
					case BTST: // Bigbutton: keep touched. Small button: keep touched,
						wBigTouchTime += 1;					// increase touch time
						wSmTouchTime += 1;					// increase touch time
						wBothTouchTime += 1;				// increase touch time
						
						#ifndef WITHBLE						// disable this function for BLE unit
						if((wBothTouchTime >= MODELOW) && (wBothTouchTime < MODECHANGELOW)){	// choice one period 
							if(NVsOpPara.FlushVolumeM == 1){ 	// Grand mode
								LED_SetVal();					// Turn on LED to indicate it was set as Grand Mode.
							}
							else{
								LED_ClrVal();				// Turn off LED in case it was turn on.
								bTick += 1;
								if(bTick >= 4){				// every second
									bTick = 0;
									BlinkLED(1);			// To indicate it was not Grand Mode
								}
							}						
							
						}
						
						if(wBothTouchTime == MODECHANGELOW){	// The moment to turn of LED if it was turned on
							LED_ClrVal();				// Turn off LED in case it was turn on.
						}
						
						if((wBothTouchTime >= MODECHANGELOW) && (wBothTouchTime < MODECHANGEHIGH)){	// Mode change period 
							if(NVsOpPara.FlushVolumeM == 0){ 	// stand mode
								LED_SetVal();					// Turn on LED.
							}
							else{
								bTick += 1;
								if(bTick >= 4){				// every second
									bTick = 0;
									BlinkLED(1);			// Let user know it is the time period to enter distance calibration
								}
							}						
						}
						#endif
						
						if(wBothTouchTime > MODECHANGEHIGH){	// After mode change period
							LED_ClrVal();				// Turn off LED in case it was turn on.
						}
						
						if(wBothTouchTime >= FORRESET){		// time to reset baseline
							MGVwSmallButtonBaseline = MGVwSmallCapInstant; 	// reset baseline
							wSmTouchTime = 0;
							MGVwBigButtonBaseline = MGVwBigCapInstant; 		// reset baseline
							wBigTouchTime = 0;
						}
						
					break;
					default:
					break;
				
				}
			break; //end of  BTST,
		
			default:
			break;
	
			}

		return bNewButtonsSts;

}
	
	


