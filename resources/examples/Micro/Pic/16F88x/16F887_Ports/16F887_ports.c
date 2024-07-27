// Compiler: Mplabx_xc8  Device: 16f887

/*
 * File:   ports_example1.c
 * Author: chicodorea
 *
 * Created on 24 de Outubro de 2021, 10:55
 * MPLAB X IDE v5.45
 * XC8 v2.30
 * Gnome Shell 3.38.4
 * Zorin OS 16 Core 64 bits - #42~20.04.1-Ubuntu SMP 
 * Linux version 5.11.0-38-generic (buildd@lgw01-amd64-041) 
 * 
 */

#define _XTAL_FREQ 20000000UL       // crysTAL FREQuency 20 MHz

// PIC16F887 Configuration Bit Settings
// CONFIG1
#pragma config FOSC  = HS           // Frequency OSCillator selection
#pragma config WDTE  = OFF          // WatchDog Time Enable
#pragma config PWRTE = OFF          // PoWeR-up Timer Enable
#pragma config MCLRE = OFF          // Master CLeaR function Enable
#pragma config CP    = OFF          // Code Protection bit
#pragma config CPD   = OFF          // Data Code Protection bit
#pragma config BOREN = ON           // BrownOut REset SelectioN bit
#pragma config IESO  = ON           // Internal External SwitchOver bit
#pragma config FCMEN = ON           // Fails-safe Clock Monitor ENable
#pragma config LVP   = OFF          // Low Voltage Programming enable

// CONFIG2
#pragma config BOR4V = BOR40V       // Brown-Out Reset selection bit
#pragma config WRT   = OFF          // flash program memory self WRiTe enable bits

// include files
#include <xc.h>
#include <pic16f887.h>              // header file for PIC16F887

// definition of global variables

// Interrupsion SubRoutine (ISR)
void __interrupt() myisr(){

}

// main program
void main(){
    // definition of local variable

    // variable initialization
    
    // general settings
    ANSEL  = 0x00;
    ANSELH = 0x00;
    C1ON   = 0;
    C2ON   = 0;
    TRISA  = 0x00;
    TRISB  = 0x00;
    TRISC  = 0x00;
    TRISD  = 0x00;
    TRISE  = 0x00;
    PORTA  = 0xFF;
    PORTB  = 0xFF;
    PORTC  = 0xFF;
    PORTD  = 0xFF;
    PORTE  = 0x0F;
    
    // configuration of adc
    /*
    ADCON0bits.CHS   = 0x00;        // AN0     - channel selection
    ADCON1bits.VCFG0 = 0x00;        // Vdd     - vref+ voltage reference
    ADCON1bits.VCFG1 = 0x00;        // Vss     - vref- voltage reference
    ADCON0bits.ADCS  = 0x02;        // Focs/32 - conversion clock
    ADCON1bits.ADFM  = 0x01;        // Right   - justified A/D conversion result format select
    ADCON0bits.ADON  = 0x01;        // On      - enable adc module
     */

    // configuration of timer 2

    
    // infinite loop
    while (1){
        // Start ADC converter
        /*
        __delay_ms(1);
        ADCON0bits.GO = 1;                      // start convertion
        while(ADCON0bits.GO_DONE);
        num_adc = ((ADRESH << 8 ) | (ADRESL));
        */

    }
}