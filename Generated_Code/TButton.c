/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : TButton.c
**     Project     : Solis_Willow
**     Processor   : MKL16Z128VFM4
**     Component   : Init_TSI
**     Version     : Component 01.000, Driver 01.00, CPU db: 3.00.000
**     Compiler    : GNU C Compiler
**     Date/Time   : 2023-07-19, 22:57, # CodeGen: 0
**     Abstract    :
**          This file implements the TSI (TSI0) module initialization
**          according to the Peripheral Initialization settings, and
**          defines interrupt service routines prototypes.
**     Settings    :
**          Component name                                 : TButton
**          Device                                         : TSI0
**          Settings                                       : 
**            Clock gate                                   : Enabled
**            Num of Scan times per electrode              : 1
**            Electrode oscillator prescaler               : divide by 1
**            Trigger Mode                                 : Software trigger
**            Ref. OSC charge current                      : 4 uA
**            External OSC charge current                  : 0.5 uA
**            Current source pair swap                     : no
**            Delta voltage                                : 1.03 V
**            Analog mode                                  : Automatic noise detection
**            Low power settings                           : 
**              TSI enabled in Low Power Modes             : yes
**              Touch sensing low threshold                : 0
**              Touch sensing high threshold               : 0
**          Pins                                           : 
**            Input 0                                      : Enabled
**              Pin                                        : ADC0_SE8/TSI0_CH0/PTB0/LLWU_P5/I2C0_SCL/TPM1_CH0
**              Pin signal                                 : Button
**            Input 1                                      : Disabled
**            Input 2                                      : Disabled
**            Input 3                                      : Disabled
**            Input 4                                      : Disabled
**            Input 5                                      : Disabled
**            Input 6                                      : Disabled
**          Interrupts/DMA                                 : 
**            Interrupt                                    : INT_TSI0
**            Interrupt request                            : Disabled
**            Interrupt priority                           : 0 (Highest)
**            ISR Name                                     : 
**            Touch sensing input interrupt                : Disabled
**            Touch sensing interrupt type                 : Out of range
**            DMA request                                  : Disabled
**          Initialization                                 : 
**            Initial channel select                       : Channel 0
**            Touch Sensing Input                          : Enabled
**            Call Init method                             : yes
**     Contents    :
**         Init - void TButton_Init(void);
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
** @file TButton.c
** @version 01.00
** @brief
**          This file implements the TSI (TSI0) module initialization
**          according to the Peripheral Initialization settings, and
**          defines interrupt service routines prototypes.
*/         
/*!
**  @addtogroup TButton_module TButton module documentation
**  @{
*/         

/* MODULE TButton. */

#include "TButton.h"
  /* Including shared modules, which are used in the whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "Cpu.h"


/*
** ===================================================================
**     Method      :  TButton_Init (component Init_TSI)
**     Description :
**         This method initializes registers of the TSI module
**         according to the Peripheral Initialization settings.
**         Call this method in user code to initialize the module. By
**         default, the method is called by PE automatically; see "Call
**         Init method" property of the component for more details.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void TButton_Init(void)
{
  /* SIM_SCGC5: TSI=1 */
  SIM_SCGC5 |= SIM_SCGC5_TSI_MASK;
  /* TSI0_GENCS: OUTRGF=1,ESOR=0,MODE=0x0C,REFCHRG=3,DVOLT=0,EXTCHRG=0,PS=0,NSCN=0,TSIIEN=0,STPE=1,STM=0,EOSF=1,CURSW=0 */
  TSI0_GENCS = (uint32_t)((TSI0_GENCS & (uint32_t)~(uint32_t)(
                TSI_GENCS_ESOR_MASK |
                TSI_GENCS_MODE(0x03) |
                TSI_GENCS_REFCHRG(0x04) |
                TSI_GENCS_DVOLT(0x03) |
                TSI_GENCS_EXTCHRG(0x07) |
                TSI_GENCS_PS(0x07) |
                TSI_GENCS_NSCN(0x1F) |
                TSI_GENCS_TSIIEN_MASK |
                TSI_GENCS_STM_MASK |
                TSI_GENCS_CURSW_MASK
               )) | (uint32_t)(
                TSI_GENCS_OUTRGF_MASK |
                TSI_GENCS_MODE(0x0C) |
                TSI_GENCS_REFCHRG(0x03) |
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


/* END TButton. */
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
