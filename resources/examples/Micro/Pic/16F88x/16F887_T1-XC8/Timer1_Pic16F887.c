// device = pic16f887

/*
 * File:   int_tmr1.c
 * Author: chicodorea
 *
 * Created on October 10, 2021, 9:08 AM
 *
 * MPLAB X IDE v5.50
 * XC8 v2.32
 * Gnome Shell 3.38.4
 * Debian 11 Bulleyes - 5.10.46-5 (2021-09-23) GNU/Linux
 * Linux 5.10.0.8-amd64 SMP
 *
 */

//#define _XTAL_FREQ 20000000UL       // crysTAL FREQuency

// PIC16F887 Configuration Bit Settings
// CONFIG1

//#pragma config MCLRE = ON           // Master CLeaR function Enable
//#pragma config CP    = OFF          // Code Protection bit
//#pragma config CPD   = OFF          // Data Code Protection bit
//#pragma config BOREN = ON           // BrownOut REset SelectioN bit
//#pragma config IESO  = ON           // Internal External SwitchOver bit
//#pragma config FCMEN = ON           // Fails-safe Clock Monitor ENable
//#pragma config LVP   = OFF          // Low Voltage Programming enable

// CONFIG2
//#pragma config BOR4V = BOR40V       // Brown-Out Reset selection bit
//#pragma config WRT   = OFF          // flash program memory self WRiTe enable bits

// include files

#include <pic16f887.h>              // header file for PIC16F887

typedef unsigned int cfg;
cfg __at(0x2007) __CONFIG = (_HS_OSC & _WDT_OFF & _PWRTE_OFF & _CP_OFF);

// function prototypes

// interrupt subroutine (ISR)
void myisr(void) __interrupt 0 {
    // timer1 interruption
    if( TMR1IF == 1 )
    {
        PORTC ^= 0b00001100;
        //TMR1H = 0;
        //TMR1L = 0;
        TMR1IF = 0;
    }
}

// definition of global variables

// main program
void main(){
    // definition of local variable
    
    // variable initialization
    
    // general settings
    ANSEL  = 0x00;
    ANSELH = 0x00;
    C1ON   = 0;
    C2ON   = 0;
    TRISB  = 0x00;
    PORTB  = 0x00;
    TRISC  = 0x00;
    PORTC  = 0x00;
    
    // configuration of timer 1 and interruption
    TMR1IF = 0;
    TMR1H = 0;
    TMR1L = 0;
    
    TMR1CS  = 0;
    T1CKPS0 = 1;
    T1CKPS1 = 1;
    TMR1IE  = 1;
    
    PEIE   = 1;
    GIE    = 1;            // General Interrupt Enable
    TMR1ON = 1;
    
    // infinite loop
    while (1){
        if(TMR1H == 0x00 && TMR1L == 0x00){
            PORTC ^= 0b00000011;
        }
    }
}

