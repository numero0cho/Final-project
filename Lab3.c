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

#define F_CY 14745600

#define PWM_PERIOD (1/10000)
#define PWM_FREQ 10000						// setting frequency to 100 Hz; MAY NEED TO BE CHANGED
#define PR_VALUE (57600/PWM_FREQ)-1			// using presalar of 256 with F_CY
#define MIDDLE 500    						// cut-off between tile and white
#define DARK 275    						// cut-off between black and red

volatile int state = -1;						// controls which state we are in for line tracking/turngin

volatile double LeftSensorADC = 0.0;			// list values for each ADC conversion from PT's
volatile double MiddleSensorADC = 0.0;
volatile double RightSensorADC = 0.0;
volatile double BarcodeSensorADC = 0.0;

volatile int barcode_counter = -2;		// value to keep track of where we are in barcode
volatile double minimum_val = 1500.0;	// value to keep track of minimum ADC value for barcode


// ******************************************************************************************* //


int main(void) {
	
	char value[8];
	float k = 1.0;
	int i = 0;
	
	// BEGIN PINS FOR MOTOR CONTROL
	// ->These are the updated pins that will be used for the RPORx pins to control the direction
	//   of the motors.
	TRISBbits.TRISB8 = 0;		// setting pin to be output	
	TRISBbits.TRISB10 = 0;		
	TRISBbits.TRISB9 = 0;
	TRISBbits.TRISB11 = 0;
	// END PINS FOR MOTOR CONTROL
	
	
	TRISBbits.TRISB0 = 1;		// set RB0 (pin 4) to be input
	AD1PCFGbits.PCFG2 = 0;		// Set RB0/AN2 (pin 4) to be analog input
	
//	TRISBbits.TRISB3 = 0;	
	
	
	// BEGIN PINS FOR MOTION
	// ->Set outputs for the ANx pins that will be used for controlling the line tracking
	//   of the robot.  Also configure these pins in analog mode to read the voltage properly.
	TRISAbits.TRISA0 = 1;		// set RA0 (pin 2) to be input
	AD1PCFGbits.PCFG0 = 0;		// set RA0/AN0 (pin 2) to be analog input

	TRISAbits.TRISA1 = 1;		// set RA1 (pin 3) to be input
	AD1PCFGbits.PCFG1 = 0;		// set RA1/AN1 (pin 3) to be analog input

	TRISBbits.TRISB2 = 1;		// set RB1 (pin 4) to be input
	AD1PCFGbits.PCFG4 = 0;		// set RB1/AN3 (pin 4) to be analog input
	// END PINS FOR MOTION


	// BEGIN SWITCH 1 CONTROL
	TRISBbits.TRISB5 = 1;		// switch 1 RB5 (pin 14) is input

	CNEN2bits.CN27IE = 1;		// enable change notification for switch 1
	IFS1bits.CNIF = 0;			// set CN flag low
	IEC1bits.CNIE = 1;			// enable CN
	// END SWITCH 1 CONTROl
	
	
	// BEGIN ADC CONTROL
	// ->Set the control values in the various ADC registers.
	AD1CON2 = 0; 				// Configure A/D voltage reference
	AD1CON3 = 0x0101;
	AD1CON1 = 0x20E4;
	AD1CHS = 0; 				// Configure input channel
	AD1CSSL = 0; 				// No inputs is scanned
	IFS0bits.AD1IF = 0;
	AD1CON1bits.ADON = 1; 		// Turn on A/D
	// END ADC CONTROL
	
	
	LCDInitialize();
	PWM_init(0.0);
	
 
	while (1) {
		
		// The following three blocks of code should update the values read from the resistors to 
		// be used in the PWM_Update function.
		
		AD1CHS = 0;								// ADC reads from AN0 (pin 2)
		DelayUs(200);
		while (AD1CON1bits.DONE != 1){};     	// keeps waiting until conversion finished
		MiddleSensorADC = ADC1BUF0;
		
		// The following lines control printing for error checking.
		sprintf(value, "%3.0f", MiddleSensorADC);
		LCDMoveCursor(0,0); 
		LCDPrintString(value);
		LCDPrintChar(' ');

		
		AD1CHS = 1;								// ADC reads from AN1 (pin 3)
		DelayUs(200);
		while (AD1CON1bits.DONE != 1){};     	// keeps waiting until conversion finished
		LeftSensorADC = ADC1BUF0;
		// The following lines control printing for error checking.	
		sprintf(value, "%3.0f", LeftSensorADC);
		LCDPrintString(value);

		
		AD1CHS = 4;								// ADC reads from AN3 (pin 5)
		DelayUs(200);
		while (AD1CON1bits.DONE != 1){};     	// keeps waiting until conversion finished
		RightSensorADC = ADC1BUF0;
		// The following lines control printing for error checking.
		sprintf(value, "%3.0f", RightSensorADC);
		LCDMoveCursor(1,0); 
		LCDPrintString(value);
		

		
		AD1CHS = 2;								// ADC reads from AN4 (pin 6)
		DelayUs(200);
		while (AD1CON1bits.DONE != 1){};     	// keeps waiting until conversion finished
		BarcodeSensorADC = ADC1BUF0;
//		sprintf(value, "%3.0f", BarcodeSensorADC);
//		LCDPrintChar(' ');
//		LCDPrintString(value);
		
		sprintf(value, "%d", state);
		LCDPrintChar(' ');
		LCDPrintString(value);
	//	barCode_Scan(BarcodeSensorADC, &barcode_counter, &minimum_val);	

		switch (state) {
            case 0: // track path there
                PWM_Update(LeftSensorADC, RightSensorADC, MiddleSensorADC);
                DelayUs(200);

               // PathDecision1();
                break;
                
            case 1: // 90 deg right turn
                //forward
                RPOR4bits.RP8R = 0;	// left wheel
                RPOR4bits.RP9R = 18;
                RPOR5bits.RP11R = 0;
                RPOR5bits.RP10R = 19;
                
                RightCorner();
                state = 0;
                break;
                
            case 2: // U Turn
            	U_Turn();
              
                RPOR4bits.RP8R = 0;	// left wheel
                RPOR4bits.RP9R = 18;
                RPOR5bits.RP11R = 0;
                RPOR5bits.RP10R = 19;
                
                state = 0;
                break;
                
            case 3: // track path back
				PWM_Update(LeftSensorADC, RightSensorADC, MiddleSensorADC);
                DelayUs(200);

                // forward
                RPOR4bits.RP8R = 0;		// left wheel
                RPOR4bits.RP9R = 18;
                RPOR5bits.RP11R = 0;	// left wheel
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
                RPOR4bits.RP9R = 0;
                RPOR5bits.RP11R = 0;	//right wheel
                RPOR5bits.RP10R = 0;
                // LCDPrintChar('VICTORY!') or something…
                break;
                
            case -1:
            	RPOR4bits.RP8R = 0;		// All grounded; no motion
                RPOR4bits.RP9R = 0;
                RPOR5bits.RP11R = 0;
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
         MiddleSensorADC  > MIDDLE &&	//black
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
    for (i=0; i<2000; i++){ 	// 30 would be 3 seconds
        DelayUs(200); 			//increments of 1/10th of a second
    }
   
    
    
    
}

void LeftCorner() {
    PWM_Update(0);
    int i =0;
    for (i=0; i<2000; i++){ 	// 30 would be 3 seconds
        DelayUs(200);			 //increments of 1/10th of a second
    }
    
}


void U_Turn() {
    RPOR4bits.RP8R = 18;	// left wheel
	RPOR4bits.RP9R = 0;
	RPOR5bits.RP11R = 0;	// right wheel
	RPOR5bits.RP10R = 19;
	
	PWM_Update(550, 600, 0);
	
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
            RPOR4bits.RP9R = 18;
           // RPOR1bits.RP2R = 18;
           RPOR5bits.RP11R = 0;
           // RPOR1bits.RP3R = 0;	// right wheel
            RPOR5bits.RP10R = 19;
            // forward end
  		}              
		else state = -1;
		
	}	
}



/***********************************************************/
//		IF A DELAY IS NEEDED, USE THE FOLLOWING:
//
//		for (i=0; i<2000; i++){ 	
//        	DelayUs(200); 			
//   	}  
/***********************************************************/
//		KEVIN'S CODE IF NEEDED
//           
//   	POT_POS = 511.5 - 1.2*(RightSensorADC) + 1.0*(LeftSensorADC);
/***********************************************************/