// Compiler: Mplabx_xc8_16Fe  Device: 16f1826

#define _XTAL_FREQ 20000000UL       // crysTAL FREQuency 20 MHz
#pragma config FOSC  = HS           // Frequency OSCillator selection
#pragma config WDTE  = OFF          // WatchDog Time Enable
#pragma config MCLRE = OFF          // Master CLeaR function Enable

#include <xc.h>
#include <pic16f1826.h>              // header file for PIC16F887

volatile int num_adc;

void __interrupt() myisr() // Interrupt SubRoutine (ISR)
{
    if( PIR1bits.ADIF )
    {
        PIR1bits.ADIF = 0;
        num_adc = ((ADRESH << 8 ) | (ADRESL));
        PORTB ^= 2;
        //PORTB = (num_adc & 0xFF);               // 8 bits LSB
        //PORTD = ((num_adc>>8) & 0x03);          // 2 bits MSB
    }
}

void main()
{
    num_adc = 0;

    TRISB  = 0x00;
    PORTB  = 0xFF;
    
    ANSELA = 0x01; // PORTA0 as analog
    ANSELB = 0x00;
    
    ADCON1bits.ADFM  = 0x01; // Right justified 
    ADCON1bits.ADCS  = 0x02; // Focs/32 - conversion clock
    ADCON0bits.ADON  = 0x01; // Enable ADC module
    
    // configuration of ADC interrupt
    PIR1bits.ADIF   = 0;
    PIE1bits.ADIE   = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE  = 1;
    
    while (1)
    {
        ADCON0bits.GO = 1;          // start conversion
        //while( ADCON0bits.GO );
        
        PORTB ^= 1;
        __delay_ms( 500 );
        PORTB ^= 1;
        __delay_ms( 500 );
    }
}