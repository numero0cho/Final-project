// ******************************************************************************************* //

#include "p24fj64ga002.h"
#include "lcd.h"

// ******************************************************************************************* //
#define F_CY 14745600
#define PWM_PERIOD (1/10000)
#define PWM_FREQ 10000					// setting frequency to 10 kHz
#define PR_VALUE (57600/PWM_FREQ)-1		// using presalar of 256 with F_CY

// preliminary final project code
// FIXME:	If I understand correctly, the voltage of the transistor will be lower with less light
//			reflected, so black will absorb the most light and have the least voltage.  The white 
//			tape and tile will both be on the upper end of the reflectivity scale (tile is glossy,
//			mostly white; tape is white), and will thus have higher ADC values.  The red tape of the
//			barcode will be somewhere in the middle.
#define lower_tile_val 400		// since tile is the most reflective, it will have the highest value
#define upper_white_val 899		// white tape will be 2nd only to the tile; white, non-reflective
#define lower_white_val 600
#define upper_red_val   599		// red range will begin just below the white
#define lower_red_val   150
#define upper_black_val 200	// black tape will reflect little, and will only be at the low end


/***********************************************************************************************/

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

/***********************************************************************************************/


void PWM_Update(double rightADC, double leftADC, double midADC) {
	double OC_value_right = 0.0;
	double OC_value_left = 0.0;
	double DUTY_CYCLE_RIGHT = 0.0;
	double DUTY_CYCLE_LEFT = 0.0;
	int i = 0;
	char value[8];
	
	if(midADC > 500 && leftADC > 250 && rightADC > 300) {
		return;
	}
	else{
		if(midADC < 350){
			leftADC=550;
			rightADC=600;	
		}
		else{	
			if(leftADC<=200){
				leftADC=150;
			}
			if(rightADC<=250){
				rightADC=200;
			}
		}
		
		// Need to check if left is actually left or if left is right
	
		DUTY_CYCLE_LEFT =0.35+0.65*(leftADC-150) / 511.5;
		DUTY_CYCLE_RIGHT =0.4+0.6*(rightADC-200) / 511.5;
	
		OC_value_right = DUTY_CYCLE_RIGHT * (PR_VALUE);		// set output compare value for new position
		OC1RS = OC_value_right;
		
		OC_value_left = DUTY_CYCLE_LEFT * (PR_VALUE);		// set output compare value for new position
		OC2RS = OC_value_left;
	
		return;
	}
}	

/*
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
		DUTY_CYCLE_LEFT = 1 - position_ratio;			// turning left
	}
	
	OC_value_right = DUTY_CYCLE_RIGHT * (PR_VALUE);		// set output compare value for new position
//	OC1R = OC_value_right;
	OC1RS = OC_value_right*0.8;								// set the right motor value for the next cycle
	
	if(DUTY_CYCLE_RIGHT < 0.0) DUTY_CYCLE_RIGHT = 0.0;
//	sprintf(value, "%3.f", DUTY_CYCLE_RIGHT*100);
//	LCDMoveCursor(1, 0);
//	LCDPrintString(value);
//	LCDPrintChar('%');

		
	OC_value_left = DUTY_CYCLE_LEFT * (PR_VALUE);		// set output compare value for new position
//	OC2R = OC_value_left;
	OC2RS = OC_value_left*0.8;								// set the left mtoor value for the next cycle
	
	if(DUTY_CYCLE_LEFT < 0.0) DUTY_CYCLE_LEFT = 0.0;
//	sprintf(value, "%3.f", DUTY_CYCLE_LEFT*100);
//	LCDPrintString(value);
//	LCDPrintChar('%');

	return;
}

/***********************************************************************************************/

void barCode_Scan(double left_val, double right_val, int barcode[], unsigned int *code_counter) {
	
	char left = '\0';
	char right = '\0';
	int i = 0;

	switch (barcode[0]) {

		// "case 2" is the polling state, checking if a barcode is being detected or not
		case 2:

			// We only have to check for the black bar in the initial case because this is the only
			// start scenario for reading a barcode.
			if (left_val <= upper_black_val) {		// if start bit detected
				barcode[0] = -1;
				left = 'Y';
				right = 'N';

				LCDClear();
				LCDMoveCursor(0,0);
				LCDPrintChar('L');
				LCDPrintChar(':');
			}
			else if (right_val <= upper_black_val) {	// if start bit detected
				barcode[0] = -1;
				left = 'N';
				right = 'Y';

				LCDClear();
				LCDMoveCursor(1,0);
				LCDPrintChar('R');
				LCDPrintChar(':');
			}
			break;


		// if the first element is not 2, then a barcode "start" bit has already been detected
		default:
			
			// We now have to consider all scenarios. At this point, we will only enter this state if
			// the start bit has been detected and a change in the transistor has been registered.
			// A change is likely to occur since any small change will be registered, so we will also
			// have to check the previous barcode entry, since we never have two repeated values.
			if (left == 'Y') {
				if (left_val <= upper_black_val) {
					if (barcode[*code_counter] == 0 || barcode[*code_counter] == -1)  break;

					barcode[*code_counter] = 0;
					LCDPrintChar(barcode[*code_counter] + '0');
				}
				else if (left_val >= lower_red_val && left_val <= upper_red_val) {
					if (barcode[*code_counter] == 1)  break;

					barcode[*code_counter] = 1;
					LCDPrintChar(barcode[*code_counter] + '0');
				}
				else if (left_val >= lower_white_val && left_val <= upper_white_val){
					*code_counter++;
				}
				else if (left_val >= lower_white_val && left_val <= upper_white_val && *code_counter == 4) {
					for (i = 0; i < 5; i++) {	// reset the barcode
						barcode[i] = 2;
					}
					*code_counter = 0;
				}
				else {	// if this executes, then we have reached the end of the barcode
					LCDClear();
					LCDMoveCursor(0, 0);
					LCDPrintString("Invalid");
					for (i = 0; i < 5; i++) {	// reset the barcode
						barcode[i] = 2;
					}
					*code_counter = 0;
				}
			}
			else if (right == 'Y') {
				if (right_val <= upper_black_val) {
					if (barcode[*code_counter] == 0 || barcode[*code_counter] == -1)  break;

					*code_counter++;
					barcode[*code_counter] = 0;
					LCDPrintChar(barcode[*code_counter] + '0');
				}
				else if (right_val >= lower_red_val && right_val <= upper_red_val) {
					if (barcode[*code_counter] == 1)  break;

					*code_counter++;
					barcode[*code_counter] = 1;
					LCDPrintChar(barcode[*code_counter] + '0');
				}
				else if (right_val >= lower_white_val && right_val <= upper_white_val){
					*code_counter = *code_counter;
				}
				else if (left_val >= lower_white_val && left_val <= upper_white_val && *code_counter == 4) {
					for (i = 0; i < 5; i++) {	// reset the barcode
						barcode[i] = 2;
					}
					*code_counter = 0;
				}
				else {	// if this executes, then we have reached the end of the barcode
					LCDClear();
					LCDMoveCursor(0, 0);
					LCDPrintString("Invalid");
					for (i = 0; i < 5; i++) {	// reset the barcode
						barcode[i] = 2;
					}
					*code_counter = 0;
				}
			}
			break;
	}

	return;
}

// Nicolas Fajardo, Paul Cross, Kevin Morris
// TEAM 202

