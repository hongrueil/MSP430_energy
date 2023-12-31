#include <msp430.h>
#include <stdint.h>
#include <stdio.h>

extern void shutdown_asm(void);

void set_freq(uint8_t clk);
void set_timer();
__attribute__((ramfunc)) void access(uint8_t *StartAddress);
void initialize_port(void);


#pragma NOINIT(arr)
uint8_t arr[2049];

unsigned long long int cnt = 0;
void main(void) {
  WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
  initialize_port();
  set_freq(1);
  access(arr);
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
void set_timer() {
    TA0CCTL0 = CCIE;                        // TACCR0 interrupt enabled
    TA0CCR0 = 1000 -1;                      //Total count = TACCR0 + 1, when 1MHZ --> 1000ticks = 1ms
    TA0CTL = TASSEL__SMCLK | MC__UP;        // SMCLK, continuous mode
    _enable_interrupt();
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
 * @brief  Performs 512 time * 2048 byte access
 *
 * @param  StartAddress: For access address
 *
 * @return none
 *************************************************************************/

void access(uint8_t *StartAddress) {
  uint8_t *FRAM_ptr;
  uint8_t data = 0x12; //1 byte = 8 bits
  int i = 0;
  int j = 0;
  int start, end;
  FRAM_ptr =  StartAddress;
  // initialize arr
  for (j = 0; j < 2048; ++j) {
    *FRAM_ptr++ = data;
  }
  // access 512 * 2048 byte
  set_timer();
  start = TA0R;
  cnt = 0;
  for (i = 512; i > 0; --i) {
    FRAM_ptr =  StartAddress;
    for (j = 0; j < 2048; ++j) {
      //__asm("\t   MOVA\t   0x0004(SP),R15"); // for emulate
      *FRAM_ptr = data; // write ->  MOV.B   0x0008(SP),0x0000(R15)
      //data = *FRAM_ptr; // read
      FRAM_ptr++;
    }
  }
  end = TA0R;
  printf("start = %x: \n", start);
  printf("start = %lu: \n", start);
  printf("end = %x: \n", end);
  printf("end = %lu: \n", end);
  printf("cnt = %lu: \n",cnt);

}
// Timer ISR
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  cnt++;
}
