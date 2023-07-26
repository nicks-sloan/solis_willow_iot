/*********************************************************************************************************************\
* BootProjectKL16
*
* File:   hardware.h
* Author: Derrick
* Change: Helen Chen
*
* Services performed by FREESCALE in this matter are performed AS IS and without any warranty.
* CUSTOMER retains the final decision relative to the total design and functionality of the end product.
* FREESCALE neither guarantees nor will be held liable by CUSTOMER for the success of this project.
* 
* FREESCALE disclaims all warranties, express, implied or statutory including, but not limited to, implied warranty
* of merchantability or fitness for a particular purpose on any hardware, software ore advise supplied to the project
* by FREESCALE, and or any product resulting from FREESCALE services.
* In no event shall FREESCALE be liable for incidental or consequential damages arising out of this agreement.
* CUSTOMER agrees to hold FREESCALE harmless against any and all claims demands or actions by anyone on account of any
* damage,or injury, whether commercial, contractual, or tortuous, rising directly or indirectly as a result of the
* advise or assistance supplied CUSTOMER in connection with product, services or goods supplied under this Agreement.
*
\*********************************************************************************************************************/

#ifndef HARDWARE_H_
#define HARDWARE_H_

#include "IO_Map.h"                           // include target MCU peripheral declarations
#include "Cpu.h"
/*********************************************************************************************************************\
* Public type definitions
\*********************************************************************************************************************/

/*********************************************************************************************************************
**  Additional standard ANSI C types
*/
#ifndef int8_t
typedef signed char int8_t;
#endif
#ifndef int16_t
typedef signed short int16_t;
#endif
#ifndef int32_t
typedef signed long int int32_t;
#endif

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif
#ifndef uint16_t
typedef unsigned short uint16_t;
#endif
#ifndef uint32_t
typedef unsigned long int uint32_t;
#endif
#ifndef char_t
typedef char char_t;
#endif


#ifndef FALSE
  #define  FALSE  0x00
#endif
#ifndef TRUE
  #define  TRUE   0x01u
#endif
#ifndef NULL
  #define  NULL   0x00u
#endif


/*********************************************************************************************************************
**  Bitfield types
*/
typedef union
{
  uint8_t Byte;
  struct
  {
    uint8_t _0    : 1;
    uint8_t _1    : 1;
    uint8_t _2    : 1;
    uint8_t _3    : 1;
    uint8_t _4    : 1;
    uint8_t _5    : 1;
    uint8_t _6    : 1;
    uint8_t _7    : 1;
  }Bit;
} typ_Bitfield_8;


/**********************************************************************************************************************
**  Interrupt vector table type definition
*/

typedef void (*const typ_ISRfunc)(void);

/*typedef struct {
  void * __ptr;
  typ_ISRfunc __fun[0x2F];
} tVectorTable;
*/

extern const tVectorTable __boot_vect_table;
extern const tVectorTable __vect_table;

#define RESERVED_HANDLER                    (0xFFFFFFFF)


/*********************************************************************************************************************\
* Public macros
\*********************************************************************************************************************/

#define MCU_DERIVATIVE_MKL16Z128	//2/18/16 change

#define MCU_INTERNAL_REFERENCE_FREQUENCY    (32768)
#define MCU_BOOTLOADER_UART_BAUD_RATE       (57600)		//4/24/14 change
//#define MCU_BOOTLOADER_UART_BAUD_RATE       (115200)		//3/25/16 change

#define MCU_RAM_FIRST_ADDRESS               (0x1FFFF000)	//2/18/16 change
#define MCU_RAM_LAST_ADDRESS                (0x20002FFF)	//2/18/16 change

#define MCU_APPLICATION_VECTOR_TABLE_START  (0x00002000)	//2/19/16 change
#define MCU_APPLICATION_MAIN_VECTOR         (0x00002004)	//2/19/16 change
#define MCU_APPLICATION_LAST_FLASH_ADDRESS  (0x0001FFFF)	//2/18/16 change
#define MCU_APPLICATION_CODE_LAST_ADDRESS	(0x0001FBFF)	//3/25/16 add


/*********************************************************************************************************************
**  Binary constant macros
**
**  credit to Tom Torfs.
*/
#define HEX__(n)    0x##n##LU
#define B8__(x)     ((x&0x0000000FLU)?1:0) \
                   +((x&0x000000F0LU)?2:0) \
                   +((x&0x00000F00LU)?4:0) \
                   +((x&0x0000F000LU)?8:0) \
                   +((x&0x000F0000LU)?16:0) \
                   +((x&0x00F00000LU)?32:0) \
                   +((x&0x0F000000LU)?64:0) \
                   +((x&0xF0000000LU)?128:0)

#define B8(d)                   ((unsigned char)B8__(HEX__(d)))
#define B16(dmsb,dlsb)          (((unsigned short)B8(dmsb)<<8) + B8(dlsb))
#define B32(dmsb,db2,db3,dlsb)  (((unsigned long)B8(dmsb)<<24) \
                               + ((unsigned long)B8(db2)<<16) \
                               + ((unsigned long)B8(db3)<<8) \
                               + B8(dlsb))


/*********************************************************************************************************************
**  Macros for the Bit Manipulation Engine (BME)
*/
#define BME_AND_B(ADDR)                 (*(volatile uint8_t*)(((uint32_t)ADDR) | (1<<26)))
#define BME_AND_H(ADDR)                 (*(volatile uint16_t*)(((uint32_t)ADDR) | (1<<26)))
#define BME_AND_W(ADDR)                 (*(volatile uint32_t*)(((uint32_t)ADDR) | (1<<26)))
#define BME_OR_B(ADDR)                  (*(volatile uint8_t*)(((uint32_t)ADDR) | (1<<27)))
#define BME_OR_H(ADDR)                  (*(volatile uint16_t*)(((uint32_t)ADDR) | (1<<27)))
#define BME_OR_W(ADDR)                  (*(volatile uint32_t*)(((uint32_t)ADDR) | (1<<27)))
#define BME_XOR_B(ADDR)                 (*(volatile uint8_t*)(((uint32_t)ADDR) | (3<<26)))
#define BME_XOR_H(ADDR)                 (*(volatile uint16_t*)(((uint32_t)ADDR) | (3<<26)))
#define BME_XOR_W(ADDR)                 (*(volatile uint32_t*)(((uint32_t)ADDR) | (3<<26)))
#define BME_BFI_B(ADDR,BIT,WIDTH)       (*(volatile uint8_t*)(((uint32_t)ADDR) | (1<<28) | (BIT<<23) | ((WIDTH-1)<<19)))
#define BME_BFI_H(ADDR,BIT,WIDTH)       (*(volatile uint16_t*)(((uint32_t)ADDR) | (1<<28) | (BIT<<23) | ((WIDTH-1)<<19)))
#define BME_BFI_W(ADDR,BIT,WIDTH)       (*(volatile uint32_t*)(((uint32_t)ADDR) | (1<<28) | (BIT<<23) | ((WIDTH-1)<<19)))
#define BME_LAC1_B(ADDR,BIT)            (*(volatile uint8_t*)(((uint32_t)ADDR) | (1<<27) | (BIT<<21)))
#define BME_LAC1_H(ADDR,BIT)            (*(volatile uint16_t*)(((uint32_t)ADDR) | (1<<27) | (BIT<<21)))
#define BME_LAC1_W(ADDR,BIT)            (*(volatile uint32_t*)(((uint32_t)ADDR) | (1<<27) | (BIT<<21)))
#define BME_LAS1_B(ADDR,BIT)            (*(volatile uint8_t*)(((uint32_t)ADDR) | (3<<26) | (BIT<<21)))
#define BME_LAS1_H(ADDR,BIT)            (*(volatile uint16_t*)(((uint32_t)ADDR) | (3<<26) | (BIT<<21)))
#define BME_LAS1_W(ADDR,BIT)            (*(volatile uint32_t*)(((uint32_t)ADDR) | (3<<26) | (BIT<<21)))
#define BME_UBFX_B(ADDR,BIT,WIDTH)      (*(volatile uint8_t*)(((uint32_t)ADDR) | (1<<28) | (BIT<<23) | ((WIDTH-1)<<19)))
#define BME_UBFX_H(ADDR,BIT,WIDTH)      (*(volatile uint16_t*)(((uint32_t)ADDR) | (1<<28) | (BIT<<23) | ((WIDTH-1)<<19)))
#define BME_UBFX_W(ADDR,BIT,WIDTH)      (*(volatile uint32_t*)(((uint32_t)ADDR) | (1<<28) | (BIT<<23) | ((WIDTH-1)<<19)))


/**********************************************************************************************************************
**  Multipurpose Clock Generator (MCG)
**
**  0x4006_4000 MCG_C1          MCG Control 1 Register
**        _4001 MCG_C2          MCG Control 2 Register
**        _4002 MCG_C3          MCG Control 3 Register
**        _4003 MCG_C4          MCG Control 4 Register
**        _4004 MCG_C5          MCG Control 5 Register
**        _4005 MCG_C6          MCG Control 6 Register
**        _4006 MCG_S           MCG Status Register
**        _4007 reserved
**        _4008 MCG_SC          MCG Status and Control Register
**        _4009 reserved
**        _400A MCG_ATCVH       MCG Auto Trim Compare Value High Register
**        _400B MCG_ATCVL       MCG Auto Trim Compare Value Low Register
**        _400C MCG_C7          MCG Control 7 Register
**        _400D MCG_C8          MCG Control 8 Register
**        _400E MCG_C9          MCG Control 9 Register
**        _400F MCG_C10         MCG Control 10 Register
**
**  Operate in FEI mode.
**  Slow Internal Reference Clock trimmed to 32.768 kHz.
**  MCGOUTCLK = 32768 * 1464 = 47.972352MHz
*/

//  MCG_C1          MCG Control 1 Register
#define init_MCG_C1                 \
  B8(00000100)
/*   00000100 = reset
**   ||||||||
**   |||||||+-- IREFSTEN        =0  : Internal reference clock is disabled in Stop mode
**   ||||||+--- IRCLKEN         =0  : MCGIRCLK inactive
**   |||||+---- IREFS           =1  : FLL uses the slow internal reference clock
**   ||+++----- FRDIV           =0  : Divide Factor is either 1 or 32 (depending upon RANGE)
**   ++-------- CLKS            =0  : MCGOUTCLK clock source is either the FLL or PLL (depending upon PLLS)
*/

//  MCG_C2          MCG Control 2 Register
#define init_MCG_C2                 \
  B8(10000000)
/*   10000000 = reset
**   |x||||||
**   | |||||+-- IRCS            =0  : Slow internal reference clock selected
**   | ||||+--- LP              =0  : FLL or PLL is not disabled in bypass modes
**   | |||+---- EREFS0          =0  : External reference clock requested
**   | ||+----- HGO0            =0  : Configure crystal oscillator for low-power operation
**   | ++------ RANGE           =0  : Low frequency range selected for the crystal oscillator
**   +--------- LOCRE0          =1  : Loss of Clock generates a reset
*/

//  MCG_C4          MCG Control 4 Register
#define init_MCG_C4                 \
  B8(10100000)
/*   000!!!!! = reset
**   ||||||||
**   |||||||+-- SCFTRIM             : Slow Internal Reference Fine Trim - factory programmed
**   |||++++--- FCTRIM              : Fast Internal Reference Trim - factory programmed
**   |++------- DRST_DRS        =1  : \/ Clock reference is 32768 Hz
**   +--------- DMX32           =1  : /\ FLL multiplication factor is 1464
*/

//  MCG_C5          MCG Control 5 Register
#define init_MCG_C5                 \
  B8(00000000)
/*   00000000 = reset
**   x|||||||
**    ||+++++-- PRDIV0          =0  : PLL external reference divide-by-1
**    |+------- PLLSTEN0        =0  : MCGPLLCLK is disabled in any of the Stop modes
**    +-------- PLLCLKEN0       =0  : MCGPLLCLK is inactive
*/

//  MCG_C6          MCG Control 6 Register
#define init_MCG_C6                 \
  B8(00000000)
/*   00000000 = reset
**   ||||||||
**   |||+++++-- VDIV0           =0  : PLL VCO divide-by-24
**   ||+------- CME0            =0  : External clock monitor is disabled for OSC0
**   |+-------- PLLS            =0  : FLL is selected
**   +--------- LOLIE0          =0  : No interrupt request is generated on loss of lock
*/

//  MCG_SC          MCG Status and Control Register
#define init_MCG_SC                 \
  B8(00000010)
/*   00000010 = reset
**   ||||||||
**   |||||||+-- LOCS0               : OSC0 Loss of Clock Status
**   ||||+++--- FCRDIV          =1  : Fast Clock Internal Reference divide-by-2
**   |||+------ FLTPRSRV        =0  : FLL filter and FLL frequency will reset on changes to currect clock mode
**   ||+------- ATMF                : Automatic Trim Machine Fail Flag
**   |+-------- ATMS            =0  : Automatic Trim Machine 32 kHz Internal Reference Clock selected
**   +--------- ATME            =0  : Auto Trim Machine disabled
*/


#define NVM_SCTRIM                          *((uint8_t*) 0x03FF)
#define NVM_SCFTRIM                         ((*((uint8_t*) 0x03FE)) & 0x01)


/**********************************************************************************************************************
**  Oscillator (OSC)
**
**  0x4006_5000 OSC0_CR         OSC Control Register
*/

//  OSC Control Register
#define init_OSC0_CR                \
  B8(00000000)
/*   00000000 = reset
**   |x|x||||
**   | | ++++-- SCP             =0  : Capacitor load = 0pF
**   | +------- EREFSTEN        =0  : External reference clock is disabled in Stop mode
**   +--------- ERCLKEN         =0  : External reference clock is inactive
*/


/**********************************************************************************************************************
**  System Integration Module (SIM)
**
**  0x4004_7000 SIM_SOPT1       System Options Register 1
**         7004 SIM_SOPT1CFG    SOPT1 Configuration Register
**         8004 SIM_SOPT2       System Options Register 2
**         800C SIM_SOPT4       System Options Register 4
**         8010 SIM_SOPT5       System Options Register 5
**         8018 SIM_SOPT7       System Options Register 7
**         8024 SIM_SDID        System Device Identification Register
**         8034 SIM_SCGC4       System Clock Gating Control Register 4
**         8038 SIM_SCGC5       System Clock Gating Control Register 5
**         803C SIM_SCGC6       System Clock Gating Control Register 6
**         8040 SIM_SCGC7       System Clock Gating Control Register 7
**         8044 SIM_CLKDIV1     System Clock Divider Register 1
**         804C SIM_FCFG1       Flash Configuration Register 1
**         8050 SIM_FCFG2       Flash Configuration Register 2
**         8058 SIM_UIDML       Unique Identification Register Mid-High
**         805C SIM_UIDML       Unique Identification Register Mid-Low
**         8060 SIM_UIDL        Unique Identification Register Low
**         8100 SIM_COPC        COP Control Register
**         8104 SIM_SRVCOP      Service COP Register
*/

//  SIM_SOPT1       System Options Register 1
#define init_SIM_SOPT1              \
  B32(10000000,00000000,00000000,00000000)
/*    10000000_00000000_00000000_00000000 = reset
**    |||xxxxx xxxx||xx xxxxxxxx xxxxxxxx
**    |||          ++---------------------- OSC32KSEL       =0  : 32kHz clock source is the system oscillator
**    ||+---------------------------------- USBVSTBY        =0  : USB VREG not in standby in VLPR & VLPW
**    |+----------------------------------- USBSSTBY        =0  : USB VREG not in standby in Stop, VLPS, LLS and VLLS
**    +------------------------------------ USBREGEN        =1  : USB voltage regulator is enabled
*/

//  SIM_SOPT1CFG    SOPT1 Configuration Register
#define init_SIM_SOPT1CFG           \
  B32(00000000,00000000,00000000,00000000)
/*    00000000_00000000_00000000_00000000 = reset
**    xxxxx||| xxxxxxxx xxxxxxxx xxxxxxxx
**        ||+------------------------------ URWE            =0  : SOPT1 USBREGEN cannot be written
**        |+------------------------------- UVSWE           =0  : SOPT1 USBVSTB cannot be written
**        +-------------------------------- USSWE           =0  : SOPT1 USBSSTB cannot be written
*/


//  SIM_SOPT2       System Options Register 2
#define init_SIM_SOPT2              \
  B32(00000000,00000000,00000000,00000000)
/*    00000000_00000000_00000000_00000000 = reset
**    xxxx|||| xxxxx|x| xxxxxxxx ||||xxxx
**        ||||      | |          |||+------ RTCCLKOUTSEL    =0  : RTC 1Hz clock is output on the RTC_CLKOUT pin
**        ||||      | |          +++------- CLKOUTSEL       =0  : reserved
**        ||||      | +-------------------- PLLFLLSEL       =0  : FLL clock selected
**        ||||      +---------------------- USBSRC          =0  : USB clock is external (USB_CLKIN)
**        ||++----------------------------- TPMSRC          =0  : TPM clock is disabled
**        ++------------------------------- UART0SRC        =0  : UART0 clock is disabled
*/

//  SIM_SOPT4       System Options Register 4
#define init_SIM_SOPT4              \
  B32(00000000,00000000,00000000,00000000)
/*    00000000_00000000_00000000_00000000 = reset
**    xxxxx||| xxx|x|xx xxxxxxxx xxxxxxxx
**         |||    | +---------------------- TPM1CH0SRC      =0  : TPM1 CH0 input capture source is TPM1_CH0 pin
**         |||    +------------------------ TPM2CH0SRC      =0  : TPM2 CH0 input capture source is TPM2_CH0 pin
**         ||+----------------------------- TPM0CLKSEL      =0  : TPM0 external clock driven by TPM_CLKIN0 pin
**         |+------------------------------ TPM1CLKSEL      =0  : TPM1 external clock driven by TPM_CLKIN0 pin
**         +------------------------------- TPM2CLKSEL      =0  : TPM2 external clock driven by TPM_CLKIN0 pin
*/

//  SIM_SOPT5       System Options Register 5
#define init_SIM_SOPT5              \
  B32(00000000,00000000,00000000,00000000)
/*    00000000_00000000_00000000_00000000 = reset
**    xxxxxxxx xxxxx||| xxxxxxxx x|||x|||
**                  |||           ||| |++-- UART0TXSRC      =0  : UART0 transmit data source is UART0_TX pin
**                  |||           ||| +---- UART0RXSRC      =0  : UART0 receive data source is UART0_RX pin
**                  |||           |++------ UART1TXSRC      =0  : UART1 transmit data source is UART1_TX pin
**                  |||           +-------- UART1RXSRC      =0  : UART1 receive data source is UART1_RX pin
**                  ||+-------------------- UART0ODE        =0  : Open drain is disabled on UART0
**                  |+--------------------- UART1ODE        =0  : Open drain is disabled on UART1
**                  +---------------------- UART2ODE        =0  : Open drain is disabled on UART2
*/

//  SIM_SOPT7       System Options Register 7
#define init_SIM_SOPT7              \
  B32(00000000,00000000,00000000,00000000)
/*    00000000_00000000_00000000_00000000 = reset
**    xxxxxxxx xxxxxxxx xxxxxxxx |xx|||||
**                               |  |++++-- ADC0TRGSEL      =0  : ADC0 trigger is external pin (EXTRIG_IN)
**                               |  +------ ADC0PRETRGSEL   =0  : ADC0 pretrigger is A
**                               +--------- ADC0ALTTRGEN    =0  : ADC0 alternate trigger is TPM1 CH0 or CH1
*/

//  SIM_SCGC4       System Clock Gating Control Register 4
#define init_SIM_SCGC4              \
  B32(11110000,00000000,00000000,00110000)
/*    11110000_00000000_00000000_00110000 = reset
**    xxxxxxxx ||xx||xx xxx|||xx ||xxxxxx
**             ||  ||      |||   |+-------- I2C0            =0  : I2C0 clock is disabled
**             ||  ||      |||   +--------- I2C1            =0  : I2C1 clock is disabled
**             ||  ||      ||+------------- UART0           =0  : UART0 clock is disabled
**             ||  ||      |+-------------- UART1           =0  : UART1 clock is disabled
**             ||  ||      +--------------- UART2           =0  : UART2 clock is disabled
**             ||  |+---------------------- USBOTG          =0  : USBOTG clock is disabled
**             ||  +----------------------- CMP             =0  : CMP clock is disabled
**             |+-------------------------- SPI0            =0  : SPI0 clock is disabled
**             +--------------------------- SPI1            =0  : SPI1 clock is disabled
*/

//  SIM_SCGC5       System Clock Gating Control Register 5
#define init_SIM_SCGC5              \
  B32(00000000,00000000,00111111,10000000)
/*    00000000_00000000_00000001_10000000 = reset
**    xxxxxxxx xxxxxxxx xx|||||x xx|xxxx|
**                        |||||    |    +-- LPTMR           =0  : LPTMR access is disabled
**                        |||||    +------- TSI             =0  : TSI access is disabled
**                        ||||+------------ PORTA           =0  : Port A clock is enabled
**                        |||+------------- PORTB           =0  : Port B clock is enabled
**                        ||+-------------- PORTC           =0  : Port C clock is enabled
**                        |+--------------- PORTD           =0  : Port D clock is enabled
**                        +---------------- PORTE           =0  : Port E clock is enabled
*/

//  SIM_SCGC6       System Clock Gating Control Register 6
#define init_SIM_SCGC6              \
  B32(00000000,00000000,00000000,00000001)
/*    00000000_00000000_00000000_00000001 = reset
**    |x|x|||| |xxxxxxx xxxxxxxx xxxxxx||
**    | | |||| |                       |+-- FTF             =1  : Flash memory clock is enabled
**    | | |||| |                       +--- DMAMUX          =0  : DMA MUX clock is disabled
**    | | |||| +--------------------------- PIT             =0  : PIT clock is disabled
**    | | |||+----------------------------- TPM0            =0  : TPM0 clock is disabled
**    | | ||+------------------------------ TPM1            =0  : TPM1 clock is disabled
**    | | |+------------------------------- TPM2            =0  : TPM2 clock is disabled
**    | | +-------------------------------- ADC0            =0  : ADC0 clock is disabled
**    | +---------------------------------- RTC             =0  : RTC clock is disabled
**    +------------------------------------ DAC0            =0  : DAC0 clock is disabled
*/

//  SIM_SCGC7       System Clock Gating Control Register 7
#define init_SIM_SCGC7              \
  B32(00000000,00000000,00000001,00000000)
/*    00000000_00000000_00000001_00000000 = reset
**    xxxxxxxx xxxxxxxxx xxxxxx| xxxxxxxx
**                             +----------- DMA             =1  : DMA clock is enabled
*/

//  SIM_CLKDIV1     System Clock Divider Register 1
#define init_SIM_CLKDIV1            \
  B32(00000000,00000001,00000000,00000000)
/*    00000000_00000001_00000000_00000000 = reset
**    ||||xxxx xxxxx||| xxxxxxxx xxxxxxxx
**    ||||          +++-------------------- OUTDIV4         =1  : Bus and Flash clock prescaler is divide-by-2
**    ++++--------------------------------- OUTDIV1         =1  : Core and System clock prescaler is divide-by-1
*/

//  SIM_COPC        COP Control Register
#define init_SIM_COPC               \
  B32(00000000,00000000,00000000,00000000)
/*    00000000_00000000_00000000_00001100 = reset
**    xxxxxxxx xxxxxxxx xxxxxxxx xxxx||||
**                                   |||+-- COPW            =0  : COP normal mode
**                                   ||+--- COPCLKS         =0  : COP clock source is internal 1kHz clock
**                                   ++---- COPT            =0  : COP is disabled
*/

#define __RESET_WATCHDOG()                  \
  register volatile uint32_t* cop_ptr;      \
  cop_ptr = &SIM_SRVCOP;                    \
  *cop_ptr = 0x55;                          \
  *cop_ptr = 0xAA


/**********************************************************************************************************************
**  Power Management Controller (PMC)
**
**  0x4007_D000 PMC_LVDSC1      Low Voltage Detect Status And Control 1 Register
**         D001 PMC_LVDSC2      Low Voltage Detect Status And Control 2 Register
**         D002 PMC_REGSC       Regulator Status And Control Register
*/

//  PMC_LVDSC1      Low Voltage Detect Status And Control 1 Register
#define init_PMC_LVDSC1             \
  B8(00010000)
/*   00010000 = reset
**   ||||xx||
**   ||||  ++-- LVDV            =0  : Low-Voltage Detect low trip point selected - V(LVDL)
**   |||+------ LVDRE           =1  : Force an MCU reset when LVDF = 1
**   ||+------- LVDIE           =0  : Low-Voltage Detect interrupt is disabled
**   |+-------- LVDACK          =0  : Low-Voltage Detect Acknowledge (w1c)
**   +--------- LVDF                : Low-Voltage Detect Flag
*/

//  PMC_LVDSC2      Low Voltage Detect Status And Control 2 Register
#define init_PMC_LVDSC2             \
  B8(00000000)
/*   00000000 = reset
**   |||xxx||
**   |||   ++-- LVWV            =0  : Low-Voltage Warning low trip point selected - V(LVW1)
**   ||+------- LVWIE           =0  : Low-Voltage Warning interrupt is disabled
**   |+-------- LVWACK          =0  : Low-Voltage Warning Acknowledge (w1c)
**   +--------- LVWF                : Low-Voltage Warning Flag
*/

//  PMC_REGSC       Regulator Status And Control Register
#define init_PMC_REGSC              \
  B8(00001100)
#define init_PMC_REGSC_RESET        \
  B8(00000100)
/*   00000100 = reset
**   xxx|||x|
**      ||| +-- BDBE            =0  : Bandgap buffer is not enabled
**      ||+---- REGONS              : Regulator In Run Regulation Status
**      |+----- ACKISO          =1  : Release the I/O pads and certain peripherals to their normal Run mode state
**      +------ BGEN            =0  : Bandgap voltage reference is disabled in VLPx , LLS , and VLLSx modes
*/


/**********************************************************************************************************************
**  Flash Memory Module (FTFA)
**
**  0x4002_0000 FTFA_FSTAT      Flash Status Register
**         0001 FTFA_FCNFG      Flash Configuration Register
**         0002 FTFA_FSEC       Flash Security Register
**         0003 FTFA_OPT        Flash Option Register
**         0004 FTFA_FCCOB3     Flash Common Command Object Register 3
**         0005 FTFA_FCCOB2     Flash Common Command Object Register 2
**         0006 FTFA_FCCOB1     Flash Common Command Object Register 1
**         0007 FTFA_FCCOB0     Flash Common Command Object Register 0
**         0008 FTFA_FCCOB7     Flash Common Command Object Register 7
**         0009 FTFA_FCCOB6     Flash Common Command Object Register 6
**         000A FTFA_FCCOB5     Flash Common Command Object Register 5
**         000B FTFA_FCCOB4     Flash Common Command Object Register 4
**         000C FTFA_FCCOBB     Flash Common Command Object Register B
**         000D FTFA_FCCOBA     Flash Common Command Object Register A
**         000E FTFA_FCCOB9     Flash Common Command Object Register 9
**         000F FTFA_FCCOB8     Flash Common Command Object Register 8
**         0010 FTFA_FPROT3     Program Flash Protection Register 3
**         0011 FTFA_FPROT2     Program Flash Protection Register 2
**         0012 FTFA_FPROT1     Program Flash Protection Register 1
**         0013 FTFA_FPROT0     Program Flash Protection Register 0
*/

//  FTFA_FCNFG      Flash Configuration Register
#define init_FTFA_FCNFG             \
  (0b00000000)
/*   00000000 = reset
**   ||||xxxx
**   |||+------ ERSSUSP         =0  : No Erase Suspended requested
**   ||+------- ERSAREQ             : Erase All Request flag
**   |+-------- RDCOLLIE        =0  : Read collision error interrupt disabled
**   +--------- CCIE            =0  : Command complete interrupt disabled
*/

//  FTFA_FSEC       Flash Security Register
#define init_FTFA_FSEC              \
  (0b01111110)
/*   01111110   <-- loaded via Flash Configuration Field
**   ||||||||
**   ||||||++-- SEC             =2  : MCU security status is unsecure
**   ||||++---- FSLACC          =3  : Freescale factory access granted
**   ||++------ MEEN            =3  : Mass erase is enabled
**   ++-------- KEYEN           =1  : Backdoor key access disabled
*/

//  FTFA_OPT        Flash Option Register
#define init_FTFA_OPT               \
  (0b11111111)
/*   11111111   <-- loaded via Flash Configuration Field
**   xx||||x|
**     |||| +-- LPBOOT          =3  : Core and system clock divider (OUTDIV1) is 0x0 (divide by 1)
**     |||+---- NMI_DIS         =1  : NMI pin/interrupts enabled
**     ||+----- RESET_PIN_CFG   =1  : RESET pin is dedicated
**     |+------ LPBOOT          =3  : Core and system clock divider (OUTDIV1) is 0x0 (divide by 1)
**     +------- FAST_INIT       =1  : Fast initialization
*/

//  FTFA_FPROT3     Program Flash Protection Register 3
#define init_FTFA_FPROT3            \
  (0b11111110)
/*   11111111   <-- loaded via Flash Configuration Field
**   ||||||||
**   |||||||+-- PROT0           =0  : 0x0000_0000 to 0x0000_0FFF is protected (4096 bytes)
**   ||||||+--- PROT1           =1  : 0x0000_1000 to 0x0000_1FFF is not protected (4096 bytes)
**   |||||+---- PROT2           =1  : 0x0000_2000 to 0x0000_2FFF is not protected (4096 bytes)
**   ||||+----- PROT3           =1  : 0x0000_3000 to 0x0000_3FFF is not protected (4096 bytes)
**   |||+------ PROT4           =1  : 0x0000_4000 to 0x0000_4FFF is not protected (4096 bytes)
**   ||+------- PROT5           =1  : 0x0000_5000 to 0x0000_5FFF is not protected (4096 bytes)
**   |+-------- PROT6           =1  : 0x0000_6000 to 0x0000_6FFF is not protected (4096 bytes)
**   +--------- PROT7           =1  : 0x0000_7000 to 0x0000_7FFF is not protected (4096 bytes)
*/

//  FTFA_FPROT2     Program Flash Protection Register 2
#define init_FTFA_FPROT2            \
  (0b11111111)
/*   11111111   <-- loaded via Flash Configuration Field
**   ||||||||
**   |||||||+-- PROT8           =1  : 0x0000_8000 to 0x0000_8FFF is not protected (4096 bytes)
**   ||||||+--- PROT9           =1  : 0x0000_9000 to 0x0000_9FFF is not protected (4096 bytes)
**   |||||+---- PROT10          =1  : 0x0000_A000 to 0x0000_AFFF is not protected (4096 bytes)
**   ||||+----- PROT11          =1  : 0x0000_B000 to 0x0000_BFFF is not protected (4096 bytes)
**   |||+------ PROT12          =1  : 0x0000_C000 to 0x0000_CFFF is not protected (4096 bytes)
**   ||+------- PROT13          =1  : 0x0000_D000 to 0x0000_DFFF is not protected (4096 bytes)
**   |+-------- PROT14          =1  : 0x0000_E000 to 0x0000_EFFF is not protected (4096 bytes)
**   +--------- PROT15          =1  : 0x0000_F000 to 0x0000_FFFF is not protected (4096 bytes)
*/

//  FTFA_FPROT1     Program Flash Protection Register 1
#define init_FTFA_FPROT1            \
  (0b11111111)
/*   11111111   <-- loaded via Flash Configuration Field
**   ||||||||
**   |||||||+-- PROT16          =1  : 0x0001_0000 to 0x0001_0FFF is not protected (4096 bytes)
**   ||||||+--- PROT17          =1  : 0x0001_1000 to 0x0001_1FFF is not protected (4096 bytes)
**   |||||+---- PROT18          =1  : 0x0001_2000 to 0x0001_2FFF is not protected (4096 bytes)
**   ||||+----- PROT19          =1  : 0x0001_3000 to 0x0001_3FFF is not protected (4096 bytes)
**   |||+------ PROT20          =1  : 0x0001_4000 to 0x0001_4FFF is not protected (4096 bytes)
**   ||+------- PROT21          =1  : 0x0001_5000 to 0x0001_5FFF is not protected (4096 bytes)
**   |+-------- PROT22          =1  : 0x0001_6000 to 0x0001_6FFF is not protected (4096 bytes)
**   +--------- PROT23          =1  : 0x0001_7000 to 0x0001_7FFF is not protected (4096 bytes)
*/

//  FTFA_FPROT0     Program Flash Protection Register 0
#define init_FTFA_FPROT0            \
  (0b11111111)
/*   11111111   <-- loaded via Flash Configuration Field
**   ||||||||
**   |||||||+-- PROT24          =1  : 0x0001_8000 to 0x0001_8FFF is not protected (4096 bytes)
**   ||||||+--- PROT25          =1  : 0x0001_9000 to 0x0001_9FFF is not protected (4096 bytes)
**   |||||+---- PROT26          =1  : 0x0001_A000 to 0x0001_AFFF is not protected (4096 bytes)
**   ||||+----- PROT27          =1  : 0x0001_B000 to 0x0001_BFFF is not protected (4096 bytes)
**   |||+------ PROT28          =1  : 0x0001_C000 to 0x0001_CFFF is not protected (4096 bytes)
**   ||+------- PROT29          =1  : 0x0001_D000 to 0x0001_DFFF is not protected (4096 bytes)
**   |+-------- PROT30          =1  : 0x0001_E000 to 0x0001_EFFF is not protected (4096 bytes)
**   +--------- PROT31          =1  : 0x0001_F000 to 0x0001_FFFF is not protected (4096 bytes)
*/


/**********************************************************************************************************************
**	Pin Control Register 
*/
//-------------------------------------------------
//2/17/16 change
/*  PORTA_PCR1
**
**  0000_0000_0000_0000_0000_0000_0000_0000 = reset
**  0000_0000_0000_0000_0000_0010_0000_0000 = 0x00000200
**  xxxx xxx| xxxx |||| xxxx x||| x|x| x|||
**          |      ||||       |||  | |  ||+-- PS              =0 : Internal pulldown
**          |      ||||       |||  | |  |+--- PE              =0 : Internal pullup or pulldown resistor is disabled
**          |      ||||       |||  | |  +---- SRE             =0 : Fast slew rate
**          |      ||||       |||  | +------- PFE             =0 : Passive input filter is disabled
**          |      ||||       |||  +--------- DSE             =0 : Low drive strength
**          |      ||||       +++------------ MUX             =2 : Pin Mux Control = ALT2 = UART0_RX
**          |      ++++---------------------- IRQC            =0 : Interrupt/DMA request disabled
**          +-------------------------------- ISF                : Interrupt Status Flag
*/
#define init_PORTA_PCR1                     (0x00000200)

/*  PORTA_PCR2
**
**  0000_0000_0000_0000_0000_0000_0000_0000 = reset
**  0000_0000_0000_0000_0000_0010_0000_0000 = 0x00000200
**  xxxx xxx| xxxx |||| xxxx x||| x|x| x|||
**          |      ||||       |||  | |  ||+-- PS              =0 : Internal pulldown
**          |      ||||       |||  | |  |+--- PE              =0 : Internal pullup or pulldown resistor is disabled
**          |      ||||       |||  | |  +---- SRE             =0 : Fast slew rate
**          |      ||||       |||  | +------- PFE             =0 : Passive input filter is disabled
**          |      ||||       |||  +--------- DSE             =0 : Low drive strength
**          |      ||||       +++------------ MUX             =2 : Pin Mux Control = ALT2 = UART0_TX
**          |      ++++---------------------- IRQC            =0 : Interrupt/DMA request disabled
**          +-------------------------------- ISF                : Interrupt Status Flag
*/
#define init_PORTA_PCR2                     (0x00000200)
//-------------------------------------------------


/*********************************************************************************************************************\
* Public memory declarations
\*********************************************************************************************************************/

/*********************************************************************************************************************\
* Public prototypes
\*********************************************************************************************************************/


#endif /* HARDWARE_H_ */
