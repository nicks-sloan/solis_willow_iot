/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : AD1.h
**     Project     : Solis_Willow
**     Processor   : MKL16Z128VFM4
**     Component   : ADC
**     Version     : Component 01.699, Driver 01.00, CPU db: 3.00.000
**     Compiler    : GNU C Compiler
**     Date/Time   : 2023-07-19, 22:57, # CodeGen: 0
**     Abstract    :
**         This device "ADC" implements an A/D converter,
**         its control methods and interrupt/event handling procedure.
**     Settings    :
**          Component name                                 : AD1
**          A/D converter                                  : ADC0
**          Sharing                                        : Disabled
**          ADC_LDD                                        : ADC_LDD
**          Interrupt service/event                        : Disabled
**          A/D channels                                   : 5
**            Channel0                                     : 
**              A/D channel (pin)                          : ADC0_SE7b/PTD6/LLWU_P15/SPI1_MOSI/UART0_RX/SPI1_MISO
**              A/D channel (pin) signal                   : 
**              Mode select                                : Single Ended
**            Channel1                                     : 
**              A/D channel (pin)                          : ADC0_SE6b/PTD5/SPI1_SCK/UART2_TX/TPM0_CH5
**              A/D channel (pin) signal                   : 
**              Mode select                                : Single Ended
**            Channel2                                     : 
**              A/D channel (pin)                          : ADC0_DP2/ADC0_SE2/PTE18/SPI0_MOSI/I2C0_SDA/SPI0_MISO
**              A/D channel (pin) signal                   : 
**              Mode select                                : Single Ended
**            Channel3                                     : 
**              A/D channel (pin)                          : ADC0_SE9/TSI0_CH6/PTB1/I2C0_SDA/TPM1_CH1
**              A/D channel (pin) signal                   : 
**              Mode select                                : Single Ended
**            Channel4                                     : 
**              A/D channel (pin)                          : ADC0_DP1/ADC0_SE1/PTE16/SPI0_PCS0/UART2_TX/TPM_CLKIN0
**              A/D channel (pin) signal                   : 
**              Mode select                                : Single Ended
**          A/D resolution                                 : 12 bits
**          Conversion time                                : 5 �s
**          Low-power mode                                 : Disabled
**          High-speed conversion mode                     : Disabled
**          Asynchro clock output                          : Disabled
**          Sample time                                    : 6
**          Internal trigger                               : Disabled
**          Number of conversions                          : 1
**          Initialization                                 : 
**            Enabled in init. code                        : no
**            Events enabled in init.                      : yes
**          CPU clock/speed selection                      : 
**            High speed mode                              : This component enabled
**            Low speed mode                               : This component disabled
**            Slow speed mode                              : This component disabled
**          High input limit                               : 1
**          Low input limit                                : 0
**          Get value directly                             : yes
**          Wait for result                                : yes
**     Contents    :
**         Enable         - byte AD1_Enable(void);
**         Disable        - byte AD1_Disable(void);
**         MeasureChan    - byte AD1_MeasureChan(bool WaitForResult, byte Channel);
**         GetChanValue   - byte AD1_GetChanValue(byte Channel, void* Value);
**         GetChanValue16 - byte AD1_GetChanValue16(byte Channel, word *Value);
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
** @file AD1.h
** @version 01.00
** @brief
**         This device "ADC" implements an A/D converter,
**         its control methods and interrupt/event handling procedure.
*/         
/*!
**  @addtogroup AD1_module AD1 module documentation
**  @{
*/         

#ifndef __AD1_H
#define __AD1_H

/* MODULE AD1. */

/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
/* Include inherited beans */
#include "AdcLdd1.h"

#include "Cpu.h"

#ifdef __cplusplus
extern "C" {
#endif 

/* This constant contains the number of channels in the "A/D channel list"
   group */
#define AD1_CHANNEL_COUNT               AdcLdd1_CHANNEL_COUNT



#define AD1_SAMPLE_GROUP_SIZE 5U
static void AD1_MainMeasure(void);
/*
** ===================================================================
**     Method      :  MainMeasure (component ADC)
**
**     Description :
**         The method performs the conversion of the input channels in 
**         the polling mode.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void AD1_HWEnDi(void);
/*
** ===================================================================
**     Method      :  AD1_HWEnDi (component ADC)
**
**     Description :
**         Enables or disables the peripheral(s) associated with the 
**         component. The method is called automatically as a part of the 
**         Enable and Disable methods and several internal methods.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

byte AD1_Enable(void);
/*
** ===================================================================
**     Method      :  AD1_Enable (component ADC)
*/
/*!
**     @brief
**         Enables A/D converter component. [Events] may be generated
**         ([DisableEvent]/[EnableEvent]). If possible, this method
**         switches on A/D converter device, voltage reference, etc.
**     @return
**                         - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
*/
/* ===================================================================*/

byte AD1_Disable(void);
/*
** ===================================================================
**     Method      :  AD1_Disable (component ADC)
*/
/*!
**     @brief
**         Disables A/D converter component. No [events] will be
**         generated. If possible, this method switches off A/D
**         converter device, voltage reference, etc.
**     @return
**                         - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
*/
/* ===================================================================*/

byte AD1_MeasureChan(bool WaitForResult,byte Channel);
/*
** ===================================================================
**     Method      :  AD1_MeasureChan (component ADC)
*/
/*!
**     @brief
**         This method performs measurement on one channel. (Note: If
**         the [number of conversions] is more than one the conversion
**         of the A/D channel is performed specified number of times.)
**     @param
**         WaitForResult   - Wait for a result of
**                           conversion. If the [interrupt service] is
**                           disabled and at the same time a [number of
**                           conversions] is greater than 1, the
**                           WaitForResult parameter is ignored and the
**                           method waits for each result every time.
**     @param
**         Channel         - Channel number. If only one
**                           channel in the component is set this
**                           parameter is ignored, because the parameter
**                           is set inside this method.
**     @return
**                         - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_DISABLED - Device is disabled
**                           ERR_BUSY - A conversion is already running
**                           ERR_RANGE - Parameter "Channel" out of range
*/
/* ===================================================================*/

byte AD1_GetChanValue(byte Channel, void* Value);
/*
** ===================================================================
**     Method      :  AD1_GetChanValue (component ADC)
*/
/*!
**     @brief
**         Returns the last measured value of the required channel.
**         Format and width of the value is a native format of the A/D
**         converter.
**     @param
**         Channel         - Channel number. If only one
**                           channel in the component is set then this
**                           parameter is ignored.
**     @param
**         Value           - Pointer to the measured value. Data
**                           type is a byte, a word or an int. It
**                           depends on the supported modes, resolution,
**                           etc. of the AD converter. See the Version
**                           specific information for the current CPU in
**                           [General Info].
**     @return
**                         - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_NOTAVAIL - Requested value not
**                           available
**                           ERR_RANGE - Parameter "Channel" out of
**                           range
**                           ERR_OVERRUN - External trigger overrun flag
**                           was detected after the last value(s) was
**                           obtained (for example by GetValue). This
**                           error may not be supported on some CPUs
**                           (see generated code).
*/
/* ===================================================================*/

byte AD1_GetChanValue16(byte Channel, word *Value);
/*
** ===================================================================
**     Method      :  AD1_GetChanValue16 (component ADC)
*/
/*!
**     @brief
**         This method returns the last measured value of the required
**         channel. Compared with [GetChanValue] method this method
**         returns more accurate result if the [number of conversions]
**         is greater than 1 and [AD resolution] is less than 16 bits.
**         In addition, the user code dependency on [AD resolution] is
**         eliminated.
**     @param
**         Channel         - Channel number. If only one
**                           channel in the component is set then this
**                           parameter is ignored.
**     @param
**         Value           - Pointer to the measured value.
**     @return
**                         - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_NOTAVAIL - Requested value not
**                           available
**                           ERR_RANGE - Parameter "Channel" out of
**                           range
**                           ERR_OVERRUN - External trigger overrun flag
**                           was detected after the last value(s) was
**                           obtained (for example by GetValue). This
**                           error may not be supported on some CPUs
**                           (see generated code).
*/
/* ===================================================================*/

void AD1_Init(void);
/*
** ===================================================================
**     Method      :  AD1_Init (component ADC)
**
**     Description :
**         Initializes the associated peripheral(s) and the component 
**         internal variables. The method is called automatically as a 
**         part of the application initialization code.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/

/* END AD1. */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

#endif
/* ifndef __AD1_H */
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
