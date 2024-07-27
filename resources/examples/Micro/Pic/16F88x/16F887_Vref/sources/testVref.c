// Compiler: Mplabx_xc8  Device: 16f887

/* ----------------------------------------------------------------------------
 * Arquivo: testVref.c                                        V R E F   T E S T
 * ----------------------------------------------------------------------------
 * UNIVERSIDADE FEDERAL DE SERGIPE
 * CENTRO DE CI�NCIAS EXATAS E TECNONOGIA
 * DEPARTAMENTO DE ENGENHARIA EL�TRICA
 * ----------------------------------------------------------------------------
 * Aluno: Francisco D�rea (chicodorea@academico.ufs.br)
 *        Aracaju/SE
 * ----------------------------------------------------------------------------
 * Criado em 23 de Novembro de 2021, 07:37
 * ----------------------------------------------------------------------------
 * MPLAB X IDE v5.50 - XC8 v2.30
 * Device: PIC16F887
 * Pack PIC16Fxxx_DFP (1.2.33)
 * Debug Tool: PICkit3, Firmware 01.56.09,  OS Version: 01.25.03
 * Gnome Shell 3.38.4 - Zorin OS 16 Core 64 bits - #42~20.04.1-Ubuntu SMP
 * Linux version 5.11.0-38-generic (buildd@lgw01-amd64-041)
 * ----------------------------------------------------------------------------
 */

// configura��o da frequ�ncia do cristal (Fosc)
#define _XTAL_FREQ 20000000UL       // Fosc frequ�ncia do clock de 20 MHz

// configura��o dos fuse bits (Configuration Bits)
#pragma config FOSC = HS            // Frequency OSCillator selection
#pragma config WDTE = OFF           // WatchDog Time Enable
#pragma config PWRTE = ON           // PoWeR-up Timer Enable
#pragma config MCLRE = ON           // Master CLeaR function Enable
#pragma config CP = OFF             // Code Protection bit
#pragma config CPD = OFF            // Data Code Protection bit
#pragma config BOREN = OFF          // BrownOut REset SelectioN bit
#pragma config IESO = OFF           // Internal External SwitchOver bit
#pragma config FCMEN = OFF          // Fails-safe Clock Monitor ENable
#pragma config LVP = OFF            // Low Voltage Programming enable
#pragma config BOR4V = BOR40V       // Brown-Out Reset selection bit
#pragma config WRT = OFF            // flash program memory self WRiTe enable bits

// inclus�o dos arquivos header e outros
#include <xc.h>                     // defini��es de uso do compilador XC8
#include <pic16f887.h>              // header file para hardware do PIC16F887

// mapeamento de pinos do LCD 16x2 HD44780 (modo 4 bits)
#define LCD_RS RD2                  // n�o usar TRIS e PORT nesses pinos
#define LCD_EN RD3
#define LCD_D4 RD4
#define LCD_D5 RD5
#define LCD_D6 RD6
#define LCD_D7 RD7
#define LCD_RS_DIR TRISD2
#define LCD_EN_DIR TRISD3
#define LCD_D4_DIR TRISD4
#define LCD_D5_DIR TRISD5
#define LCD_D6_DIR TRISD6
#define LCD_D7_DIR TRISD7
#include "lcd_sintaxe_mikroc.c"     // biblioteca LCD com sintaxe mikroc

// convers�o para sitaxe mikroc
#define delay_ms(a) __delay_ms(a)   // delay na sintaxe mikroc
#define bit         __bit           // atributo para tipo "bit"

// defini��o de macros e outras diretivas
#define ON          1               // defina o valor ON
#define OFF         0               // defina o valor OFF
#define HIGH        1               // defina o valor HIGH
#define LOW         0               // defina o valor LOW

// prot�tipos de fun��es
void ItoStr(unsigned int valor, unsigned char tam, char * str, char volt, char dot);

/*****************************************************************************
            P  R  O  G  R  A  M  A     P  R  I  N  C  I  P  A  L
******************************************************************************/
void main() {

    // defini��o de vari�veis locais
    unsigned int num_adc;
    float tensao;
    char texto[]="1.234";
        
    // configura��o de registradores gerais    
    ANSEL       = 0x01;             // portas AN0 a AN7 como digitais, exceto RA0
    ANSELH      = 0x00;             // portas AN8 a AN13 como digitais
    C1ON        = 0x00;             // comparador 1 desligado
    C2ON        = 0x00;             // comparador 2 desligado
    TRISA0      = 0x01;             // pino de entrada anal�gica
    
    // Configure ADC module
    ADCON0bits.ADCS = 0x02; // Focs/32 - Select ADC clock
    ADCON0bits.CHS  = 0x00; // AN0     - Select ADC channel
    
    VCFG1 = 0;              // Vss     - Configure vref-
    VCFG0 = 0;              // Vdd     - Configure vref+
    ADFM  = 1;              // Right   - Select result format
    ADON  = 1;              // On      - Turn on ADC module

    Lcd_Init();
    //             1234567890123456
    Lcd_Write(1,1,"ADC in: ");
    Lcd_Write(2,1,"Volt  : ");
    
    // loop infinito
    while(1){
        delay_ms(1);                // aguarde o necss�rio p/ carga capacitor
        ADCON0bits.GO = 1;          // inicie a convers�o
        while(ADCON0bits.GO_DONE);  // aguarde a convers�o ser completada
        num_adc = (unsigned int)((ADRESH << 8 ) | (ADRESL));
        tensao = (float)(num_adc * 0.004887585533);
        ItoStr((unsigned int)(num_adc),4,texto,0,0); 
        Lcd_Write(1,11, texto);
        ItoStr((unsigned int)(tensao * 10000),6,texto,1,1); 
        Lcd_Write(2, 9, texto);
        delay_ms(100);
        
        VCFG0 = RC0;               // Configure Vref+
        VCFG1 = RC1;               // Configure Vref-
    } // fim while

} // fim main

// Fun��o para converter Inteiro para String (ItoStr)
void ItoStr(unsigned int valor,unsigned char tam, char * str, char volt, char dot){
    unsigned int termo;
    unsigned char digito, pos;
    for(int k=0;k<tam;k++) str[k] = ' ';    // limpe o vetor
    pos    = (tam - 1);                     // defina quant. posi��es 
    termo  = valor;                         // trabalhe sem alterar o valor
    while(termo >= 10){                     // avalie cada posi��o decimal
        digito   = (unsigned char) (termo % 10); // pegue um d�gito
        str[pos] = 48 + digito;             // convertar em ascii
        pos--;                              // mude para posi��o anterior
        if(pos==dot && volt){               // chegou na posi��o do ponto
            str[pos]=',';                   // insira o ponto
            pos--;                          // mude para a posi��o anterior
        } // fim if pos
        termo = (unsigned int)(termo / 10.0);  // pegue mais um digito 
    } // fim while
    digito   = (unsigned char) (termo % 10);  // pegue o �ltimo digito
    str[pos] = 48 + digito;                 // converta em ascii
    if(pos>dot && volt){                    // verifique se foi fracion�rio
        str[dot] = ',';                     // insira o ponto
        str[dot-1] = '0';                   // insira o zero a esquerda
    } // fim if pos
    str[tam] = '\0';                        // finalize a string com null
} //  fim fun��o ItoStr