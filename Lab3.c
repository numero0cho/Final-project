// ******************************************************************************************* //
// Include file for PIC24FJ64GA002 microcontroller. This include file defines
// MACROS for special function registers (SFR) and control bits within those
// registers.

#include "p24fj64ga002.h"
#include "RobotMove.h"
#include <stdio.h>
#include "lcd.h"

// ******************************************************************************************* //
// Configuration bits for CONFIG1 settings. 
//
// Make sure "Configuration Bits set in code." option is checked in MPLAB.
//
// These settings are appropriate for debugging the PIC microcontroller. If you need to 
// program the PIC for standalone operation, change the COE_ON option to COE_OFF.

_CONFIG1(JTAGEN_OFF & GCP_OFF & GWRP_OFF & BKBUG_ON & COE_OFF & ICS_PGx1 &
	FWDTEN_OFF & WINDIS_OFF & FWPSA_PR128 & WDTPS_PS32768)

	// ******************************************************************************************* //
	// Configuration bits for CONFIG2 settings.
	// Make sure "Configuration Bits set in code." option is checked in MPLAB.

	_CONFIG2(IESO_OFF & SOSCSEL_SOSC & WUTSEL_LEG & FNOSC_PRIPLL & FCKSM_CSDCMD & OSCIOFNC_OFF &
	IOL1WAY_OFF & I2C1SEL_PRI & POSCMOD_XT)

	// ******************************************************************************************* //

	// Varible used to indicate that the current configuration of the keypad has been changed,
	// and the KeypadScan() function needs to be called.

#define F_CY 14745600

	// FIXME: MAY NEED TO CHANGE FREQUENCY BELOW
#define PWM_PERIOD (1/10000)
#define PWM_FREQ 10000						// setting frequency to 100 Hz; MAY NEED TO BE CHANGED
#define PR_VALUE (57600/PWM_FREQ)-1		// using presalar of 256 with F_CY

volatile int state = 0;
volatile double POT_POS = 0.0;

int main(void) {
	char value[8];

	TRISBbits.TRISB8 = 0;	// setting pin to be output
	TRISBbits.TRISB2 = 0;
	TRISBbits.TRISB3 = 0;
	TRISBbits.TRISB10 = 0;
	

	TRISBbits.TRISB5 = 1;	// switch 1 is input; pin 14

	CNEN2bits.CN27IE = 1;	// enable change notification for switch 1

	IFS1bits.CNIF = 0;		// set CN flag low
	IEC1bits.CNIE = 1;		// enable CN

	TRISAbits.TRISA0 = 1;	// set pin 2 to be input
	AD1PCFGbits.PCFG0 = 0;	// set pin 2 to be analog input

	AD1PCFG &= 0xFFFE;	// AN0 input pin is analog
	AD1CON2 = 0; // Configure A/D voltage reference
	AD1CON3 = 0x0101;
	AD1CON1 = 0x20E4;
	AD1CHS = 0; // Configure input channel
	AD1CSSL = 0; // No inputs is scanned
	IFS0bits.AD1IF = 0;
	AD1CON1bits.ADON = 1; // Turn on A/D
	
	LCDInitialize();
	PWM_init(POT_POS);

	/***************************************
	
	TODO: Analog to digital conversions will go in here to find out the position of the
	potentiometer.  This position will be used to set the duty cycle for the motion of
	the motors in the Pulse Width Modulation (PWM).
	
	***************************************/

	// will arbitrarily define the result of the ADC as the double pot_pos (see above)
	
	// we have a 10-bit ADC, so the values from the approximation range from 0-1023 [2^10-1]
	// this means that our middle value is 511.5

	while (1) {

		while (AD1CON1bits.DONE != 1){};     // keeps waiting until conversion finished
		POT_POS = ADC1BUF0;
		sprintf(value, "%4.2f", POT_POS);
		LCDMoveCursor(0, 0);
		LCDPrintString(value);

		PWM_Update(POT_POS);

		switch (state) {

			case 0:		// Idle
				RPOR4bits.RP8R = 0;	// left wheel
				RPOR1bits.RP2R = 0;
				RPOR1bits.RP3R = 0;	// right wheel
				RPOR5bits.RP10R = 0;
				break;
			case 1:		// Forward
				RPOR4bits.RP8R = 0;	// left wheel
				RPOR1bits.RP2R = 18;
				RPOR1bits.RP3R = 0;	// right wheel
				RPOR5bits.RP10R = 19;
				break;
			case 2:     // Idle
				RPOR4bits.RP8R = 0;	// left wheel
				RPOR1bits.RP2R = 0;
				RPOR1bits.RP3R = 0;	// right wheel
				RPOR5bits.RP10R = 0;
				break;
			case 3:		// Backward
				RPOR4bits.RP8R = 18;	// left wheel
				RPOR1bits.RP2R = 0;
				RPOR1bits.RP3R = 19;	// right wheel
				RPOR5bits.RP10R = 0;
				break;
		}
		
	}
	return 0;
}


void __attribute__((interrupt)) _CNInterrupt(void) { // for SW1 RB5

	IFS1bits.CNIF = 0;
//	LATBbits.LATB15 = 1;

	if (PORTBbits.RB5 == 1) {   // press
	
	//pressed (let's switch states)
	if (state == 3) {
		state = 0;
	}
		else {
		state++;
	}
	}
}