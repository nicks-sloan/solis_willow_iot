/*********************************************************************************************************************\
* BootProjectKL16
*
* File:   boot.c
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



#include "hardware.h"                       // include MCU declarations
#include "boot.h"                           // include bootloader hardware declarations
#include "UART.h"

__attribute__ ((section (".NonVolatileData"))) uint8_t RunApp;	//11/9/14 change
__attribute__ ((section (".NonVolatileData"))) typ_BootFlag BootFlag;	//11/9/14 change
//
//
/*********************************************************************************************************************\
* Private type definitions
\*********************************************************************************************************************/

/*********************************************************************************************************************
**  Bootloader operational states
*/
typedef enum
{
  StateStart = 0,
  StateHello,
  StateHelloDone,
  StateNewPrompt,
  StatePrompt,
  StateGetCommand,
  StatePrintString,
  StateEraseFlash,
  StateFlashErasing,
  StateVerifyErase,
  StateEraseStatus,
  StateWaitForSrecord,
  StateGetSrecord,
  StateProgramSrecord,
  StateProgramNext,
  StateProgramDone,
  StateBadHexCharacter,
  StateLast,
} typ_StateID;

typedef enum
{
  PhaseWaitForS = 0,
  PhaseWaitForType,
  PhaseGetCountMSB,
  PhaseGetCountLSB,
  PhaseGetAddressMSB,
  PhaseGetAddressLSB,
  PhaseGetDataMSB,
  PhaseGetDataLSB,
  PhaseGetChecksumMSB,
  PhaseGetChecksumLSB,
  PhaseLast
} typ_PhaseID;


/*********************************************************************************************************************
**  Special SIM register structure to improve bootloader code efficiency.
**
**  SIM - Peripheral register structure
*/
typedef struct _BOOT_SIM_MemMap {
  uint32_t SOPT2;                           /**< System Options Register 2, offset: 0x00 */
  uint8_t RESERVED_1[4];
  uint32_t SOPT4;                           /**< System Options Register 4, offset: 0x08 */
  uint32_t SOPT5;                           /**< System Options Register 5, offset: 0x0C */
  uint8_t RESERVED_2[4];
  uint32_t SOPT7;                           /**< System Options Register 7, offset: 0x14 */
  uint8_t RESERVED_3[8];
  uint32_t SDID;                            /**< System Device Identification Register, offset: 0x20 */
  uint8_t RESERVED_4[12];
  uint32_t SCGC4;                           /**< System Clock Gating Control Register 4, offset: 0x30 */
  uint32_t SCGC5;                           /**< System Clock Gating Control Register 5, offset: 0x34 */
  uint32_t SCGC6;                           /**< System Clock Gating Control Register 6, offset: 0x38 */
  uint32_t SCGC7;                           /**< System Clock Gating Control Register 7, offset: 0x3C */
  uint32_t CLKDIV1;                         /**< System Clock Divider Register 1, offset: 0x40 */
  uint8_t RESERVED_5[4];
  uint32_t FCFG1;                           /**< Flash Configuration Register 1, offset: 0x48 */
  uint32_t FCFG2;                           /**< Flash Configuration Register 2, offset: 0x4C */
  uint8_t RESERVED_6[4];
  uint32_t UIDMH;                           /**< Unique Identification Register Mid-High, offset: 0x54 */
  uint32_t UIDML;                           /**< Unique Identification Register Mid Low, offset: 0x58 */
  uint32_t UIDL;                            /**< Unique Identification Register Low, offset: 0x5C */
  uint8_t RESERVED_7[156];
  uint32_t COPC;                            /**< COP Control Register, offset: 0xFC */
  uint32_t SRVCOP;                          /**< Service COP Register, offset: 0x100 */
} volatile *BOOT_SIM_MemMapPtr;
/*
**  Peripheral SIM base pointer
*/
#define BOOT_SIM_BASE_PTR                   ((BOOT_SIM_MemMapPtr)0x40048004u)


/*********************************************************************************************************************
**  Local copy of the register descriptions for the Kinetis L Flash Memory Module (FTFA).
**  Slightly modified to provide 32-bit accesses for improved code efficiency.
*/
typedef struct E2emu_FTFA_MemMap {
  uint8_t FSTAT;                                // offset: 0x00 = Flash Status Register
  uint8_t FCNFG;                                // offset: 0x01 = Flash Configuration Register
  uint8_t FSEC;                                 // offset: 0x02 = Flash Security Register
  uint8_t FOPT;                                 // offset: 0x03 = Flash Option Register

  union
  {
    uint32_t FCCOB0_3;                          // offset: 0x04 = Flash Common Command Object Registers 0-3 (32-bit)
    struct
    {
      uint8_t FCCOB3;                           // offset: 0x04 = Flash Common Command Object Register 3
      uint8_t FCCOB2;                           // offset: 0x05 = Flash Common Command Object Register 2
      uint8_t FCCOB1;                           // offset: 0x06 = Flash Common Command Object Register 1
      uint8_t FCCOB0;                           // offset: 0x07 = Flash Common Command Object Register 0
    } BYTE;
  }FCC_dword0_3;

  union
  {
    uint32_t FCCOB4_7;                          // offset: 0x08 = Flash Common Command Object Registers 4-7 (32-bit)
    struct
    {
      uint8_t FCCOB7;                           // offset: 0x08 = Flash Common Command Object Register 7
      uint8_t FCCOB6;                           // offset: 0x09 = Flash Common Command Object Register 6
      uint8_t FCCOB5;                           // offset: 0x0A = Flash Common Command Object Register 5
      uint8_t FCCOB4;                           // offset: 0x0B = Flash Common Command Object Register 4
    } BYTE;
  }FCC_dword4_7;

  union
  {
    uint32_t FCCOB8_B;                          // offset: 0x0C = Flash Common Command Object Registers 8-B (32-bit)
    struct
    {
      uint8_t FCCOBB;                           // offset: 0x0C = Flash Common Command Object Register B
      uint8_t FCCOBA;                           // offset: 0x0D = Flash Common Command Object Register A
      uint8_t FCCOB9;                           // offset: 0x0E = Flash Common Command Object Register 9
      uint8_t FCCOB8;                           // offset: 0x0F = Flash Common Command Object Register 8
    } BYTE;
  }FCC_dword8_B;

  uint8_t FPROT3;                               /**< Program Flash Protection Registers, offset: 0x10 */
  uint8_t FPROT2;                               /**< Program Flash Protection Registers, offset: 0x11 */
  uint8_t FPROT1;                               /**< Program Flash Protection Registers, offset: 0x12 */
  uint8_t FPROT0;                               /**< Program Flash Protection Registers, offset: 0x13 */
} volatile *BOOT_FTFA_MemMapPtr;
/*
**  Peripheral FTFA base pointer
*/
#define BOOT_FTFA_BASE_PTR                  ((BOOT_FTFA_MemMapPtr)0x40020000u)

/*********************************************************************************************************************
**  Data type definition used to ease manipulation of a 32-bit pointer.
*/
typedef union
{
  void (*func)(void);                           // use as a pointer to a function
  uint32_t *data;                               // use as a pointer to 32-bit data
  uint32_t value;                               // access the pointer as a 32-bit variable
} typ_srcPtr;


/*********************************************************************************************************************\
* Private macros
\*********************************************************************************************************************/

#define ASCII_NULL      0x00
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_ESC       0x1B
#define ASCII_DEL       0x7F

#define BOOT_SIZE_OF_RAM_CODE               (9)

#define BOOT_CMD_FLASH_ERASE                (0x09)
#define BOOT_CMD_FLASH_PROGRAM              (0x06)

#define BOOT_APP_VECTOR_START               (0x00002000)	//2/22/16 change
#define BOOT_TOTAL_FLASH_SECTOR_COUNT       (128)			//2/18/16 change
#define BOOT_FLASH_SECTOR_COUNT             (8)				//2/19/16 change
#define BOOT_APP_FLASH_SECTOR_COUNT         (BOOT_TOTAL_FLASH_SECTOR_COUNT - BOOT_FLASH_SECTOR_COUNT)

//10/23/14 add for update software without change parameters 
#define BOOT_APP_CODE_START              	(0x00002000)	//3/15/16 change
#define BOOT_FLASH_SECTOR_COUNT1            (9)				//2/22/16 change
#define BOOT_APP_FLASH_SECTOR_COUNT1		(BOOT_TOTAL_FLASH_SECTOR_COUNT - BOOT_FLASH_SECTOR_COUNT1)

#define BOOT_BUFFER_SIZE                    (64)

#define BOOT_ERROR_BAD_HEX                  (0xFF)

/*********************************************************************************************************************\
* Private prototypes
\*********************************************************************************************************************/

uint8_t boot_Putc (register uint8_t data);
uint8_t boot_PutCRLF (void);
uint8_t boot_Getc (void);
uint8_t boot_Gethex (void);
uint8_t boot_isHex (register uint8_t data);
uint8_t boot_ASCIItoHex (register uint8_t data);
uint8_t boot_ASCIItoUpper (register uint8_t data);

static void boot_RAMcode (void);
void Boot_Command (void);


/*********************************************************************************************************************\
* Private memory declarations
\*********************************************************************************************************************/

__attribute__ ((section (".bootstring"))) const uint8_t str_Hello [] = "\r\nBootloader v2.1\r\n";
__attribute__ ((section (".bootstring"))) const uint8_t str_Prompt []  = "\r\nboot> ";
__attribute__ ((section (".bootstring"))) const uint8_t str_What [] = " <- what?";
__attribute__ ((section (".bootstring"))) const uint8_t str_List [] =
    " Command List:\r\n P = Program\r\n U = Update Program\r\n G = Go Run\r\n";	//10/24/14 change--4/12/16 change back
__attribute__ ((section (".bootstring"))) const uint8_t str_Erasing [] = "\r\nErase...";	//11/25/15 change
__attribute__ ((section (".bootstring"))) const uint8_t str_Erasing1 [] = "\r\nCode Erase...";	//11/25/15 change
__attribute__ ((section (".bootstring"))) const uint8_t str_Verified [] = "Verified";
__attribute__ ((section (".bootstring"))) const uint8_t str_Failed [] = "Failed\r\n";
__attribute__ ((section (".bootstring"))) const uint8_t str_SendSrec [] = "\r\nSend S-Record...\r\n";
__attribute__ ((section (".bootstring"))) const uint8_t str_Programming [] = "\r\nProgram...";	//11/25/15 change
__attribute__ ((section (".bootstring"))) const uint8_t str_Done [] = "\r\nDone\r\n";

static uint32_t boot_RAMroutine [BOOT_SIZE_OF_RAM_CODE] __attribute__ ((aligned(2)));
static void (*RAMfunction_ptr)(void);

/*********************************************************************************************************************\
* External declarations
\*********************************************************************************************************************/

extern uint32_t __SP_INIT[];                // initial Stack Pointer value from linker - used in vector table

extern void __init_hardware(void);          // Application program hardware initialization entry point


/*********************************************************************************************************************\
* Public memory declarations
\*********************************************************************************************************************/

extern uint32_t __APP_PC_INIT[];            // Application initial Program Counter location, from linker
extern uint32_t __APP_SP_INIT[];            // Application initial Stack Pointer location, from linker


/*********************************************************************************************************************\
* Public functions
\*********************************************************************************************************************/

/*********************************************************************************************************************
**  Power-On Reset Entry Point
**
**  Code execution begins here right out of reset and will jump directly to either __init_hardware() or Boot_Entry().
**  This routine does not have the normal function prologue or epilogue : __attribute__((naked)).
**  This routine is placed in the Startup Code section : __attribute__ ((section (".startcode"))).
*/
__attribute__ ((naked, section (".startcode"))) void Boot_Reset (void)
{
  register volatile uint32_t* reg_ptr32;    // generic register pointer
  /*
  **  Determine if a valid application program exists.
  */
  reg_ptr32 = __APP_PC_INIT;
  if (*reg_ptr32 != RESERVED_HANDLER)
  {
	if(RunApp == 0xAA)	//5/16/14 change
	{

		Boot_Entry();
	}
    /*
    **  Go execute the application program.
    **
    **  Point to the application programs vector table with the Vector Table Offset Register. 
    */
    SCB_VTOR = (uint32_t)(&__APP_SP_INIT);
    /*
    **  Initialize the Stack Pointer and Program Counter (which launches the application program).
    */
    int addr = (int)__APP_SP_INIT;

    __asm
    (
      "ldr  r0,[%0,#0]\n\t"
      "mov  sp,r0\n\t"
      "ldr  r0,[%0,#4]\n\t"
      "mov  pc,r0\n\t"
      ::"r"(addr)
    );
  }
  /*
  **  Otherwise, enter the Bootloader.
  */
  Boot_Entry();
}


/*********************************************************************************************************************
**  Bootloader Entry Point
**
**  This routine does not have the normal function prologue or epilogue : __attribute__((naked)).
**  This routine is compiled with optimization level 1 : __atribute__ ((optimze (1))).
**  This routine is placed in the Startup Code section : __attribute__ ((section (".startcode"))).
*/
__attribute__ ((naked, optimize (1), section (".startcode"))) void Boot_Entry (void)
{
  /*
  **  Create address pointers to speed up peripheral register accesses.
  */
  register volatile uint32_t* reg_ptr32;    // generic register pointer
  register BOOT_SIM_MemMapPtr sim_ptr;      // pointer to SIM register structure
  register MCG_MemMapPtr mcg_ptr;           // pointer to MCG register structure
 // register UART0_MemMapPtr uart_ptr;        // pointer to UART register structure, 3/25/16 change
  register PORT_MemMapPtr port_ptr;         // pointer to PORT register structure
  /*
  **  Create variable and pointers used to copy RAM executable routine.
  */
  register uint32_t count;                  // data count
  register uint32_t* dst_ptr;               // data destination pointer
  register typ_srcPtr src_ptr;              // data source pointer
  
  /*
  **  Assume that the COP watchdog is enabled and needs to be serviced.
  */
 
  reg_ptr32 = &SIM_SRVCOP;                  // point to the Service COP Register
  *reg_ptr32 = 0x55;                        // write 1st byte
  *reg_ptr32 = 0xAA;                        // write 2nd byte
  /*
  **  Disable all interrupts and initialize the Vector Table Offset Register.
  */
  __asm ("CPSID i");
  SCB_VTOR = (uint32_t)(&__boot_vect_table);
  /*
  **  Configure the bootloader clocking scheme.
  **  Use FEI mode with the CPU/Bus clocks at 48/24MHz.
  **  Access only the minimum registers required.
  **
  **  Set the system prescalers to a safe value.
  */
/* 
  sim_ptr = BOOT_SIM_BASE_PTR;                   // point to the SIM register structure
  sim_ptr->CLKDIV1 = boot_SIM_CLKDIV1_SAFE;
 */
  /*
  **  Enable clock gating for the bootloader UART and its assigned port.
  */
  /*
  sim_ptr->SCGC4 |= boot_SIM_SCGC4;
  sim_ptr->SCGC5 |= boot_SIM_SCGC5;
  */
  /*
  **	Enable clock gating for other port (PORTC). 5/6/14 add 
  */
  //sim_ptr->SCGC5 |= SIM_SCGC5_PORTC_MASK;	//2/22/16 delete
  /*
  **  Release the IO pads if waking up from VLLS mode.
  */
 /* 
  if ((PMC_REGSC & PMC_REGSC_ACKISO_MASK) != 0x0U)
  {
    PMC_REGSC |= PMC_REGSC_ACKISO_MASK;
  }
 */
  /*
  **  Set the system prescalers to their desired run values.
  */
  /*
  sim_ptr->CLKDIV1 = boot_SIM_CLKDIV1_RUN;
  */
  //4/24/14 change
  /*
  **  Configure the UART0 clock source.
  */
//  sim_ptr->SOPT2 |= boot_SIM_SOPT2;
  /*
  **  Configure the bootloader UART pins
  */
  //
  //port_ptr = PORTD_BASE_PTR;
  port_ptr = PORTA_BASE_PTR;	//2/22/16 change
  /*
  port_ptr->PCR[1] = boot_PORTA_PCR1;         // PTA1 = UART0 RX
  port_ptr->PCR[2] = boot_PORTA_PCR2;         // PTA2 = UART0 TX
  */
  //
  /*
  **  Trim the internal reference clock if necessary.
  */
 /* 
  mcg_ptr = MCG_BASE_PTR;                   // point to the MCG register structure
  if (NVM_SCTRIM != 0xFF)
  {
    mcg_ptr->C3 = NVM_SCTRIM;
    mcg_ptr->C4 = (mcg_ptr->C4 & 0xFE) | NVM_SCFTRIM;
  }
  */
  /*
  **  Configure the Multipurpose Clock Generator (MCG).
  */
 
  /*
  mcg_ptr->C1 = boot_MCG_C1;
  mcg_ptr->C2 = boot_MCG_C2;
  BME_BFI_B(&mcg_ptr->C4, MCG_C4_DRST_DRS_SHIFT, 3) = boot_MCG_C4;
  mcg_ptr->C5 &= boot_MCG_C5;
  mcg_ptr->C6 &= boot_MCG_C6;
 */
  /*
  **  Check that the source of the FLL reference clock is the internal reference clock.
  */
//  while ((mcg_ptr->S & MCG_S_IREFST_MASK) == 0) {}
  /*
  **  Wait until the output of the FLL is selected.
  */
//  while ((mcg_ptr->S & MCG_S_CLKST_MASK) != 0) {}
  /*
  **  the bootloader clocking scheme is now configured.
  **
  **  Clear all RAM, thereby setting all global and static variables to zero.
  */
  
  for (reg_ptr32 = (uint32_t*)MCU_RAM_FIRST_ADDRESS; reg_ptr32 < (uint32_t*)MCU_RAM_LAST_ADDRESS; reg_ptr32++)
  {
	  *reg_ptr32 = 0;	//11/25/15 change
  }
  /*
  **  Configure the bootloader UART.
  */
  //
 
/* 
  uart_ptr = UART0_BASE_PTR;                // point to the UART0 register structure
  uart_ptr->C2 = 0;                         // reset UART Control Register 2
  uart_ptr->MA1 = 0;	//2/22/16 add
  uart_ptr->MA2 = 0;	//2/22/16 add
  uart_ptr->BDH = boot_UART0_BDH;
  uart_ptr->BDL = boot_UART0_BDL;
  uart_ptr->C1 = boot_UART0_C1;
  uart_ptr->C3 = boot_UART0_C3;
  uart_ptr->C4 = boot_UART0_C4;
  uart_ptr->C5 = boot_UART0_C5;	//2/22/16 add
  uart_ptr->S1 = boot_UART0_S1;
  uart_ptr->S2 = boot_UART0_S2;
  uart_ptr->C2 = boot_UART0_C2;             // enable UART operations--2/25/16 change
*/ 
  //---------------------------
  
  /*
  **  Copy the RAM executable routine from Flash to RAM and configure a pointer to the routine.
  */
  src_ptr.func = &boot_RAMcode;             // point to the source code for the routine located in Flash
  dst_ptr = &boot_RAMroutine[0];            // point to the destination in RAM
  src_ptr.value -= 1;                       // compensate for ARM weirdness
  for (count = BOOT_SIZE_OF_RAM_CODE; count != 0; count--)
  {
    *dst_ptr++ = *src_ptr.data++;
  }
  RAMfunction_ptr = (void*)&boot_RAMroutine[0] + 1;
  /*  
  **  Jump into the Bootloader Command Interpreter.
  */
  Boot_Command();
}


/*********************************************************************************************************************
**  Bootloader Command Interpreter
**
**  This routine does not have the normal function prologue or epilogue : __attribute__((naked)).
**  This routine is compiled with optimization level 1 : __atribute__ ((optimze (1))).
**  This routine is placed in the Bootloader Code section : __attribute__ ((section (".bootcode"))).
*/
__attribute__ ((optimize (0), section (".bootcode"))) void Boot_Command (void)
{
  register volatile uint32_t* reg_ptr32;    // generic register pointer
  register uint8_t* ptr8;
  register typ_StateID state;
  register uint32_t* address_ptr;
  register uint32_t count;
  register uint8_t n;
  register uint8_t i;
  register uint8_t j;
  uint8_t command;
  uint8_t value;
  
  uint8_t type;
  uint8_t kount;
  uint8_t data;
  typ_srcPtr address;
  uint8_t checksum;
  uint8_t index;
  uint8_t buffer[BOOT_BUFFER_SIZE];
  
  typ_PhaseID phase;
  
  /*
  **  Start executing state machine control for the Bootloader.
  */
  RunApp = 0;	//3/24/16 add back
  //------------------------
  //11/25/15 change
  state = StateStart;
  phase = PhaseWaitForS;
  //------------------------
  i = 0;
  n = 0;
  j	= 0;
  for (;;)
  {
    /*
    **  Assume that the COP watchdog is enabled and needs to be serviced.
    */
    reg_ptr32 = &SIM_SRVCOP;                  // point to the Service COP Register
    *reg_ptr32 = 0x55;                        // write 1st byte
    *reg_ptr32 = 0xAA;                        // write 2nd byte
    /*
    **  Execute the current state.
    **  Cannot use "switch/case" as this causes a library call outside of the bootcode space.
    **
    *** State *************************************************************************************
    **
    **  Initialize bootloader UART interface.
    */
    if (state == StateStart)
    {
      /*
      **  Send out a string of asterisks.
      */
      if (n == 0)
      {
        if (boot_PutCRLF() == TRUE)
        {
          n = 40;
        }
      }
      else
      {
        if (boot_Putc('*') == TRUE)
        {
          if (--n == 0)
          {
            n = 40;
            ptr8 = (uint8_t*)&str_Hello[0];
            state = StateHello;
          }
        }
      }
    }
    /** State *************************************************************************************
    **
    **  Continue initialization text.
    **
    **  Send out bootloader "hello" message.
    */
    else if (state == StateHello)
    {
      if (boot_Putc(*ptr8) == TRUE)
      {
        ptr8++;
        if (*ptr8 == 0)
        {
          state = StateHelloDone;
        }
      }
    }
    /** State *************************************************************************************
    **
    **  Finish initial hello and display prompt.
    **
    **  Send out a string of asterisks.
    */
    else if (state == StateHelloDone)
    {
      if (n != 0)
      {
        if (boot_Putc('*') == TRUE)
        {
          n--;
        }
      }
      else
      {
        if (boot_PutCRLF() == TRUE)
        {
          state = StateNewPrompt;
        }
      }
    }
    /** State *************************************************************************************
    **
    **  Start displaying a new prompt.
    */
    else if (state == StateNewPrompt)
    {
      ptr8 = (uint8_t*)&str_Prompt[0];
      //
      if (boot_Putc(*ptr8) == TRUE)
      {
    	ptr8++;
        state = StatePrompt;
      }
    }
    /** State *************************************************************************************
    **
    **  Complete prompt display and then wait for user input.
    */
    else if (state == StatePrompt)
    {
      if (boot_Putc(*ptr8) == TRUE)
      {
    	ptr8++;
        if (*ptr8 == 0)
        {
        	state = StateGetCommand;
        }
      }
    }
    /** State *************************************************************************************
    **
    **  Interpret single character command input.
    **
    **  Do not use the following characters for commands: A, B, C, D, E, F, or S.
    */
    else if (state == StateGetCommand)
    {
    	//--------------------------------------------
    	//3/24/16 change
    
    	ResetUpdateOnly;
    	command = boot_Getc();
    	if(command != 0){
    		if(command >= ' '){
    	    	//boot_Putc(command);                     // echo back character
    	    	
    			while(!boot_Putc(command));                     // echo back character
    	    	command = boot_ASCIItoUpper(command);   // convert to uppercase

    	    	/** Command *********************************************************
    	    	**
    	    	**  ? = Help
    	    	*/
    	    	if(command == '?'){
    	    		ptr8 = (uint8_t*)&str_List[0];
    	    		state = StatePrintString;
    	    	}
    	    	/** Command *********************************************************
    	    	**
    	    	**  P = Program
    	    	*/
    	    	else if(command == 'P'){
    	    		i=0;
    	    		ptr8 = (uint8_t*)&str_Erasing[0];
    	    		state = StateEraseFlash;
    	    	}
    	    	else if(command == 'U'){	
    	    		i=0;
    	    		SetUpdateOnly;
    	    		ptr8 = (uint8_t*)&str_Erasing1[0];
    	    		state = StateEraseFlash;
    	    	}
    	    	/** Command *********************************************************
    	    	**
    	    	**  G = Go Run Application
    	    	*/
    	    	else if (command == 'G'){
    	    		Boot_Reset();
    	    	}
    	    	/** Command *********************************************************
    	    	**
    	    	**  Unknown command
    	    	*/
    	    	else{
    	    		ptr8 = (uint8_t*)&str_What[0];
    	    		state = StatePrintString;
    	    	}
    		}
    	}
    	//--------------------------------------------
    }
    /** State *************************************************************************************
    **
    **  Finish sending a string and then display a new prompt.
    */
    else if (state == StatePrintString)
    {
      if (boot_Putc(*ptr8) == TRUE)
      {
    	ptr8++;
        if (*ptr8 == 0)
        {
          state = StateNewPrompt;
        }
      }
    }
    /** State *************************************************************************************
    **
    **  Erase the application code Flash memory.
    **
    **  Point to the start of the application code and load the erase command.
    **
    **  The FCCOB register format for the erase command is:
    **    FCCOB0 = Erase Flash Sector command
    **    FCCOB1 = Flash address[23:16]
    **    FCCOB2 = Flash address[15:8]
    **    FCCOB3 = Flash address[7:0]
    **
    **    FCCOB0   FCCOB1   FCCOB2   FCCOB3
    **    xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx
    **    |||||||| |||||||| |||||||| ||||||||
    **    |||||||| ++++++++-++++++++-++++++++-- Flash address[23:0]
    **    ++++++++----------------------------- Flash command
    */
    else if (state == StateEraseFlash)
    {
    	//------------------------------------??????
    	//11/25/15 change
    	/*
    	**  Send out "Erasing..." message.
    	*/
    	if (boot_Putc(*ptr8) == TRUE)
    	{
    	    ptr8++;
    	    if (*ptr8 == 0)
    	    {
    	    	/*
    	    	**  Go erase the application Flash
    	    	*/
    	    	if(IsUpdateOnly){
    	    		BOOT_FTFA_BASE_PTR->FCC_dword0_3.FCCOB0_3 = BOOT_APP_CODE_START;
    	    	    n = BOOT_APP_FLASH_SECTOR_COUNT1;
    	    	}
    	    	else{
    	    	    BOOT_FTFA_BASE_PTR->FCC_dword0_3.FCCOB0_3 = BOOT_APP_VECTOR_START;
    	    	    n = BOOT_APP_FLASH_SECTOR_COUNT;
    	    	}
    	    	BOOT_FTFA_BASE_PTR->FCC_dword0_3.BYTE.FCCOB0 = BOOT_CMD_FLASH_ERASE;
    	    	state = StateFlashErasing;
    	    }
    	}
    	//------------------------------------
    }
    /** State *************************************************************************************
    **
    **  Erase the application code Flash memory.
    */
    else if (state == StateFlashErasing)
    {
      if (n != 0)
      {
        n--;
        /*
        **  Launch the Erase Flash Sector command via the RAM function.
        */
        (*RAMfunction_ptr)();
        /*
        **  Advance the Flash sector pointer by 1k (i.e., add 0x0400).
        */
        BOOT_FTFA_BASE_PTR->FCC_dword0_3.BYTE.FCCOB2 += 0x04;
        if (BOOT_FTFA_BASE_PTR->FCC_dword0_3.BYTE.FCCOB2 == 0)
        {
          BOOT_FTFA_BASE_PTR->FCC_dword0_3.BYTE.FCCOB1++;
        }
      }
      else
      {
        /*
        **  When erasing is complete, prepare to verify.
        */
        //---------------------
    	//10/24/14 change
    	if(IsUpdateOnly){
    		n = BOOT_APP_FLASH_SECTOR_COUNT1;
    		address_ptr = (uint32_t*)BOOT_APP_CODE_START;
    	}
    	else{
    		n = BOOT_APP_FLASH_SECTOR_COUNT;
    		address_ptr = (uint32_t*)BOOT_APP_VECTOR_START;
    	}
    	//---------------------
        value = TRUE;
        state = StateVerifyErase;
      }
    }
    /** State *************************************************************************************
    **
    **  Verify the erase procedure.
    */
    else if (state == StateVerifyErase)
    {
      if (n != 0)
      {
        /*
        **  Verify that a Flash sector has been erased.
        */
        n--;
        for (count = 256; count != 0; count--)
        {
          if (*address_ptr != 0xFFFFFFFF)
          {
            value = FALSE;
          }
        }
      }
      else
      {
        /*
        **  Report status of erase verification.
        */
        //---------------------??????
    	//11/25/15 change
    	/*
    	**  Report status of erase verification.
    	*/
    	if (value == FALSE)	
    	{
    		 ptr8 = (uint8_t*)&str_Failed[0];
    	}
    	else
    	{
    	     ptr8 = (uint8_t*)&str_Verified[0];
    	}
    	//---------------------
    	/*
    	**  Check if we're executing the Program command.
    	*/
    	if(command == 'P' || command == 'U')	//11/25/15 change
    	{
    		state = StateEraseStatus;
    	}
    	else
    	{
    		state = StatePrintString;	//11/25/15 change
    	}
    	//-------------------------
      }
    }
    /** State *************************************************************************************
    **
    **  Report the erase status.
    */
    else if (state == StateEraseStatus)
    {
    	//-----------------------------??????
    	//11/25/15 change
        if (boot_Putc(*ptr8) == TRUE)
        {
        	ptr8++;
        	if (*ptr8 == 0)
        	{
        	    /*
        	    **  Stop if the erase procedure failed.
        	    **  Otherwise, continue with the Program command execution.
        	    */
        	    if (value == FALSE)
        	    {
        	    	state = StateNewPrompt;
        	    }
        	    else
        	    {
        	    	ptr8 = (uint8_t*)&str_SendSrec[0];
        	    	state = StateWaitForSrecord;
        	    }
        	   }
        }
        //-----------------------------
    }
    /** State *************************************************************************************
    **
    **  Prepare to receive an S-Record.
    */
    else if (state == StateWaitForSrecord)
    {
    	//---------------------??????
    	//11/25/15 change
    	if (boot_Putc(*ptr8) == TRUE)
    	{
    	    ptr8++;
    	    if (*ptr8 == 0)
    	    {
    	    	for (n = (BOOT_BUFFER_SIZE); n != 0; n--)
    	    	{
    	    	    buffer[n - 1] = 0xFF;
    	    	}
    	    	//i = 32;
    	    	state = StateGetSrecord;
    	    	phase = PhaseWaitForS;
    	    }
    	}
    	//---------------------
    }
    /** State *************************************************************************************
    **
    **  Retrieve an S-Record.
    **
    **  Supported S-Record formats are:
    **    S1ccAAAAdd...ddss
    **    S2ccAAAAAAdd...ddss
    **    S804AAAAAAss
    **    S903AAAAss
    **
    **  where,
    **    cc     = count of bytes remaining in this record.
    **    AAAA   = 16-bit address (S1 record).
    **    AAAAAA = 24-bit address (S2 record).
    **    dd     = data byte
    **    ss     = LSB of the one's compliment of the sum of bytes of the count, address and data.
    **
    **  All other S-Record formats are simply ignored.
    */
    else if (state == StateGetSrecord)
    {
      /** Phase ***************************************************************
      **
      **  Wait for the start of a record as indicated by an 'S' character.
      */
      if (phase == PhaseWaitForS)
      {
        if (boot_Getc() == 'S')
        {
        	/*
        	if(i == 0){
        	   if (boot_PutCRLF() == TRUE){
        		   i = 32;     
        	   }
        	}
        	else if(boot_Putc('>')){
        		i--;	
        	}
        	*/
          phase = PhaseWaitForType;
        }
      }
      /** Phase ***************************************************************
      **
      **  Wait for the S-Record type.
      */
      else if (phase == PhaseWaitForType)
      {
        n = boot_Getc();
        if (n != 0)
        {
          if (n == '1')
          {
            type = 1;
            phase = PhaseGetCountMSB;
          }
          else if (n == '2')
          {
            type = 2;
            phase = PhaseGetCountMSB;
          }
          else if (n == '3')	//11/6/14 add
          {
              type = 3;
              phase = PhaseGetCountMSB;
          }
          else if (n == '7')	//11/6/14 add
          {
              kount = 13;
              state = StateProgramDone;
          }
          else if (n == '8')
          {
            kount = 10;
            state = StateProgramDone;
          }
          else if (n == '9')
          {
            kount = 8;
            state = StateProgramDone;
          }
          else
          {
            phase = PhaseWaitForS;
          }
        }
      }
      /** Phase ***************************************************************
      **
      **  Get the data count for this S-Record.
      */
      else if (phase == PhaseGetCountMSB)
      {
        n = boot_Gethex();
        if (n == BOOT_ERROR_BAD_HEX)
        {
          data = '?';
          state = StateBadHexCharacter;
        }
        else if (n != NULL)
        {
          /*
          **  Get the data count MSB.
          */
          kount = (boot_ASCIItoHex(n) << 4);
          if (kount == BOOT_ERROR_BAD_HEX)
          {
            //  bad hex character
          }
          phase = PhaseGetCountLSB;
        }
      }
      else if (phase == PhaseGetCountLSB)
      {
        n = boot_Gethex();
        if (n == BOOT_ERROR_BAD_HEX)
        {
          data = '?';
          state = StateBadHexCharacter;
        }
        else if (n != NULL)
        {
          /*
          **  Get the data count LSB.
          **  Initialize the checksum and the target address.
          */
          kount += boot_ASCIItoHex(n);
          checksum = kount;
          kount--;
          address.value = 0;
          phase = PhaseGetAddressMSB;
        }
      }
      /** Phase ***************************************************************
      **
      **  Get the target address for this S-Record.
      */
      else if (phase == PhaseGetAddressMSB)
      {
        n = boot_Gethex();
        if (n == BOOT_ERROR_BAD_HEX)
        {
          data = '?';
          state = StateBadHexCharacter;
        }
        else if (n != NULL)
        {
          /*
          **  Get the address MSB.
          */
          data = (boot_ASCIItoHex(n) << 4);
          phase = PhaseGetAddressLSB;
        }
      }
      else if (phase == PhaseGetAddressLSB)
      {
        n = boot_Gethex();
        if (n == BOOT_ERROR_BAD_HEX)
        {
          data = '?';
          state = StateBadHexCharacter;
        }
        else if (n != NULL)
        {
          /*
          **  Get the address LSB.
          **  Add the address byte to the checksum.
          */
          data += boot_ASCIItoHex(n);
          address.value <<= 8;
          address.value += data;
          checksum += data;
          kount--;
          /*
          **  If there are more address bytes then loop back.
          */
          if (type != 0)
          {
            type--;
            phase = PhaseGetAddressMSB;
          }
          /*
          **  If the address is complete then get ready for the data.
          */
          else
          {
            index = 0;
            phase = PhaseGetDataMSB;
          }
        }
      }
      /** Phase ***************************************************************
      **
      **  Get the S-Record data.
      */
      else if (phase == PhaseGetDataMSB)
      {
        /*
        **  Exit to the next phase when all of the data has been received.
        */
        if (kount == 0)
        {
          phase = PhaseGetChecksumMSB;
        }
        else
        {
          n = boot_Gethex();
          if (n == BOOT_ERROR_BAD_HEX)
          {
            data = '?';
            state = StateBadHexCharacter;
          }
          else if (n != NULL)
          {
            /*
            **  Get the data MSB.
            */
            data = (boot_ASCIItoHex(n) << 4);
            phase = PhaseGetDataLSB;
          }
        }
      }
      else if (phase == PhaseGetDataLSB)
      {
        n = boot_Gethex();
        if (n == BOOT_ERROR_BAD_HEX)
        {
          data = '?';
          state = StateBadHexCharacter;
        }
        else if (n != NULL)
        {
          /*
          **  Get the data LSB.
          **  Add the data byte to the checksum.
          **  Loop back and get the next data byte.
          */
          data += boot_ASCIItoHex(n);
          buffer[index++] = data;
          checksum += data;
          kount--;
          phase = PhaseGetDataMSB;
        }
      }
      /** Phase ***************************************************************
      **
      **  Get and verify the Checksum.
      */
      else if (phase == PhaseGetChecksumMSB)
      {
        n = boot_Gethex();
        if (n == BOOT_ERROR_BAD_HEX)
        {
          data = '?';
          state = StateBadHexCharacter;
        }
        else if (n != NULL)
        {
          /*
          **  Get the checksum MSB.
          */
          data = (boot_ASCIItoHex(n) << 4);
          phase = PhaseGetChecksumLSB;
        }
      }
      else if (phase == PhaseGetChecksumLSB)
      {
        n = boot_Gethex();
        if (n == BOOT_ERROR_BAD_HEX)
        {
          data = '?';
          state = StateBadHexCharacter;
        }
        else if (n != NULL)
        {
          /*
          **  Compare the calculated and transmitted checksums.
          */
          data += boot_ASCIItoHex(n);
          data = data - ~checksum;
          if (data == 0)
          {
            state = StateProgramSrecord;
          }
          else
          {
            /*
            **  Bad checksum, abort this record.
            */
            data = '!';
            state = StateBadHexCharacter;

          }
        }
      }
    }
    /** State *************************************************************************************
    **
    **  Program the application code Flash memory.
    **
    **  The target address must be in the valid application Flash space.
    **  The target address must be on a 32-bit word boundary.
    **
    **  The FCCOB register format for the erase command is:
    **    FCCOB0 = Program Longword command
    **    FCCOB1 = Flash address[23:16]
    **    FCCOB2 = Flash address[15:8]
    **    FCCOB3 = Flash address[7:0]
    **    FCCOB4 = Byte 0 data
    **    FCCOB5 = Byte 1 data
    **    FCCOB6 = Byte 2 data
    **    FCCOB7 = Byte 3 data
    **
    **    FCCOB0   FCCOB1   FCCOB2   FCCOB3   FCCOB4   FCCOB5   FCCOB6   FCCOB7
    **    xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx
    **    |||||||| |||||||| |||||||| |||||||| |||||||| |||||||| |||||||| ||||||||
    **    |||||||| |||||||| |||||||| |||||||| ++++++++-++++++++-++++++++-++++++++-- Flash data[31:0]
    **    |||||||| ++++++++-++++++++-++++++++-------------------------------------- Flash address[23:0]
    **    ++++++++----------------------------------------------------------------- Flash command
    */
    else if (state == StateProgramSrecord)
    {
      /*
      **  Check if the address is within the allowable range.
      */
      if (address.value < MCU_APPLICATION_VECTOR_TABLE_START)
      {
        // abort this record and go to next record, 5/8/14 add
    	  //i = 0;
    	  state = StateProgramNext;  
      }
      else if (address.value > MCU_APPLICATION_LAST_FLASH_ADDRESS)
      {
    	  // abort this record and go to wait a new prompt, 5/8/14 add
    	 // state = StateNewPrompt;
    	 // phase = PhaseWaitForS;
    	 // abort this record and go to next record, Hex file includes RAM area when define non bss sections
    	      //i = 0;
    	      state = StateProgramNext; 
      }
      else{	//3/25/16 change
    	  if(IsUpdateOnly){
    		  if(address.value > MCU_APPLICATION_CODE_LAST_ADDRESS){
    			  // abort this record and go to wait a new prompt
    			  state = StateNewPrompt;
    			  phase = PhaseWaitForS;
    		  }
    		  else{
    			  BOOT_FTFA_BASE_PTR->FCC_dword0_3.FCCOB0_3 = address.value;
    			  BOOT_FTFA_BASE_PTR->FCC_dword0_3.BYTE.FCCOB0 = BOOT_CMD_FLASH_PROGRAM;
    			  BOOT_FTFA_BASE_PTR->FCC_dword4_7.BYTE.FCCOB7 = buffer[kount++];
    			  BOOT_FTFA_BASE_PTR->FCC_dword4_7.BYTE.FCCOB6 = buffer[kount++];
    			  BOOT_FTFA_BASE_PTR->FCC_dword4_7.BYTE.FCCOB5 = buffer[kount++];
    			  BOOT_FTFA_BASE_PTR->FCC_dword4_7.BYTE.FCCOB4 = buffer[kount++];
    			  /*
    			  **  Launch the Program Longword command via the RAM function.
    			  */
    			  (*RAMfunction_ptr)();
    			  /*
    			  **  Advance the address pointer.
    			  */
    			  address.value += 4;
    			  for (n = 4; n != 0; n--)
    			  {
    				  index--;
    			      if (index == 0)
    			      {
    			    	  /*
    			    	  boot_Putc('*');	//4/29/14 delete, enable for two wires--4/27/16 add back
    			    	  i--;
    			    	  if (i == 0)
    			    	  {
    			    		  i = 32;
    			    	  }
    			    	  */
    			    	  state = StateProgramNext;
    			      }
    			  }
    		  }
    	  }
    	  else{
    		  BOOT_FTFA_BASE_PTR->FCC_dword0_3.FCCOB0_3 = address.value;
    		  BOOT_FTFA_BASE_PTR->FCC_dword0_3.BYTE.FCCOB0 = BOOT_CMD_FLASH_PROGRAM;
    		  BOOT_FTFA_BASE_PTR->FCC_dword4_7.BYTE.FCCOB7 = buffer[kount++];
    		  BOOT_FTFA_BASE_PTR->FCC_dword4_7.BYTE.FCCOB6 = buffer[kount++];
    		  BOOT_FTFA_BASE_PTR->FCC_dword4_7.BYTE.FCCOB5 = buffer[kount++];
    		  BOOT_FTFA_BASE_PTR->FCC_dword4_7.BYTE.FCCOB4 = buffer[kount++];
    		  /*
    		   **  Launch the Program Longword command via the RAM function.
    		   */
    		  (*RAMfunction_ptr)();
    		  /*
    		   **  Advance the address pointer.
    		   */
    		  address.value += 4;
    		  for (n = 4; n != 0; n--)
    		  {
    			  index--;
    			  if (index == 0)
    			  {
    				  /*
    				  boot_Putc('*');	//4/29/14 delete, enable for two wires
    				  i--;
    				  if (i == 0)
    				  {
    					  i = 32;
    				  }
    				  */
    				  state = StateProgramNext;
    			  }
    		  }
    	  }  
      }
    }
    /** State *************************************************************************************
    **
    **  Prepare to retrieve the next S-Record.
    */
    else if (state == StateProgramNext)
    {
    	/*
      if (i == 32)
      {
        if (boot_PutCRLF() == TRUE)	//for two wires
        {
          state = StateGetSrecord;
          phase = PhaseWaitForS;
        }
      }
      else
      {
        state = StateGetSrecord;
        phase = PhaseWaitForS;
      }
      */
    	
    	if(i == 0){
    	    if (boot_PutCRLF() == TRUE){
    	    	i = 32;     
    	    }
    	}
    	else if( j == 0){
    		if(boot_Putc('>') == TRUE){
    			i--;
    			j=100;
    		}
    	}
    	else{
    			j--;
    		}
    	state = StateGetSrecord;
    	phase = PhaseWaitForS;
    	
    }
    /** State *************************************************************************************
    **
    **  Finished programming.
    */
    else if (state == StateProgramDone)
    {
      n = boot_Getc();
      if (n != 0)
      {
        kount--;
      }
      if (kount == 0)
      {
    	
    	  //---------------------------
    	  //11/25/15 change
    	  //ptr8 = (uint8_t*)&str_Done[0];	
    	 // state = StatePrintString;
    	 // phase = PhaseWaitForS;
    	  //---------------------------
    	  ptr8 = (uint8_t*)&str_Done[0];
    	  while(*ptr8){
    		  if (boot_Putc(*ptr8) == TRUE)
    		  {
    			  ptr8++;
    	      	   
    	      	}
    	  }
    	  
    	  Boot_Reset(); // out of boot loader
    	  
      }
    }
    /** State *************************************************************************************
    **
    **  Received a non-hexadecimal character when one was received.
    */
    else if (state == StateBadHexCharacter)
    {
      n = boot_Getc();
      if (n == ASCII_CR)
      {
    	//boot_Putc(data);	//4/5/16 change
    	  /*
        i--;
        if (i == 0)
        {
          i = 32;
        }
        */
        state = StateProgramNext;
      }
    }
  }
}


/*********************************************************************************************************************\
* Private functions
\*********************************************************************************************************************/

/*********************************************************************************************************************
**  Source code for RAM executable function.
**
**  This routine is compiled with optimization level 1 : __atribute__ ((optimze (1))).
**  This routine is placed in the Startup Code section : __attribute__ ((section (".startcode"))).
*/
__attribute__ ((optimize (0), section (".startcode"))) void boot_RAMcode (void)
{
  register volatile uint8_t* ptr;
  /*
  **  Launch the Flash command and wait for it to complete.
  */
  ptr = &FTFA_BASE_PTR->FSTAT;
  *ptr = 0x80;
  ptr += ((1 << 28) + (FTFA_FSTAT_CCIF_SHIFT << 23) + (1 << 19));
  while (*ptr == 0){};
}



/*********************************************************************************************************************
**  Output a character through the UART
**
**  This routine is compiled with optimization level 1 : __atribute__ ((optimze (1))).
**  This routine is placed in the Startup Code section : __attribute__ ((section (".startcode"))).
*/
__attribute__ ((optimize (1), section (".startcode"))) uint8_t boot_Putc (register uint8_t data)
{
  if ((UART0_S1 & UART0_S1_TDRE_MASK) != 0){	//3/25/16 change
	  UART0_D = data;
	  return (TRUE);
  }
  return (FALSE);
}


/*********************************************************************************************************************
**  Output an ASCII CR/LF sequence through the UART
**
**  This routine is compiled with optimization level 1 : __atribute__ ((optimze (1))).
**  This routine is placed in the Startup Code section : __attribute__ ((section (".startcode"))).
*/
__attribute__ ((optimize (1), section (".startcode"))) uint8_t boot_PutCRLF (void)
{
  static uint8_t x;
  
  if (x == 0)
  {
    if (boot_Putc(ASCII_CR) == TRUE)
    {
      x = 1;
    }
  }
  else
  {
    if (boot_Putc(ASCII_LF) == TRUE)
    {
      x = 0;
      return (TRUE);
    }
  }
  return (FALSE);
}

/*********************************************************************************************************************
**  Input a character through the UART
*/
 __attribute__ ((section (".startcode"))) uint8_t boot_Getc (void)
{
  if (BME_UBFX_B(&UART0_S1, UART0_S1_RDRF_SHIFT, 1) == 1){	//3/25/16 change
	 return (UART0_D);
  }
  return (NULL);
}


 /*********************************************************************************************************************
 **  Input a hexadecimal character through the UART
 */
__attribute__ ((section (".startcode"))) uint8_t boot_Gethex (void)
{
  uint8_t data;
  data = boot_Getc();
  if (data == NULL)
  {
    return (NULL);
  }
  else if ((data < '0') || ((data > '9') && (data < 'A')) || ((data > 'F') && (data < 'a')) || (data > 'f'))
  {
    return (BOOT_ERROR_BAD_HEX);
  }
  else
  {
  return (data);
  }
}

/*********************************************************************************************************************
**  Convert an ASCII hexadecimal value to its binary equivalent
**
**  Data passed forward must be an ASCII hexadecimal character:
**    - '0' to '9'
**    - 'A' to 'F'
**    - 'a' to 'f'
*/
 __attribute__ ((section (".startcode"))) uint8_t boot_ASCIItoHex (register uint8_t data)
{
  data -= '0';
  if (data > 0x09)
  {
    data -= 0x07;
  }
  if (data > 0x0F)
  {
    data -= 0x20;
  }
  return (data);
}


 /********************************************************************************************************************
 ** Convert an ASCII alpha lower case to upper case 
 */
 __attribute__ ((section (".startcode"))) uint8_t boot_ASCIItoUpper (register uint8_t data)
 {
   if (data >= 'a')
     data -= ('a' - 'A');
   return (data);
 }

 
/*********************************************************************************************************************\
* Flash Configuration Field
\*********************************************************************************************************************/

/*
**  This table is placed in the Flash Configuration Field section - __attribute__ ((section (".cfmconfig"))).
*/
__attribute__ ((section (".cfmconfig"))) const uint8_t _cfm[0x10] =
{
  //  Backdoor Comparison Key
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  //  Program Flash Protection
  init_FTFA_FPROT3, init_FTFA_FPROT2, init_FTFA_FPROT1, init_FTFA_FPROT0,
  //  Flash Security (FSEC)
  init_FTFA_FSEC,
  //  Flash Options (FOPT)
  init_FTFA_OPT,
  //  reserved / unused
  0xFF, 0xFF
};


/*********************************************************************************************************************
**  Default Interrupt Handlers
*/
void __attribute__ ((interrupt)) Boot_Default_Handler(void)
{
  __asm("bkpt");
}

/*
**  Weak definitions of handlers point to Default_Handler if not implemented
*/
void Boot_NMI_Handler()          __attribute__ ((weak, alias("Boot_Default_Handler")));
void Boot_HardFault_Handler()    __attribute__ ((weak, alias("Boot_Default_Handler")));
void Boot_SVC_Handler()          __attribute__ ((weak, alias("Boot_Default_Handler")));
void Boot_PendSV_Handler()       __attribute__ ((weak, alias("Boot_Default_Handler")));
void Boot_SysTick_Handler()      __attribute__ ((weak, alias("Boot_Default_Handler")));


/*********************************************************************************************************************\
* Bootloader Vector Table
\*********************************************************************************************************************/

__attribute__ ((section (".bootvectortable"))) const tVectorTable __boot_vect_table =
{
    (uint32_t *)__SP_INIT,                  // 0x00 0x00000000 -  Initial Stack Pointer
  {
    //        ISR name                         num    address     source
    (typ_ISRfunc)Boot_Reset,                // 0x01 0x00000004 -  Initial Program Counter
    (typ_ISRfunc)Boot_NMI_Handler,          // 0x02 0x00000008 -  NMI
    (typ_ISRfunc)Boot_HardFault_Handler,    // 0x03 0x0000000C -  Hard Fault
    (typ_ISRfunc)RESERVED_HANDLER,          // 0x04 0x00000010 -  reserved
    (typ_ISRfunc)RESERVED_HANDLER,          // 0x05 0x00000014 -  reserved
    (typ_ISRfunc)RESERVED_HANDLER,          // 0x06 0x00000018 -  reserved
    (typ_ISRfunc)RESERVED_HANDLER,          // 0x07 0x0000001C -  reserved
    (typ_ISRfunc)RESERVED_HANDLER,          // 0x08 0x00000020 -  reserved
    (typ_ISRfunc)RESERVED_HANDLER,          // 0x09 0x00000024 -  reserved
    (typ_ISRfunc)RESERVED_HANDLER,          // 0x0A 0x00000028 -  reserved
    (typ_ISRfunc)Boot_SVC_Handler,          // 0x0B 0x0000002C -  Superviser Call
    (typ_ISRfunc)RESERVED_HANDLER,          // 0x0C 0x00000030 -  reserved
    (typ_ISRfunc)RESERVED_HANDLER,          // 0x0D 0x00000034 -  reserved
    (typ_ISRfunc)Boot_PendSV_Handler,       // 0x0E 0x00000038 -  Pendable Service Request
    (typ_ISRfunc)Boot_SysTick_Handler,      // 0x0F 0x0000003C -  System Tick Timer
    (typ_ISRfunc)Boot_Default_Handler,      // 0x10 0x00000040 -  DMA channel 0
    (typ_ISRfunc)Boot_Default_Handler,      // 0x11 0x00000044 -  DMA channel 1
    (typ_ISRfunc)Boot_Default_Handler,      // 0x12 0x00000048 -  DMA channel 2
    (typ_ISRfunc)Boot_Default_Handler,      // 0x13 0x0000004C -  DMA channel 3
    (typ_ISRfunc)RESERVED_HANDLER,          // 0x14 0x00000050 -  reserved
    (typ_ISRfunc)Boot_Default_Handler,      // 0x15 0x00000054 -  Flash Memory Command Complete and Read Collision
    (typ_ISRfunc)Boot_Default_Handler,      // 0x16 0x00000058 -  Low-Voltage Detect, Low-Voltage Warning
    (typ_ISRfunc)Boot_Default_Handler,      // 0x17 0x0000005C -  Low Leakage Wakeup
    (typ_ISRfunc)Boot_Default_Handler,      // 0x18 0x00000060 -  I2C0
    (typ_ISRfunc)Boot_Default_Handler,      // 0x19 0x00000064 -  I2C1
    (typ_ISRfunc)Boot_Default_Handler,      // 0x1A 0x00000068 -  SPI0
    (typ_ISRfunc)Boot_Default_Handler,      // 0x1B 0x0000006C -  SPI1
    (typ_ISRfunc)Boot_Default_Handler,      // 0x1C 0x00000070 -  UART0
    (typ_ISRfunc)Boot_Default_Handler,      // 0x1D 0x00000074 -  UART1
    (typ_ISRfunc)Boot_Default_Handler,      // 0x1E 0x00000078 -  UART2
    (typ_ISRfunc)Boot_Default_Handler,      // 0x1F 0x0000007C -  ADC0
    (typ_ISRfunc)Boot_Default_Handler,      // 0x20 0x00000080 -  CMP0
    (typ_ISRfunc)Boot_Default_Handler,      // 0x21 0x00000084 -  TPM0
    (typ_ISRfunc)Boot_Default_Handler,      // 0x22 0x00000088 -  TPM1
    (typ_ISRfunc)Boot_Default_Handler,      // 0x23 0x0000008C -  TPM2
    (typ_ISRfunc)Boot_Default_Handler,      // 0x24 0x00000090 -  RTC Alarm
    (typ_ISRfunc)Boot_Default_Handler,      // 0x25 0x00000094 -  RTC Seconds
    (typ_ISRfunc)Boot_Default_Handler,      // 0x26 0x00000098 -  PIT
    (typ_ISRfunc)RESERVED_HANDLER,          // 0x27 0x0000009C -  reserved
    (typ_ISRfunc)Boot_Default_Handler,      // 0x28 0x000000A0 -  USB OTG
    (typ_ISRfunc)Boot_Default_Handler,      // 0x29 0x000000A4 -  DAC0
    (typ_ISRfunc)Boot_Default_Handler,      // 0x2A 0x000000A8 -  TSI0
    (typ_ISRfunc)Boot_Default_Handler,      // 0x2B 0x000000AC -  MCG
    (typ_ISRfunc)Boot_Default_Handler,      // 0x2C 0x000000B0 -  LPTMR0
    (typ_ISRfunc)RESERVED_HANDLER,          // 0x2D 0x000000B4 -  reserved
    (typ_ISRfunc)Boot_Default_Handler,      // 0x2E 0x000000B8 -  Port A
    (typ_ISRfunc)Boot_Default_Handler       // 0x2F 0x000000BC -  Port D
  }
};

