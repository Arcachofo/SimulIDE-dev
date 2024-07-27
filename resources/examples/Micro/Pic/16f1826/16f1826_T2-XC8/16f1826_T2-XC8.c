// Compiler: Mplabx_xc8_16Fe  Device: 16f1826

#define _XTAL_FREQ 16000000UL 

#include <xc.h>
#include <pic16f1826.h>

void __interrupt() myisr() // Interrupt (ISR)
{
    if( TMR2IF ){
        TMR2IF = 0;
        PORTB ^= 1<<4; // Toggle PORTB4
    }
}

void main()
{
    ANSELA = 0x00;
    ANSELB = 0x00;

    PORTB = 0;
    TRISB = 0b00000000;
    //WPU = 0;      // Clear pullups
    
    T2CON = 0b00000001; // Prescaler = 4, Instr=4MHz, step=1us
    PR2 = 100;          // Freq = 10 KHz (5 KHz toggle)
    
    TMR2IE = 1;   // Enable T2 Match Interrupt
    PEIE = 1;     // Enable periferical Ints. INTCON
    GIE  = 1;     // Enable global ints. INTCON
    
    TMR2ON = 1;   // Start Timer2
    
    while(1)
    {
        __delay_ms( 500 );
        PORTB ^= 1<<5; // Toggle PORTB5
    }
}
