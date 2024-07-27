// Compiler: Mplabx_xc8_16Fe  Device: 16f1826

#define _XTAL_FREQ 20000000UL 

#include <xc.h>
#include <pic16f1826.h>

void __interrupt() myisr() // Interrupt (ISR)
{
    if( TMR1IF ){
        TMR1IF = 0;
        PORTB ^= 1<<4; // Toggle PORTB4
    }
}

void main()
{
    ANSELA = 0x00;
    ANSELB = 0x00;

    PORTB = 0;
    TRISB = 0b01000000; // PORTB6 as input
    
    TMR1CS0  = 0;  // Internal Clock
    TMR1CS1  = 0;  //    F OSC /4
    T1CKPS0 = 1;  // Prescaler
    T1CKPS1 = 1;  //    = 8

    TMR1IE = 1;   // Enable T1 overflow Interrupt
    PEIE = 1;     // Enable periferical Ints. INTCON
    GIE  = 1;     // Enable global ints. INTCON
    
    TMR1ON = 1;   // Enable Timer1
    
    __delay_ms( 2000 );
    
    TMR1CS1 = 1;  // External Clock (100KHz -> 0.7629 Hz)
    
    while(1)
    {
        __delay_ms( 500 );
        PORTB ^= 1<<5; // Toggle PORTB5
    }
}
