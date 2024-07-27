# 1 "/media/user/software/simulide/tests/MCU/pic/0_Test/16f1826/16f1826_Usart/16f1826_Rx.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 288 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/language_support.h" 1 3
# 2 "<built-in>" 2
# 1 "/media/user/software/simulide/tests/MCU/pic/0_Test/16f1826/16f1826_Usart/16f1826_Rx.c" 2




# 1 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/xc.h" 1 3
# 18 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/xc.h" 3
extern const char __xc8_OPTIM_SPEED;

extern double __fpnormalize(double);



# 1 "/opt/microchip/xc8/v2.32/pic/include/c99/xc8debug.h" 1 3



# 1 "/opt/microchip/xc8/v2.32/pic/include/c99/stdlib.h" 1 3



# 1 "/opt/microchip/xc8/v2.32/pic/include/c99/musl_xc8.h" 1 3
# 5 "/opt/microchip/xc8/v2.32/pic/include/c99/stdlib.h" 2 3





# 1 "/opt/microchip/xc8/v2.32/pic/include/c99/features.h" 1 3
# 11 "/opt/microchip/xc8/v2.32/pic/include/c99/stdlib.h" 2 3
# 21 "/opt/microchip/xc8/v2.32/pic/include/c99/stdlib.h" 3
# 1 "/opt/microchip/xc8/v2.32/pic/include/c99/bits/alltypes.h" 1 3
# 18 "/opt/microchip/xc8/v2.32/pic/include/c99/bits/alltypes.h" 3
typedef long int wchar_t;
# 122 "/opt/microchip/xc8/v2.32/pic/include/c99/bits/alltypes.h" 3
typedef unsigned size_t;
# 168 "/opt/microchip/xc8/v2.32/pic/include/c99/bits/alltypes.h" 3
typedef __int24 int24_t;
# 204 "/opt/microchip/xc8/v2.32/pic/include/c99/bits/alltypes.h" 3
typedef __uint24 uint24_t;
# 22 "/opt/microchip/xc8/v2.32/pic/include/c99/stdlib.h" 2 3

int atoi (const char *);
long atol (const char *);
long long atoll (const char *);
double atof (const char *);

float strtof (const char *restrict, char **restrict);
double strtod (const char *restrict, char **restrict);
long double strtold (const char *restrict, char **restrict);



long strtol (const char *restrict, char **restrict, int);
unsigned long strtoul (const char *restrict, char **restrict, int);
long long strtoll (const char *restrict, char **restrict, int);
unsigned long long strtoull (const char *restrict, char **restrict, int);

int rand (void);
void srand (unsigned);

          void abort (void);
int atexit (void (*) (void));
          void exit (int);
          void _Exit (int);

void *bsearch (const void *, const void *, size_t, size_t, int (*)(const void *, const void *));

__attribute__((nonreentrant)) void qsort (void *, size_t, size_t, int (*)(const void *, const void *));

int abs (int);
long labs (long);
long long llabs (long long);

typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;
typedef struct { long long quot, rem; } lldiv_t;

div_t div (int, int);
ldiv_t ldiv (long, long);
lldiv_t lldiv (long long, long long);

typedef struct { unsigned int quot, rem; } udiv_t;
typedef struct { unsigned long quot, rem; } uldiv_t;
udiv_t udiv (unsigned int, unsigned int);
uldiv_t uldiv (unsigned long, unsigned long);





size_t __ctype_get_mb_cur_max(void);
# 5 "/opt/microchip/xc8/v2.32/pic/include/c99/xc8debug.h" 2 3







#pragma intrinsic(__builtin_software_breakpoint)
extern void __builtin_software_breakpoint(void);
# 24 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/xc.h" 2 3



# 1 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/pic.h" 1 3




# 1 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/htc.h" 1 3



# 1 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/xc.h" 1 3
# 5 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/htc.h" 2 3
# 6 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/pic.h" 2 3







# 1 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/pic_chip_select.h" 1 3
# 384 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/pic_chip_select.h" 3
# 1 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 1 3
# 44 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
# 1 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/__at.h" 1 3
# 45 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 2 3







extern volatile unsigned char INDF0 __attribute__((address(0x000)));

__asm("INDF0 equ 00h");


typedef union {
    struct {
        unsigned INDF0 :8;
    };
} INDF0bits_t;
extern volatile INDF0bits_t INDF0bits __attribute__((address(0x000)));
# 72 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char INDF1 __attribute__((address(0x001)));

__asm("INDF1 equ 01h");


typedef union {
    struct {
        unsigned INDF1 :8;
    };
} INDF1bits_t;
extern volatile INDF1bits_t INDF1bits __attribute__((address(0x001)));
# 92 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char PCL __attribute__((address(0x002)));

__asm("PCL equ 02h");


typedef union {
    struct {
        unsigned PCL :8;
    };
} PCLbits_t;
extern volatile PCLbits_t PCLbits __attribute__((address(0x002)));
# 112 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char STATUS __attribute__((address(0x003)));

__asm("STATUS equ 03h");


typedef union {
    struct {
        unsigned C :1;
        unsigned DC :1;
        unsigned Z :1;
        unsigned nPD :1;
        unsigned nTO :1;
    };
    struct {
        unsigned CARRY :1;
        unsigned :1;
        unsigned ZERO :1;
    };
} STATUSbits_t;
extern volatile STATUSbits_t STATUSbits __attribute__((address(0x003)));
# 171 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned short FSR0 __attribute__((address(0x004)));



extern volatile unsigned char FSR0L __attribute__((address(0x004)));

__asm("FSR0L equ 04h");


typedef union {
    struct {
        unsigned FSR0L :8;
    };
} FSR0Lbits_t;
extern volatile FSR0Lbits_t FSR0Lbits __attribute__((address(0x004)));
# 195 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char FSR0H __attribute__((address(0x005)));

__asm("FSR0H equ 05h");


typedef union {
    struct {
        unsigned FSR0H :8;
    };
} FSR0Hbits_t;
extern volatile FSR0Hbits_t FSR0Hbits __attribute__((address(0x005)));
# 215 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned short FSR1 __attribute__((address(0x006)));



extern volatile unsigned char FSR1L __attribute__((address(0x006)));

__asm("FSR1L equ 06h");


typedef union {
    struct {
        unsigned FSR1L :8;
    };
} FSR1Lbits_t;
extern volatile FSR1Lbits_t FSR1Lbits __attribute__((address(0x006)));
# 239 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char FSR1H __attribute__((address(0x007)));

__asm("FSR1H equ 07h");


typedef union {
    struct {
        unsigned FSR1H :8;
    };
} FSR1Hbits_t;
extern volatile FSR1Hbits_t FSR1Hbits __attribute__((address(0x007)));
# 259 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char BSR __attribute__((address(0x008)));

__asm("BSR equ 08h");


typedef union {
    struct {
        unsigned BSR0 :1;
        unsigned BSR1 :1;
        unsigned BSR2 :1;
        unsigned BSR3 :1;
        unsigned BSR4 :1;
    };
    struct {
        unsigned BSR :5;
    };
} BSRbits_t;
extern volatile BSRbits_t BSRbits __attribute__((address(0x008)));
# 311 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char WREG __attribute__((address(0x009)));

__asm("WREG equ 09h");


typedef union {
    struct {
        unsigned WREG0 :8;
    };
} WREGbits_t;
extern volatile WREGbits_t WREGbits __attribute__((address(0x009)));
# 331 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char PCLATH __attribute__((address(0x00A)));

__asm("PCLATH equ 0Ah");


typedef union {
    struct {
        unsigned PCLATH :7;
    };
} PCLATHbits_t;
extern volatile PCLATHbits_t PCLATHbits __attribute__((address(0x00A)));
# 351 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char INTCON __attribute__((address(0x00B)));

__asm("INTCON equ 0Bh");


typedef union {
    struct {
        unsigned IOCIF :1;
        unsigned INTF :1;
        unsigned TMR0IF :1;
        unsigned IOCIE :1;
        unsigned INTE :1;
        unsigned TMR0IE :1;
        unsigned PEIE :1;
        unsigned GIE :1;
    };
    struct {
        unsigned :2;
        unsigned T0IF :1;
        unsigned :2;
        unsigned T0IE :1;
    };
} INTCONbits_t;
extern volatile INTCONbits_t INTCONbits __attribute__((address(0x00B)));
# 429 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char PORTA __attribute__((address(0x00C)));

__asm("PORTA equ 0Ch");


typedef union {
    struct {
        unsigned RA0 :1;
        unsigned RA1 :1;
        unsigned RA2 :1;
        unsigned RA3 :1;
        unsigned RA4 :1;
        unsigned RA5 :1;
        unsigned RA6 :1;
        unsigned RA7 :1;
    };
} PORTAbits_t;
extern volatile PORTAbits_t PORTAbits __attribute__((address(0x00C)));
# 491 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char PORTB __attribute__((address(0x00D)));

__asm("PORTB equ 0Dh");


typedef union {
    struct {
        unsigned RB0 :1;
        unsigned RB1 :1;
        unsigned RB2 :1;
        unsigned RB3 :1;
        unsigned RB4 :1;
        unsigned RB5 :1;
        unsigned RB6 :1;
        unsigned RB7 :1;
    };
} PORTBbits_t;
extern volatile PORTBbits_t PORTBbits __attribute__((address(0x00D)));
# 553 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char PIR1 __attribute__((address(0x011)));

__asm("PIR1 equ 011h");


typedef union {
    struct {
        unsigned TMR1IF :1;
        unsigned TMR2IF :1;
        unsigned CCP1IF :1;
        unsigned SSP1IF :1;
        unsigned TXIF :1;
        unsigned RCIF :1;
        unsigned ADIF :1;
        unsigned TMR1GIF :1;
    };
} PIR1bits_t;
extern volatile PIR1bits_t PIR1bits __attribute__((address(0x011)));
# 615 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char PIR2 __attribute__((address(0x012)));

__asm("PIR2 equ 012h");


typedef union {
    struct {
        unsigned :3;
        unsigned BCL1IF :1;
        unsigned EEIF :1;
        unsigned C1IF :1;
        unsigned C2IF :1;
        unsigned OSFIF :1;
    };
} PIR2bits_t;
extern volatile PIR2bits_t PIR2bits __attribute__((address(0x012)));
# 660 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char TMR0 __attribute__((address(0x015)));

__asm("TMR0 equ 015h");


typedef union {
    struct {
        unsigned TMR0 :8;
    };
} TMR0bits_t;
extern volatile TMR0bits_t TMR0bits __attribute__((address(0x015)));
# 680 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned short TMR1 __attribute__((address(0x016)));

__asm("TMR1 equ 016h");




extern volatile unsigned char TMR1L __attribute__((address(0x016)));

__asm("TMR1L equ 016h");


typedef union {
    struct {
        unsigned TMR1L :8;
    };
} TMR1Lbits_t;
extern volatile TMR1Lbits_t TMR1Lbits __attribute__((address(0x016)));
# 707 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char TMR1H __attribute__((address(0x017)));

__asm("TMR1H equ 017h");


typedef union {
    struct {
        unsigned TMR1H :8;
    };
} TMR1Hbits_t;
extern volatile TMR1Hbits_t TMR1Hbits __attribute__((address(0x017)));
# 727 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char T1CON __attribute__((address(0x018)));

__asm("T1CON equ 018h");


typedef union {
    struct {
        unsigned TMR1ON :1;
        unsigned :1;
        unsigned nT1SYNC :1;
        unsigned T1OSCEN :1;
        unsigned T1CKPS0 :1;
        unsigned T1CKPS1 :1;
        unsigned TMR1CS0 :1;
        unsigned TMR1CS1 :1;
    };
    struct {
        unsigned :4;
        unsigned T1CKPS :2;
        unsigned TMR1CS :2;
    };
} T1CONbits_t;
extern volatile T1CONbits_t T1CONbits __attribute__((address(0x018)));
# 799 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char T1GCON __attribute__((address(0x019)));

__asm("T1GCON equ 019h");


typedef union {
    struct {
        unsigned T1GSS0 :1;
        unsigned T1GSS1 :1;
        unsigned T1GVAL :1;
        unsigned T1GGO :1;
        unsigned T1GSPM :1;
        unsigned T1GTM :1;
        unsigned T1GPOL :1;
        unsigned TMR1GE :1;
    };
    struct {
        unsigned T1GSS :2;
    };
} T1GCONbits_t;
extern volatile T1GCONbits_t T1GCONbits __attribute__((address(0x019)));
# 869 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char TMR2 __attribute__((address(0x01A)));

__asm("TMR2 equ 01Ah");


typedef union {
    struct {
        unsigned TMR2 :8;
    };
} TMR2bits_t;
extern volatile TMR2bits_t TMR2bits __attribute__((address(0x01A)));
# 889 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char PR2 __attribute__((address(0x01B)));

__asm("PR2 equ 01Bh");


typedef union {
    struct {
        unsigned PR2 :8;
    };
} PR2bits_t;
extern volatile PR2bits_t PR2bits __attribute__((address(0x01B)));
# 909 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char T2CON __attribute__((address(0x01C)));

__asm("T2CON equ 01Ch");


typedef union {
    struct {
        unsigned T2CKPS0 :1;
        unsigned T2CKPS1 :1;
        unsigned TMR2ON :1;
        unsigned T2OUTPS0 :1;
        unsigned T2OUTPS1 :1;
        unsigned T2OUTPS2 :1;
        unsigned T2OUTPS3 :1;
    };
    struct {
        unsigned T2CKPS :2;
        unsigned :1;
        unsigned T2OUTPS :4;
    };
} T2CONbits_t;
extern volatile T2CONbits_t T2CONbits __attribute__((address(0x01C)));
# 980 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char CPSCON0 __attribute__((address(0x01E)));

__asm("CPSCON0 equ 01Eh");


typedef union {
    struct {
        unsigned T0XCS :1;
        unsigned CPSOUT :1;
        unsigned CPSRNG0 :1;
        unsigned CPSRNG1 :1;
        unsigned :3;
        unsigned CPSON :1;
    };
    struct {
        unsigned :2;
        unsigned CPSRNG :2;
    };
} CPSCON0bits_t;
extern volatile CPSCON0bits_t CPSCON0bits __attribute__((address(0x01E)));
# 1034 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char CPSCON1 __attribute__((address(0x01F)));

__asm("CPSCON1 equ 01Fh");


typedef union {
    struct {
        unsigned CPSCH0 :1;
        unsigned CPSCH1 :1;
        unsigned CPSCH2 :1;
        unsigned CPSCH3 :1;
    };
    struct {
        unsigned CPSCH :4;
    };
} CPSCON1bits_t;
extern volatile CPSCON1bits_t CPSCON1bits __attribute__((address(0x01F)));
# 1080 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char TRISA __attribute__((address(0x08C)));

__asm("TRISA equ 08Ch");


typedef union {
    struct {
        unsigned TRISA0 :1;
        unsigned TRISA1 :1;
        unsigned TRISA2 :1;
        unsigned TRISA3 :1;
        unsigned TRISA4 :1;
        unsigned TRISA5 :1;
        unsigned TRISA6 :1;
        unsigned TRISA7 :1;
    };
} TRISAbits_t;
extern volatile TRISAbits_t TRISAbits __attribute__((address(0x08C)));
# 1142 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char TRISB __attribute__((address(0x08D)));

__asm("TRISB equ 08Dh");


typedef union {
    struct {
        unsigned TRISB0 :1;
        unsigned TRISB1 :1;
        unsigned TRISB2 :1;
        unsigned TRISB3 :1;
        unsigned TRISB4 :1;
        unsigned TRISB5 :1;
        unsigned TRISB6 :1;
        unsigned TRISB7 :1;
    };
} TRISBbits_t;
extern volatile TRISBbits_t TRISBbits __attribute__((address(0x08D)));
# 1204 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char PIE1 __attribute__((address(0x091)));

__asm("PIE1 equ 091h");


typedef union {
    struct {
        unsigned TMR1IE :1;
        unsigned TMR2IE :1;
        unsigned CCP1IE :1;
        unsigned SSP1IE :1;
        unsigned TXIE :1;
        unsigned RCIE :1;
        unsigned ADIE :1;
        unsigned TMR1GIE :1;
    };
} PIE1bits_t;
extern volatile PIE1bits_t PIE1bits __attribute__((address(0x091)));
# 1266 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char PIE2 __attribute__((address(0x092)));

__asm("PIE2 equ 092h");


typedef union {
    struct {
        unsigned :3;
        unsigned BCL1IE :1;
        unsigned EEIE :1;
        unsigned C1IE :1;
        unsigned C2IE :1;
        unsigned OSFIE :1;
    };
} PIE2bits_t;
extern volatile PIE2bits_t PIE2bits __attribute__((address(0x092)));
# 1311 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char OPTION_REG __attribute__((address(0x095)));

__asm("OPTION_REG equ 095h");


typedef union {
    struct {
        unsigned PS0 :1;
        unsigned PS1 :1;
        unsigned PS2 :1;
        unsigned PSA :1;
        unsigned TMR0SE :1;
        unsigned TMR0CS :1;
        unsigned INTEDG :1;
        unsigned nWPUEN :1;
    };
    struct {
        unsigned PS :3;
        unsigned :1;
        unsigned T0SE :1;
        unsigned T0CS :1;
    };
} OPTION_REGbits_t;
extern volatile OPTION_REGbits_t OPTION_REGbits __attribute__((address(0x095)));
# 1394 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char PCON __attribute__((address(0x096)));

__asm("PCON equ 096h");


typedef union {
    struct {
        unsigned nBOR :1;
        unsigned nPOR :1;
        unsigned nRI :1;
        unsigned nRMCLR :1;
        unsigned :2;
        unsigned STKUNF :1;
        unsigned STKOVF :1;
    };
} PCONbits_t;
extern volatile PCONbits_t PCONbits __attribute__((address(0x096)));
# 1445 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char WDTCON __attribute__((address(0x097)));

__asm("WDTCON equ 097h");


typedef union {
    struct {
        unsigned SWDTEN :1;
        unsigned WDTPS0 :1;
        unsigned WDTPS1 :1;
        unsigned WDTPS2 :1;
        unsigned WDTPS3 :1;
        unsigned WDTPS4 :1;
    };
    struct {
        unsigned :1;
        unsigned WDTPS :5;
    };
} WDTCONbits_t;
extern volatile WDTCONbits_t WDTCONbits __attribute__((address(0x097)));
# 1504 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char OSCTUNE __attribute__((address(0x098)));

__asm("OSCTUNE equ 098h");


typedef union {
    struct {
        unsigned TUN0 :1;
        unsigned TUN1 :1;
        unsigned TUN2 :1;
        unsigned TUN3 :1;
        unsigned TUN4 :1;
        unsigned TUN5 :1;
    };
    struct {
        unsigned TUN :6;
    };
} OSCTUNEbits_t;
extern volatile OSCTUNEbits_t OSCTUNEbits __attribute__((address(0x098)));
# 1562 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char OSCCON __attribute__((address(0x099)));

__asm("OSCCON equ 099h");


typedef union {
    struct {
        unsigned SCS0 :1;
        unsigned SCS1 :1;
        unsigned :1;
        unsigned IRCF0 :1;
        unsigned IRCF1 :1;
        unsigned IRCF2 :1;
        unsigned IRCF3 :1;
        unsigned SPLLEN :1;
    };
    struct {
        unsigned SCS :2;
        unsigned :1;
        unsigned IRCF :4;
    };
} OSCCONbits_t;
extern volatile OSCCONbits_t OSCCONbits __attribute__((address(0x099)));
# 1634 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char OSCSTAT __attribute__((address(0x09A)));

__asm("OSCSTAT equ 09Ah");


typedef union {
    struct {
        unsigned HFIOFS :1;
        unsigned LFIOFR :1;
        unsigned MFIOFR :1;
        unsigned HFIOFL :1;
        unsigned HFIOFR :1;
        unsigned OSTS :1;
        unsigned PLLR :1;
        unsigned T1OSCR :1;
    };
} OSCSTATbits_t;
extern volatile OSCSTATbits_t OSCSTATbits __attribute__((address(0x09A)));
# 1696 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned short ADRES __attribute__((address(0x09B)));

__asm("ADRES equ 09Bh");




extern volatile unsigned char ADRESL __attribute__((address(0x09B)));

__asm("ADRESL equ 09Bh");


typedef union {
    struct {
        unsigned ADRESL :8;
    };
} ADRESLbits_t;
extern volatile ADRESLbits_t ADRESLbits __attribute__((address(0x09B)));
# 1723 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char ADRESH __attribute__((address(0x09C)));

__asm("ADRESH equ 09Ch");


typedef union {
    struct {
        unsigned ADRESH :8;
    };
} ADRESHbits_t;
extern volatile ADRESHbits_t ADRESHbits __attribute__((address(0x09C)));
# 1743 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char ADCON0 __attribute__((address(0x09D)));

__asm("ADCON0 equ 09Dh");


typedef union {
    struct {
        unsigned ADON :1;
        unsigned GO_nDONE :1;
        unsigned CHS0 :1;
        unsigned CHS1 :1;
        unsigned CHS2 :1;
        unsigned CHS3 :1;
        unsigned CHS4 :1;
    };
    struct {
        unsigned :1;
        unsigned ADGO :1;
        unsigned CHS :5;
    };
    struct {
        unsigned :1;
        unsigned GO :1;
    };
} ADCON0bits_t;
extern volatile ADCON0bits_t ADCON0bits __attribute__((address(0x09D)));
# 1823 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char ADCON1 __attribute__((address(0x09E)));

__asm("ADCON1 equ 09Eh");


typedef union {
    struct {
        unsigned ADPREF0 :1;
        unsigned ADPREF1 :1;
        unsigned ADNREF :1;
        unsigned :1;
        unsigned ADCS0 :1;
        unsigned ADCS1 :1;
        unsigned ADCS2 :1;
        unsigned ADFM :1;
    };
    struct {
        unsigned ADPREF :2;
        unsigned :2;
        unsigned ADCS :3;
    };
} ADCON1bits_t;
extern volatile ADCON1bits_t ADCON1bits __attribute__((address(0x09E)));
# 1895 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char LATA __attribute__((address(0x10C)));

__asm("LATA equ 010Ch");


typedef union {
    struct {
        unsigned LATA0 :1;
        unsigned LATA1 :1;
        unsigned LATA2 :1;
        unsigned LATA3 :1;
        unsigned LATA4 :1;
        unsigned :1;
        unsigned LATA6 :1;
        unsigned LATA7 :1;
    };
} LATAbits_t;
extern volatile LATAbits_t LATAbits __attribute__((address(0x10C)));
# 1952 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char LATB __attribute__((address(0x10D)));

__asm("LATB equ 010Dh");


typedef union {
    struct {
        unsigned LATB0 :1;
        unsigned LATB1 :1;
        unsigned LATB2 :1;
        unsigned LATB3 :1;
        unsigned LATB4 :1;
        unsigned LATB5 :1;
        unsigned LATB6 :1;
        unsigned LATB7 :1;
    };
} LATBbits_t;
extern volatile LATBbits_t LATBbits __attribute__((address(0x10D)));
# 2014 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char CM1CON0 __attribute__((address(0x111)));

__asm("CM1CON0 equ 0111h");


typedef union {
    struct {
        unsigned C1SYNC :1;
        unsigned C1HYS :1;
        unsigned C1SP :1;
        unsigned :1;
        unsigned C1POL :1;
        unsigned C1OE :1;
        unsigned C1OUT :1;
        unsigned C1ON :1;
    };
} CM1CON0bits_t;
extern volatile CM1CON0bits_t CM1CON0bits __attribute__((address(0x111)));
# 2071 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char CM1CON1 __attribute__((address(0x112)));

__asm("CM1CON1 equ 0112h");


typedef union {
    struct {
        unsigned C1NCH0 :1;
        unsigned C1NCH1 :1;
        unsigned :2;
        unsigned C1PCH0 :1;
        unsigned C1PCH1 :1;
        unsigned C1INTN :1;
        unsigned C1INTP :1;
    };
    struct {
        unsigned C1NCH :2;
        unsigned :2;
        unsigned C1PCH :2;
    };
} CM1CON1bits_t;
extern volatile CM1CON1bits_t CM1CON1bits __attribute__((address(0x112)));
# 2137 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char CM2CON0 __attribute__((address(0x113)));

__asm("CM2CON0 equ 0113h");


typedef union {
    struct {
        unsigned C2SYNC :1;
        unsigned C2HYS :1;
        unsigned C2SP :1;
        unsigned :1;
        unsigned C2POL :1;
        unsigned C2OE :1;
        unsigned C2OUT :1;
        unsigned C2ON :1;
    };
} CM2CON0bits_t;
extern volatile CM2CON0bits_t CM2CON0bits __attribute__((address(0x113)));
# 2194 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char CM2CON1 __attribute__((address(0x114)));

__asm("CM2CON1 equ 0114h");


typedef union {
    struct {
        unsigned C2NCH0 :1;
        unsigned C2NCH1 :1;
        unsigned :2;
        unsigned C2PCH0 :1;
        unsigned C2PCH1 :1;
        unsigned C2INTN :1;
        unsigned C2INTP :1;
    };
    struct {
        unsigned C2NCH :2;
        unsigned :2;
        unsigned C2PCH :2;
    };
} CM2CON1bits_t;
extern volatile CM2CON1bits_t CM2CON1bits __attribute__((address(0x114)));
# 2260 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char CMOUT __attribute__((address(0x115)));

__asm("CMOUT equ 0115h");


typedef union {
    struct {
        unsigned MC1OUT :1;
        unsigned MC2OUT :1;
    };
} CMOUTbits_t;
extern volatile CMOUTbits_t CMOUTbits __attribute__((address(0x115)));
# 2286 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char BORCON __attribute__((address(0x116)));

__asm("BORCON equ 0116h");


typedef union {
    struct {
        unsigned BORRDY :1;
        unsigned :6;
        unsigned SBOREN :1;
    };
} BORCONbits_t;
extern volatile BORCONbits_t BORCONbits __attribute__((address(0x116)));
# 2313 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char FVRCON __attribute__((address(0x117)));

__asm("FVRCON equ 0117h");


typedef union {
    struct {
        unsigned ADFVR0 :1;
        unsigned ADFVR1 :1;
        unsigned CDAFVR0 :1;
        unsigned CDAFVR1 :1;
        unsigned TSRNG :1;
        unsigned TSEN :1;
        unsigned FVRRDY :1;
        unsigned FVREN :1;
    };
    struct {
        unsigned ADFVR :2;
        unsigned CDAFVR :2;
    };
} FVRCONbits_t;
extern volatile FVRCONbits_t FVRCONbits __attribute__((address(0x117)));
# 2389 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char DACCON0 __attribute__((address(0x118)));

__asm("DACCON0 equ 0118h");


typedef union {
    struct {
        unsigned DACNSS :1;
        unsigned :1;
        unsigned DACPSS0 :1;
        unsigned DACPSS1 :1;
        unsigned :1;
        unsigned DACOE :1;
        unsigned DACLPS :1;
        unsigned DACEN :1;
    };
    struct {
        unsigned :2;
        unsigned DACPSS :2;
    };
} DACCON0bits_t;
extern volatile DACCON0bits_t DACCON0bits __attribute__((address(0x118)));
# 2450 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char DACCON1 __attribute__((address(0x119)));

__asm("DACCON1 equ 0119h");


typedef union {
    struct {
        unsigned DACR0 :1;
        unsigned DACR1 :1;
        unsigned DACR2 :1;
        unsigned DACR3 :1;
        unsigned DACR4 :1;
    };
    struct {
        unsigned DACR :5;
    };
} DACCON1bits_t;
extern volatile DACCON1bits_t DACCON1bits __attribute__((address(0x119)));
# 2502 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char SRCON0 __attribute__((address(0x11A)));

__asm("SRCON0 equ 011Ah");


typedef union {
    struct {
        unsigned SRPR :1;
        unsigned SRPS :1;
        unsigned SRNQEN :1;
        unsigned SRQEN :1;
        unsigned SRCLK0 :1;
        unsigned SRCLK1 :1;
        unsigned SRCLK2 :1;
        unsigned SRLEN :1;
    };
    struct {
        unsigned :4;
        unsigned SRCLK :3;
    };
} SRCON0bits_t;
extern volatile SRCON0bits_t SRCON0bits __attribute__((address(0x11A)));
# 2573 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char SRCON1 __attribute__((address(0x11B)));

__asm("SRCON1 equ 011Bh");


typedef union {
    struct {
        unsigned SRRC1E :1;
        unsigned SRRC2E :1;
        unsigned SRRCKE :1;
        unsigned SRRPE :1;
        unsigned SRSC1E :1;
        unsigned SRSC2E :1;
        unsigned SRSCKE :1;
        unsigned SRSPE :1;
    };
} SRCON1bits_t;
extern volatile SRCON1bits_t SRCON1bits __attribute__((address(0x11B)));
# 2635 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char APFCON0 __attribute__((address(0x11D)));

__asm("APFCON0 equ 011Dh");


typedef union {
    struct {
        unsigned CCP1SEL :1;
        unsigned P1CSEL :1;
        unsigned P1DSEL :1;
        unsigned :2;
        unsigned SS1SEL :1;
        unsigned SDO1SEL :1;
        unsigned RXDTSEL :1;
    };
} APFCON0bits_t;
extern volatile APFCON0bits_t APFCON0bits __attribute__((address(0x11D)));
# 2686 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char APFCON1 __attribute__((address(0x11E)));

__asm("APFCON1 equ 011Eh");


typedef union {
    struct {
        unsigned TXCKSEL :1;
    };
} APFCON1bits_t;
extern volatile APFCON1bits_t APFCON1bits __attribute__((address(0x11E)));
# 2706 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char ANSELA __attribute__((address(0x18C)));

__asm("ANSELA equ 018Ch");


typedef union {
    struct {
        unsigned ANSA0 :1;
        unsigned ANSA1 :1;
        unsigned ANSA2 :1;
        unsigned ANSA3 :1;
        unsigned ANSA4 :1;
    };
    struct {
        unsigned ANSELA :5;
    };
} ANSELAbits_t;
extern volatile ANSELAbits_t ANSELAbits __attribute__((address(0x18C)));
# 2758 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char ANSELB __attribute__((address(0x18D)));

__asm("ANSELB equ 018Dh");


typedef union {
    struct {
        unsigned :1;
        unsigned ANSB1 :1;
        unsigned ANSB2 :1;
        unsigned ANSB3 :1;
        unsigned ANSB4 :1;
        unsigned ANSB5 :1;
        unsigned ANSB6 :1;
        unsigned ANSB7 :1;
    };
    struct {
        unsigned ANSELB :8;
    };
} ANSELBbits_t;
extern volatile ANSELBbits_t ANSELBbits __attribute__((address(0x18D)));
# 2823 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned short EEADR __attribute__((address(0x191)));

__asm("EEADR equ 0191h");




extern volatile unsigned char EEADRL __attribute__((address(0x191)));

__asm("EEADRL equ 0191h");


typedef union {
    struct {
        unsigned EEADRL :8;
    };
} EEADRLbits_t;
extern volatile EEADRLbits_t EEADRLbits __attribute__((address(0x191)));
# 2850 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char EEADRH __attribute__((address(0x192)));

__asm("EEADRH equ 0192h");


typedef union {
    struct {
        unsigned EEADRH :7;
    };
} EEADRHbits_t;
extern volatile EEADRHbits_t EEADRHbits __attribute__((address(0x192)));
# 2870 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned short EEDAT __attribute__((address(0x193)));

__asm("EEDAT equ 0193h");




extern volatile unsigned char EEDATL __attribute__((address(0x193)));

__asm("EEDATL equ 0193h");


extern volatile unsigned char EEDATA __attribute__((address(0x193)));

__asm("EEDATA equ 0193h");


typedef union {
    struct {
        unsigned EEDATL :8;
    };
} EEDATLbits_t;
extern volatile EEDATLbits_t EEDATLbits __attribute__((address(0x193)));







typedef union {
    struct {
        unsigned EEDATL :8;
    };
} EEDATAbits_t;
extern volatile EEDATAbits_t EEDATAbits __attribute__((address(0x193)));
# 2915 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char EEDATH __attribute__((address(0x194)));

__asm("EEDATH equ 0194h");


typedef union {
    struct {
        unsigned EEDATH :6;
    };
} EEDATHbits_t;
extern volatile EEDATHbits_t EEDATHbits __attribute__((address(0x194)));
# 2935 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char EECON1 __attribute__((address(0x195)));

__asm("EECON1 equ 0195h");


typedef union {
    struct {
        unsigned RD :1;
        unsigned WR :1;
        unsigned WREN :1;
        unsigned WRERR :1;
        unsigned FREE :1;
        unsigned LWLO :1;
        unsigned CFGS :1;
        unsigned EEPGD :1;
    };
} EECON1bits_t;
extern volatile EECON1bits_t EECON1bits __attribute__((address(0x195)));
# 2997 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char EECON2 __attribute__((address(0x196)));

__asm("EECON2 equ 0196h");


typedef union {
    struct {
        unsigned EECON2 :8;
    };
} EECON2bits_t;
extern volatile EECON2bits_t EECON2bits __attribute__((address(0x196)));
# 3017 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char RCREG __attribute__((address(0x199)));

__asm("RCREG equ 0199h");


typedef union {
    struct {
        unsigned RCREG :8;
    };
} RCREGbits_t;
extern volatile RCREGbits_t RCREGbits __attribute__((address(0x199)));
# 3037 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char TXREG __attribute__((address(0x19A)));

__asm("TXREG equ 019Ah");


typedef union {
    struct {
        unsigned TXREG :8;
    };
} TXREGbits_t;
extern volatile TXREGbits_t TXREGbits __attribute__((address(0x19A)));
# 3057 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned short SP1BRG __attribute__((address(0x19B)));

__asm("SP1BRG equ 019Bh");




extern volatile unsigned char SP1BRGL __attribute__((address(0x19B)));

__asm("SP1BRGL equ 019Bh");


extern volatile unsigned char SPBRG __attribute__((address(0x19B)));

__asm("SPBRG equ 019Bh");

extern volatile unsigned char SPBRGL __attribute__((address(0x19B)));

__asm("SPBRGL equ 019Bh");


typedef union {
    struct {
        unsigned SPBRGL :8;
    };
} SP1BRGLbits_t;
extern volatile SP1BRGLbits_t SP1BRGLbits __attribute__((address(0x19B)));







typedef union {
    struct {
        unsigned SPBRGL :8;
    };
} SPBRGbits_t;
extern volatile SPBRGbits_t SPBRGbits __attribute__((address(0x19B)));






typedef union {
    struct {
        unsigned SPBRGL :8;
    };
} SPBRGLbits_t;
extern volatile SPBRGLbits_t SPBRGLbits __attribute__((address(0x19B)));
# 3118 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char SP1BRGH __attribute__((address(0x19C)));

__asm("SP1BRGH equ 019Ch");


extern volatile unsigned char SPBRGH __attribute__((address(0x19C)));

__asm("SPBRGH equ 019Ch");


typedef union {
    struct {
        unsigned SPBRGH :8;
    };
} SP1BRGHbits_t;
extern volatile SP1BRGHbits_t SP1BRGHbits __attribute__((address(0x19C)));







typedef union {
    struct {
        unsigned SPBRGH :8;
    };
} SPBRGHbits_t;
extern volatile SPBRGHbits_t SPBRGHbits __attribute__((address(0x19C)));
# 3156 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char RCSTA __attribute__((address(0x19D)));

__asm("RCSTA equ 019Dh");


typedef union {
    struct {
        unsigned RX9D :1;
        unsigned OERR :1;
        unsigned FERR :1;
        unsigned ADDEN :1;
        unsigned CREN :1;
        unsigned SREN :1;
        unsigned RX9 :1;
        unsigned SPEN :1;
    };
} RCSTAbits_t;
extern volatile RCSTAbits_t RCSTAbits __attribute__((address(0x19D)));
# 3218 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char TXSTA __attribute__((address(0x19E)));

__asm("TXSTA equ 019Eh");


typedef union {
    struct {
        unsigned TX9D :1;
        unsigned TRMT :1;
        unsigned BRGH :1;
        unsigned SENDB :1;
        unsigned SYNC :1;
        unsigned TXEN :1;
        unsigned TX9 :1;
        unsigned CSRC :1;
    };
} TXSTAbits_t;
extern volatile TXSTAbits_t TXSTAbits __attribute__((address(0x19E)));
# 3280 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char BAUDCON __attribute__((address(0x19F)));

__asm("BAUDCON equ 019Fh");


typedef union {
    struct {
        unsigned ABDEN :1;
        unsigned WUE :1;
        unsigned :1;
        unsigned BRG16 :1;
        unsigned SCKP :1;
        unsigned :1;
        unsigned RCIDL :1;
        unsigned ABDOVF :1;
    };
} BAUDCONbits_t;
extern volatile BAUDCONbits_t BAUDCONbits __attribute__((address(0x19F)));
# 3332 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char WPUA __attribute__((address(0x20C)));

__asm("WPUA equ 020Ch");


typedef union {
    struct {
        unsigned :5;
        unsigned WPUA5 :1;
    };
    struct {
        unsigned WPUA :8;
    };
} WPUAbits_t;
extern volatile WPUAbits_t WPUAbits __attribute__((address(0x20C)));
# 3361 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char WPUB __attribute__((address(0x20D)));

__asm("WPUB equ 020Dh");


typedef union {
    struct {
        unsigned WPUB0 :1;
        unsigned WPUB1 :1;
        unsigned WPUB2 :1;
        unsigned WPUB3 :1;
        unsigned WPUB4 :1;
        unsigned WPUB5 :1;
        unsigned WPUB6 :1;
        unsigned WPUB7 :1;
    };
    struct {
        unsigned WPUB :8;
    };
} WPUBbits_t;
extern volatile WPUBbits_t WPUBbits __attribute__((address(0x20D)));
# 3431 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char SSP1BUF __attribute__((address(0x211)));

__asm("SSP1BUF equ 0211h");


extern volatile unsigned char SSPBUF __attribute__((address(0x211)));

__asm("SSPBUF equ 0211h");


typedef union {
    struct {
        unsigned SSPBUF :8;
    };
} SSP1BUFbits_t;
extern volatile SSP1BUFbits_t SSP1BUFbits __attribute__((address(0x211)));







typedef union {
    struct {
        unsigned SSPBUF :8;
    };
} SSPBUFbits_t;
extern volatile SSPBUFbits_t SSPBUFbits __attribute__((address(0x211)));
# 3469 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char SSP1ADD __attribute__((address(0x212)));

__asm("SSP1ADD equ 0212h");


extern volatile unsigned char SSPADD __attribute__((address(0x212)));

__asm("SSPADD equ 0212h");


typedef union {
    struct {
        unsigned SSPADD :8;
    };
} SSP1ADDbits_t;
extern volatile SSP1ADDbits_t SSP1ADDbits __attribute__((address(0x212)));







typedef union {
    struct {
        unsigned SSPADD :8;
    };
} SSPADDbits_t;
extern volatile SSPADDbits_t SSPADDbits __attribute__((address(0x212)));
# 3507 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char SSP1MSK __attribute__((address(0x213)));

__asm("SSP1MSK equ 0213h");


extern volatile unsigned char SSPMSK __attribute__((address(0x213)));

__asm("SSPMSK equ 0213h");


typedef union {
    struct {
        unsigned SSPMSK :8;
    };
} SSP1MSKbits_t;
extern volatile SSP1MSKbits_t SSP1MSKbits __attribute__((address(0x213)));







typedef union {
    struct {
        unsigned SSPMSK :8;
    };
} SSPMSKbits_t;
extern volatile SSPMSKbits_t SSPMSKbits __attribute__((address(0x213)));
# 3545 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char SSP1STAT __attribute__((address(0x214)));

__asm("SSP1STAT equ 0214h");


extern volatile unsigned char SSPSTAT __attribute__((address(0x214)));

__asm("SSPSTAT equ 0214h");


typedef union {
    struct {
        unsigned BF :1;
        unsigned UA :1;
        unsigned R_nW :1;
        unsigned S :1;
        unsigned P :1;
        unsigned D_nA :1;
        unsigned CKE :1;
        unsigned SMP :1;
    };
} SSP1STATbits_t;
extern volatile SSP1STATbits_t SSP1STATbits __attribute__((address(0x214)));
# 3610 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
typedef union {
    struct {
        unsigned BF :1;
        unsigned UA :1;
        unsigned R_nW :1;
        unsigned S :1;
        unsigned P :1;
        unsigned D_nA :1;
        unsigned CKE :1;
        unsigned SMP :1;
    };
} SSPSTATbits_t;
extern volatile SSPSTATbits_t SSPSTATbits __attribute__((address(0x214)));
# 3667 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char SSP1CON1 __attribute__((address(0x215)));

__asm("SSP1CON1 equ 0215h");


extern volatile unsigned char SSPCON1 __attribute__((address(0x215)));

__asm("SSPCON1 equ 0215h");

extern volatile unsigned char SSPCON __attribute__((address(0x215)));

__asm("SSPCON equ 0215h");


typedef union {
    struct {
        unsigned SSPM0 :1;
        unsigned SSPM1 :1;
        unsigned SSPM2 :1;
        unsigned SSPM3 :1;
        unsigned CKP :1;
        unsigned SSPEN :1;
        unsigned SSPOV :1;
        unsigned WCOL :1;
    };
    struct {
        unsigned SSPM :4;
    };
} SSP1CON1bits_t;
extern volatile SSP1CON1bits_t SSP1CON1bits __attribute__((address(0x215)));
# 3744 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
typedef union {
    struct {
        unsigned SSPM0 :1;
        unsigned SSPM1 :1;
        unsigned SSPM2 :1;
        unsigned SSPM3 :1;
        unsigned CKP :1;
        unsigned SSPEN :1;
        unsigned SSPOV :1;
        unsigned WCOL :1;
    };
    struct {
        unsigned SSPM :4;
    };
} SSPCON1bits_t;
extern volatile SSPCON1bits_t SSPCON1bits __attribute__((address(0x215)));
# 3806 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
typedef union {
    struct {
        unsigned SSPM0 :1;
        unsigned SSPM1 :1;
        unsigned SSPM2 :1;
        unsigned SSPM3 :1;
        unsigned CKP :1;
        unsigned SSPEN :1;
        unsigned SSPOV :1;
        unsigned WCOL :1;
    };
    struct {
        unsigned SSPM :4;
    };
} SSPCONbits_t;
extern volatile SSPCONbits_t SSPCONbits __attribute__((address(0x215)));
# 3871 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char SSP1CON2 __attribute__((address(0x216)));

__asm("SSP1CON2 equ 0216h");


extern volatile unsigned char SSPCON2 __attribute__((address(0x216)));

__asm("SSPCON2 equ 0216h");


typedef union {
    struct {
        unsigned SEN :1;
        unsigned RSEN :1;
        unsigned PEN :1;
        unsigned RCEN :1;
        unsigned ACKEN :1;
        unsigned ACKDT :1;
        unsigned ACKSTAT :1;
        unsigned GCEN :1;
    };
} SSP1CON2bits_t;
extern volatile SSP1CON2bits_t SSP1CON2bits __attribute__((address(0x216)));
# 3936 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
typedef union {
    struct {
        unsigned SEN :1;
        unsigned RSEN :1;
        unsigned PEN :1;
        unsigned RCEN :1;
        unsigned ACKEN :1;
        unsigned ACKDT :1;
        unsigned ACKSTAT :1;
        unsigned GCEN :1;
    };
} SSPCON2bits_t;
extern volatile SSPCON2bits_t SSPCON2bits __attribute__((address(0x216)));
# 3993 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char SSP1CON3 __attribute__((address(0x217)));

__asm("SSP1CON3 equ 0217h");


extern volatile unsigned char SSPCON3 __attribute__((address(0x217)));

__asm("SSPCON3 equ 0217h");


typedef union {
    struct {
        unsigned DHEN :1;
        unsigned AHEN :1;
        unsigned SBCDE :1;
        unsigned SDAHT :1;
        unsigned BOEN :1;
        unsigned SCIE :1;
        unsigned PCIE :1;
        unsigned ACKTIM :1;
    };
} SSP1CON3bits_t;
extern volatile SSP1CON3bits_t SSP1CON3bits __attribute__((address(0x217)));
# 4058 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
typedef union {
    struct {
        unsigned DHEN :1;
        unsigned AHEN :1;
        unsigned SBCDE :1;
        unsigned SDAHT :1;
        unsigned BOEN :1;
        unsigned SCIE :1;
        unsigned PCIE :1;
        unsigned ACKTIM :1;
    };
} SSPCON3bits_t;
extern volatile SSPCON3bits_t SSPCON3bits __attribute__((address(0x217)));
# 4115 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned short CCPR1 __attribute__((address(0x291)));

__asm("CCPR1 equ 0291h");




extern volatile unsigned char CCPR1L __attribute__((address(0x291)));

__asm("CCPR1L equ 0291h");


typedef union {
    struct {
        unsigned CCPR1L :8;
    };
} CCPR1Lbits_t;
extern volatile CCPR1Lbits_t CCPR1Lbits __attribute__((address(0x291)));
# 4142 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char CCPR1H __attribute__((address(0x292)));

__asm("CCPR1H equ 0292h");


typedef union {
    struct {
        unsigned CCPR1H :8;
    };
} CCPR1Hbits_t;
extern volatile CCPR1Hbits_t CCPR1Hbits __attribute__((address(0x292)));
# 4162 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char CCP1CON __attribute__((address(0x293)));

__asm("CCP1CON equ 0293h");


typedef union {
    struct {
        unsigned CCP1M0 :1;
        unsigned CCP1M1 :1;
        unsigned CCP1M2 :1;
        unsigned CCP1M3 :1;
        unsigned DC1B0 :1;
        unsigned DC1B1 :1;
        unsigned P1M0 :1;
        unsigned P1M1 :1;
    };
    struct {
        unsigned CCP1M :4;
        unsigned DC1B :2;
        unsigned P1M :2;
    };
} CCP1CONbits_t;
extern volatile CCP1CONbits_t CCP1CONbits __attribute__((address(0x293)));
# 4244 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char PWM1CON __attribute__((address(0x294)));

__asm("PWM1CON equ 0294h");


typedef union {
    struct {
        unsigned P1DC0 :1;
        unsigned P1DC1 :1;
        unsigned P1DC2 :1;
        unsigned P1DC3 :1;
        unsigned P1DC4 :1;
        unsigned P1DC5 :1;
        unsigned P1DC6 :1;
        unsigned P1RSEN :1;
    };
    struct {
        unsigned P1DC :7;
    };
} PWM1CONbits_t;
extern volatile PWM1CONbits_t PWM1CONbits __attribute__((address(0x294)));
# 4314 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char CCP1AS __attribute__((address(0x295)));

__asm("CCP1AS equ 0295h");


extern volatile unsigned char ECCP1AS __attribute__((address(0x295)));

__asm("ECCP1AS equ 0295h");


typedef union {
    struct {
        unsigned PSS1BD0 :1;
        unsigned PSS1BD1 :1;
        unsigned PSS1AC0 :1;
        unsigned PSS1AC1 :1;
        unsigned CCP1AS0 :1;
        unsigned CCP1AS1 :1;
        unsigned CCP1AS2 :1;
        unsigned CCP1ASE :1;
    };
    struct {
        unsigned PSS1BD :2;
        unsigned PSS1AC :2;
        unsigned CCP1AS :3;
    };
} CCP1ASbits_t;
extern volatile CCP1ASbits_t CCP1ASbits __attribute__((address(0x295)));
# 4399 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
typedef union {
    struct {
        unsigned PSS1BD0 :1;
        unsigned PSS1BD1 :1;
        unsigned PSS1AC0 :1;
        unsigned PSS1AC1 :1;
        unsigned CCP1AS0 :1;
        unsigned CCP1AS1 :1;
        unsigned CCP1AS2 :1;
        unsigned CCP1ASE :1;
    };
    struct {
        unsigned PSS1BD :2;
        unsigned PSS1AC :2;
        unsigned CCP1AS :3;
    };
} ECCP1ASbits_t;
extern volatile ECCP1ASbits_t ECCP1ASbits __attribute__((address(0x295)));
# 4476 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char PSTR1CON __attribute__((address(0x296)));

__asm("PSTR1CON equ 0296h");


typedef union {
    struct {
        unsigned STR1A :1;
        unsigned STR1B :1;
        unsigned STR1C :1;
        unsigned STR1D :1;
        unsigned STR1SYNC :1;
    };
} PSTR1CONbits_t;
extern volatile PSTR1CONbits_t PSTR1CONbits __attribute__((address(0x296)));
# 4520 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char IOCBP __attribute__((address(0x394)));

__asm("IOCBP equ 0394h");


typedef union {
    struct {
        unsigned IOCBP0 :1;
        unsigned IOCBP1 :1;
        unsigned IOCBP2 :1;
        unsigned IOCBP3 :1;
        unsigned IOCBP4 :1;
        unsigned IOCBP5 :1;
        unsigned IOCBP6 :1;
        unsigned IOCBP7 :1;
    };
    struct {
        unsigned IOCBP :8;
    };
} IOCBPbits_t;
extern volatile IOCBPbits_t IOCBPbits __attribute__((address(0x394)));
# 4590 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char IOCBN __attribute__((address(0x395)));

__asm("IOCBN equ 0395h");


typedef union {
    struct {
        unsigned IOCBN0 :1;
        unsigned IOCBN1 :1;
        unsigned IOCBN2 :1;
        unsigned IOCBN3 :1;
        unsigned IOCBN4 :1;
        unsigned IOCBN5 :1;
        unsigned IOCBN6 :1;
        unsigned IOCBN7 :1;
    };
    struct {
        unsigned IOCBN :8;
    };
} IOCBNbits_t;
extern volatile IOCBNbits_t IOCBNbits __attribute__((address(0x395)));
# 4660 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char IOCBF __attribute__((address(0x396)));

__asm("IOCBF equ 0396h");


typedef union {
    struct {
        unsigned IOCBF0 :1;
        unsigned IOCBF1 :1;
        unsigned IOCBF2 :1;
        unsigned IOCBF3 :1;
        unsigned IOCBF4 :1;
        unsigned IOCBF5 :1;
        unsigned IOCBF6 :1;
        unsigned IOCBF7 :1;
    };
    struct {
        unsigned IOCBF :8;
    };
} IOCBFbits_t;
extern volatile IOCBFbits_t IOCBFbits __attribute__((address(0x396)));
# 4730 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char CLKRCON __attribute__((address(0x39A)));

__asm("CLKRCON equ 039Ah");


typedef union {
    struct {
        unsigned CLKRDIV0 :1;
        unsigned CLKRDIV1 :1;
        unsigned CLKRDIV2 :1;
        unsigned CLKRDC0 :1;
        unsigned CLKRDC1 :1;
        unsigned CLKRSLR :1;
        unsigned CLKROE :1;
        unsigned CLKREN :1;
    };
    struct {
        unsigned CLKRDIV :3;
        unsigned CLKRDC :2;
    };
} CLKRCONbits_t;
extern volatile CLKRCONbits_t CLKRCONbits __attribute__((address(0x39A)));
# 4806 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char MDCON __attribute__((address(0x39C)));

__asm("MDCON equ 039Ch");


typedef union {
    struct {
        unsigned MDBIT :1;
        unsigned :2;
        unsigned MDOUT :1;
        unsigned MDOPOL :1;
        unsigned MDSLR :1;
        unsigned MDOE :1;
        unsigned MDEN :1;
    };
} MDCONbits_t;
extern volatile MDCONbits_t MDCONbits __attribute__((address(0x39C)));
# 4857 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char MDSRC __attribute__((address(0x39D)));

__asm("MDSRC equ 039Dh");


typedef union {
    struct {
        unsigned MDMS0 :1;
        unsigned MDMS1 :1;
        unsigned MDMS2 :1;
        unsigned MDMS3 :1;
        unsigned :3;
        unsigned MDMSODIS :1;
    };
    struct {
        unsigned MDMS :4;
    };
} MDSRCbits_t;
extern volatile MDSRCbits_t MDSRCbits __attribute__((address(0x39D)));
# 4910 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char MDCARL __attribute__((address(0x39E)));

__asm("MDCARL equ 039Eh");


typedef union {
    struct {
        unsigned MDCL0 :1;
        unsigned MDCL1 :1;
        unsigned MDCL2 :1;
        unsigned MDCL3 :1;
        unsigned :1;
        unsigned MDCLSYNC :1;
        unsigned MDCLPOL :1;
        unsigned MDCLODIS :1;
    };
    struct {
        unsigned MDCL :4;
    };
} MDCARLbits_t;
extern volatile MDCARLbits_t MDCARLbits __attribute__((address(0x39E)));
# 4975 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char MDCARH __attribute__((address(0x39F)));

__asm("MDCARH equ 039Fh");


typedef union {
    struct {
        unsigned MDCH0 :1;
        unsigned MDCH1 :1;
        unsigned MDCH2 :1;
        unsigned MDCH3 :1;
        unsigned :1;
        unsigned MDCHSYNC :1;
        unsigned MDCHPOL :1;
        unsigned MDCHODIS :1;
    };
    struct {
        unsigned MDCH :4;
    };
} MDCARHbits_t;
extern volatile MDCARHbits_t MDCARHbits __attribute__((address(0x39F)));
# 5040 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char STATUS_SHAD __attribute__((address(0xFE4)));

__asm("STATUS_SHAD equ 0FE4h");


typedef union {
    struct {
        unsigned C_SHAD :1;
        unsigned DC_SHAD :1;
        unsigned Z_SHAD :1;
    };
} STATUS_SHADbits_t;
extern volatile STATUS_SHADbits_t STATUS_SHADbits __attribute__((address(0xFE4)));
# 5072 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char WREG_SHAD __attribute__((address(0xFE5)));

__asm("WREG_SHAD equ 0FE5h");


typedef union {
    struct {
        unsigned WREG_SHAD :8;
    };
} WREG_SHADbits_t;
extern volatile WREG_SHADbits_t WREG_SHADbits __attribute__((address(0xFE5)));
# 5092 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char BSR_SHAD __attribute__((address(0xFE6)));

__asm("BSR_SHAD equ 0FE6h");


typedef union {
    struct {
        unsigned BSR_SHAD :5;
    };
} BSR_SHADbits_t;
extern volatile BSR_SHADbits_t BSR_SHADbits __attribute__((address(0xFE6)));
# 5112 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char PCLATH_SHAD __attribute__((address(0xFE7)));

__asm("PCLATH_SHAD equ 0FE7h");


typedef union {
    struct {
        unsigned PCLATH_SHAD :7;
    };
} PCLATH_SHADbits_t;
extern volatile PCLATH_SHADbits_t PCLATH_SHADbits __attribute__((address(0xFE7)));
# 5132 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char FSR0L_SHAD __attribute__((address(0xFE8)));

__asm("FSR0L_SHAD equ 0FE8h");


typedef union {
    struct {
        unsigned FSR0L_SHAD :8;
    };
} FSR0L_SHADbits_t;
extern volatile FSR0L_SHADbits_t FSR0L_SHADbits __attribute__((address(0xFE8)));
# 5152 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char FSR0H_SHAD __attribute__((address(0xFE9)));

__asm("FSR0H_SHAD equ 0FE9h");


typedef union {
    struct {
        unsigned FSR0H_SHAD :8;
    };
} FSR0H_SHADbits_t;
extern volatile FSR0H_SHADbits_t FSR0H_SHADbits __attribute__((address(0xFE9)));
# 5172 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char FSR1L_SHAD __attribute__((address(0xFEA)));

__asm("FSR1L_SHAD equ 0FEAh");


typedef union {
    struct {
        unsigned FSR1L_SHAD :8;
    };
} FSR1L_SHADbits_t;
extern volatile FSR1L_SHADbits_t FSR1L_SHADbits __attribute__((address(0xFEA)));
# 5192 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char FSR1H_SHAD __attribute__((address(0xFEB)));

__asm("FSR1H_SHAD equ 0FEBh");


typedef union {
    struct {
        unsigned FSR1H_SHAD :8;
    };
} FSR1H_SHADbits_t;
extern volatile FSR1H_SHADbits_t FSR1H_SHADbits __attribute__((address(0xFEB)));
# 5212 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char STKPTR __attribute__((address(0xFED)));

__asm("STKPTR equ 0FEDh");


typedef union {
    struct {
        unsigned STKPTR :5;
    };
} STKPTRbits_t;
extern volatile STKPTRbits_t STKPTRbits __attribute__((address(0xFED)));
# 5232 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char TOSL __attribute__((address(0xFEE)));

__asm("TOSL equ 0FEEh");


typedef union {
    struct {
        unsigned TOSL :8;
    };
} TOSLbits_t;
extern volatile TOSLbits_t TOSLbits __attribute__((address(0xFEE)));
# 5252 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile unsigned char TOSH __attribute__((address(0xFEF)));

__asm("TOSH equ 0FEFh");


typedef union {
    struct {
        unsigned TOSH :7;
    };
} TOSHbits_t;
extern volatile TOSHbits_t TOSHbits __attribute__((address(0xFEF)));
# 5278 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/proc/pic16f1826.h" 3
extern volatile __bit ABDEN __attribute__((address(0xCF8)));


extern volatile __bit ABDOVF __attribute__((address(0xCFF)));


extern volatile __bit ACKDT __attribute__((address(0x10B5)));


extern volatile __bit ACKEN __attribute__((address(0x10B4)));


extern volatile __bit ACKSTAT __attribute__((address(0x10B6)));


extern volatile __bit ACKTIM __attribute__((address(0x10BF)));


extern volatile __bit ADCS0 __attribute__((address(0x4F4)));


extern volatile __bit ADCS1 __attribute__((address(0x4F5)));


extern volatile __bit ADCS2 __attribute__((address(0x4F6)));


extern volatile __bit ADDEN __attribute__((address(0xCEB)));


extern volatile __bit ADFM __attribute__((address(0x4F7)));


extern volatile __bit ADFVR0 __attribute__((address(0x8B8)));


extern volatile __bit ADFVR1 __attribute__((address(0x8B9)));


extern volatile __bit ADGO __attribute__((address(0x4E9)));


extern volatile __bit ADIE __attribute__((address(0x48E)));


extern volatile __bit ADIF __attribute__((address(0x8E)));


extern volatile __bit ADNREF __attribute__((address(0x4F2)));


extern volatile __bit ADON __attribute__((address(0x4E8)));


extern volatile __bit ADPREF0 __attribute__((address(0x4F0)));


extern volatile __bit ADPREF1 __attribute__((address(0x4F1)));


extern volatile __bit AHEN __attribute__((address(0x10B9)));


extern volatile __bit ANSA0 __attribute__((address(0xC60)));


extern volatile __bit ANSA1 __attribute__((address(0xC61)));


extern volatile __bit ANSA2 __attribute__((address(0xC62)));


extern volatile __bit ANSA3 __attribute__((address(0xC63)));


extern volatile __bit ANSA4 __attribute__((address(0xC64)));


extern volatile __bit ANSB1 __attribute__((address(0xC69)));


extern volatile __bit ANSB2 __attribute__((address(0xC6A)));


extern volatile __bit ANSB3 __attribute__((address(0xC6B)));


extern volatile __bit ANSB4 __attribute__((address(0xC6C)));


extern volatile __bit ANSB5 __attribute__((address(0xC6D)));


extern volatile __bit ANSB6 __attribute__((address(0xC6E)));


extern volatile __bit ANSB7 __attribute__((address(0xC6F)));


extern volatile __bit BCL1IE __attribute__((address(0x493)));


extern volatile __bit BCL1IF __attribute__((address(0x93)));


extern volatile __bit BF __attribute__((address(0x10A0)));


extern volatile __bit BOEN __attribute__((address(0x10BC)));


extern volatile __bit BORRDY __attribute__((address(0x8B0)));


extern volatile __bit BRG16 __attribute__((address(0xCFB)));


extern volatile __bit BRGH __attribute__((address(0xCF2)));


extern volatile __bit BSR0 __attribute__((address(0x40)));


extern volatile __bit BSR1 __attribute__((address(0x41)));


extern volatile __bit BSR2 __attribute__((address(0x42)));


extern volatile __bit BSR3 __attribute__((address(0x43)));


extern volatile __bit BSR4 __attribute__((address(0x44)));


extern volatile __bit C1HYS __attribute__((address(0x889)));


extern volatile __bit C1IE __attribute__((address(0x495)));


extern volatile __bit C1IF __attribute__((address(0x95)));


extern volatile __bit C1INTN __attribute__((address(0x896)));


extern volatile __bit C1INTP __attribute__((address(0x897)));


extern volatile __bit C1NCH0 __attribute__((address(0x890)));


extern volatile __bit C1NCH1 __attribute__((address(0x891)));


extern volatile __bit C1OE __attribute__((address(0x88D)));


extern volatile __bit C1ON __attribute__((address(0x88F)));


extern volatile __bit C1OUT __attribute__((address(0x88E)));


extern volatile __bit C1PCH0 __attribute__((address(0x894)));


extern volatile __bit C1PCH1 __attribute__((address(0x895)));


extern volatile __bit C1POL __attribute__((address(0x88C)));


extern volatile __bit C1SP __attribute__((address(0x88A)));


extern volatile __bit C1SYNC __attribute__((address(0x888)));


extern volatile __bit C2HYS __attribute__((address(0x899)));


extern volatile __bit C2IE __attribute__((address(0x496)));


extern volatile __bit C2IF __attribute__((address(0x96)));


extern volatile __bit C2INTN __attribute__((address(0x8A6)));


extern volatile __bit C2INTP __attribute__((address(0x8A7)));


extern volatile __bit C2NCH0 __attribute__((address(0x8A0)));


extern volatile __bit C2NCH1 __attribute__((address(0x8A1)));


extern volatile __bit C2OE __attribute__((address(0x89D)));


extern volatile __bit C2ON __attribute__((address(0x89F)));


extern volatile __bit C2OUT __attribute__((address(0x89E)));


extern volatile __bit C2PCH0 __attribute__((address(0x8A4)));


extern volatile __bit C2PCH1 __attribute__((address(0x8A5)));


extern volatile __bit C2POL __attribute__((address(0x89C)));


extern volatile __bit C2SP __attribute__((address(0x89A)));


extern volatile __bit C2SYNC __attribute__((address(0x898)));


extern volatile __bit CARRY __attribute__((address(0x18)));


extern volatile __bit CCP1AS0 __attribute__((address(0x14AC)));


extern volatile __bit CCP1AS1 __attribute__((address(0x14AD)));


extern volatile __bit CCP1AS2 __attribute__((address(0x14AE)));


extern volatile __bit CCP1ASE __attribute__((address(0x14AF)));


extern volatile __bit CCP1IE __attribute__((address(0x48A)));


extern volatile __bit CCP1IF __attribute__((address(0x8A)));


extern volatile __bit CCP1M0 __attribute__((address(0x1498)));


extern volatile __bit CCP1M1 __attribute__((address(0x1499)));


extern volatile __bit CCP1M2 __attribute__((address(0x149A)));


extern volatile __bit CCP1M3 __attribute__((address(0x149B)));


extern volatile __bit CCP1SEL __attribute__((address(0x8E8)));


extern volatile __bit CDAFVR0 __attribute__((address(0x8BA)));


extern volatile __bit CDAFVR1 __attribute__((address(0x8BB)));


extern volatile __bit CFGS __attribute__((address(0xCAE)));


extern volatile __bit CHS0 __attribute__((address(0x4EA)));


extern volatile __bit CHS1 __attribute__((address(0x4EB)));


extern volatile __bit CHS2 __attribute__((address(0x4EC)));


extern volatile __bit CHS3 __attribute__((address(0x4ED)));


extern volatile __bit CHS4 __attribute__((address(0x4EE)));


extern volatile __bit CKE __attribute__((address(0x10A6)));


extern volatile __bit CKP __attribute__((address(0x10AC)));


extern volatile __bit CLKRDC0 __attribute__((address(0x1CD3)));


extern volatile __bit CLKRDC1 __attribute__((address(0x1CD4)));


extern volatile __bit CLKRDIV0 __attribute__((address(0x1CD0)));


extern volatile __bit CLKRDIV1 __attribute__((address(0x1CD1)));


extern volatile __bit CLKRDIV2 __attribute__((address(0x1CD2)));


extern volatile __bit CLKREN __attribute__((address(0x1CD7)));


extern volatile __bit CLKROE __attribute__((address(0x1CD6)));


extern volatile __bit CLKRSLR __attribute__((address(0x1CD5)));


extern volatile __bit CPSCH0 __attribute__((address(0xF8)));


extern volatile __bit CPSCH1 __attribute__((address(0xF9)));


extern volatile __bit CPSCH2 __attribute__((address(0xFA)));


extern volatile __bit CPSCH3 __attribute__((address(0xFB)));


extern volatile __bit CPSON __attribute__((address(0xF7)));


extern volatile __bit CPSOUT __attribute__((address(0xF1)));


extern volatile __bit CPSRNG0 __attribute__((address(0xF2)));


extern volatile __bit CPSRNG1 __attribute__((address(0xF3)));


extern volatile __bit CREN __attribute__((address(0xCEC)));


extern volatile __bit CSRC __attribute__((address(0xCF7)));


extern volatile __bit C_SHAD __attribute__((address(0x7F20)));


extern volatile __bit DACEN __attribute__((address(0x8C7)));


extern volatile __bit DACLPS __attribute__((address(0x8C6)));


extern volatile __bit DACNSS __attribute__((address(0x8C0)));


extern volatile __bit DACOE __attribute__((address(0x8C5)));


extern volatile __bit DACPSS0 __attribute__((address(0x8C2)));


extern volatile __bit DACPSS1 __attribute__((address(0x8C3)));


extern volatile __bit DACR0 __attribute__((address(0x8C8)));


extern volatile __bit DACR1 __attribute__((address(0x8C9)));


extern volatile __bit DACR2 __attribute__((address(0x8CA)));


extern volatile __bit DACR3 __attribute__((address(0x8CB)));


extern volatile __bit DACR4 __attribute__((address(0x8CC)));


extern volatile __bit DC __attribute__((address(0x19)));


extern volatile __bit DC1B0 __attribute__((address(0x149C)));


extern volatile __bit DC1B1 __attribute__((address(0x149D)));


extern volatile __bit DC_SHAD __attribute__((address(0x7F21)));


extern volatile __bit DHEN __attribute__((address(0x10B8)));


extern volatile __bit D_nA __attribute__((address(0x10A5)));


extern volatile __bit EEIE __attribute__((address(0x494)));


extern volatile __bit EEIF __attribute__((address(0x94)));


extern volatile __bit EEPGD __attribute__((address(0xCAF)));


extern volatile __bit FERR __attribute__((address(0xCEA)));


extern volatile __bit FREE __attribute__((address(0xCAC)));


extern volatile __bit FVREN __attribute__((address(0x8BF)));


extern volatile __bit FVRRDY __attribute__((address(0x8BE)));


extern volatile __bit GCEN __attribute__((address(0x10B7)));


extern volatile __bit GIE __attribute__((address(0x5F)));


extern volatile __bit GO __attribute__((address(0x4E9)));


extern volatile __bit GO_nDONE __attribute__((address(0x4E9)));


extern volatile __bit HFIOFL __attribute__((address(0x4D3)));


extern volatile __bit HFIOFR __attribute__((address(0x4D4)));


extern volatile __bit HFIOFS __attribute__((address(0x4D0)));


extern volatile __bit INTE __attribute__((address(0x5C)));


extern volatile __bit INTEDG __attribute__((address(0x4AE)));


extern volatile __bit INTF __attribute__((address(0x59)));


extern volatile __bit IOCBF0 __attribute__((address(0x1CB0)));


extern volatile __bit IOCBF1 __attribute__((address(0x1CB1)));


extern volatile __bit IOCBF2 __attribute__((address(0x1CB2)));


extern volatile __bit IOCBF3 __attribute__((address(0x1CB3)));


extern volatile __bit IOCBF4 __attribute__((address(0x1CB4)));


extern volatile __bit IOCBF5 __attribute__((address(0x1CB5)));


extern volatile __bit IOCBF6 __attribute__((address(0x1CB6)));


extern volatile __bit IOCBF7 __attribute__((address(0x1CB7)));


extern volatile __bit IOCBN0 __attribute__((address(0x1CA8)));


extern volatile __bit IOCBN1 __attribute__((address(0x1CA9)));


extern volatile __bit IOCBN2 __attribute__((address(0x1CAA)));


extern volatile __bit IOCBN3 __attribute__((address(0x1CAB)));


extern volatile __bit IOCBN4 __attribute__((address(0x1CAC)));


extern volatile __bit IOCBN5 __attribute__((address(0x1CAD)));


extern volatile __bit IOCBN6 __attribute__((address(0x1CAE)));


extern volatile __bit IOCBN7 __attribute__((address(0x1CAF)));


extern volatile __bit IOCBP0 __attribute__((address(0x1CA0)));


extern volatile __bit IOCBP1 __attribute__((address(0x1CA1)));


extern volatile __bit IOCBP2 __attribute__((address(0x1CA2)));


extern volatile __bit IOCBP3 __attribute__((address(0x1CA3)));


extern volatile __bit IOCBP4 __attribute__((address(0x1CA4)));


extern volatile __bit IOCBP5 __attribute__((address(0x1CA5)));


extern volatile __bit IOCBP6 __attribute__((address(0x1CA6)));


extern volatile __bit IOCBP7 __attribute__((address(0x1CA7)));


extern volatile __bit IOCIE __attribute__((address(0x5B)));


extern volatile __bit IOCIF __attribute__((address(0x58)));


extern volatile __bit IRCF0 __attribute__((address(0x4CB)));


extern volatile __bit IRCF1 __attribute__((address(0x4CC)));


extern volatile __bit IRCF2 __attribute__((address(0x4CD)));


extern volatile __bit IRCF3 __attribute__((address(0x4CE)));


extern volatile __bit LATA0 __attribute__((address(0x860)));


extern volatile __bit LATA1 __attribute__((address(0x861)));


extern volatile __bit LATA2 __attribute__((address(0x862)));


extern volatile __bit LATA3 __attribute__((address(0x863)));


extern volatile __bit LATA4 __attribute__((address(0x864)));


extern volatile __bit LATA6 __attribute__((address(0x866)));


extern volatile __bit LATA7 __attribute__((address(0x867)));


extern volatile __bit LATB0 __attribute__((address(0x868)));


extern volatile __bit LATB1 __attribute__((address(0x869)));


extern volatile __bit LATB2 __attribute__((address(0x86A)));


extern volatile __bit LATB3 __attribute__((address(0x86B)));


extern volatile __bit LATB4 __attribute__((address(0x86C)));


extern volatile __bit LATB5 __attribute__((address(0x86D)));


extern volatile __bit LATB6 __attribute__((address(0x86E)));


extern volatile __bit LATB7 __attribute__((address(0x86F)));


extern volatile __bit LFIOFR __attribute__((address(0x4D1)));


extern volatile __bit LWLO __attribute__((address(0xCAD)));


extern volatile __bit MC1OUT __attribute__((address(0x8A8)));


extern volatile __bit MC2OUT __attribute__((address(0x8A9)));


extern volatile __bit MDBIT __attribute__((address(0x1CE0)));


extern volatile __bit MDCH0 __attribute__((address(0x1CF8)));


extern volatile __bit MDCH1 __attribute__((address(0x1CF9)));


extern volatile __bit MDCH2 __attribute__((address(0x1CFA)));


extern volatile __bit MDCH3 __attribute__((address(0x1CFB)));


extern volatile __bit MDCHODIS __attribute__((address(0x1CFF)));


extern volatile __bit MDCHPOL __attribute__((address(0x1CFE)));


extern volatile __bit MDCHSYNC __attribute__((address(0x1CFD)));


extern volatile __bit MDCL0 __attribute__((address(0x1CF0)));


extern volatile __bit MDCL1 __attribute__((address(0x1CF1)));


extern volatile __bit MDCL2 __attribute__((address(0x1CF2)));


extern volatile __bit MDCL3 __attribute__((address(0x1CF3)));


extern volatile __bit MDCLODIS __attribute__((address(0x1CF7)));


extern volatile __bit MDCLPOL __attribute__((address(0x1CF6)));


extern volatile __bit MDCLSYNC __attribute__((address(0x1CF5)));


extern volatile __bit MDEN __attribute__((address(0x1CE7)));


extern volatile __bit MDMS0 __attribute__((address(0x1CE8)));


extern volatile __bit MDMS1 __attribute__((address(0x1CE9)));


extern volatile __bit MDMS2 __attribute__((address(0x1CEA)));


extern volatile __bit MDMS3 __attribute__((address(0x1CEB)));


extern volatile __bit MDMSODIS __attribute__((address(0x1CEF)));


extern volatile __bit MDOE __attribute__((address(0x1CE6)));


extern volatile __bit MDOPOL __attribute__((address(0x1CE4)));


extern volatile __bit MDOUT __attribute__((address(0x1CE3)));


extern volatile __bit MDSLR __attribute__((address(0x1CE5)));


extern volatile __bit MFIOFR __attribute__((address(0x4D2)));


extern volatile __bit OERR __attribute__((address(0xCE9)));


extern volatile __bit OSFIE __attribute__((address(0x497)));


extern volatile __bit OSFIF __attribute__((address(0x97)));


extern volatile __bit OSTS __attribute__((address(0x4D5)));


extern volatile __bit P1CSEL __attribute__((address(0x8E9)));


extern volatile __bit P1DC0 __attribute__((address(0x14A0)));


extern volatile __bit P1DC1 __attribute__((address(0x14A1)));


extern volatile __bit P1DC2 __attribute__((address(0x14A2)));


extern volatile __bit P1DC3 __attribute__((address(0x14A3)));


extern volatile __bit P1DC4 __attribute__((address(0x14A4)));


extern volatile __bit P1DC5 __attribute__((address(0x14A5)));


extern volatile __bit P1DC6 __attribute__((address(0x14A6)));


extern volatile __bit P1DSEL __attribute__((address(0x8EA)));


extern volatile __bit P1M0 __attribute__((address(0x149E)));


extern volatile __bit P1M1 __attribute__((address(0x149F)));


extern volatile __bit P1RSEN __attribute__((address(0x14A7)));


extern volatile __bit PCIE __attribute__((address(0x10BE)));


extern volatile __bit PEIE __attribute__((address(0x5E)));


extern volatile __bit PEN __attribute__((address(0x10B2)));


extern volatile __bit PLLR __attribute__((address(0x4D6)));


extern volatile __bit PS0 __attribute__((address(0x4A8)));


extern volatile __bit PS1 __attribute__((address(0x4A9)));


extern volatile __bit PS2 __attribute__((address(0x4AA)));


extern volatile __bit PSA __attribute__((address(0x4AB)));


extern volatile __bit PSS1AC0 __attribute__((address(0x14AA)));


extern volatile __bit PSS1AC1 __attribute__((address(0x14AB)));


extern volatile __bit PSS1BD0 __attribute__((address(0x14A8)));


extern volatile __bit PSS1BD1 __attribute__((address(0x14A9)));


extern volatile __bit RA0 __attribute__((address(0x60)));


extern volatile __bit RA1 __attribute__((address(0x61)));


extern volatile __bit RA2 __attribute__((address(0x62)));


extern volatile __bit RA3 __attribute__((address(0x63)));


extern volatile __bit RA4 __attribute__((address(0x64)));


extern volatile __bit RA5 __attribute__((address(0x65)));


extern volatile __bit RA6 __attribute__((address(0x66)));


extern volatile __bit RA7 __attribute__((address(0x67)));


extern volatile __bit RB0 __attribute__((address(0x68)));


extern volatile __bit RB1 __attribute__((address(0x69)));


extern volatile __bit RB2 __attribute__((address(0x6A)));


extern volatile __bit RB3 __attribute__((address(0x6B)));


extern volatile __bit RB4 __attribute__((address(0x6C)));


extern volatile __bit RB5 __attribute__((address(0x6D)));


extern volatile __bit RB6 __attribute__((address(0x6E)));


extern volatile __bit RB7 __attribute__((address(0x6F)));


extern volatile __bit RCEN __attribute__((address(0x10B3)));


extern volatile __bit RCIDL __attribute__((address(0xCFE)));


extern volatile __bit RCIE __attribute__((address(0x48D)));


extern volatile __bit RCIF __attribute__((address(0x8D)));


extern volatile __bit RD __attribute__((address(0xCA8)));


extern volatile __bit RSEN __attribute__((address(0x10B1)));


extern volatile __bit RX9 __attribute__((address(0xCEE)));


extern volatile __bit RX9D __attribute__((address(0xCE8)));


extern volatile __bit RXDTSEL __attribute__((address(0x8EF)));


extern volatile __bit R_nW __attribute__((address(0x10A2)));


extern volatile __bit SBCDE __attribute__((address(0x10BA)));


extern volatile __bit SBOREN __attribute__((address(0x8B7)));


extern volatile __bit SCIE __attribute__((address(0x10BD)));


extern volatile __bit SCKP __attribute__((address(0xCFC)));


extern volatile __bit SCS0 __attribute__((address(0x4C8)));


extern volatile __bit SCS1 __attribute__((address(0x4C9)));


extern volatile __bit SDAHT __attribute__((address(0x10BB)));


extern volatile __bit SDO1SEL __attribute__((address(0x8EE)));


extern volatile __bit SEN __attribute__((address(0x10B0)));


extern volatile __bit SENDB __attribute__((address(0xCF3)));


extern volatile __bit SMP __attribute__((address(0x10A7)));


extern volatile __bit SPEN __attribute__((address(0xCEF)));


extern volatile __bit SPLLEN __attribute__((address(0x4CF)));


extern volatile __bit SRCLK0 __attribute__((address(0x8D4)));


extern volatile __bit SRCLK1 __attribute__((address(0x8D5)));


extern volatile __bit SRCLK2 __attribute__((address(0x8D6)));


extern volatile __bit SREN __attribute__((address(0xCED)));


extern volatile __bit SRLEN __attribute__((address(0x8D7)));


extern volatile __bit SRNQEN __attribute__((address(0x8D2)));


extern volatile __bit SRPR __attribute__((address(0x8D0)));


extern volatile __bit SRPS __attribute__((address(0x8D1)));


extern volatile __bit SRQEN __attribute__((address(0x8D3)));


extern volatile __bit SRRC1E __attribute__((address(0x8D8)));


extern volatile __bit SRRC2E __attribute__((address(0x8D9)));


extern volatile __bit SRRCKE __attribute__((address(0x8DA)));


extern volatile __bit SRRPE __attribute__((address(0x8DB)));


extern volatile __bit SRSC1E __attribute__((address(0x8DC)));


extern volatile __bit SRSC2E __attribute__((address(0x8DD)));


extern volatile __bit SRSCKE __attribute__((address(0x8DE)));


extern volatile __bit SRSPE __attribute__((address(0x8DF)));


extern volatile __bit SS1SEL __attribute__((address(0x8ED)));


extern volatile __bit SSP1IE __attribute__((address(0x48B)));


extern volatile __bit SSP1IF __attribute__((address(0x8B)));


extern volatile __bit SSPEN __attribute__((address(0x10AD)));


extern volatile __bit SSPM0 __attribute__((address(0x10A8)));


extern volatile __bit SSPM1 __attribute__((address(0x10A9)));


extern volatile __bit SSPM2 __attribute__((address(0x10AA)));


extern volatile __bit SSPM3 __attribute__((address(0x10AB)));


extern volatile __bit SSPOV __attribute__((address(0x10AE)));


extern volatile __bit STKOVF __attribute__((address(0x4B7)));


extern volatile __bit STKUNF __attribute__((address(0x4B6)));


extern volatile __bit STR1A __attribute__((address(0x14B0)));


extern volatile __bit STR1B __attribute__((address(0x14B1)));


extern volatile __bit STR1C __attribute__((address(0x14B2)));


extern volatile __bit STR1D __attribute__((address(0x14B3)));


extern volatile __bit STR1SYNC __attribute__((address(0x14B4)));


extern volatile __bit SWDTEN __attribute__((address(0x4B8)));


extern volatile __bit SYNC __attribute__((address(0xCF4)));


extern volatile __bit T0CS __attribute__((address(0x4AD)));


extern volatile __bit T0IE __attribute__((address(0x5D)));


extern volatile __bit T0IF __attribute__((address(0x5A)));


extern volatile __bit T0SE __attribute__((address(0x4AC)));


extern volatile __bit T0XCS __attribute__((address(0xF0)));


extern volatile __bit T1CKPS0 __attribute__((address(0xC4)));


extern volatile __bit T1CKPS1 __attribute__((address(0xC5)));


extern volatile __bit T1GGO __attribute__((address(0xCB)));


extern volatile __bit T1GPOL __attribute__((address(0xCE)));


extern volatile __bit T1GSPM __attribute__((address(0xCC)));


extern volatile __bit T1GSS0 __attribute__((address(0xC8)));


extern volatile __bit T1GSS1 __attribute__((address(0xC9)));


extern volatile __bit T1GTM __attribute__((address(0xCD)));


extern volatile __bit T1GVAL __attribute__((address(0xCA)));


extern volatile __bit T1OSCEN __attribute__((address(0xC3)));


extern volatile __bit T1OSCR __attribute__((address(0x4D7)));


extern volatile __bit T2CKPS0 __attribute__((address(0xE0)));


extern volatile __bit T2CKPS1 __attribute__((address(0xE1)));


extern volatile __bit T2OUTPS0 __attribute__((address(0xE3)));


extern volatile __bit T2OUTPS1 __attribute__((address(0xE4)));


extern volatile __bit T2OUTPS2 __attribute__((address(0xE5)));


extern volatile __bit T2OUTPS3 __attribute__((address(0xE6)));


extern volatile __bit TMR0CS __attribute__((address(0x4AD)));


extern volatile __bit TMR0IE __attribute__((address(0x5D)));


extern volatile __bit TMR0IF __attribute__((address(0x5A)));


extern volatile __bit TMR0SE __attribute__((address(0x4AC)));


extern volatile __bit TMR1CS0 __attribute__((address(0xC6)));


extern volatile __bit TMR1CS1 __attribute__((address(0xC7)));


extern volatile __bit TMR1GE __attribute__((address(0xCF)));


extern volatile __bit TMR1GIE __attribute__((address(0x48F)));


extern volatile __bit TMR1GIF __attribute__((address(0x8F)));


extern volatile __bit TMR1IE __attribute__((address(0x488)));


extern volatile __bit TMR1IF __attribute__((address(0x88)));


extern volatile __bit TMR1ON __attribute__((address(0xC0)));


extern volatile __bit TMR2IE __attribute__((address(0x489)));


extern volatile __bit TMR2IF __attribute__((address(0x89)));


extern volatile __bit TMR2ON __attribute__((address(0xE2)));


extern volatile __bit TRISA0 __attribute__((address(0x460)));


extern volatile __bit TRISA1 __attribute__((address(0x461)));


extern volatile __bit TRISA2 __attribute__((address(0x462)));


extern volatile __bit TRISA3 __attribute__((address(0x463)));


extern volatile __bit TRISA4 __attribute__((address(0x464)));


extern volatile __bit TRISA5 __attribute__((address(0x465)));


extern volatile __bit TRISA6 __attribute__((address(0x466)));


extern volatile __bit TRISA7 __attribute__((address(0x467)));


extern volatile __bit TRISB0 __attribute__((address(0x468)));


extern volatile __bit TRISB1 __attribute__((address(0x469)));


extern volatile __bit TRISB2 __attribute__((address(0x46A)));


extern volatile __bit TRISB3 __attribute__((address(0x46B)));


extern volatile __bit TRISB4 __attribute__((address(0x46C)));


extern volatile __bit TRISB5 __attribute__((address(0x46D)));


extern volatile __bit TRISB6 __attribute__((address(0x46E)));


extern volatile __bit TRISB7 __attribute__((address(0x46F)));


extern volatile __bit TRMT __attribute__((address(0xCF1)));


extern volatile __bit TSEN __attribute__((address(0x8BD)));


extern volatile __bit TSRNG __attribute__((address(0x8BC)));


extern volatile __bit TUN0 __attribute__((address(0x4C0)));


extern volatile __bit TUN1 __attribute__((address(0x4C1)));


extern volatile __bit TUN2 __attribute__((address(0x4C2)));


extern volatile __bit TUN3 __attribute__((address(0x4C3)));


extern volatile __bit TUN4 __attribute__((address(0x4C4)));


extern volatile __bit TUN5 __attribute__((address(0x4C5)));


extern volatile __bit TX9 __attribute__((address(0xCF6)));


extern volatile __bit TX9D __attribute__((address(0xCF0)));


extern volatile __bit TXCKSEL __attribute__((address(0x8F0)));


extern volatile __bit TXEN __attribute__((address(0xCF5)));


extern volatile __bit TXIE __attribute__((address(0x48C)));


extern volatile __bit TXIF __attribute__((address(0x8C)));


extern volatile __bit UA __attribute__((address(0x10A1)));


extern volatile __bit WCOL __attribute__((address(0x10AF)));


extern volatile __bit WDTPS0 __attribute__((address(0x4B9)));


extern volatile __bit WDTPS1 __attribute__((address(0x4BA)));


extern volatile __bit WDTPS2 __attribute__((address(0x4BB)));


extern volatile __bit WDTPS3 __attribute__((address(0x4BC)));


extern volatile __bit WDTPS4 __attribute__((address(0x4BD)));


extern volatile __bit WPUA5 __attribute__((address(0x1065)));


extern volatile __bit WPUB0 __attribute__((address(0x1068)));


extern volatile __bit WPUB1 __attribute__((address(0x1069)));


extern volatile __bit WPUB2 __attribute__((address(0x106A)));


extern volatile __bit WPUB3 __attribute__((address(0x106B)));


extern volatile __bit WPUB4 __attribute__((address(0x106C)));


extern volatile __bit WPUB5 __attribute__((address(0x106D)));


extern volatile __bit WPUB6 __attribute__((address(0x106E)));


extern volatile __bit WPUB7 __attribute__((address(0x106F)));


extern volatile __bit WR __attribute__((address(0xCA9)));


extern volatile __bit WREN __attribute__((address(0xCAA)));


extern volatile __bit WRERR __attribute__((address(0xCAB)));


extern volatile __bit WUE __attribute__((address(0xCF9)));


extern volatile __bit ZERO __attribute__((address(0x1A)));


extern volatile __bit Z_SHAD __attribute__((address(0x7F22)));


extern volatile __bit nBOR __attribute__((address(0x4B0)));


extern volatile __bit nPD __attribute__((address(0x1B)));


extern volatile __bit nPOR __attribute__((address(0x4B1)));


extern volatile __bit nRI __attribute__((address(0x4B2)));


extern volatile __bit nRMCLR __attribute__((address(0x4B3)));


extern volatile __bit nT1SYNC __attribute__((address(0xC2)));


extern volatile __bit nTO __attribute__((address(0x1C)));


extern volatile __bit nWPUEN __attribute__((address(0x4AF)));
# 385 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/pic_chip_select.h" 2 3
# 14 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/pic.h" 2 3
# 30 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/pic.h" 3
#pragma intrinsic(__nop)
extern void __nop(void);
# 78 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/pic.h" 3
__attribute__((__unsupported__("The " "FLASH_READ" " macro function is no longer supported. Please use the MPLAB X MCC."))) unsigned char __flash_read(unsigned short addr);

__attribute__((__unsupported__("The " "FLASH_WRITE" " macro function is no longer supported. Please use the MPLAB X MCC."))) void __flash_write(unsigned short addr, unsigned short data);

__attribute__((__unsupported__("The " "FLASH_ERASE" " macro function is no longer supported. Please use the MPLAB X MCC."))) void __flash_erase(unsigned short addr);



# 1 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/eeprom_routines.h" 1 3
# 114 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/eeprom_routines.h" 3
extern void eeprom_write(unsigned char addr, unsigned char value);
extern unsigned char eeprom_read(unsigned char addr);
# 86 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/pic.h" 2 3





#pragma intrinsic(_delay)
extern __attribute__((nonreentrant)) void _delay(unsigned long);
#pragma intrinsic(_delaywdt)
extern __attribute__((nonreentrant)) void _delaywdt(unsigned long);

#pragma intrinsic(_delay3)
extern __attribute__((nonreentrant)) void _delay3(unsigned char);
# 137 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/pic.h" 3
extern __bank0 unsigned char __resetbits;
extern __bank0 __bit __powerdown;
extern __bank0 __bit __timeout;
# 28 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC12-16F1xxx_DFP/1.2.63/xc8/pic/include/xc.h" 2 3
# 6 "/media/user/software/simulide/tests/MCU/pic/0_Test/16f1826/16f1826_Usart/16f1826_Rx.c" 2


void __attribute__((picinterrupt(("")))) myisr()
{
    if( RCIF ){
        PORTA = RCREG;
        PORTB ^= 1<<4;
    }
}

void main()
{
    ANSELA = 0x00;
    ANSELB = 0x00;

    PORTB = 0;
    PORTA = 0;
    TRISB = 0b11001111;
    TRISA = 0;


    SPBRG = 32;
    SPEN = 1;
    CREN = 1;

    RCIE = 1;
    PEIE = 1;
    GIE = 1;

    TMR2ON = 1;

    while(1)
    {
        _delay((unsigned long)((500)*(20000000UL/4000.0)));
        PORTB ^= 1<<5;
    }
}
