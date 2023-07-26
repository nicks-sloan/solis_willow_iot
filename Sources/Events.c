/* ###################################################################
**     Filename    : Events.c
**     Project     : GV_SingleBTSolis
**     Processor   : MKL16Z128VFM4
**     Component   : Events
**     Version     : Driver 01.00
**     Compiler    : GNU C Compiler
**     Date/Time   : 2017-03-30, 14:02, # CodeGen: 5
**     Abstract    :
**         This is user's event module.
**         Put your event handler code here.
**     Settings    :
**     Contents    :
**         LatchCurrent_OnInterrupt - void LatchCurrent_OnInterrupt(LDD_TUserData *UserDataPtr);
**         TI1_OnInterrupt          - void TI1_OnInterrupt(void);
**         AS1_OnTxComplete         - void AS1_OnTxComplete(void);
**         Cpu_OnReset              - void Cpu_OnReset(uint16_t Reason);
**         Cpu_OnLLSWakeUpINT       - void Cpu_OnLLSWakeUpINT(void);
**
** ###################################################################*/
/*!
** @file Events.c
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup Events_module Events module documentation
**  @{
*/         
/* MODULE Events */

#include "Timing.h"

#include "Events.h"
#include "Cpu.h"
#ifdef __cplusplus
extern "C" {
#endif 


/* User includes (#include below this line is not maintained by Processor Expert) */

//Global Varables
volatile uint16 StartReason;	// start reason set in event of Cpu_OnReset. Used by system initialisation.
volatile bool SendComplete; 	// Flag of TX completion, set by TX completion interrupt. Used in UART methods
volatile uint16 wTimerTick;		// Timer tick, set by timer expired event
volatile uint16 wTimerTick2;	// Timer tick, set by timer expired event
volatile uint32 millisTick;		// Timer tick, set by timer expired event
volatile bool SerialIn; 		// Flag of RX, set by RX interrupt. Used in UART methods
volatile bool Latchhappened; 	// Flag of solenoid latched
PE_ISR(LPTmr)
{
// NOTE: The routine should include actions to clear the appropriate interrupt flags.
	LPTMR0_CSR |=  LPTMR_CSR_TCF_MASK;   // write 1 to TCF to clear the LPT timer compare flag
}

/*
** ===================================================================
**     Event       :  Cpu_OnReset (module Events)
**
**     Component   :  Cpu [MKL16Z256LH4]
*/
/*!
**     @brief
**         This software event is called after a reset.
**     @param
**         Reason          - Content of the reset status register.
**                           You can use predefined constants RSTSRC_*
**                           defined in generated PE_Const.h file to
**                           determine a reason of the last reset. See
**                           definition of these constants in this file
**                           for details.
*/
/* ===================================================================*/
void Cpu_OnReset(uint16_t Reason)
{
  /* Write your code here ... */
		
	StartReason = Reason; //just record the reason. Take action later after everything initialized.
	
	
}

/*
** ===================================================================
**     Event       :  Cpu_OnLLSWakeUpINT (module Events)
**
**     Component   :  Cpu [MKL16Z256LH4]
*/
/*!
**     @brief
**         This event is called when Low Leakage WakeUp interrupt
**         occurs. LLWU flags indicating source of the wakeup can be
**         obtained by calling the [GetLLSWakeUpFlags] method. Flags
**         indicating the external pin wakeup source are automatically
**         cleared after this event is executed. It is responsibility
**         of user to clear flags corresponding to internal modules.
**         This event is automatically enabled when [LLWU interrupt
**         request] is enabled.
*/
/* ===================================================================*/
void Cpu_OnLLSWakeUpINT(void)
{
  /* Write your code here ... */
	
/* Sheng MVP Mod begin */
    if (Cpu_GetLLSWakeUpFlags() == LLWU_EXT_PIN10) 
	{
		GVbWakeBLE = TRUE;
		
	} 
	
	if (Cpu_GetLLSWakeUpFlags() == LLWU_INT_MODULE0)	 
	{
		/* Original begin */
		SIM_SCGC5 |= SIM_SCGC5_LPTMR_MASK;
		LPTMR0_CSR |=  LPTMR_CSR_TCF_MASK;   // write 1 to TCF to clear the LPT timer compare flag
		LPTMR0_CSR = ( LPTMR_CSR_TEN_MASK | LPTMR_CSR_TIE_MASK | LPTMR_CSR_TCF_MASK  );
		/* Original end */
	}
/* Sheng added in  end */ 
	
}

/*
** ===================================================================
**     Event       :  AS1_OnTxComplete (module Events)
**
**     Component   :  AS1 [AsynchroSerial]
**     Description :
**         This event indicates that the transmitter is finished
**         transmitting all data, preamble, and break characters and is
**         idle. It can be used to determine when it is safe to switch
**         a line driver (e.g. in RS-485 applications).
**         The event is available only when both <Interrupt
**         service/event> and <Transmitter> properties are enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void AS1_OnTxComplete(void)
{
  /* Write your code here ... */
	SendComplete = TRUE;	//set this global variable
}



void __thumb_startup( void );	//Make compiler happy

void UnUseINT(void){
	__DI();
	__thumb_startup();
}


/*
** ===================================================================
**     Event       :  TI1_OnInterrupt (module Events)
**
**     Component   :  TI1 [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the component is enabled - <Enable> and the events are
**         enabled - <EnableEvent>). This event is enabled only if a
**         <interrupt service/event> is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void TI1_OnInterrupt(void)
{
  /* Write your code here ... */
	wTimerTick++; 		//tick it
	wTimerTick2++; 		//tick it
	millisTick++;
}

/*
** ===================================================================
**     Event       :  LatchCurrent_OnInterrupt (module Events)
**
**     Component   :  LatchCurrent [ExtInt_LDD]
*/
/*!
**     @brief
**         This event is called when an active signal edge/level has
**         occurred.
**     @param
**         UserDataPtr     - Pointer to RTOS device
**                           data structure pointer.
*/
/* ===================================================================*/
void LatchCurrent_OnInterrupt(LDD_TUserData *UserDataPtr)
{
  /* Write your code here ... */
	Latchhappened = TRUE;
}

/*
** ===================================================================
**     Event       :  AS1_OnRxChar (module Events)
**
**     Component   :  AS1 [AsynchroSerial]
**     Description :
**         This event is called after a correct character is received.
**         The event is available only when the <Interrupt
**         service/event> property is enabled and either the <Receiver>
**         property is enabled or the <SCI output mode> property (if
**         supported) is set to Single-wire mode.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void AS1_OnRxChar(void)
{
  /* Write your code here ... */
	SerialIn = TRUE;
}

/* Sheng MVP Mod begin */

/*
** ===================================================================
**     Event       :  UartWakeInt_OnInterrupt (module Events)
**
**     Component   :  UartWakeInt [ExtInt]
**     Description :
**         This event is called when an active signal edge/level has
**         occurred.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void UartWakeInt_OnInterrupt(void)
{
  /* Write your code here ... */
	GVbWakeBLE = TRUE;
	
}

/* Sheng MVP Mod end */

/* END Events */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.3 [05.09]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
