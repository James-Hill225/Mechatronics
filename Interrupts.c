/* 
* lab_4.c
*
* created: 2/9/2025 11:13:00 am
* author : James Hill
*/
//including necessary libraries
#include <avr/io.h>
#include <avr/interrupt.h>

//defining clock frequency
#define FREQ_CLK 1000000  

//**Global Variables**//
volatile int LED_Spot=0;  //LED_Spot is used to tell the program which LED the program is on. Goes from 0-5
volatile int New_Pattern=0;  //This flag is used so that when LED 6 is turned on the scrolling pattern shifts so that only LEDs 3,4,and 5 are scrolled
volatile int Reset_Pattern=0; //Another flag variable that will reset to the original pattern if triggered

//**Functions**//
void wait(volatile int multiple);

/*=================================
*INTERRUPT SERVICE ROUTINE FOR INT0
*==================================*/
ISR(INT0_vect){
	
	//Using & operator to perform bitwise operation to compare all bits and see if values match 
	if((PORTC & (1 << PC2)) == 0){		// Logic operator is used to detect if the bit at PC2 is a 0. If yes then the LED is on
		PORTB =0b00000010;				// Setting bit at PB0 to 0 so LED turns on
		New_Pattern=1;					//Use to enter if statement that will loop through the new pattern including PC3, PC4, and PC5
	}	
}//End of INT0


/*=================================
*INTERRUPT SERVICE ROUTINE FOR INT1
*==================================*/
ISR(INT1_vect){

	// Check if both PC5 AND PB0 are ON  1<<PC5 shifts to the 5th bit of the register to the left and the && checks that its true for both PC5 and PB0
	if (((PORTC & (1 << PC5)) == 0) && ((PORTB & (1 << PB0)) == 0)){
		
		//If both things are true then turn the both PB0 and PB1 on
		PORTB=0b00000000;
		
		//Setting all the LEDS to off for 3 seconds
		PORTC=0b11111111;
		wait(3000);
		
		Reset_Pattern=1;   //Setting reset pattern to 1 so that the pattern will end
	}
}//End of INT1


/*===========================
*MAIN FUNCTION
*============================*/
int main(void){
	
	//Setting all pins as either Inputs or outputs
	DDRC=0b00111111;   //Setting pins 0-5 on register C as OUTPUTS. Rest are inputs so it doesn't matter if I accidentally output some information from them
	DDRB=0b00000011;   //Setting pins 0 and 1 on register B as OUTPUTS. Last 2 LEDS connected here
	
	DDRD=0b00000000;   //Setting all pins on register D as INPUTS. switches are connected to PD2 and PD3
	PORTD=0b00001100;  //Enabling PD2 and PD3 pull up resistors
	
	//Setting all LEDS to off before starting the program loop
	PORTC=0b00111111;  //Wired as a sink so 1=off and 0=on
	PORTB=0b00000011;  //Wired as a sink so 1=off and 0=on
	
	//Enabling INT0 and INT1
	EIMSK=1<<INT0 | 1<<INT1;
	EICRA=0<<ISC00 | 1<<ISC01 | 0<<ISC10 | 1<<ISC11;  //Setting INT0 and INT1 to trigger on a falling edge.(Switch goes from high voltage to low voltage)
	sei();  //Enabling the global interrupt
	
	
	//===========================================
	//PROGRAM LOOP
	//===========================================
	while(1){  //Setting to while(1) so it loops indefinitely
		
		//Checking to see if the flag to reset the pattern was triggered
		if(Reset_Pattern==1){
			
			//Once the 3 seconds are over reset both flag variables so the sequence restarts AND set the LEDS at Port B to off
			New_Pattern=0;
			Reset_Pattern=0;
			PORTB=0b00000011;
			
			LED_Spot=0;			//Setting LED_Spot back to 0 so the pattern restarts from the beginning
			
		}//End of reset pattern if statement
		
		if(New_Pattern==1){
			// Light up only LEDs 3, 4, and 5 in a loop
			if (LED_Spot < 3 || LED_Spot > 5){          //Checks the value of LED spot using or operator. This will ALWAYS be less than 3 or greater than 5 so it's set to 3 
				LED_Spot = 3;  // Start scrolling at PC3
			}
			
			PORTC = ~(1 << LED_Spot);      // Set only the current LED on. All others off. Using invert operator so it sets bit to low state which is ON in this case
			wait(500);                    // Delay for 500 msec

			LED_Spot++;

			if (LED_Spot > 5){
				LED_Spot = 3; // Restart at PC3 
			}
		
		}//End of new pattern if statement
		
		else{  //Else statement so that everything runs as normal
			//Setting light sequence
			PORTC = ~(1 << LED_Spot);      // Set only the current LED on, all others off using << to shift bit LEFT as it loops. Using ~ to invert logic so that it flips a bit to a 0 instead of a 1
			wait(500);				       // Delay to keep LED on for a period of 500 msec

			// Increment the LED location
			LED_Spot++;

			// Reset the counter to 0 after PC5 is used so it begins looping again
			if (LED_Spot > 5){				//Using 5 as that represents PC5 
				LED_Spot = 0;               // Restart the sequence at PC0
			}
			
		}//End of else statement
		
	}//End of infinite program loop
	
}//End of the main function


/*========================================================
*WAIT FUNCTION USED WITH PERMISSION FROM DR. WILLIAM CLARK
*=========================================================*/
void wait(volatile int number_of_msec) {
	// This subroutine creates a delay equal to number_of_msec*T, where T is 1 msec
	// It changes depending on the frequency defined by FREQ_CLK
	char register_B_setting;
	char count_limit;
	
	// Some typical clock frequencies:
	switch(FREQ_CLK){
		case 16000000:
		register_B_setting = 0b00000011; // this will start the timer in Normal mode with prescaler of 64 (CS02 = 0, CS01 = CS00 = 1).
		count_limit = 250; // For prescaler of 64, a count of 250 will require 1 msec
		break;
		case 8000000:
		register_B_setting =  0b00000011; // this will start the timer in Normal mode with prescaler of 64 (CS02 = 0, CS01 = CS00 = 1).
		count_limit = 125; // for prescaler of 64, a count of 125 will require 1 msec
		break;
		case 1000000:
		register_B_setting = 0b00000010; // this will start the timer in Normal mode with prescaler of 8 (CS02 = 0, CS01 = 1, CS00 = 0).
		count_limit = 125; // for prescaler of 8, a count of 125 will require 1 msec
		break;
	}
	
	while (number_of_msec > 0) {
		TCCR0A = 0x00; // clears WGM00 and WGM01 (bits 0 and 1) to ensure Timer/Counter is in normal mode.
		TCNT0 = 0;  // preload value for testing on count = 250
		TCCR0B =  register_B_setting;  // Start TIMER0 with the settings defined above
		while (TCNT0 < count_limit); // exits when count = the required limit for a 1 msec delay
		TCCR0B = 0x00; // Stop TIMER0
		number_of_msec--;
	}
} // end wait()
