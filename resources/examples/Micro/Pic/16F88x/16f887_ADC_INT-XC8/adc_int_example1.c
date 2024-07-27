// Compiler: Mplabx_xc8  Device: 16f887

/*
 * File:   adc_int_example1.c
 * Author: chicodorea
 *
 * Created on October 20, 2021, 6:37 AM
 * 
 * MPLAB X IDE v5.50
 * XC8 v2.32
 * SimulIDE 0.4.15 (September 13: Updated to SR7-Stable) 
 * Gnome Shell 3.38.4
 * Debian 11 Bulleyes - 5.10.46-5 (2021-09-23) GNU/Linux
 * Linux 5.10.0.8-amd64 SMP 
 * 
 */

#define _XTAL_FREQ 20000000UL       // crysTAL FREQuency

// PIC16F887 Configuration Bit Settings
// CONFIG1
#pragma config FOSC  = HS           // Frequency OSCillator selection
#pragma config WDTE  = OFF          // WatchDog Time Enable
#pragma config PWRTE = OFF          // PoWeR-up Timer Enable
#pragma config MCLRE = ON           // Master CLeaR function Enable
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
unsigned int num_adc;

// Interrupsion SubRoutine (ISR)
void __interrupt() myisr(){
    if(PIR1bits.ADIF){
        PIR1bits.ADIF = 0;
        num_adc = ((ADRESH << 8 ) | (ADRESL));
        PORTB = (num_adc & 0xFF);               // 8 bits LSB
        PORTD = ((num_adc>>8) & 0x03);          // 2 bits MSB
    }
}

// main program
void main(){
    // definition of local variable

    // variable initialization
    num_adc = 0;
    
    // general settings
    ANSEL  = 0x01;
    ANSELH = 0x00;
    C1ON   = 0;
    C2ON   = 0;
    TRISA  = 0xFF;
    TRISB  = 0x00;
    TRISD  = 0xF8;
    PORTB  = 0x00;
    PORTD  = 0XF8;
    
    // configuration of adc
    ADCON0bits.CHS   = 0x00;        // AN0     - channel selection
    ADCON1bits.VCFG0 = 0x00;        // Vdd     - vref+ voltage reference
    ADCON1bits.VCFG1 = 0x00;        // Vss     - vref- voltage reference
    ADCON0bits.ADCS  = 0x02;        // Focs/32 - conversion clock
    ADCON1bits.ADFM  = 0x01;        // Right   - justified A/D conversion result format select
    ADCON0bits.ADON  = 0x01;        // On      - enable adc module
    
    // configuration of adc interrupt
    INTCONbits.GIE   = 1;
    INTCONbits.PEIE  = 1;
    PIR1bits.ADIF    = 0;
    PIE1bits.ADIE    = 1;
    
    // infinite loop
    while (1){
        __delay_us(20);
        ADCON0bits.GO = 1;                      // start conversion
        while(ADCON0bits.GO_DONE);
        PORTD ^= 0b00000100;
    }
}