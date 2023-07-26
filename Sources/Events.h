/* ###################################################################
**     Filename    : Events.h
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
** @file Events.h
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup Events_module Events module documentation
**  @{
*/         

#ifndef __Events_H
#define __Events_H
/* MODULE Events */

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "WDog1.h"
#include "LPTMR0.h"
#include "LED.h"
#include "BitIoLdd1.h"
#include "IFsh1.h"
#include "IntFlashLdd1.h"
#include "LATCH.h"
#include "BitIoLdd2.h"
#include "IRDRV.h"
#include "AS1.h"
#include "ASerialLdd1.h"
#include "UNLATCH.h"
#include "BitIoLdd3.h"
#include "BatteryCheck.h"
#include "BitIoLdd5.h"
#include "IRRecPwr.h"
#include "BitIoLdd6.h"
#include "TU1.h"
#include "TI1.h"
#include "TimerIntLdd1.h"
#include "AD1.h"
#include "AdcLdd1.h"
#include "COMCable.h"
#include "BitIoLdd7.h"
#include "TButton.h"
#include "MCUC1.h"
#include "SolarCheck.h"
#include "BitIoLdd4.h"
#include "Sentinel.h"
#include "BitIoLdd8.h"
#include "LatchCurrent.h"
#include "LatchCheck.h"
#include "BitIoLdd9.h"
#include "BleXres.h"
#include "BitIoLdd11.h"
#include "UTIL1.h"

/* Sheng MVP Mod begin */
#include "UartWake.h"
#include "UartWakeInt.h"
#include "ExtIntLdd1.h"
#include "BatteryMidCheck.h"
#include "BitIoLdd10.h"
#include "UartWake.h"
#include "BitIoLdd12.h"
#include "SystemInit.h"
/* Sheng MVP Mod end */

#ifdef __cplusplus
extern "C" {
#endif 

extern volatile uint16 StartReason;	//start reason set in event of Cpu_OnReset
extern volatile bool SendComplete; 	//Flag of TX completion, set by TX completion interrupt. Used in UART methods
extern volatile uint16 wTimerTick;
extern volatile uint16 wTimerTick2;
extern volatile bool SerialIn;
extern volatile bool Latchhappened; 	// Flag of solenoid latched

extern volatile uint32 millisTick;  //millis tick

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
void LatchCurrent_OnInterrupt(LDD_TUserData *UserDataPtr);

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
void TI1_OnInterrupt(void);

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
void AS1_OnTxComplete(void);

/*
** ===================================================================
**     Event       :  Cpu_OnReset (module Events)
**
**     Component   :  Cpu [MKL16Z128FM4]
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
void Cpu_OnReset(uint16_t Reason);

/*
** ===================================================================
**     Event       :  Cpu_OnLLSWakeUpINT (module Events)
**
**     Component   :  Cpu [MKL16Z128FM4]
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
void Cpu_OnLLSWakeUpINT(void);


void UnUseINT(void);

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
void AS1_OnRxChar(void);

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
void UartWakeInt_OnInterrupt(void);

/* Sheng MVP Mod end */

/* END Events */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

#endif 
/* ifndef __Events_H*/
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
