#ifndef F_CPU
#define F_CPU 100000UL
#endif

#include<avr/io.h>
#include<avr/interrupt.h>

ISR (INT0_vect)        // Interrupt service routine 
{
  PORTB ^= (1<<PB1);    // Toggling the PB2 pin 
}

ISR (PCINT0_vect)        // Interrupt service routine 
{
  PORTB ^= (1<<PB0);    // Toggling the PB2 pin 
}


void external_interrupt()
{
  GIMSK |= (1<<INT0);     // enabling the INT0 (external interrupt) 
  MCUCR |= (1<<ISC01);    // Configuring as falling edge 
}

void pin_change_interrupt()
{
  GIMSK|= (1<<PCIE);
  PCMSK|= (1<<PCINT1);
  MCUCR |= (1<<ISC00);    // Configuring as falling edge 
}

int main()
{
  DDRB |= (1<<PB1)|(1<<PB0);     // set PB2 as output(LED)
  DDRB &= ~(1 << PB2);  //set PB2 as input
  PORTB |= (1 << PB2);  //activate pull-up resistor for PB2

  external_interrupt();
  pin_change_interrupt();

  sei();     //enabling global interrupt

  while(1)
  {

  }
}