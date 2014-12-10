// ******************************************************************************************* //

#include "p24fj64ga002.h"
#include "lcd.h"

// ******************************************************************************************* //
#define F_CY 14745600
#define PWM_PERIOD (1/10000)
#define PWM_FREQ 10000					// setting frequency to 10 kHz
#define PR_VALUE (57600/PWM_FREQ)-1		// using presalar of 256 with F_CY

#define lower_white_val 38.0		// white will reflect the most IR, and will have the highest ADC
#define upper_black_val 10.0		// black tape will reflect little, and will only be at the low end
#define lower_tile_val 25.0

/***********************************************************************************************/

void PWM_init(double pot_position) {

	OC1R = 0.0;						// set the initial duty cycle to have motor OFF
	OC1RS = 0.0;					// set duty cycle to update with 0.0 on second cycle
	OC1CONbits.OCTSEL = 1;
	OC1CONbits.OCM = 6;

	OC2R = 0.0;						// set the initial duty cycle to have the motor OFF
	OC2RS = 0.0;					// set duty cycle to update with 0.0 on second cycle
	OC2CONbits.OCTSEL = 1;
	OC2CONbits.OCM = 6;

	TMR3 = 0;
	PR3 = PR_VALUE;
	T3CON = 0x8030;		// configures with a prescalar of 256 and turns timer 3 ON

	return;
}

/***********************************************************************************************/


void PWM_Update(double rightADC, double leftADC, double midADC, int bcState) { // right and left motor
	double OC_value_right = 0.0;
	double OC_value_left = 0.0;
	double DUTY_CYCLE_RIGHT = 0.0;
	double DUTY_CYCLE_LEFT = 0.0;
	
	if(midADC > 500 && leftADC > 250 && rightADC > 300) {
		return;
	}
	else{
		if(midADC < 350){
			leftADC=600;	
			rightADC=650;	
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
	
		DUTY_CYCLE_LEFT =0.30+0.65*(leftADC-100) / 511.5;
		DUTY_CYCLE_RIGHT =0.35+0.6*(rightADC-150) / 511.5;
		
//		if(bcState>=0){
//			DUTY_CYCLE_LEFT =0.15+0.65*(leftADC-150) / 511.5;
//			DUTY_CYCLE_RIGHT =0.2+0.6*(rightADC-200) / 511.5;
//		}	
		OC_value_right = DUTY_CYCLE_RIGHT * (PR_VALUE);		// set output compare value for new position
		OC1RS = OC_value_right;
		
		OC_value_left = DUTY_CYCLE_LEFT * (PR_VALUE);		// set output compare value for new position
		OC2RS = OC_value_left;
	
		return;
	}
}	

/***********************************************************************************************/

void barCode_Scan(double barcode_val, int *code_counter, double *min_val, double *max_val) {
	
	
	switch (*code_counter) {

		// "case -2" is the polling state, checking if a barcode is being detected or not
		case -2:

			// We only have to check for the black bar in the initial case because this is the only
			// start scenario for reading a barcode.
			if (barcode_val <= upper_black_val) {		// if start bit detected
				*code_counter = -1;
			}	
			break;

		case -1:
			
			if (barcode_val > *max_val) {
				*max_val = barcode_val;
			}	
			
			// if white is detected, we have left the start bit 
			if (*max_val >= lower_white_val) {
				LCDClear();
				LCDMoveCursor(1, 0);
				LCDPrintString("BC");
				LCDPrintChar(':');
				*code_counter = 0;
				*max_val = 0.0;
			}
			// else if we find tile
			else if (barcode_val <= upper_black_val && *max_val < lower_white_val && *max_val >= lower_tile_val) {
				*max_val = 0.0;
				*min_val = 1500.0;
				LCDClear();
				*code_counter = -2;
			}	
			break;

		default:

			// We now consider the rest of the barcode.  We will check if the value is white first.
			// If the value is white, we stay where we are.  If the value is not white, we will begin
			// checking for and updating the minimum value.  We do this until white is detected again,
			// at which point we will compare the minimum to our ranges and print the appropriate value
			// for the barcode.  We will then reset the state and the minimum value to ensure that the
			// entire process will repeat itself upon detection of another barcode.
			if (barcode_val > lower_white_val && *min_val >= 1000) {
				*code_counter = *code_counter;
				*min_val = 1500.0;
				*max_val = 0.0;
			}
			else {
				if (barcode_val < *min_val) { 
					*min_val = barcode_val;
				}		
				else if (barcode_val >= lower_white_val) {
				//	If this happens, then we are back to white.
					LCDMoveCursor(1, *code_counter + 3);
					
					if (*min_val <= upper_black_val)
						LCDPrintChar('0');
					else if (*min_val > upper_black_val && *min_val < lower_white_val)
						LCDPrintChar('1');

					*code_counter = *code_counter + 1;
					*min_val = 1500.0;
					
				}
			}

			if (*code_counter == 4) {
				*code_counter = -2;
				*min_val = 1500.0;
			}
				*max_val = 0.0;
			break;
	}

	return;
}




/*
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
*/
// Nicolas Fajardo, Paul Cross, Kevin Morris
// TEAM 202

