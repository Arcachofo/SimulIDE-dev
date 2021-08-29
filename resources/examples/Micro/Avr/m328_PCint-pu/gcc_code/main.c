// Include von Header-Dateien
#include <avr/interrupt.h>
 
// Konstanten
#define SET_BIT(PORT, BIT)  ((PORT) |=  (1 << (BIT))) // Port-Bit Zustand setzen
#define CLR_BIT(PORT, BIT)  ((PORT) &= ~(1 << (BIT))) // Port-Bit Zustand loeschen
#define TGL_BIT(PORT, BIT)  ((PORT) ^=  (1 << (BIT))) // Port-Bit Zustand wechseln (toggle)
  
int main(void)
{
	DDRB= 0b00001000;
	PORTB=0b00000000;
	PORTD=0b00000001;

    SET_BIT(PCICR , PCIE2);     // Interrupt, if PC at Port D detected
    SET_BIT(PCMSK2, 0);         // Interrupt mask: only interupt, when chang at pin0 
    sei();
 
    TGL_BIT(PORTB, PORTB3);                             // DEBUG
    while (1){};
}
 
//////////////////////////////////////////////////
// Pin Change Interupt Handler
//////////////////////////////////////////////////
ISR(PCINT2_vect)
{
    TGL_BIT(PORTB, PORTB3);                             // DEBUG
}
