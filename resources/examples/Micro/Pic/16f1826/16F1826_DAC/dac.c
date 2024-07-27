/*
 *  Microcontroller: 16F1826 @ 32MHz
 *  Board: Breadboard
 *  File:  dac.c
 *  Lernsituation: Alarmanlage
 *  Sinussignale @ 649Hz und 476Hz
 */

#include <pic14regs.h>
#include <stdint.h> 

//Sinussignale

char sin649[22]={16,20,24,28,30,31,31,30,28,24,20,16,12,8,4,2,1,1,2,4,8,12}; 						//649Hz

char sin476[30]={16,19,22,25,27,29,30,31,31,30,29,27,25,22,19,16,13,10,7,5,3,2,1,1,2,3,5,7,10,13};	//476Hz


// CONFIG REGISTER 1 settings
__code unsigned short int __at (_CONFIG1) __configword1 =  
				_FOSC_INTOSC  & 			// Oscillator Selection	INTOSC oscillator: I/O function on CLKIN pin
				_WDTE_OFF     & 			// Watchdog Timer disabled					
				_PWRTE_OFF    &				// Power-up Timer disabled
				_MCLRE_ON     &				// MCLR Reset enabled
				_CP_OFF       & 			// Flash Program Memory Code Protection	Program memory code protection is disabled
				_BOREN_OFF    &  			// Brown-out reset disabled
				_IESO_OFF     &				// Internal-External switchover mode is disabled
				_CLKOUTEN_OFF; 				// CLKOUT function is disabled
						 
// CONFIG REGISTER 2 settings
__code unsigned short int __at (_CONFIG2) __configword2 =  
				_PLLEN_ON    &				// 4xPLL-enable ON
				_STVREN_ON   &				// Stack Overflow/Underflow Reset Enable
				_BORV_LO     &        		// Brown out reset voltage low trip point selected
				_DEBUG_OFF   &        		// Background debugger disabled	
				_LVP_ON;					// Low voltage programming enabled  	 				

//Input
//#define TASTER_1 RA2						// Macro definition								

//Output
#define PROBE  RB0															

//Constants
#define ON  1
#define OFF 0
#define PRESSED   1
#define UNPRESSED 0

//Global variables
char counter=0;

static void irqHandler(void) __interrupt 0
{
	if(T0IF)
	{
		PROBE = 1;
		DACCON1=sin649[counter++];  // 649Hz		
		//DACCON1=sin476[counter++];  // 476Hz		
		TMR0 = 118; 		// Berechnet: TMR0 = 116;
		T0IF=0;
		//Für 649Hz
		if(counter==22)
			counter=0;
		//Für 476Hz
		//if(counter==30)
		//	counter=0;
		PROBE = 0;
	}
}

void main(void)
{
	int i=0;
																		// OSCCON-Reg. on page 65 (datasheet PIC16F127)
																		// OSCCON-Reg. on page 65 (datasheet PIC16F127)	
    OSCCON=0b01110000;													// Bit 7:  0 SPLLEN is ignored
                                                                        // Bit6-3: 1110 16F1827 (8MHz + PLL ON) = 32MHz
                                                                        // Bit2:   Unimplemented
                                                                        // Bit1=0,Bit0=0:Clock implemented by FOSC<2:0> in config. word 1 
          
    FVRCON=0b10001000;      
          
          
                                                                        // Bit1-0: 00 Internal oscillator (word 1)
																		
	DACCON0=0b11101000;													// DACCON-Reg. on page 156 (datasheet PIC 16F1827)			
																		// Bit 7:  1 = DAC is enabled
																		// BIT 6:  1 = DAC positive reference source selected
																		// Bit 5:  1 = DAC voltage level is also an output  on the DACOUT pin
																		// Bit 4:  Unimplemented
																		// Bit 3-2:10=FVRBUFFER2  		
																		// Bit 1:  Unimplemented																		
																		// Bit 0:  0=VSS
																										
																		// TRIS-Reg. on page 122 (datasheet PIC16F1827)
	TRISA=0b11111011;													// RA7 RA6 RA5 RA4 RA3 RA2 RA1 RA0
                                                                        //  1   1   1   1   1   0   1   1
																		// 0=Output
                                                                        // 1=Input
                                                                        // RA2=DACOUT+
																	
																		// TRIS-Reg. on page 122 (datasheet PIC16F1827)
	TRISB=0b11111110;													// RB7 RB6 RB5 RB4 RB3 RB2 RB1 RB0
                                                                        //  1   1   1   1   1   1   1   1
																		// 0=Output
                                                                        // 1=Input
                                                                        // RB0=Probe

																		// OPTION Reg. on page 176 (datasheet PIC16F1827)
	OPTION_REG=0b11000001;                                          	// BIT2=0,BIT1=0,BIT0=1 Prescaler at 1:4
																		// BIT3=0 PSA=0 Prescaler is assigned to the Timer0 module
																		// BIT4=0 Increment on low-to-high transition
																		// BIT5=0 TMR0 CLock select bit: Internal instruction cycle clock (FOSC/4)
																		// BIT6=1 Interrupt on rising edge of INT pin
																		// BIT7=0 Weak pull ups are enabled by WPUx-Register vaule

																		// ANSELA-Reg. (ADC) on page 123 (datasheet PIC16F1827)
	ANSELA=0b00000000;													// 7 6 5   4    3    2     1     0     BIT
                                                                        // - - -  RA4  RA3   RA2   RA1   RA0
                                                                        // 1 = Analog
                                                                        // 0 = Digital

																		// ANSELB-Reg. on page 128 (datasheet PIC16F1827)
	ANSELB=0b00000000;													//  7   6   5   4   3   2   1   0     BIT
                                                                        // RB7 RB6 RB5 RB4 RB3 RB2 RB1  /
                                                                        // 1 = Analog
                                                                        // 0 = Digital

	WPUA=0b00000000;													// Weak pull up resistors PORTA disabled
	WPUB=0b00000000;													// Weak pull up resistors PORTB disabled


	GIE=1;
	TMR0IE=1;
	
	PROBE = 0;
	DACCON1=0;
	
	while(1) 
	{
		//Tut irgend etwas
		for(i=0;i <2048; i++);
	}
}
