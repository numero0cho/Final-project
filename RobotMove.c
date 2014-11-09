// ******************************************************************************************* //

#include "p24fj64ga002.h"

// ******************************************************************************************* //
#define F_CY 14745600
#define PWM_PERIOD (1/10000)
#define PWM_FREQ 10000					// setting frequency to 10 kHz
#define PR_VALUE (57600/PWM_FREQ)-1		// using presalar of 256 with F_CY


void PWM_init(double pot_position) {

	double OC_value_right = 0.0;
	double OC_value_left = 0.0;
	double DUTY_CYCLE_RIGHT = 0.0;
	double DUTY_CYCLE_LEFT = 0.0;
	double position_ratio = 0.0;

	position_ratio = pot_position / (1023 / 2);			// update duty cycle for new position
	if (position_ratio < 1.0) {							// turning right
		DUTY_CYCLE_LEFT = 1.0;
		DUTY_CYCLE_RIGHT = position_ratio;
	}
	else {												// else turning left or idle
		DUTY_CYCLE_RIGHT = 1.0;							// right motor on full
		DUTY_CYCLE_LEFT = 2 - position_ratio;			// turning left
	}

	OC_value_right = DUTY_CYCLE_RIGHT * (PR_VALUE);		// set output compare value for new position
	OC_value_left = DUTY_CYCLE_LEFT * (PR_VALUE);		// set output compare value for new position

	OC1R = 0.0;													// set the initial duty cycle to have motor OFF
	OC1RS = OC_value_right;
	OC1CONbits.OCTSEL = 1;
	OC1CONbits.OCM = 6;

	OC2R = 0.0;													// set the initial duty cycle to have the motor OFF
	OC2RS = OC_value_left;
	OC2CONbits.OCTSEL = 1;
	OC2CONbits.OCM = 6;


	TMR3 = 0;
	PR3 = PR_VALUE;
	T3CON = 0x8030;		// configures with a prescalar of 256 and turns timer 3 ON

	return;
}

void PWM_Update(double pot_position) {

	double OC_value_right = 0.0;
	double OC_value_left = 0.0;
	double DUTY_CYCLE_RIGHT = 0.0;
	double DUTY_CYCLE_LEFT = 0.0;
	double position_ratio = 0.0;
	char value[8];

	position_ratio = pot_position / (1023 / 2);			// update duty cycle for new position
	if (position_ratio < 1.0) {							// turning right
		DUTY_CYCLE_LEFT = 1.0;
		DUTY_CYCLE_RIGHT = position_ratio;
	}
	else {												// else turning left or idle
		DUTY_CYCLE_RIGHT = 1.0;							// right motor on full
		DUTY_CYCLE_LEFT = 2 - position_ratio;			// turning left
	}
	
	OC_value_right = DUTY_CYCLE_RIGHT * (PR_VALUE);		// set output compare value for new position
	OC1R = OC_value_right;
	OC1RS = OC_value_right;								// set the right motor value for the next cycle
	
	if(DUTY_CYCLE_LEFT < 0.0) DUTY_CYCLE_LEFT = 0.0;
	sprintf(value, "%3.f", DUTY_CYCLE_RIGHT*100);
	LCDMoveCursor(1, 0);
	LCDPrintString(value);
	LCDPrintChar('%');

		
	OC_value_left = DUTY_CYCLE_LEFT * (PR_VALUE);		// set output compare value for new position
	OC2R = OC_value_left;
	OC2RS = OC_value_left;								// set the left mtoor value for the next cycle
	
	if(DUTY_CYCLE_LEFT < 0.0) DUTY_CYCLE_LEFT = 0.0;
	sprintf(value, "%3.f", DUTY_CYCLE_LEFT*100);
	LCDPrintString(value);
	LCDPrintChar('%');

	return;
}




// Nicolas Fajardo, Paul Cross, Kevin Morris
// TEAM 202

