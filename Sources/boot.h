/*********************************************************************************************************************\
* BootProjectKL16
*
* File:   boot.h
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

#ifndef BOOT_H_
#define BOOT_H_

/*********************************************************************************************************************\
* Public type definitions
\*********************************************************************************************************************/
extern uint8_t RunApp;	//11/9/14 change
//
typedef union{	//11/7/14 add
	uint8_t reg;
	struct{
		unsigned UPDATE_ONLY		:1;	
	}bit;
}typ_BootFlag;
extern typ_BootFlag BootFlag;
//
#define SetUpdateOnly (BootFlag.bit.UPDATE_ONLY = 1)
#define ResetUpdateOnly (BootFlag.bit.UPDATE_ONLY = 0)
#define IsUpdateOnly (BootFlag.bit.UPDATE_ONLY == 1)
#define NotUpdateOnly (BootFlag.bit.UPDATE_ONLY == 0)
//
//
/*********************************************************************************************************************\
* Public macros
\*********************************************************************************************************************/

/*
**  The bootloader interface uses UART0 on PTA1 and PTA2.
*/

#define BOOT_PIN_CLOCK_GATE                 (SIM_SCGC5_PORTB_MASK)
#define BOOT_PIN_PORT_CONTROL_REGISTER      (PORTB_PCR0)
#define BOOT_PIN_PORT_CONTROL_INIT_VALUE    (0x00000103)
#define BOOT_PIN_PORT_INPUT_REGISTER        (GPIOB_PDIR)
#define BOOT_PIN_MASK                       (0x00000001)


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
**
**  If the Slow Internal Reference Clock (IRC) trimmed to 32.768 kHz, then
**    MCGOUTCLK = 32768 * 1464 = 47.972352 MHz
**  Otherwise,
**    MCGOUTCLK = f(IRC) * 1280 = 40 to 50 MHz
*/
#if (MCU_INTERNAL_REFERENCE_FREQUENCY == 32768)
#define boot_MCG_FLL_FACTOR                 (1464)
#else
#define boot_MCG_FLL_FACTOR                 (1280)
#endif
#define boot_MCU_BUS_FREQUENCY              (MCU_INTERNAL_REFERENCE_FREQUENCY * boot_MCG_FLL_FACTOR)

//  MCG_C1          MCG Control 1 Register
#define boot_MCG_C1                 \
  B8(00000100)
/*   00000100 = reset
**   ||||||||
**   |||||||+-- IREFSTEN        =0  : Internal reference clock is disabled in Stop mode
**   ||||||+--- IRCLKEN         =0  : MCGIRCLK inactive
**   |||||+---- IREFS        -> =1  : FLL uses the slow internal reference clock
**   ||+++----- FRDIV        -> =0  : Divide Factor is either 1 or 32 (depending upon RANGE)
**   ++-------- CLKS         -> =0  : MCGOUTCLK clock source is either the FLL or PLL (depending upon PLLS)
*/

//  MCG_C2          MCG Control 2 Register
#define boot_MCG_C2                 \
  B8(00000000)
/*   10000000 = reset
**   |x||||||
**   | |||||+-- IRCS            =0  : Slow internal reference clock selected
**   | ||||+--- LP              =0  : FLL or PLL is not disabled in bypass modes
**   | |||+---- EREFS0          =0  : External reference clock requested
**   | ||+----- HGO0            =0  : Configure crystal oscillator for low-power operation
**   | ++------ RANGE        -> =0  : Low frequency range selected for the crystal oscillator
**   +--------- LOCRE0          =0  : Loss of Clock generates an interrupt
*/

//  MCG_C4          MCG Control 4 Register
#define boot_MCG_C4                 \
  B8(10100000)
/*   000!!!!! = reset
**   ||||||||
**   |||||||+-- SCFTRIM             : Slow Internal Reference Fine Trim - factory programmed
**   |||++++--- FCTRIM              : Fast Internal Reference Trim - factory programmed
**   |++------- DRST_DRS     -> =1  : \/ Clock reference is 32768 Hz
**   +--------- DMX32        -> =1  : /\ FLL multiplication factor is 1464
*/

//  MCG_C5          MCG Control 5 Register
#define boot_MCG_C5                 \
  B8(10111111)
/*   00000000 = reset               mask for PLLCLKEN0 = 0
**   x|||||||
**    ||+++++-- PRDIV0          =0  : PLL external reference divide-by-1
**    |+------- PLLSTEN0        =0  : MCGPLLCLK is disabled in any of the Stop modes
**    +-------- PLLCLKEN0    -> =0  : MCGPLLCLK is inactive
*/

//  MCG_C6          MCG Control 6 Register
#define boot_MCG_C6                 \
  B8(10111111)
/*   00000000 = reset               mask for PLLS = 0
**   ||||||||
**   |||+++++-- VDIV0           =0  : PLL VCO divide-by-24
**   ||+------- CME0            =0  : External clock monitor is disabled for OSC0
**   |+-------- PLLS         -> =0  : FLL is selected
**   +--------- LOLIE0          =0  : No interrupt request is generated on loss of lock
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

//  SIM_SOPT2       System Options Register 2
#define boot_SIM_SOPT2			\
  B32(00000100,00000000,00000000,00000000)	//2/16/16 change
/*    00000000 00000000 00000000 00000000 = reset
**    xxxx|||| xxxxx|x| xxxxxxxx ||||xxxx
**        ||||      | |          |||+------ RTCCLKOUTSEL    =0  : RTC 1Hz clock is output on the RTC_CLKOUT pin
**        ||||      | |          +++------- CLKOUTSEL       =0  : reserved
**        ||||      | +-------------------- PLLFLLSEL       =0  : FLL clock selected
**        ||||      +---------------------- USBSRC          =0  : USB clock is external (USB_CLKIN)
**        ||++----------------------------- TPMSRC          =0  : TPM clock is disabled
**        ++------------------------------- UART0SRC        =0  : UART0 clock is MCGFLLCLK
*/

//  SIM_SCGC4       System Clock Gating Control Register 4
#define boot_SIM_SCGC4                      (SIM_SCGC4_UART0_MASK)	//2/16/16 change
/*    11110000 00000000 00000000 00110000 = reset
**    xxxxxxxx ||xx||xx xxx|||xx ||xxxxxx
**             ||  ||      |||   |+-------- I2C0            =0  : I2C0 clock is disabled
**             ||  ||      |||   +--------- I2C1            =0  : I2C1 clock is disabled
**             ||  ||      ||+------------- UART0           =0  : UART0 clock is enabled
**             ||  ||      |+-------------- UART1           =0  : UART1 clock is disabled
**             ||  ||      +--------------- UART2           =1  : UART2 clock is disabled
**             ||  |+---------------------- USBOTG          =0  : USBOTG clock is disabled
**             ||  +----------------------- CMP             =0  : CMP clock is disabled
**             |+-------------------------- SPI0            =0  : SPI0 clock is disabled
**             +--------------------------- SPI1            =0  : SPI1 clock is disabled
*/

//  SIM_SCGC5       System Clock Gating Control Register 5
#define boot_SIM_SCGC5                      (SIM_SCGC5_PORTA_MASK)	//2/16/16 change
/*    00000000 00000000 00000001 10000000 = reset
**    xxxxxxxx xxxxxxxx xx|||||x xx|xxxx|
**                        |||||    |    +-- LPTMR           =0  : LPTMR access is disabled
**                        |||||    +------- TSI             =0  : TSI access is disabled
**                        ||||+------------ PORTA           =0  : Port A clock is enabled
**                        |||+------------- PORTB           =0  : Port B clock is disabled
**                        ||+-------------- PORTC           =0  : Port C clock is disabled
**                        |+--------------- PORTD           =1  : Port D clock is disabled
**                        +---------------- PORTE           =0  : Port E clock is disabled
*/
  

//  SIM_CLKDIV1     System Clock Divider Register 1
#define boot_SIM_CLKDIV1_SAFE       \
  B32(00000000,00000011,00000000,00000000)
/*    00000000 00000001 00000000 00000000 = reset
**    ||||xxxx xxxxx||| xxxxxxxx xxxxxxxx
**    ||||          +++-------------------- OUTDIV4         =3  : Bus and Flash clock prescaler is divide-by-4
**    ++++--------------------------------- OUTDIV1         =0  : Core and System clock prescaler is divide-by-1
*/
#define boot_SIM_CLKDIV1_RUN        \
  B32(00000000,00000001,00000000,00000000)
/*    00000000 00000001 00000000 00000000 = reset
**    ||||xxxx xxxxx||| xxxxxxxx xxxxxxxx
**    ||||          +++-------------------- OUTDIV4         =1  : Bus and Flash clock prescaler is divide-by-2
**    ++++--------------------------------- OUTDIV1         =1  : Core and System clock prescaler is divide-by-1
*/


//2/16/16 change
/**********************************************************************************************************************
**  Port control and interrupts (PORT)
**
**  0x4004_9004 PORTA_PCR1      PTA1 Pin Control Register     = UART0_RX
**        _9008 PORTA_PCR2      PTA2 Pin Control Register     = UART0_TX
*/
//  PORTA_PCR1      PTA1 Pin Control Register     = UART0_RX
#define boot_PORTA_PCR1             \
  B32(00000000,00000000,00000010,00000000)
/*    00000000 00000000 00000000 0X0X0XXX = reset
**    xxxxxxx| xxxx|||| xxxxx||| x|x|x|||
**           |     ||||      |||  | | ||+-- PS              =0 : Internal pulldown
**           |     ||||      |||  | | |+--- PE              =0 : Internal pullup or pulldown resistor is not enabled
**           |     ||||      |||  | | +---- SRE             =0 : Fast slew rate
**           |     ||||      |||  | +------ PFE             =0 : Passive input filter is disabled
**           |     ||||      |||  +-------- DSE             =0 : Low drive strength
**           |     ||||      +++----------- MUX             =2 : Pin Mux Control = ALT2 = UART0_RX
**           |     ++++-------------------- IRQC            =0 : Interrupt/DMA request disabled
**           +----------------------------- ISF                : Interrupt Status Flag
*/
  
//  PORTA_PCR2      PTA2 Pin Control Register     = UART0_TX
#define boot_PORTA_PCR2             \
  B32(00000000,00000000,00000010,00000000)
/*    00000000 00000000 00000000 0X0X0XXX = reset
**    xxxxxxx| xxxx|||| xxxxx||| x|x|x|||
**           |     ||||      |||  | | ||+-- PS              =0 : Internal pulldown
**           |     ||||      |||  | | |+--- PE              =0 : Internal pullup or pulldown resistor is not enabled
**           |     ||||      |||  | | +---- SRE             =0 : Fast slew rate
**           |     ||||      |||  | +------ PFE             =0 : Passive input filter is disabled
**           |     ||||      |||  +-------- DSE             =0 : Low drive strength
**           |     ||||      +++----------- MUX             =2 : Pin Mux Control = ALT2 = UART0_TX
**           |     ++++-------------------- IRQC            =0 : Interrupt/DMA request disabled
**           +----------------------------- ISF                : Interrupt Status Flag
*/

//2/16/16 change
/**********************************************************************************************************************
**  Universal Asynchronous Receiver/Transmitter (UART0)
**
**  0x4006_A000 UART0_BDH       UART0 Baud Rate Register High
**        _A001 UART0_BDL       UART0 Baud Rate Register Low
**        _A002 UART0_C1        UART0 Control Register 1
**        _A003 UART0_C2        UART0 Control Register 2
**        _A004 UART0_S1        UART0 Status Register 1
**        _A005 UAAR0_S2        UART0 Status Register 2
**        _A006 UART0_C3        UART0 Control Register 3
**        _A007 UART0_D         UART0 Data Register
**        _A008 UART0_MA1		UART0 Match Address Register 1
**        _A008 UART0_MA2		UART0 Match Address Register 2
**        _A00A UART0_C4        UART0 Control Register 4
**        _A00B UART0_C5        UART0 Control Register 5
**
**  The UART is configured for 57.6 k baud operation.
**  The UART is configured for 115.2 kbaud operation.
**  UART0 clock is MCGFLLCLK (as per SOPT2.UART0SRC) = 40 to 50 MHz.
**  The UART over sampling ratio is 16
*/
//#define boot_UART_BAUD_DIVIDER              (boot_MCU_BUS_FREQUENCY / MCU_BOOTLOADER_UART_BAUD_RATE / 16 / 2)
#define boot_UART_BAUD_DIVIDER              (boot_MCU_BUS_FREQUENCY / MCU_BOOTLOADER_UART_BAUD_RATE / 16)	//3/25/16 change

//  UART0 Baud Rate Register High
#define boot_UART0_BDH              \
  B8(00000000)
/*   00000000 = reset
**   ||||||||
**   |||+++++-- SBRU            =0 : Baud rate modulo divider upper value
**   ||+------- SBNS            =0 : One stop bit
**   |+-------- RXEDGIE         =0 : RX input active edge interrupts are disabled
**   +--------- LBKDIE          =0 : LIN break detect interrupts are disabled
*/

//  UART2_BDL       UART0 Baud Rate Register Low
#define boot_UART0_BDL              \
  boot_UART_BAUD_DIVIDER
/*   00000100 = reset
**   ||||||||
**   ++++++++-- SBRL            =1A : Baud rate modulo divider lower value
*/

//  UART0_C1        UART0 Control Register 1
#define boot_UART0_C1               \
  B8(00000000)
/*   00000000 = reset
**   ||||||||
**   |||||||+-- PT              =0 : Even parity
**   ||||||+--- PE              =0 : No hardware parity generation or checking
**   |||||+---- ILT             =0 : Idle character bit count starts after start bit
**   ||||+----- WAKE            =0 : Idle-line wakeup
**   |||+------ M               =0 : 8-bit data
**   ||+------- RSRC            =0 : Loop mode is internal (if enabled)
**   |+-------- DOZEEN          =0 : UART is enabled in Wait mode
**   +--------- LOOPS           =0 : Normal operation - loop mode is not selected
*/

//  UART0_C2        UART0 Control Register 2
#define boot_UART0_C2               \
  B8(00001100)
/*   00000000 = reset
**   ||||||||
**   |||||||+-- SBK             =0 : Normal operation - transmitter does not send a break character
**   ||||||+--- RWU             =0 : Normal operation - receiver is not in standby for wakeup condition
**   |||||+---- RE              =1 : Receiver is Enabled
**   ||||+----- TE              =1 : Transmitter is enabled
**   |||+------ ILIE            =0 : Hardware interrupts from IDLE flag are disabled
**   ||+------- RIE             =0 : Hardware interrupts from RDRF flag are disabled
**   |+-------- TCIE            =0 : Hardware interrupts from TC flag are disabled
**   +--------- TIE             =0 : Hardware interrupts from TDRE flag are disabled
*/
//
#define boot_UART0_TX_ON	0x08
#define boot_UART0_TX_OFF	0x00  		
#define boot_UART0_RX_ON	0x04
#define boot_UART0_RX_OFF  	0x00

//  UART0_S1        UART0 Status Register 1
#define boot_UART0_S1               \
  B8(00011111)
/*   11000000 = reset
**   ||||||||
**   |||||||+-- PF                 : Parity error flag (w1c)
**   ||||||+--- FE                 : Framing error flag (w1c)
**   |||||+---- NF                 : Noise flag (w1c)
**   ||||+----- OR                 : Receiver overrun flag (w1c)
**   |||+------ IDLE               : Idle line flag (w1c)
**   ||+------- RDRF               : Receive data register full flag
**   |+-------- TC                 : Transmission complete flag
**   +--------- TDRE               : Transmit data register empty flag
*/

//  UART0_S2        UART0 Status Register 2
#define boot_UART0_S2               \
  B8(11000000)
/*   00000000 = reset
**   ||||||||
**   |||||||+-- RAF                : Receiver active flag
**   ||||||+--- LBKDE           =0 : Break character is detected at length 10-13 bit times
**   |||||+---- BRK13           =0 : Break character is transmitted with length of 10-13 bit times
**   ||||+----- RWUID           =0 : If RWU = 1, the IDLE flag is not affected upon detection of an idle character
**   |||+------ RXINV           =0 : Receive data is not inverted
**   ||+------- MSBF            =0 : LSB (bit0) is the first bit that is transmitted following the start bit
**   |+-------- RXEDGIF            : UART _RX pin active edge interrupt flag (w1c)
**   +--------- LBKDIF             : LIN break detect interrupt flag (w1c)
*/

//  UART0_C3        UART0 Control Register 3
#define boot_UART0_C3               \
  B8(00000000)
/*   00000000 = reset
**   ||||||||
**   |||||||+-- PEIE            =0 : Parity error interrupts are disabled
**   ||||||+--- FEIE            =0 : Framing error interrupts are disabled
**   |||||+---- NEIE            =0 : Noise error interrupts are disabled
**   ||||+----- ORIE            =0 : Overrun interrupts are disabled
**   |||+------ TXINV           =0 : Transmit data is not inverted
**   ||+------- TXDIR           =0 : UART _TXD pin is an input in single-wire mode
**   |+-------- R9T8               : Receive Bit 9 / Transmit Bit 8
**   +--------- R8T9               : Receive Bit 8 / Transmit Bit 9
*/

//  UART0_C4        UART0 Control Register 4
#define boot_UART0_C4               \
  B8(00001111)
/*   00001111 = reset
**   ||||||||
**   |||+++++-- OSR             =F : Over sampling ratio is 16
**   ||+------- M10             =0 : Receiver and transmitter use 8-bit or 9-bit data characters
**   |+-------- MAEN2           =0 : Match address register 2 is not used
**   +--------- MAEN1           =0 : Match address register 1 is not used
*/

//  UART0_C5        UART0 Control Register 5
#define boot_UART0_C5               \
  B8(00000000)
/*   00000000 = reset
**   |x|xxx||
**   | |   |+-- RESYNCDIS       =0 : Resynchronization during received data word is supported
**   | |   +--- BOTHEDGE        =0 : Receiver samples input data using the rising edge of the baud rate clock
**   | +------- RDMAE           =0 : Receiver full (RDRF) DMA requests are disabled
**   +--------- TDMAE           =0 : Transmitter empty (TDRE) DMA requests are disabled
*/

/*********************************************************************************************************************\
* Public memory declarations
\*********************************************************************************************************************/

/*********************************************************************************************************************\
* Public prototypes
\*********************************************************************************************************************/

void Boot_Entry (void);	//2/18/16 delete--2/22/16 add back
void Boot_Reset (void);

uint8_t boot_Putc (register uint8_t data);

#endif /* BOOT_H_ */
