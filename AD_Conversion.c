/*
* Lab_5.c
*
* Created: 2/15/2025 12:27:20 PM
* Author : James Hill
*/
#include <avr/io.h>

//**Global Variables**//
char ADC_Value;

int main(void)
{
	//**Setup**//
	
	//Setting All register C pin directions
	DDRC=0b00011111;    //Setting pins 0-4 at output and the rest as input
	PORTC=0b00011111;   //Setting all LEDS to off initially. Wired as a sink so 1=off and 0=on
	
	//--Setting up the ADC--//
	PRR=0b00000000;      //Clearing the power reduction in PRR register
	
	ADCSRA=0b10000111;   //Setting ADPS0-ADPS2 to 1 to allow for 128 prescaler divider
	//Set ADEN(bit 7) to 1 to enable the ADC
	
	ADMUX=0b01100101;    //Bits 7-6: 0<<REFS1 || 1<<REFS0 so AVCC and AREF are invoked
					     //Bit 5: 1<<ADLAR -->Left justification
	                     //Bit 4: not used yet
	                     //Bits 3-0: 0<<MUX3 || 1<<MUX2 || 0<<MUX1 || 1<<MUX0 --> Done to invoke ADC5 which is connected to PC5
	
	/*======================
	/PROGRAM LOOP
	/=======================*/
	while(1){
		
		//Reading analog input
		ADCSRA = ADCSRA | 0b01000000;            //Setting bit 6 to a 1 to start A/D conversion(ADSC)
		
		while((ADCSRA & 0b00010000) == 0);       //Polling ADIF at bit 4 to check in conversion is done
		
		
		ADC_Value=ADCH;					 //Keeping high byte of the 10 bit result Recall when reading the high value the range goes from 0-1023 to 0-255. Using the high byte so that resolution is reduced.
		

		//ADC_value=(input voltage/5[V])*255 as it has a 5 volt reference. USING THIS EQUATION FOR ALL
		
		if (ADC_Value <= 51) {              // 0V - 1V
			PORTC = 0b00011110;             // LED at PC0 on (0), others off (1)
		}
		else if (ADC_Value <= 102) {        // 1V - 2V
			PORTC = 0b00011101;				// LED at PC1 on (0), others off (1)
		}
		else if (ADC_Value <= 153) {        // 2V - 3V
			PORTC = 0b00011011;				// LED at PC2 on (0), others off (1)
		}
		else if (ADC_Value <= 204) {        // 3V - 4V
			PORTC = 0b00010111;             // LED at PC3 on (0), others off (1)
		}
		else if(ADC_Value<=255){            // 4V - 5V
			PORTC = 0b00001111;             // LED at PC4 on (0), others off (1)
		}
	} //End of infinite while loop
} //End of main function
