// Compiler: Mplabx_xc8_16Fe  Device: 16f1826

#define _XTAL_FREQ 20000000UL 

#include <xc.h>
#include <pic16f1826.h>

void __interrupt() myisr() // Interrupt (ISR)
{
    if( RCIF ){
        PORTA = RCREG;
        PORTB ^= 1<<4; // Toggle PORTB4
    }
}

void main()
{
    ANSELA = 0x00;
    ANSELB = 0x00;

    PORTB = 0;
    PORTA = 0;
    TRISB = 0b11001111; // PORTB4,5 Outputs
    TRISA = 0;
    //WPU = 0;      // Clear pullups
    
    SPBRG = 32; // 9600 bauds
    SPEN = 1;     // Enable Usart
    CREN = 1;     // Enable receiver

    RCIE = 1;     // Enable RC Interrupt
    PEIE = 1;     // Enable periferical Ints. INTCON
    GIE  = 1;     // Enable global ints. INTCON
    
    TMR2ON = 1;   // Start Timer2
    
    while(1)
    {
        __delay_ms( 500 );
        PORTB ^= 1<<5; // Toggle PORTB5
    }
}
