/*
 * main.c
 *
 *  	Created on: Feb 17, 2017
 *      Author: Scott Wang
 *      This contains main entry
 *      
 *      Modify: 8/5/20
 *      Author: Sheng Deng
 *      -Added ANT cases
 */

#include "Cpu.h"
#include "Events.h"
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
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "SystemInit.h"
#include "Operation.h"
#include "Timing.h"

/* IoT begin */
#include "UartWake.h"
#include "UartWakeInt.h"
#include "ExtIntLdd1.h"
#include "BatteryMidCheck.h"
#include "BitIoLdd10.h"
#include "UartWake.h"
#include "BitIoLdd12.h"
/* IoT end */


#include "NCP_Helpers.h"
/*
** ===========================================================================================================================
**     Method      : int main(void)
**     Description :
**         This is the application main function.
**     Parameters  : Nothing
**     Returns     : Nothing
** ===========================================================================================================================
*/


int main(void)
{

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/
  
 //WDReset();							// kick the dog  
    AS1_Enable();
  	AS1_TurnRxOn();
  	AS1_TurnTxOn();
  	
  NCP_init();
  
  for(;;) {								// main loop
	  
	 WDReset();							// kick the dog 
	 
	 rpc_run();
	 sendPeriodicMessage();
	   	
  }

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/





