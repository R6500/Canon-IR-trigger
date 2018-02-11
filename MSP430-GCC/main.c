
/***************/
/* IR TRigger  */
/***************/

// RC-1 Equivalent trigger for Canon EOS
// Frequency: 32700 Hz (29800 to 35500 Hz)
// Pulses/Burst: 16 (9 to 22)
// Two burst start delay: Immediate 7.82ms
//                        Delayed (2s) 5.86ms

// History
//   25/11/2014 First version (v1.0)

// Use low power LPM4 mode
// Energia first firmware had 3.2mA idle current
// This firmware has a 0.1uA idle current
// This is just the typical value t 25ºC reported on the datasheet

// TODO : The first burst is not always correct
//        Perhaps we can go to LPM3 and then to LPM4 after several seconds

// Use IR LED TSHF5210

#include <msp430.h>
#include "io430masks.h"

// Main configuration definitions

//#define RELEASE    // Release version (Don't include test functions)
                     // Comment it during product debugging

// Hardware definitions

// PORT 1 USAGE

#define LED BIT0 // Active High Red LED at pin 2 (P1.0)
#define SW1 BIT3 // Button between pin 5 (P1.3) and GND
#define SW2 BIT5 // Button between pin 7 (P1.5) and GND

#define IRa BIT4 // Active High IR LED at pin 6 (P1.4)
                 // P1.4 can be used as SMCLK output

//#define RD_H BIT7 // Upper end of resistor divider at pin 15 (P1.7)
//#define RD_M BIT6 // Middle point of resistor divider at pin 14 (P1.6) (A6)
#define RD_M BIT7 // Middle point of resistor divider at pin 15 (P1.7) (A7)

#define UNUSED_P1  ( BIT1 | BIT2 | BIT6 )  // Unused P1 lines

// PORT 2 USAGE

#define IRb  BIT1 // Active High IR LED also on pin 9 (P2.1)
                  // P2.1 is Timer1_A Out1 output in compare mode

#define RD_H BIT3 // Upper end of resistor divider at pin 11 (P2.3)

// P2.6 and P2.7 are used by the 32768 Xtal

#define UNUSED_P2  ( BIT0 | BIT2 | BIT4 | BIT5 ) // Unused P2 lines

// PORT 3 USAGE
//
// There is no available Port3 in the 20 pin DIP package
// however we must disable port3 to obtain low current LPM4
#define UNUSED_P3  ( ALL_BITS )

/************* STATUS MACHINE DEFINITIONS *******************/

#define ST_NONE  0  // Nothing to do
#define ST_NFB   1  // Normal trigger first burst           ( 16 cycles)
#define ST_ND    2  // Normal trigger delay between bursts  (240 cycles)
#define ST_SB    3  // Normal/Delayed trigger second burst  ( 16 cycles)
#define ST_DFB   5  // Delayed trigger first burst          ( 16 cycles)
#define ST_DD    6  // Delayed trigger delay between bursts (176 cycles)

/**************** VARIABLES *********************************/

unsigned int fclk_MHz=1;   // Clock frequency in MHz
                           // Used by the delay function

volatile int buttons=0;    // Buttons to be processed

// Status variable for timer
//
// 0 : Nothing to do
//
// Immediate trigger codes
//  1 : First burst
//  2 : Wait for second burst
//  3 : Second burst
//
// Delayed trigger codes
//  5 : First burst
//  6 : Wait for second burst
//  7 : Second burst
//
volatile int status=ST_NONE;

/************** FUNCTION PROTOTYPES *************************/

int checkVdd(void);

/**************** FUNCTIONS *********************************/

// Delay of about len ms
void delay(unsigned int len)
 {
 if (!len) return; // Nothing to do if len=0

 // Set number of cycles depending on clk frequency
 len*=fclk_MHz;

 do
   {
   // 1000 cycles delay
   __delay_cycles (1000);

   len--; // Decrease counter
   }
   while(len);
 }

// Sends several blinks to the RED LED
// Parameters:
//           n : Number of blinks
//      length : ON, OFF Duration in ms
void doBlinks(unsigned int n,unsigned int length)
 {
 do
  {
  SET_FLAG(P1OUT,LED); // Turn on led
  delay(length);
  CLEAR_FLAG(P1OUT,LED); // Turn off led
  delay(length);
  }
  while(--n);

 }

/**************** TEST FUNCTIONS ***************************/
// These functions are used only during the development
// They can be eliminated in the release product

#ifndef RELEASE

// Changes blink frequency depending on SW1 and SW2
// This is a test function. Never returns
void blinkTest(void)
 {
 while (1)
    {
	SET_FLAG(P1OUT,LED); // Turn on led
	if (P1IN&SW1)
		delay(1000); // Time ON 1s if SW1 not pressed
	    else
		delay(200);  // Time ON 0.2s if SW1 pressed
	CLEAR_FLAG(P1OUT,LED); // Turn off led
	if (P1IN&SW2)
		delay(1000); // Time OFF 1s if SW2 not pressed
		else
		delay(200);  // Time OFF 0.2s if SW2 pressed
    }
 }

// Test outputting SMCLK at P1.4 (IRa)
// This is a test function. Never returns
void smclkTest(void)
 {
 // Configurations to set SMCLK
 SET_FLAG(P1DIR,IRa);
 SET_FLAG(P1SEL,IRa);
 }

#endif /* RELEASE */

/********** CAMERA TRIGGER FUNCTIONS **********************/

// Normal camera trigger switch SW1 is pressed
// Two burst of 16 ACLK cycles
// Starts separated 7.82ms (256 cycles)
void normalTrigger(void)
 {
 // doBlinks(2,200);  // Test that we enter OK

 SET_FLAG(P1SEL,IRa); // Activate burst

 SET_FLAG(TACTL,TACLR);             // Clear the counter
 TACCR0=16;                         // First interrupt set at 16 cycles
 status=ST_NFB;                     // This is first burst of normal trigger
 SET_FIELD(TACTL,TAMC_MASK,MC_2);   // Set mode to Up Continuous

 delay(100);

 if (checkVdd())
	 SET_FLAG(P1OUT,LED); // Turn on led

 delay(400);

 CLEAR_FLAG(P1OUT,LED); // Turn off led
 }

// Delayed camera trigger switch SW2 is pressed
// Two burst of 16 ACLK cycles
// Starts separated 5.86ms (192 cycles)
void delayedTrigger(void)
 {
 // doBlinks(4,200);  // Test that we enter OK

 SET_FLAG(P1OUT,LED); // Turn on led
 SET_FLAG(P1SEL,IRa); // Activate burst

 SET_FLAG(TACTL,TACLR);             // Clear the counter
 TACCR0=16;                         // First interrupt set at 16 cycles
 status=ST_DFB;                     // This is first burst of delayed trigger
 SET_FIELD(TACTL,TAMC_MASK,MC_2);   // Set mode to Up Continuous

 delay(100);

 if (checkVdd())
	 SET_FLAG(P1OUT,LED); // Turn on led

 delay(400);

 CLEAR_FLAG(P1OUT,LED); // Turn off led
 }

/*************** ADC FUNCTIONS ****************************/

unsigned int value;

// Reads the indicated channel on the ADC10
// Uses an internal reference of 1,5V
unsigned int readADC10(int channel)
 {
 unsigned int val;

 //ADC10CTL0
 // Activation of the ADC10 core
 //SET_FLAG(ADC10CTL0,ADC10ON);
 // Use default internal clock source
 // Use default divider
 // Enable the internal 2.5V reference
 //SET_FLAG(ADC10CTL0,REFON);
 // Set the maximum S/H
 //SET_FIELD(ADC10CTL0,ADC10SHT_MASK,ADC10SHT_3);
 // Use VR+ = VREF+ VR- = AVSS
 // SET_FIELD(ADC10CTL0,ADC10SREF_MASK,SREF_1);
 // More compact solution
 // Default triggering with ADC10SC
 // Default single channel single conversion
 ADC10CTL0=ADC10ON|REFON|ADC10SHT_3|SREF_1;
 // Enable the channel
 ADC10AE0=(1<<channel);

 // Set the input
 ADC10CTL1=(((unsigned int)channel)*INCH_OFFS)|ADC10DIV_7;

 delay(10);

 // Set the conversion
 ADC10CTL0|=(ENC|ADC10SC);

 // Wait for conversion termination
 while (ADC10CTL1&ADC10BUSY) {};

 // ENC must be cleared before accessing
 // the rest of the flags on ADC10CTL0
 // Failing to do that can leave some peripherals
 // on ADC activated and ruin low power in LPM4 mode
 RESET_FLAG(ADC10CTL0,ENC);

 // Get the converted value
 val=ADC10MEM;

 // Disable the channel
 ADC10AE0=0;

 // Disable OSC ADC core and Reference
 CLEAR_FLAG(ADC10CTL0,ADC10SC|ADC10ON|REFON);
 //ADC10CTL0=0;
 //ADC10CTL1=0;

 return val;
 }

// Check the Vdd voltage
// TODO Test that it works ok
// Returns 0 if below 2.6V
// Returns 1 otherwise
int checkVdd(void)
 {
 // Activate the resistor divider
 SET_FLAG(P2OUT,RD_H);

 // Wait 10ms to settle
 delay(10);

 // Read ADC at center point
 value=readADC10(7);
 value=readADC10(7);

 // Deactivate the divider
 CLEAR_FLAG(P2OUT,RD_H);

 // We consider the battery nearly depleted when
 // the voltage falls below 2.6V
 // That means 1024*(2.6/2/1.5) = 890
 if (value<890) return 0;
 return 1;
 }

/*************** MAIN FUNCTION ****************************/

int main(void)
 {
 WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog timer
 P1DIR |= LED | IRa;        // Output mode pins

 P1OUT = 0;  // Guarantee that LED and IR are off

 P1REN |= SW1 | SW2;    // Activate pull-up/pull-down
 P1OUT |= SW1 | SW2;    // Set to pull-up

 // Invalidate unused pins using pull-downs
 P1REN |= UNUSED_P1;
 CLEAR_FLAG(P1OUT,UNUSED_P1);
 CLEAR_FLAG(P1DIR,UNUSED_P1);

 P2REN |= UNUSED_P2;
 CLEAR_FLAG(P2OUT,UNUSED_P2);
 CLEAR_FLAG(P2DIR,UNUSED_P2);

 P3REN |= UNUSED_P3;
 CLEAR_FLAG(P3OUT,UNUSED_P3);
 CLEAR_FLAG(P3DIR,UNUSED_P3);

 // Set resistor divider high node
 P2DIR |= RD_H;
 CLEAR_FLAG(P2OUT,RD_H);

 // Set 32768kHz Xtal capacitance to 12.5pF
 SET_FIELD(BCSCTL3,XCAP_MASK,XCAP_3);

 // Select 32768kHz LFXTCLK for SMCLK
 SET_FLAG(BCSCTL2,SELS);

 // Load calibrated data for 16MHz clock
 DCOCTL=CALDCO_16MHZ;  // DCOCTL Calibration Data for 16MHz
 BCSCTL1=CALBC1_16MHZ; // BCSCTL1 Calibration Data for 16MHz
 fclk_MHz=16;          // Set clk data for timings

 // Wait 1 second
 delay(1000);

 // Interrupt configuration for SW1 and SW2
 P1IES |= SW1|SW2;   // Sets pins SW1 and SW2 port 1 to falling edge
 P1IE  |= SW1|SW2;   // Enables interrupts on SW1 and SW2
 P1IFG=0;            // Clear interrupt flags

 // Configure Timer0A
 SET_FIELD(TACTL,TASSEL_MASK,TASSEL_1);  // Set clock to ACLK (32768Hz)
 SET_FIELD(TACTL,TAID_MASK,ID_0);        // Set clock divider to :1
 SET_FIELD(TACTL,TAMC_MASK,MC_0);        // Set mode to STOP
 SET_FLAG(TACTL,TACLR);                  // Clear the counter

 // Configure Capture block 0
 CLEAR_FLAG(TACCTL0,CAP);   // Mode set to compare
 TACCR0=16;                 // First interrupt set at 16 cycles
 SET_FLAG(TACCTL0,CCIE);    // Enable interrupt

 // Blink test
 // At 25/11/2014 it works OK, we don't need it anymore
 // blinkTest();

 // SMCLK Test
 // At 25/11/2014 it works OK, we get 32768Hz on the IR LED
 // smclkTest();

 // Signal the end of the configuration
 doBlinks(2,200);  // 2 blinks of 200ms

 _enable_interrupts();  // Enable interrupts

 // Infinite loop
 while (1)
   {
   // Check if there is any SW to respond to

   if (!buttons)
       {
	   LPM4;       // Go to sleep if there are no buttons
	   delay(200); // Little delay going out of sleep
       }


   if (buttons&SW1)   // Normal camera trigger
	   {
	   normalTrigger();
	   buttons=0;
	   }

   if (buttons&SW2)    // Delayed camera trigger
	   {
	   delayedTrigger();
	   buttons=0;
	   }

   };
 }

/*********************** GPIO INTERRUPT **************************/

// The port 1 is associated with vectors PORT1_VECTOR
// The RSI must test the different bits

// It seems that the interrupt() macro in iomacros.h is broken
// #define __interrupt(vec) __attribute__((__interrupt__(vec)))
// We can redefine a new macro:

#define interruptNew(vec) __attribute__((interrupt(vec)))

// PORT 1 RSI
void interruptNew(PORT1_VECTOR) PORT1_ISR (void)
 {
 // Don't do anything if we have not processed last order
 if (!buttons)
    {
    buttons|=P1IFG&(SW1|SW2);

    // Exit LPM4 mode on exit
    LPM4_EXIT;
    }

  // Clear the flags
  P1IFG=0;
  }

/************** TIMER0A COMPARE 0 INTERRUPT ********************/

void interruptNew(TIMER0_A0_VECTOR) TA0Capture0_ISR(void)
 {
 switch (status)
  {
  case ST_NONE:
	  // Nothing to do
	  break;

  case ST_NFB:
	  // We were in the first burst
	  CLEAR_FLAG(P1SEL,IRa);    // Deactivate burst
	  TACCR0+=240;              // 240 cycles to next change
	  status=ST_ND;             // Now in Delay between Bursts
	  break;

  case ST_ND:
	  // We were in the space between bursts
	  SET_FLAG(P1SEL,IRa);    // Activate burst
	  TACCR0+=16;             // 16 cycles to next change
	  status=ST_SB;           // Now in Second Burst
	  break;

  case ST_SB:
  	  // We were in the second burst
  	  CLEAR_FLAG(P1SEL,IRa);            // Deactivate burst
  	  SET_FIELD(TACTL,TAMC_MASK,MC_0);  // Set mode to STOP
  	  status=ST_NONE;	                // Now in Idle mode
  	  break;

  case ST_DFB:
	  // We were in the first burst
	  CLEAR_FLAG(P1SEL,IRa);    // Deactivate burst
	  TACCR0+=176;              // 176 cycles to next change
	  status=ST_DD;             // Now in Delay between bursts
	  break;

  case ST_DD:
	  // We were in the space between bursts
	  SET_FLAG(P1SEL,IRa);    // Activate burst
	  TACCR0+=16;             // 16 cycles to next change
	  status=ST_SB;	          // Now in Second Burst
	  break;

  default:
	  // Default case if anything goes wrong
	  SET_FIELD(TACTL,TAMC_MASK,MC_0);    // Set mode to STOP
	  CLEAR_FLAG(P1SEL,IRa);              // Deactivate burst
	  status=ST_NONE;                     // Go to a valid state
	  break;
  }

 }


