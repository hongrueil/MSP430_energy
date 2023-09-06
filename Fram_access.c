#include <msp430.h>
#include <stdint.h>
#include <stdio.h>

extern void shutdown_asm(void);
void set_freq(uint8_t clk);
void FRAM_Write(unsigned int StartAddress);
void initialize_port(void);

#pragma NOINIT(arr)
uint8_t arr[2049];


void main(void) {
  WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
  initialize_port();
  set_freq(1);
  int i = 0;
  int j = 0;
  uint8_t data = 0x1;
  uint8_t *arr_ptr = arr;
  // initialize arr
  for (i = 0; i < 2048; ++i) {
    *arr_ptr++ = data;
  }
  data = 0x2;
  // access 512 * 2048 = 1048576 byte
  for (i = 0; i < 512; ++i) {
    arr_ptr = arr;
    for (j = 0; j < 2048; ++j) {
      //*arr_ptr = data; //FRAM write
      //data = *arr_ptr; //FRAM read
      arr_ptr++;
    }
    *arr_ptr = data; // avoid compiler optimization
  }
  shutdown_asm();
}
// set system clk to 1, 8, 16, 24
void set_freq(uint8_t clk) {
  CSCTL0_H = CSKEY_H;                     // Unlock CS registers
  if(clk == 1) {
    CSCTL0_H = CSKEY_H;  
    CSCTL1 = DCORSEL;                                     //  set DCO to 1 MHZ
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;  // Set SMCLK = MCLK = DCO
                                                          // ACLK = VLOCLK
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;                 // Set all dividers to 1
    CSCTL0_H = 0; 
    __delay_cycles(60);

  } else if (clk == 8) {
     // Clock System Setup
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz
    // Set SMCLK = MCLK = DCO, ACLK = VLOCLK
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;

    // Per Device Errata set divider to 4 before changing frequency to
    // prevent out of spec operation from overshoot transient
    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4;   // Set all corresponding clk sources to divide by 4 for errata
    CSCTL1 = DCOFSEL_6;                     // Set DCO to 8MHz

    // Delay by ~10us to let DCO settle. 60 cycles = 20 cycles buffer + (10us / (1/4MHz))
    __delay_cycles(60);
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers to 1 for 8MHz operation
    CSCTL0_H = 0;                           // Lock CS Registers 

  } else if (clk == 16) {
    CSCTL0_H = CSKEY_H;  
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;
    
    CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz
    
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;        // Set SMCLK = MCLK = DCO, ACLK = VLOCLK

    // Per Device Errata set divider to 4 before changing frequency to
    // prevent out of spec operation from overshoot transient
    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4;                       // Set all corresponding clk sources to divide by 4 for errata
    CSCTL1 = DCOFSEL_4 | DCORSEL;                               // Set DCO to 16MHz

    // Delay by ~10us to let DCO settle. 60 cycles = 20 cycles buffer + (10us / (1/4MHz))
    __delay_cycles(60);
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers to 1 for 16MHz operation
    CSCTL0_H = 0;

  } else if (clk == 24) {
    CSCTL0_H = CSKEY_H;  

    FRCTL0 = FRCTLPW | NWAITS_2;
    CSCTL1 = DCOFSEL_0;   
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4;   // Set all corresponding clk sources to divide by 4 for errata
    CSCTL1 = DCOFSEL_6 | DCORSEL;      // Set DCO to 24MHz
    __delay_cycles(60);
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers to 1 for 16MHz operation
    CSCTL0_H = 0;
  }
}
// turn off unused port
void initialize_port(void) {
  P1DIR = 0xFF;
  P1OUT = 0x00;

  P2DIR = 0xFF;
  P2OUT = 0x00;

  P3DIR = 0xFF;
  P3OUT = 0x00;

  P4DIR = 0xFF;
  P4OUT = 0x00;

  P5DIR = 0xFF;
  P5OUT = 0x00;

  P6DIR = 0xFF;
  P6OUT = 0x00;

  P7DIR = 0xFF;
  P7OUT = 0x00;

  P8DIR = 0xFF;
  P8OUT = 0x00;

  P9DIR = 0xFF;
  P9OUT = 0x00;

  PADIR = 0xFF;
  PAOUT = 0x00;

  PBDIR = 0xFF;
  PBOUT = 0x00;

  PCDIR = 0xFF;
  PCOUT = 0x00;

  PDDIR = 0xFF;
  PDOUT = 0x00;

  PEDIR = 0xFF;
  PEOUT = 0x00;

  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;
}
/**********************************************************************//**
 * @brief  Performs 512 byte FRAM Write
 *
 * @param  StartAddress: For FRAM Write
 *
 * @return none
 *************************************************************************/
/*
void FRAM_Write(unsigned int StartAddress) {
  unsigned long *FRAM_write_ptr;
  unsigned long data = 0x12345678; //4 byte = 32 bits
  unsigned int i=0;
  FRAM_write_ptr = (unsigned long *) StartAddress;
  for (i = 0; i < 128; ++i) {
    *FRAM_write_ptr++ = data;
  }

}
*/
