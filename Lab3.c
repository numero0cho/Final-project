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
#define MIDDLE           320    // cut-off between tile and white
//#define MIDDLE          140    // cut-off between red and tile
#define DARK            200    // cut-off between black and red

volatile int state = -1;
volatile double POT_POS = 513.5;

//	preliminary final project code
volatile double LeftSensorADC = 0.0;
volatile double MiddleSensorADC = 0.0;
volatile double RightSensorADC = 0.0;
volatile int barcode[5] = { 2, 2, 2, 2, 2 };
volatile unsigned int barcode_counter = 0;

int main(void) {
	char value[8];
	float k = 1.0;

	TRISBbits.TRISB8 = 0;	// setting pin to be output
	TRISBbits.TRISB2 = 0;
	TRISBbits.TRISB3 = 0;
	TRISBbits.TRISB10 = 0;

	// FOR MOTION TRANSISTORS
	TRISAbits.TRISA0 = 1;	// set pin 2 to be input
	AD1PCFGbits.PCFG0 = 0;	// set pin 2 to be analog input

	TRISAbits.TRISA1 = 1;	// set pin 3 to be input
	AD1PCFGbits.PCFG1 = 0;	// set pin 3 to be analog input

	TRISBbits.TRISB0 = 1;	// set pin 4 to be input
	AD1PCFGbits.PCFG2 = 0;	// set pin 4 to be analog input
	// END PINS FOR MOTION

	TRISBbits.TRISB5 = 1;	// switch 1 is input; pin 14

	CNEN2bits.CN27IE = 1;	// enable change notification for switch 1
//
	IFS1bits.CNIF = 0;		// set CN flag low
	IEC1bits.CNIE = 1;		// enable CN
//
//	AD1PCFG &= 0xFFFE;	// AN0 input pin is analog
	AD1CON2 = 0; // Configure A/D voltage reference
	AD1CON3 = 0x0101;
	AD1CON1 = 0x20E4;
	AD1CHS = 0; // Configure input channel
	AD1CSSL = 0; // No inputs is scanned
	IFS0bits.AD1IF = 0;
	AD1CON1bits.ADON = 1; // Turn on A/D
	
	LCDInitialize();
	PWM_init(POT_POS);
	
          
 
	while (1) {
		
		// The following three blocks of code should update the values read from the resistors to 
		// be used in Kevin's conversion function.
		
		// FIXME: Must configure which input is being converted at this stage (for first one, set
		//		AN0 to be converted; second, set AN1, etc.) [Might already be fixed?]
		AD1CHS = 0;	// AN1 input pin is analog
		DelayUs(100);
		while (AD1CON1bits.DONE != 1){};     // keeps waiting until conversion finished
		LeftSensorADC = ADC1BUF0;
		sprintf(value, "%3.0f", LeftSensorADC);
		LCDMoveCursor(0,0); LCDPrintString(value);
		LCDPrintChar(' ');

		
		AD1CHS = 1;
		DelayUs(100);
		while (AD1CON1bits.DONE != 1){};     // keeps waiting until conversion finished
		MiddleSensorADC = ADC1BUF0;	
		sprintf(value, "%3.0f", MiddleSensorADC);
//		LCDMoveCursor(1,0);
		LCDPrintString(value);

		
		AD1CHS = 2;
		DelayUs(100);
		while (AD1CON1bits.DONE != 1){};     // keeps waiting until conversion finished
		RightSensorADC = (405/14)*(ADC1BUF0-3)+210;
	//	RightSensorADC =ADC1BUF0;
		sprintf(value, "%3.0f", RightSensorADC);
		LCDMoveCursor(1,0); LCDPrintString(value);
		
		LCDPrintChar(' ');
		LCDPrintChar(' ');
		sprintf(value, "%1d", state);
		LCDPrintString(value);






//		PWM_Update(POT_POS);



		// preliminary final project code
		// FIXME:	we will need to set-up the ADC to convert the transistor values from  
		//			analog to digital.  For now, I will arbitrarily name the two values I
		//			I will be using as leftPT_val and rightPT_val.  These represent the 
		//			already converted values.
	//	barCode_Scan(leftPT_val, rightPT_val, barcode, &barcode_counter);

		switch (state) {
            case 0: // track path there
//            	if (MiddleSensorADC < 350) { 
//	            	k = 0.8;
//	            	
//	            }		
//	            else k = 1.0;
	            
             //   POT_POS = 511.5 - 1.2*(RightSensorADC) + 1.0*(LeftSensorADC);
                
                PWM_Update(LeftSensorADC, RightSensorADC, MiddleSensorADC);
                DelayUs(200);

                PathDecision1();
                break;
            case 1: // 90 deg right turn
                //forward
                RPOR4bits.RP8R = 0;	// left wheel
                RPOR1bits.RP2R = 18;
                RPOR1bits.RP3R = 0;	// right wheel
                RPOR5bits.RP10R = 19;
                
                RightCorner();
                state = 0;
                break;
            case 2: // U Turn
                U_Turn();
              
                RPOR4bits.RP8R = 0;	// left wheel
                RPOR1bits.RP2R = 18;
                RPOR1bits.RP3R = 0;	// right wheel
                RPOR5bits.RP10R = 19;
                state = 0;
                break;
            case 3: // track path back
                 POT_POS = 512 - 1.0*(RightSensorADC) + 1.0*(LeftSensorADC);
             //   if (POT_POS > 1023) POT_POS = 1023;
               // if (POT_POS < 0)	POT_POS = 0;
                PWM_Update(POT_POS);
                // forward
                RPOR4bits.RP8R = 0;	// left wheel
                RPOR1bits.RP2R = 18;
                RPOR1bits.RP3R = 0;	// right wheel
                RPOR5bits.RP10R = 19;
                PathDecision2();
                break;
            case 4: // 90 deg left turn
                LeftCorner();
                state = 3;
                break;
            case 5: // Idle and victory!
                // idle
                RPOR4bits.RP8R = 0;	// left wheel
                RPOR1bits.RP2R = 0;
                RPOR1bits.RP3R = 0;	// right wheel
                RPOR5bits.RP10R = 0;
                // LCDPrintChar('VICTORY!') or something…
                break;
            case -1:
            	RPOR4bits.RP8R = 0;	// left wheel
                RPOR1bits.RP2R = 0;
                RPOR1bits.RP3R = 0;	// right wheel
                RPOR5bits.RP10R = 0;
                break;
                
                
        }
		
	}
	return 0;
}


int PathDecision1() {	// before the u-turn
    
    
//    if // Gamma Intersection
//        ( LeftSensorADC   > MIDDLE &&     //tile
// 		 MiddleSensorADC  < DARK &&	//black
//         RightSensorADC   < DARK) {	//black
//            state = 1;  } 	//90 deg right turn
////    else if // Capital T intersection
//        ( LeftSensorADC   < DARK &&	//black
//         MiddleSensorADC  > MIDDLE &&	//tile
//         RightSensorADC   < DARK) {	//black
//            state = 1;  }	//90 deg right turn
     if( LeftSensorADC   < DARK &&	//black
         MiddleSensorADC  < MIDDLE &&	//black
         RightSensorADC   < DARK) {	//black
            state = 2; }	//U-Turn
//      else if( LeftSensorADC   > 400 &&	//black
//         MiddleSensorADC  > 400 &&	//black
//         RightSensorADC   > 400) {
//	         U_Turn();
//	        } 
         
    return;
}

int PathDecision2() {   // return journey
    
    
//    if // 7 Intersection
//        ( RightSensorADC  > MIDDLE &&     //tile
//         MiddleSensorADC  < DARK &&	//black
//         LeftSensorADC    < DARK) {	//black
//            state = 4;  } 	//90 deg left turn
//    else if // Capital T intersection
//        ( LeftSensorADC   < DARK &&		//black
//         MiddleSensorADC  > MIDDLE &&     //tile
//         RightSensorADC   < DARK) {	//black
//            state = 4;  }	//90 deg left turn
     if // end of track: open tile
        ( LeftSensorADC   > MIDDLE &&     //Tile
         MiddleSensorADC  > MIDDLE &&     //Tile
         RightSensorADC   > MIDDLE) {     //Tile
            state = 5; }	//idle
    return;
}

void RightCorner() {
    PWM_Update(1023);
    int i=0;
    for (i=0; i<2000; i++){ // 30 would be 3 seconds
        DelayUs(200); //increments of 1/10th of a second
    }
   
    
    
    
}

void LeftCorner() {
    PWM_Update(0);
    int i =0;
    for (i=0; i<10; i++){ // 30 would be 3 seconds
        DelayUs(100000); //increments of 1/10th of a second
    }
    
}


void U_Turn() {
    RPOR4bits.RP8R = 18;	// left wheel
	RPOR1bits.RP2R = 0;
	RPOR1bits.RP3R = 0;	// right wheel
	RPOR5bits.RP10R = 19;
	PWM_Update(511.5);
	int i = 0;
    for(i = 0; i < 1800; i++){
	   DelayUs(1000);
			}    
    return;
 }


void __attribute__((interrupt,auto_psv)) _CNInterrupt(void){

	IFS1bits.CNIF = 0;		// set CN flag low
	
	if(PORTBbits.RB5 == 0) {
		if (state == -1) { 
			state = 0;
			// forward start
            RPOR4bits.RP8R = 0;	// left wheel
            RPOR1bits.RP2R = 18;
            RPOR1bits.RP3R = 0;	// right wheel
            RPOR5bits.RP10R = 19;
            // forward end
  		}              
		else state = -1;
		
	}	
}

