// Compiler: Mplabx_xc8_16Fe  Device: 16f1826

#define _XTAL_FREQ 20000000UL 
#define BAUD9600 (_XTAL_FREQ/(long)(64 * 9600)) - 1

#include <xc.h>
#include <pic16f1826.h>

char data = 1;

void __interrupt() myisr() // Interrupt (ISR)
{
    if( TXIF )
    {
        TXIE = 0;     // Disable TX Interrupt
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
    
    SPBRG = 32;   // 9600 bauds
    SPEN = 1;     // Enable Usart
    TXEN = 1;     // Enable Transmitter
    
    PEIE = 1;     // Enable periferical Ints. INTCON
    GIE  = 1;     // Enable global ints. INTCON

    __delay_ms( 100 );
    
    while(1)
    {
        PORTA = data;
        TXREG = data;
        TXIE = 1;     // Enable TX Interrupt
        data += 1;
        if( data == 10 ) data = 0;
        
        __delay_ms( 500 );
        PORTB ^= 1<<5; // Toggle PORTB5
    }
}
