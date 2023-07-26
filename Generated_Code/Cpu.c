/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : Cpu.c
**     Project     : Solis_Willow
**     Processor   : MKL16Z128VFM4
**     Component   : MKL16Z128FM4
**     Version     : Component 01.007, Driver 01.04, CPU db: 3.00.000
**     Datasheet   : KL16P121M48SF4RM, Rev.2, Dec 2012
**     Compiler    : GNU C Compiler
**     Date/Time   : 2023-07-19, 22:57, # CodeGen: 0
**     Abstract    :
**
**     Settings    :
**
**     Contents    :
**         SetOperationMode  - LDD_TError Cpu_SetOperationMode(LDD_TDriverOperationMode OperationMode,...
**         EnableInt         - void Cpu_EnableInt(void);
**         DisableInt        - void Cpu_DisableInt(void);
**         GetLLSWakeUpFlags - uint32_t Cpu_GetLLSWakeUpFlags(void);
**
**     Copyright : 1997 - 2014 Freescale Semiconductor, Inc. 
**     All Rights Reserved.
**     
**     Redistribution and use in source and binary forms, with or without modification,
**     are permitted provided that the following conditions are met:
**     
**     o Redistributions of source code must retain the above copyright notice, this list
**       of conditions and the following disclaimer.
**     
**     o Redistributions in binary form must reproduce the above copyright notice, this
**       list of conditions and the following disclaimer in the documentation and/or
**       other materials provided with the distribution.
**     
**     o Neither the name of Freescale Semiconductor, Inc. nor the names of its
**       contributors may be used to endorse or promote products derived from this
**       software without specific prior written permission.
**     
**     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
**     ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**     WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
**     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
**     ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
**     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
**     ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
**     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
**     SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**     
**     http: www.freescale.com
**     mail: support@freescale.com
** ###################################################################*/
/*!
** @file Cpu.c
** @version 01.04
** @brief
**
*/         
/*!
**  @addtogroup Cpu_module Cpu module documentation
**  @{
*/         

/* MODULE Cpu. */

/* {Default RTOS Adapter} No RTOS includes */
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
#include "UartWake.h"
#include "BitIoLdd12.h"
#include "UartWakeInt.h"
#include "ExtIntLdd1.h"
#include "BatteryMidCheck.h"
#include "BitIoLdd10.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "Events.h"
#include "Cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Global variables */
volatile uint8_t SR_reg;               /* Current value of the FAULTMASK register */
volatile uint8_t SR_lock = 0x00U;      /* Lock */


/*
** ===================================================================
**     Method      :  Cpu_GetLLSWakeUpFlags (component MKL16Z128FM4)
*/
/*!
**     @brief
**         This method returns the current status of the LLWU wake-up
**         flags indicating which wake-up source caused the MCU to exit
**         LLS or VLLSx low power mode.
**         The following predefined constants can be used to determine
**         the wake-up source:
**         LLWU_EXT_PIN0, ... LLWU_EXT_PIN15 - external pin 0 .. 15
**         caused the wake-up
**         LLWU_INT_MODULE0 .. LLWU_INT_MODULE7 - internal module 0..15
**         caused the wake-up.
**     @return
**                         - Returns the current status of the LLWU
**                           wake-up flags indicating which wake-up
**                           source caused the MCU to exit LLS or VLLSx
**                           low power mode.
*/
/* ===================================================================*/
uint32_t Cpu_GetLLSWakeUpFlags(void)
{
  uint32_t Flags;

  Flags = LLWU_F1;
  Flags |= (uint32_t)((uint32_t)LLWU_F2 << 8U);
  Flags |= (uint32_t)((uint32_t)LLWU_F3 << 16U);
  if ((LLWU_FILT1 & 0x80U) != 0x00U ) {
    Flags |= LLWU_FILTER1;
  }
  if ((LLWU_FILT2 & 0x80U) != 0x00U ) {
    Flags |= LLWU_FILTER2;
  }
  return Flags;
}

/*
** ===================================================================
**     Method      :  Cpu_Cpu_ivINT_PORTC_PORTD (component MKL16Z128FM4)
**
**     Description :
**         This ISR services the ivINT_PORTC_PORTD interrupt shared by 
**         several components.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
PE_ISR(Cpu_ivINT_PORTC_PORTD)
{
  LatchCurrent_Interrupt();            /* Call the service routine */
  ExtIntLdd1_Interrupt();              /* Call the service routine */
}

/*
** ===================================================================
**     Method      :  Cpu_INT_LLWInterrupt (component MKL16Z128FM4)
**
**     Description :
**         This ISR services the 'LLWU' interrupt.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
PE_ISR(Cpu_INT_LLWInterrupt)
{
  Cpu_OnLLSWakeUpINT();
  /* LLWU_F1: WUF7=1,WUF6=1,WUF5=1,WUF4=1,WUF3=1,WUF2=1,WUF1=1,WUF0=1 */
  LLWU_F1 = LLWU_F1_WUF7_MASK |
            LLWU_F1_WUF6_MASK |
            LLWU_F1_WUF5_MASK |
            LLWU_F1_WUF4_MASK |
            LLWU_F1_WUF3_MASK |
            LLWU_F1_WUF2_MASK |
            LLWU_F1_WUF1_MASK |
            LLWU_F1_WUF0_MASK;         /* Clear external pin flags */
  /* LLWU_F2: WUF15=1,WUF14=1,WUF13=1,WUF12=1,WUF11=1,WUF10=1,WUF9=1,WUF8=1 */
  LLWU_F2 = LLWU_F2_WUF15_MASK |
            LLWU_F2_WUF14_MASK |
            LLWU_F2_WUF13_MASK |
            LLWU_F2_WUF12_MASK |
            LLWU_F2_WUF11_MASK |
            LLWU_F2_WUF10_MASK |
            LLWU_F2_WUF9_MASK |
            LLWU_F2_WUF8_MASK;         /* Clear external pin flags */
  /* LLWU_F3: MWUF7=1,MWUF6=1,MWUF5=1,MWUF4=1,MWUF3=1,MWUF2=1,MWUF1=1,MWUF0=1 */
  LLWU_F3 = LLWU_F3_MWUF7_MASK |
            LLWU_F3_MWUF6_MASK |
            LLWU_F3_MWUF5_MASK |
            LLWU_F3_MWUF4_MASK |
            LLWU_F3_MWUF3_MASK |
            LLWU_F3_MWUF2_MASK |
            LLWU_F3_MWUF1_MASK |
            LLWU_F3_MWUF0_MASK;        /* Clear Error detect flag */
  /* LLWU_FILT1: FILTF=1 */
  LLWU_FILT1 |= LLWU_FILT1_FILTF_MASK; /* Clear filter flag */
  /* LLWU_FILT2: FILTF=1 */
  LLWU_FILT2 |= LLWU_FILT2_FILTF_MASK; /* Clear filter flag */

}

/*
** ===================================================================
**     Method      :  Cpu_Cpu_Interrupt (component MKL16Z128FM4)
**
**     Description :
**         This ISR services an unused interrupt/exception vector.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
PE_ISR(Cpu_Interrupt)
{
  /* This code can be changed using the CPU component property "Build Options / Unhandled int code" */
  UnUseINT();
}

/*
** ===================================================================
**     Method      :  Cpu_SetOperationMode (component MKL16Z128FM4)
*/
/*!
**     @brief
**         This method requests to change the component's operation
**         mode (RUN, WAIT, SLEEP, STOP). The target operation mode
**         will be entered immediately. 
**         See [Operation mode settings] for further details of the
**         operation modes mapping to low power modes of the cpu.
**     @param
**         OperationMode   - Requested driver
**                           operation mode
**     @param
**         ModeChangeCallback - Callback to
**                           notify the upper layer once a mode has been
**                           changed. Parameter is ignored, only for
**                           compatibility of API with other components.
**     @param
**         ModeChangeCallbackParamPtr 
**                           - Pointer to callback parameter to notify
**                           the upper layer once a mode has been
**                           changed. Parameter is ignored, only for
**                           compatibility of API with other components.
**     @return
**                         - Error code
**                           ERR_OK - OK
**                           ERR_PARAM_MODE - Invalid operation mode
*/
/* ===================================================================*/
LDD_TError Cpu_SetOperationMode(LDD_TDriverOperationMode OperationMode, LDD_TCallback ModeChangeCallback, LDD_TCallbackParam *ModeChangeCallbackParamPtr)
{
  (void) ModeChangeCallback;           /* Parameter is not used, suppress unused argument warning */
  (void) ModeChangeCallbackParamPtr;   /* Parameter is not used, suppress unused argument warning */
  switch (OperationMode) {
    case DOM_RUN:
      /* SCB_SCR: SLEEPDEEP=0,SLEEPONEXIT=0 */
      SCB_SCR &= (uint32_t)~(uint32_t)(
                  SCB_SCR_SLEEPDEEP_MASK |
                  SCB_SCR_SLEEPONEXIT_MASK
                 );
      break;
    case DOM_WAIT:
      /* SCB_SCR: SLEEPDEEP=0 */
      SCB_SCR &= (uint32_t)~(uint32_t)(SCB_SCR_SLEEPDEEP_MASK);
      /* SCB_SCR: SLEEPONEXIT=0 */
      SCB_SCR &= (uint32_t)~(uint32_t)(SCB_SCR_SLEEPONEXIT_MASK);
      PE_WFI();
      break;
    case DOM_SLEEP:
      /* SCB_SCR: SLEEPDEEP=1 */
      SCB_SCR |= SCB_SCR_SLEEPDEEP_MASK;
      /* SMC_STOPCTRL: PSTOPO=0,PORPO=0,??=0,??=0,VLLSM=0 */
      SMC_STOPCTRL = (SMC_STOPCTRL_PSTOPO(0x00) | SMC_STOPCTRL_VLLSM(0x00));
      /* SMC_PMCTRL: STOPM=0 */
      SMC_PMCTRL &= (uint8_t)~(uint8_t)(SMC_PMCTRL_STOPM(0x07));
      (void)(SMC_PMCTRL == 0U);        /* Dummy read of SMC_PMCTRL to ensure the register is written before enterring low power mode */
      /* SCB_SCR: SLEEPONEXIT=0 */
      SCB_SCR &= (uint32_t)~(uint32_t)(SCB_SCR_SLEEPONEXIT_MASK);
      PE_WFI();
      break;
    case DOM_STOP:
    /* Clear LLWU flags */
      /* LLWU_F1: WUF7=1,WUF6=1,WUF5=1,WUF4=1,WUF3=1,WUF2=1,WUF1=1,WUF0=1 */
      LLWU_F1 = LLWU_F1_WUF7_MASK |
                LLWU_F1_WUF6_MASK |
                LLWU_F1_WUF5_MASK |
                LLWU_F1_WUF4_MASK |
                LLWU_F1_WUF3_MASK |
                LLWU_F1_WUF2_MASK |
                LLWU_F1_WUF1_MASK |
                LLWU_F1_WUF0_MASK;
      /* LLWU_F2: WUF15=1,WUF14=1,WUF13=1,WUF12=1,WUF11=1,WUF10=1,WUF9=1,WUF8=1 */
      LLWU_F2 = LLWU_F2_WUF15_MASK |
                LLWU_F2_WUF14_MASK |
                LLWU_F2_WUF13_MASK |
                LLWU_F2_WUF12_MASK |
                LLWU_F2_WUF11_MASK |
                LLWU_F2_WUF10_MASK |
                LLWU_F2_WUF9_MASK |
                LLWU_F2_WUF8_MASK;
      /* LLWU_F3: MWUF7=1,MWUF6=1,MWUF5=1,MWUF4=1,MWUF3=1,MWUF2=1,MWUF1=1,MWUF0=1 */
      LLWU_F3 = LLWU_F3_MWUF7_MASK |
                LLWU_F3_MWUF6_MASK |
                LLWU_F3_MWUF5_MASK |
                LLWU_F3_MWUF4_MASK |
                LLWU_F3_MWUF3_MASK |
                LLWU_F3_MWUF2_MASK |
                LLWU_F3_MWUF1_MASK |
                LLWU_F3_MWUF0_MASK;
      /* LLWU_FILT1: FILTF=1 */
      LLWU_FILT1 |= LLWU_FILT1_FILTF_MASK;
      /* LLWU_FILT2: FILTF=1 */
      LLWU_FILT2 |= LLWU_FILT2_FILTF_MASK;
      /* SCB_SCR: SLEEPONEXIT=0 */
      SCB_SCR &= (uint32_t)~(uint32_t)(SCB_SCR_SLEEPONEXIT_MASK);
      /* SCB_SCR: SLEEPDEEP=1 */
      SCB_SCR |= SCB_SCR_SLEEPDEEP_MASK;
      /* SMC_PMCTRL: STOPM=3 */
      SMC_PMCTRL = (uint8_t)((SMC_PMCTRL & (uint8_t)~(uint8_t)(
                    SMC_PMCTRL_STOPM(0x04)
                   )) | (uint8_t)(
                    SMC_PMCTRL_STOPM(0x03)
                   ));
      (void)(SMC_PMCTRL == 0U);        /* Dummy read of SMC_PMCTRL to ensure the register is written before enterring low power mode */
      PE_WFI();
      break;
    default:
      return ERR_PARAM_MODE;
  }
  return ERR_OK;
}

/*
** ===================================================================
**     Method      :  Cpu_EnableInt (component MKL16Z128FM4)
*/
/*!
**     @brief
**         Enables all maskable interrupts.
*/
/* ===================================================================*/
void Cpu_EnableInt(void)
{
 __EI();
}

/*
** ===================================================================
**     Method      :  Cpu_DisableInt (component MKL16Z128FM4)
*/
/*!
**     @brief
**         Disables all maskable interrupts.
*/
/* ===================================================================*/
void Cpu_DisableInt(void)
{
 __DI();
}


/*** !!! Here you can place your own code using property "User data declarations" on the build options tab. !!! ***/

/*lint -esym(765,__init_hardware) Disable MISRA rule (8.10) checking for symbols (__init_hardware). The function is linked to the EWL library */
/*lint -esym(765,Cpu_Interrupt) Disable MISRA rule (8.10) checking for symbols (Cpu_Interrupt). */
void __init_hardware(void)
{

  /*** !!! Here you can place your own code before PE initialization using property "User code before PE initialization" on the build options tab. !!! ***/

  /*** ### MKL16Z128VFM4 "Cpu" init code ... ***/
  /*** PE initialization code after reset ***/
  SCB_VTOR = (uint32_t)(&__vect_table); /* Set the interrupt vector table position */
  /* System clock initialization */
  /* SIM_CLKDIV1: OUTDIV1=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,OUTDIV4=3,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0 */
  SIM_CLKDIV1 = (SIM_CLKDIV1_OUTDIV1(0x00) | SIM_CLKDIV1_OUTDIV4(0x03)); /* Set the system prescalers to safe value */
  /* SIM_SCGC5: PORTE=1,PORTD=1,PORTC=1,PORTB=1,PORTA=1 */
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK |
               SIM_SCGC5_PORTD_MASK |
               SIM_SCGC5_PORTC_MASK |
               SIM_SCGC5_PORTB_MASK |
               SIM_SCGC5_PORTA_MASK;   /* Enable clock gate for ports to enable pin routing */
  /* SIM_SCGC5: LPTMR=1 */
  SIM_SCGC5 |= SIM_SCGC5_LPTMR_MASK;
  if ((PMC_REGSC & PMC_REGSC_ACKISO_MASK) != 0x0U) {
    /* PMC_REGSC: ACKISO=1 */
    PMC_REGSC |= PMC_REGSC_ACKISO_MASK; /* Release IO pads after wakeup from VLLS mode. */
  }
  /* SIM_CLKDIV1: OUTDIV1=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,OUTDIV4=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0 */
  SIM_CLKDIV1 = (SIM_CLKDIV1_OUTDIV1(0x00) | SIM_CLKDIV1_OUTDIV4(0x00)); /* Update system prescalers */
  /* SIM_SOPT2: PLLFLLSEL=0 */
  SIM_SOPT2 &= (uint32_t)~(uint32_t)(SIM_SOPT2_PLLFLLSEL_MASK); /* Select FLL as a clock source for various peripherals */
  /* SIM_SOPT1: OSC32KSEL=3 */
  SIM_SOPT1 |= SIM_SOPT1_OSC32KSEL(0x03); /* LPO 1kHz oscillator drives 32 kHz clock for various peripherals */
  /* SIM_SOPT2: TPMSRC=1 */
  SIM_SOPT2 = (uint32_t)((SIM_SOPT2 & (uint32_t)~(uint32_t)(
               SIM_SOPT2_TPMSRC(0x02)
              )) | (uint32_t)(
               SIM_SOPT2_TPMSRC(0x01)
              ));                      /* Set the TPM clock */
  /* MCG_SC: FCRDIV=0 */
  MCG_SC &= (uint8_t)~(uint8_t)(MCG_SC_FCRDIV(0x07));
  /* Switch to FEI Mode */
  /* MCG_C1: CLKS=0,FRDIV=0,IREFS=1,IRCLKEN=1,IREFSTEN=1 */
  MCG_C1 = MCG_C1_CLKS(0x00) |
           MCG_C1_FRDIV(0x00) |
           MCG_C1_IREFS_MASK |
           MCG_C1_IRCLKEN_MASK |
           MCG_C1_IREFSTEN_MASK;
  /* MCG_C2: LOCRE0=0,RANGE0=0,HGO0=0,EREFS0=0,LP=0,IRCS=1 */
  MCG_C2 = (uint8_t)((MCG_C2 & (uint8_t)~(uint8_t)(
            MCG_C2_LOCRE0_MASK |
            MCG_C2_RANGE0(0x03) |
            MCG_C2_HGO0_MASK |
            MCG_C2_EREFS0_MASK |
            MCG_C2_LP_MASK
           )) | (uint8_t)(
            MCG_C2_IRCS_MASK
           ));
  /* MCG_C4: DMX32=0,DRST_DRS=0 */
  MCG_C4 &= (uint8_t)~(uint8_t)((MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x03)));
  /* OSC0_CR: ERCLKEN=0,??=0,EREFSTEN=0,??=0,SC2P=0,SC4P=0,SC8P=0,SC16P=0 */
  OSC0_CR = 0x00U;
  /* MCG_C5: ??=0,PLLCLKEN0=0,PLLSTEN0=0,PRDIV0=0 */
  MCG_C5 = MCG_C5_PRDIV0(0x00);
  /* MCG_C6: LOLIE0=0,PLLS=0,CME0=0,VDIV0=0 */
  MCG_C6 = MCG_C6_VDIV0(0x00);
  while((MCG_S & MCG_S_IREFST_MASK) == 0x00U) { /* Check that the source of the FLL reference clock is the internal reference clock. */
  }
  while((MCG_S & 0x0CU) != 0x00U) {    /* Wait until output of the FLL is selected */
  }
  /*** End of PE initialization code after reset ***/

  /*** !!! Here you can place your own code after PE initialization using property "User code after PE initialization" on the build options tab. !!! ***/

}



/*
** ===================================================================
**     Method      :  PE_low_level_init (component MKL16Z128FM4)
**
**     Description :
**         Initializes beans and provides common register initialization. 
**         The method is called automatically as a part of the 
**         application initialization code.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void PE_low_level_init(void)
{
  uint16_t ResetSource;                /* Source of reset */

  #ifdef PEX_RTOS_INIT
    PEX_RTOS_INIT();                   /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
      /* Initialization of the SIM module */
  /* SIM_SCGC7: DMA=0 */
  SIM_SCGC7 &= (uint32_t)~(uint32_t)(SIM_SCGC7_DMA_MASK);
  /* SIM_SCGC4: SPI1=0,SPI0=0,CMP=0,UART2=0,UART1=0,UART0=1,I2C1=0,I2C0=0 */
  SIM_SCGC4 = (uint32_t)((SIM_SCGC4 & (uint32_t)~(uint32_t)(
               SIM_SCGC4_SPI1_MASK |
               SIM_SCGC4_SPI0_MASK |
               SIM_SCGC4_CMP_MASK |
               SIM_SCGC4_UART2_MASK |
               SIM_SCGC4_UART1_MASK |
               SIM_SCGC4_I2C1_MASK |
               SIM_SCGC4_I2C0_MASK
              )) | (uint32_t)(
               SIM_SCGC4_UART0_MASK
              ));
  /* SIM_SCGC5: PORTE=1,PORTD=1,PORTC=1,PORTB=1,PORTA=1,TSI=0,LPTMR=1 */
  SIM_SCGC5 = (uint32_t)((SIM_SCGC5 & (uint32_t)~(uint32_t)(
               SIM_SCGC5_TSI_MASK
              )) | (uint32_t)(
               SIM_SCGC5_PORTE_MASK |
               SIM_SCGC5_PORTD_MASK |
               SIM_SCGC5_PORTC_MASK |
               SIM_SCGC5_PORTB_MASK |
               SIM_SCGC5_PORTA_MASK |
               SIM_SCGC5_LPTMR_MASK
              ));
  /* SIM_SCGC6: DAC0=0,RTC=0,ADC0=0,TPM2=0,TPM1=0,TPM0=0,PIT=0,I2S=0,DMAMUX=0,FTF=1 */
  SIM_SCGC6 = (uint32_t)((SIM_SCGC6 & (uint32_t)~(uint32_t)(
               SIM_SCGC6_DAC0_MASK |
               SIM_SCGC6_RTC_MASK |
               SIM_SCGC6_ADC0_MASK |
               SIM_SCGC6_TPM2_MASK |
               SIM_SCGC6_TPM1_MASK |
               SIM_SCGC6_TPM0_MASK |
               SIM_SCGC6_PIT_MASK |
               SIM_SCGC6_I2S_MASK |
               SIM_SCGC6_DMAMUX_MASK
              )) | (uint32_t)(
               SIM_SCGC6_FTF_MASK
              ));
        /* Initialization of the RCM module */
  /* RCM_RPFW: RSTFLTSEL=0 */
  RCM_RPFW &= (uint8_t)~(uint8_t)(RCM_RPFW_RSTFLTSEL(0x1F));
  /* RCM_RPFC: RSTFLTSS=0,RSTFLTSRW=0 */
  RCM_RPFC &= (uint8_t)~(uint8_t)(
               RCM_RPFC_RSTFLTSS_MASK |
               RCM_RPFC_RSTFLTSRW(0x03)
              );
      /* Initialization of the PMC module */
  /* PMC_LVDSC1: LVDACK=1,LVDIE=0,LVDRE=1,LVDV=1 */
  PMC_LVDSC1 = (uint8_t)((PMC_LVDSC1 & (uint8_t)~(uint8_t)(
                PMC_LVDSC1_LVDIE_MASK |
                PMC_LVDSC1_LVDV(0x02)
               )) | (uint8_t)(
                PMC_LVDSC1_LVDACK_MASK |
                PMC_LVDSC1_LVDRE_MASK |
                PMC_LVDSC1_LVDV(0x01)
               ));
  /* PMC_LVDSC2: LVWACK=1,LVWIE=0,LVWV=3 */
  PMC_LVDSC2 = (uint8_t)((PMC_LVDSC2 & (uint8_t)~(uint8_t)(
                PMC_LVDSC2_LVWIE_MASK
               )) | (uint8_t)(
                PMC_LVDSC2_LVWACK_MASK |
                PMC_LVDSC2_LVWV(0x03)
               ));
  /* PMC_REGSC: BGEN=0,ACKISO=0,BGBE=0 */
  PMC_REGSC &= (uint8_t)~(uint8_t)(
                PMC_REGSC_BGEN_MASK |
                PMC_REGSC_ACKISO_MASK |
                PMC_REGSC_BGBE_MASK
               );
        /* Initialization of the LLWU module */
  /* LLWU_PE2: WUPE7=0,WUPE6=0,WUPE5=0 */
  LLWU_PE2 &= (uint8_t)~(uint8_t)(
               LLWU_PE2_WUPE7(0x03) |
               LLWU_PE2_WUPE6(0x03) |
               LLWU_PE2_WUPE5(0x03)
              );
  /* LLWU_PE3: WUPE10=2,WUPE9=0,WUPE8=0 */
  LLWU_PE3 = (uint8_t)((LLWU_PE3 & (uint8_t)~(uint8_t)(
              LLWU_PE3_WUPE10(0x01) |
              LLWU_PE3_WUPE9(0x03) |
              LLWU_PE3_WUPE8(0x03)
             )) | (uint8_t)(
              LLWU_PE3_WUPE10(0x02)
             ));
  /* LLWU_PE4: WUPE15=0,WUPE14=0 */
  LLWU_PE4 &= (uint8_t)~(uint8_t)(
               LLWU_PE4_WUPE15(0x03) |
               LLWU_PE4_WUPE14(0x03)
              );
  /* LLWU_ME: WUME7=0,WUME5=0,WUME4=0,WUME1=0,WUME0=1 */
  LLWU_ME = (uint8_t)((LLWU_ME & (uint8_t)~(uint8_t)(
             LLWU_ME_WUME7_MASK |
             LLWU_ME_WUME5_MASK |
             LLWU_ME_WUME4_MASK |
             LLWU_ME_WUME1_MASK
            )) | (uint8_t)(
             LLWU_ME_WUME0_MASK
            ));
  /* LLWU_FILT1: FILTF=1,FILTE=0,??=0,FILTSEL=0 */
  LLWU_FILT1 = LLWU_FILT1_FILTF_MASK |
               LLWU_FILT1_FILTE(0x00) |
               LLWU_FILT1_FILTSEL(0x00);
  /* LLWU_FILT2: FILTF=1,FILTE=0,??=0,FILTSEL=0 */
  LLWU_FILT2 = LLWU_FILT2_FILTF_MASK |
               LLWU_FILT2_FILTE(0x00) |
               LLWU_FILT2_FILTSEL(0x00);
  /* SMC_PMPROT: ??=0,??=0,AVLP=0,??=0,ALLS=1,??=0,AVLLS=0,??=0 */
  SMC_PMPROT = SMC_PMPROT_ALLS_MASK;   /* Setup Power mode protection register */
  /* Common initialization of the CPU registers */
  /* NVIC_ISER: SETENA|=0x10000080 */
  NVIC_ISER |= NVIC_ISER_SETENA(0x10000080);
  /* NVIC_IPR7: PRI_28=0 */
  NVIC_IPR7 &= (uint32_t)~(uint32_t)(NVIC_IP_PRI_28(0xFF));
  /* PORTB_PCR0: ISF=0,MUX=0 */
  PORTB_PCR0 &= (uint32_t)~(uint32_t)((PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x07)));
  /* PORTC_PCR1: ISF=0,MUX=0 */
  PORTC_PCR1 &= (uint32_t)~(uint32_t)((PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x07)));
  /* NVIC_IPR6: PRI_26=0 */
  NVIC_IPR6 &= (uint32_t)~(uint32_t)(NVIC_IP_PRI_26(0xFF));
  /* GPIOC_PDDR: PDD&=~0x44 */
  GPIOC_PDDR &= (uint32_t)~(uint32_t)(GPIO_PDDR_PDD(0x44));
  /* PORTA_PCR20: ISF=0,MUX=7 */
  PORTA_PCR20 = (uint32_t)((PORTA_PCR20 & (uint32_t)~(uint32_t)(
                 PORT_PCR_ISF_MASK
                )) | (uint32_t)(
                 PORT_PCR_MUX(0x07)
                ));
  /* NVIC_IPR1: PRI_7=0,PRI_6=0 */
  NVIC_IPR1 &= (uint32_t)~(uint32_t)(
                NVIC_IP_PRI_7(0xFF) |
                NVIC_IP_PRI_6(0xFF)
               );
  ResetSource = (uint16_t)(((uint16_t)RCM_SRS1) << 8U);
  ResetSource |= RCM_SRS0;
  Cpu_OnReset(ResetSource);            /* Invoke an user event */
  /* ### WatchDog_LDD "WDog1" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)WDog1_Init(NULL);
  /* ### Init_LPTMR "LPTMR0" init code ... */
  LPTMR0_Init();
  /* ### BitIO_LDD "BitIoLdd1" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)BitIoLdd1_Init(NULL);
  /* ### IntFLASH "IFsh1" init code ... */
  IFsh1_Init();
  /* ### BitIO_LDD "BitIoLdd2" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)BitIoLdd2_Init(NULL);
  /* ### DAC_LDD "IRDRV" component auto initialization. Auto initialization feature can be disabled by component's property "Auto initialization". */
  (void)IRDRV_Init(NULL);
  /* ### Asynchro serial "AS1" init code ... */
  AS1_Init();
  /* ### BitIO_LDD "BitIoLdd3" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)BitIoLdd3_Init(NULL);
  /* ### BitIO_LDD "BitIoLdd5" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)BitIoLdd5_Init(NULL);
  /* ### BitIO_LDD "BitIoLdd6" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)BitIoLdd6_Init(NULL);
  /* ### TimerInt_LDD "TimerIntLdd1" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)TimerIntLdd1_Init(NULL);
  /* ### TimerInt "TI1" init code ... */
  /* ### ADC "AD1" init code ... */
  AD1_Init();
  /* ### BitIO_LDD "BitIoLdd7" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)BitIoLdd7_Init(NULL);
  /* ### Init_TSI "TButton" init code ... */
  TButton_Init();
  /* ### McuLibConfig "MCUC1" init code ... */
  /* ### BitIO_LDD "BitIoLdd4" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)BitIoLdd4_Init(NULL);
  /* ### BitIO_LDD "BitIoLdd8" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)BitIoLdd8_Init(NULL);
  /* ### ExtInt_LDD "LatchCurrent" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)LatchCurrent_Init(NULL);
  /* ### BitIO_LDD "BitIoLdd9" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)BitIoLdd9_Init(NULL);
  /* ### BitIO_LDD "BitIoLdd11" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)BitIoLdd11_Init(NULL);
  /* ### BitIO_LDD "BitIoLdd12" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)BitIoLdd12_Init(NULL);
  /* ### ExtInt_LDD "ExtIntLdd1" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)ExtIntLdd1_Init(NULL);
  /* ### BitIO_LDD "BitIoLdd10" component auto initialization. Auto initialization feature can be disabled by component property "Auto initialization". */
  (void)BitIoLdd10_Init(NULL);
  __EI();
}

/* END Cpu. */

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