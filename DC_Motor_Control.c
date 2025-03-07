/*
* Lab_6.c
*
* Created: 2/21/2025 12:54:31 PM
* Author : James Hill
*/
#include <avr/io.h>

//**Functions**//
void Motor_Control();

//**Global Variables**//
char ADC_Value;

/*===============================
//MAIN FUNCTION
//===============================*/
int main(void){
	
	//**Setup**//
	//Setting all Register C pin directions
	DDRC=0b00011111;    //Setting pins 0-4 at output and the rest as input
	PORTC=0b00011111;   //Setting all LEDS to off initially. Wired as a sink so 1=off and 0=on
	
	//Setting all Register D pin directions
	DDRD=0b01000011;    //Settings bit 6 to a 1 as it's output(OC0A). This output will connect PD6 to pin 1 on H-bridge.
						//Setting 0 and 1 to output as they are used to control motor direction(PD0 forward and PD1 reverse)
	
	//Setting the duty cycle to 0(Motor off)
	OCR0A=0b00000000;
	
	//Setting up Timer/counter registers
	TCCR0A=0b10000011;   //Bits 7-6: 1<<COM0A1 | 0<<COM0A0(Done to set to non-inverting mode for PWM)
						 //Bits 5-4: 0<<COM0B1 | 0<<COM0B0(At 0 as they are to be ignored since OC0B is not used. Only using OC0A)
						 //Bits 3-2: Ignored for now
						 //Bits 1-0: 1<<WGM01 | 1<<WGM00 Both set to 1 for fast pulse width modulation
	
	TCCR0B=0b00000011;   //Bits 2-0: 0<<CS02 | 1<<CS01 | 1<<CS00; 
						 //Set base PWM frequency (CS02:0 - bits 2-0 = 010 for prescaler of 64, for approximately 1kHz base frequency)

	//--Setting up the ADC--//
	PRR=0b00000000;      //Clearing the power reduction in PRR register
	ADCSRA=0b10000111;   //Setting ADPS0-ADPS2 to 1 to allow for 128 prescaler divider
						 //Set ADEN(bit 7) to 1 to enable the ADC
	
	ADMUX=0b01100101;    //Bits 7-6: 0<<REFS1 | 1<<REFS0 so AVCC and AREF are invoked
						 //Bit 5: 1<<ADLAR -->Left justification
						 //Bit 4: not used yet
						 //Bits 3-0: 0<<MUX3 | 1<<MUX2 | 0<<MUX1 | 1<<MUX0 --> Done to invoke ADC5 which is connected to PC5 on ATMEGA328P
	
	/*==========================
	/PROGRAM LOOP
	/===========================*/
	while(1){
		//Reading analog input
		ADCSRA = ADCSRA | 0b01000000;            //Setting bit 6 to a 1 to start A/D conversion(ADSC)
		
		while((ADCSRA & 0b00010000)==0);         //Polling ADIF at bit 4 to check in conversion is done
		
		ADC_Value=ADCH;					         //Keeping high byte of the 10 bit result Recall when reading the high value the range goes from 0-1023 to 0-255.
		
		//ADC_value=(input voltage/5[V])*255 as it has a 5 volt reference. USING THIS EQUATION FOR ALL

		if (ADC_Value <= 51){               // 0V - 1V
			PORTC = 0b00011110;             // LED at PC0 on (0), others off (1)
		}
		else if (ADC_Value <= 102){         // 1V - 2V
			PORTC = 0b00011101;				// LED at PC1 on (0), others off (1)
		}
		else if (ADC_Value <= 153){         // 2V - 3V
			PORTC = 0b00011011;				// LED at PC2 on (0), others off (1)
		}
		else if (ADC_Value <= 204){         // 3V - 4V
			PORTC = 0b00010111;             // LED at PC3 on (0), others off (1)
		}
		else if(ADC_Value<=255){            // 4V - 5V
			PORTC = 0b00001111;             // LED at PC4 on (0), others off (1)
		}	
		
		//Calling the Motor control function to control motor speed/direction depending on the ADC_Value
		Motor_Control();	
	}//End of infinite while loop
}//End of main function

/*================================
//MOTOR CONTROL FUNCTION
//================================*/
void Motor_Control(){
	//Recall the duty cycle(OCR0A) is controlled by a 8 bit register(0-255) so 50% of it is 128	
	char Margin=10;											  //Creating margin variable to change ADC_Value range slightly in an attempt to account for friction in the motor                 
	
	if (ADC_Value < 128-Margin){                              // 0V - 2.5V (Reverse: 50%-0% duty cycle)
		PORTD=(PORTD & 0b11111100) | 0b00000010;			  // PD1 = HIGH, PD0 = LOW (Reverse)
		OCR0A=128-ADC_Value;                                  //Decreases the duty cycle from 50% to 
	}
	
	else if (ADC_Value >= 128-Margin && ADC_Value <= 128+Margin){                        // ~2.5V (Stop the motor completely)
		PORTD &=0b11111100;																 // PD1 = LOW, PD0 = LOW (Stop motor)
		OCR0A=0;																		 // Turning PWM off
	}
	
	else if (ADC_Value > 128+Margin){                         // 2.5V - 5V (Forward: 0%-50% duty cycle)
		PORTD=(PORTD & 0b11111100) | 0b00000001;			  // PD1 = LOW, PD0 = HIGH (Forward)
		OCR0A=ADC_Value-128;                                  // Increase from 0%-50% duty cycle                        
	}
}//End of motor control function
