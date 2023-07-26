/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : IRDRV.c
**     Project     : Solis_Willow
**     Processor   : MKL16Z128VFM4
**     Component   : DAC_LDD
**     Version     : Component 01.084, Driver 01.09, CPU db: 3.00.000
**     Compiler    : GNU C Compiler
**     Date/Time   : 2023-07-19, 22:57, # CodeGen: 0
**     Abstract    :
**         This component implements an internal D/A converter of the MCU.
**         It contains settings for converting various format of a values
**         to supported format of the D/A converter.
**
**     Settings    :
**          Component name                                 : IRDRV
**          D/A converter                                  : DAC0
**          Interrupt service/event                        : Disabled
**          Output pin                                     : yes
**            D/A channel (pin)                            : DAC0_OUT/ADC0_SE23/CMP0_IN4/PTE30/TPM0_CH3/TPM_CLKIN1
**            D/A channel (pin) signal                     : 
**          Init value                                     : 0
**          D/A resolution                                 : 12 bits
**          Data mode                                      : unsigned 16 bits, right justified
**          Low power mode                                 : Disabled
**          Voltage reference source                       : internal
**          Hardware buffer                                : Disabled
**          DMA                                            : Disabled
**          Initialization                                 : 
**            Enabled in init. code                        : yes
**            Auto initialization                          : yes
**            Event mask                                   : 
**              OnBufferEnd                                : Disabled
**              OnBufferStart                              : Disabled
**              OnComplete                                 : Disabled
**              OnError                                    : Disabled
**          CPU clock/configuration selection              : 
**            Clock configuration 0                        : This component enabled
**            Clock configuration 1                        : This component disabled
**            Clock configuration 2                        : This component disabled
**            Clock configuration 3                        : This component disabled
**            Clock configuration 4                        : This component disabled
**            Clock configuration 5                        : This component disabled
**            Clock configuration 6                        : This component disabled
**            Clock configuration 7                        : This component disabled
**     Contents    :
**         Init     - LDD_TDeviceData* IRDRV_Init(LDD_TUserData *UserDataPtr);
**         Enable   - LDD_TError IRDRV_Enable(LDD_TDeviceData *DeviceDataPtr);
**         Disable  - LDD_TError IRDRV_Disable(LDD_TDeviceData *DeviceDataPtr);
**         SetValue - LDD_TError IRDRV_SetValue(LDD_TDeviceData *DeviceDataPtr, LDD_DAC_TData Data);
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
** @file IRDRV.c
** @version 01.09
** @brief
**         This component implements an internal D/A converter of the MCU.
**         It contains settings for converting various format of a values
**         to supported format of the D/A converter.
**
*/         
/*!
**  @addtogroup IRDRV_module IRDRV module documentation
**  @{
*/         

/* MODULE IRDRV. */

/* {Default RTOS Adapter} No RTOS includes */
#include "IRDRV.h"

#ifdef __cplusplus
extern "C" {
#endif 


typedef struct {
  bool EnUser;                         /* Enable/Disable device by user */
  LDD_TDeviceData *DmaTransferDeviceDataPtr; /* DMATransfer device data structure */
  LDD_TUserData *UserDataPtr;          /* RTOS device data structure */
} IRDRV_TDeviceData;                   /* Device data structure type */

/* {Default RTOS Adapter} Static object used for simulation of dynamic driver memory allocation */
static IRDRV_TDeviceData DeviceDataPtr__DEFAULT_RTOS_ALLOC;
/*
** ===================================================================
**     Method      :  IRDRV_Init (component DAC_LDD)
*/
/*!
**     @brief
**         Initializes the device according to design-time
**         configuration properties. Allocates memory for the device
**         data structure. 
**         If the [Enable in init. code] is set to "yes" then the
**         device is also enabled (see the description of the Enable
**         method).
**         This method can be called only once. Before the second call
**         of Init the Deinit method must be called first. If DMA
**         service is enabled this method also initializes inherited
**         DMA Transfer component.
**     @param
**         UserDataPtr     - Pointer to the user or
**                           RTOS specific data. This pointer will be
**                           passed as an events or callback parameter.
**     @return
**                         - Device data structure pointer.
*/
/* ===================================================================*/
LDD_TDeviceData* IRDRV_Init(LDD_TUserData *UserDataPtr)
{
  IRDRV_TDeviceData *DeviceDataPtr;    /* LDD device structure */

  /* Allocate HAL device structure */
  /* {Default RTOS Adapter} Driver memory allocation: Dynamic allocation is simulated by a pointer to the static object */
  DeviceDataPtr = &DeviceDataPtr__DEFAULT_RTOS_ALLOC;
  DeviceDataPtr->DmaTransferDeviceDataPtr = NULL; /* DMA is not used */
  DeviceDataPtr->UserDataPtr = UserDataPtr; /* Store the RTOS device structure */
  /* SIM_SCGC6: DAC0=1 */
  SIM_SCGC6 |= SIM_SCGC6_DAC0_MASK;
  DAC_PDD_EnableDevice(DAC0_BASE_PTR,PDD_DISABLE); /* Disable device */
  /* Initialization of pin routing */
  /* PORTE_PCR30: ISF=0,MUX=0 */
  PORTE_PCR30 &= (uint32_t)~(uint32_t)((PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x07)));
  /* DAC0_DAT0H: ??=0,??=0,??=0,??=0,DATA1=0 */
  DAC0_DAT0H = DAC_DATH_DATA1(0x00);
  /* DAC0_DAT0L: DATA0=0 */
  DAC0_DAT0L = DAC_DATL_DATA0(0x00);
  /* DAC0_C2: ??=0,??=0,??=0,DACBFRP=0,??=1,??=1,??=1,DACBFUP=1 */
  DAC0_C2 = (DAC_C2_DACBFUP_MASK | 0x0EU);
  /* DAC0_C1: DMAEN=0,??=0,??=0,??=0,??=0,DACBFMD=0,??=0,DACBFEN=0 */
  DAC0_C1 = 0x00U;
  /* DAC0_SR: ??=0,??=0,??=0,??=0,??=0,??=0,DACBFRPTF=0,DACBFRPBF=0 */
  DAC0_SR = 0x00U;
  /* DAC0_C0: DACEN=1,DACRFS=0,DACTRGSEL=0,DACSWTRG=0,LPEN=0,??=0,DACBTIEN=0,DACBBIEN=0 */
  DAC0_C0 = DAC_C0_DACEN_MASK;
  DeviceDataPtr->EnUser = TRUE;        /* Set the flag "device enabled by user" */
  /* Registration of the device structure */
  PE_LDD_RegisterDeviceStructure(PE_LDD_COMPONENT_IRDRV_ID,DeviceDataPtr);
  return ((LDD_TDeviceData*)DeviceDataPtr); /* Return pointer to the data data structure */
}

/*
** ===================================================================
**     Method      :  IRDRV_Enable (component DAC_LDD)
*/
/*!
**     @brief
**         Enables DAC device. If possible, this method switches on
**         digital-to-analog converter device, voltage reference, etc.
**         This method is intended to be used together with Disable
**         method to temporary switch On/Off the device after the
**         device is initialized.
**         This method is required if the [Enable in init. code]
**         property is set to "no" value.
**     @param
**         DeviceDataPtr   - Device data structure
**                           pointer returned by [Init] method.
**     @return
**                         - Error code, possible codes:
**                           - ERR_OK - OK.
**                           - ERR_SPEED - This device does not work in
**                           the active clock configuration.
*/
/* ===================================================================*/
LDD_TError IRDRV_Enable(LDD_TDeviceData *DeviceDataPtr)
{
  DAC_PDD_EnableDevice(DAC0_BASE_PTR,PDD_ENABLE); /* Enable device */
  ((IRDRV_TDeviceData*)DeviceDataPtr)->EnUser = TRUE; /* Set the flag "device enabled by user" */
  return ERR_OK;
}

/*
** ===================================================================
**     Method      :  IRDRV_Disable (component DAC_LDD)
*/
/*!
**     @brief
**         Disables the DAC device. If possible, this method switches
**         off digital-to-analog converter device, voltage reference,
**         etc. When the device is disabled, some component methods
**         should not be called. If so, error ERR_DISABLED is reported.
**         This method is intended to be used together with Enable
**         method to temporary switch On/Off the device after the
**         device is initialized.
**         This method is not required. The Deinit method can be used
**         to switch off and uninstall the device.
**     @param
**         DeviceDataPtr   - Device data structure
**                           pointer returned by [Init] method.
**     @return
**                         - Error code, possible codes:
**                           - ERR_OK - OK.
**                           - ERR_SPEED - This device does not work in
**                           the active clock configuration.
*/
/* ===================================================================*/
LDD_TError IRDRV_Disable(LDD_TDeviceData *DeviceDataPtr)
{
  DAC_PDD_EnableDevice(DAC0_BASE_PTR,PDD_DISABLE); /* Disable device */
  ((IRDRV_TDeviceData*)DeviceDataPtr)->EnUser = FALSE; /* Set the flag "device disabled by user" */
  return ERR_OK;
}

/*
** ===================================================================
**     Method      :  IRDRV_SetValue (component DAC_LDD)
*/
/*!
**     @brief
**         Sets DAC output voltage according to specified value.
**         Input data format is specified by [Data mode] property
**         settings. If selected formatting is not native for DAC
**         device which is used then any necessary transformations (e.g.
**         shifting) are done._/Note: This method is available only if
**         Data buffer is disabled./_
**     @param
**         DeviceDataPtr   - Pointer to device data
**                           structure.
**     @param
**         Data            - User data.
**     @return
**                         - Error code, possible codes:
**                           - ERR_OK - OK.
**                           - ERR_SPEED - This device does not work in
**                           the active clock configuration.
**                           - ERR_DISABLED - Component or device is
**                           disabled.
*/
/* ===================================================================*/
LDD_TError IRDRV_SetValue(LDD_TDeviceData *DeviceDataPtr, LDD_DAC_TData Data)
{
  /* Device state test - this test can be disabled by setting the "Ignore enable test"
     property to the "yes" value in the "Configuration inspector" */
  if (!((IRDRV_TDeviceData*)DeviceDataPtr)->EnUser) { /* Is device enabled? */
    return ERR_DISABLED;               /* No, return ERR_DISABLED */
  }
  DAC_PDD_SetData(DAC0_BASE_PTR,(uint16_t)Data,0U);
  return ERR_OK;
}



/* END IRDRV. */

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