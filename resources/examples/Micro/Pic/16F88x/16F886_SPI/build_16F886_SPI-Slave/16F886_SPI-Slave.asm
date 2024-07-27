;Program compiled by Great Cow BASIC (1.00.00 2022-12-01 (Linux 64 bit) : Build 1199) for Microchip MPASM/MPLAB-X Assembler
;Need help? 
;  See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;  Check the documentation and Help at http://gcbasic.sourceforge.net/help/,
;or, email us:
;   w_cholmondeley at users dot sourceforge dot net
;   evanvennn at users dot sourceforge dot net

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=16F886, r=DEC
#include <P16F886.inc>
 __CONFIG _CONFIG1, _LVP_OFF & _FCMEN_ON & _CPD_OFF & _CP_OFF & _MCLRE_OFF & _WDTE_OFF & _FOSC_HS
 __CONFIG _CONFIG2, _WRT_OFF

;********************************************************************************

;Set aside memory locations for variables
DISPCHAR                         EQU      32          ; 0x20
DISPPORT                         EQU      33          ; 0x21
DISPTEMP                         EQU      34          ; 0x22
SYSSTRINGA                       EQU     119          ; 0x77
_DISPDOT                         EQU      35          ; 0x23

;********************************************************************************

;Vectors
	ORG	0
	pagesel	BASPROGRAMSTART
	goto	BASPROGRAMSTART
	ORG	4
	retfie

;********************************************************************************

;Start of program memory page 0
	ORG	5
BASPROGRAMSTART
;Call initialisation routines
	call	INITSYS
	call	INITSEVENSEG

;Start of the main program
	banksel	TRISC
	bcf	TRISC,5
	bsf	TRISC,3
	movlw	37
	banksel	SSPCON
	movwf	SSPCON
MAIN
	btfss	PIR1,SSPIF
	goto	ENDIF1
	movlw	1
	movwf	DISPPORT
	movf	SSPBUF,W
	movwf	DISPCHAR
	clrf	_DISPDOT
	call	DISPLAYVALUE
	bcf	PIR1,SSPIF
ENDIF1
	goto	MAIN
BASPROGRAMEND
	sleep
	goto	BASPROGRAMEND

;********************************************************************************

DISPLAYVALUE
	incf	DISPCHAR,W
	movwf	SYSSTRINGA
	call	SEVENSEGDISPDIGIT
	movwf	DISPTEMP
	btfss	_DISPDOT,0
	goto	ENDIF2
	movlw	128
	iorwf	DISPTEMP,F
ENDIF2
	bcf	PORTB,7
	bcf	PORTB,6
	bcf	PORTB,5
	bcf	PORTB,4
	bcf	PORTB,3
	bcf	PORTB,2
	bcf	PORTB,1
	bcf	PORTB,0
	bcf	PORTC,7
	decf	DISPPORT,W
	btfsc	STATUS, Z
	bsf	PORTC,7
	btfsc	DISPTEMP,0
	bsf	PORTB,7
	btfsc	DISPTEMP,1
	bsf	PORTB,6
	btfsc	DISPTEMP,2
	bsf	PORTB,5
	btfsc	DISPTEMP,3
	bsf	PORTB,4
	btfsc	DISPTEMP,4
	bsf	PORTB,3
	btfsc	DISPTEMP,5
	bsf	PORTB,2
	btfsc	DISPTEMP,6
	bsf	PORTB,1
	btfsc	DISPTEMP,7
	bsf	PORTB,0
	return

;********************************************************************************

INITSEVENSEG
	banksel	TRISB
	bcf	TRISB,7
	bcf	TRISB,6
	bcf	TRISB,5
	bcf	TRISB,4
	bcf	TRISB,3
	bcf	TRISB,2
	bcf	TRISB,1
	bcf	TRISB,0
	bcf	TRISC,7
	banksel	STATUS
	return

;********************************************************************************

INITSYS
;asm showdebug _For_selected_frequency_-_the_external_oscillator_has_been_selected_by_compiler ChipMHz
;asm showdebug _Complete_the_chip_setup_of_BSR,ADCs,ANSEL_and_other_key_setup_registers_or_register_bits
	banksel	ADCON1
	bcf	ADCON1,ADFM
	banksel	ADCON0
	bcf	ADCON0,ADON
	banksel	ANSEL
	clrf	ANSEL
	clrf	ANSELH
	banksel	CM2CON0
	bcf	CM2CON0,C2ON
	bcf	CM1CON0,C1ON
	banksel	PORTA
	clrf	PORTA
	clrf	PORTB
	clrf	PORTC
	clrf	PORTE
	return

;********************************************************************************

SEVENSEGDISPDIGIT
	movlw	17
	subwf	SysStringA, W
	btfsc	STATUS, C
	retlw	0
	movf	SysStringA, W
	addlw	low TABLESEVENSEGDISPDIGIT
	movwf	SysStringA
	movlw	high TABLESEVENSEGDISPDIGIT
	btfsc	STATUS, C
	addlw	1
	movwf	PCLATH
	movf	SysStringA, W
	movwf	PCL
TABLESEVENSEGDISPDIGIT
	retlw	16
	retlw	63
	retlw	6
	retlw	91
	retlw	79
	retlw	102
	retlw	109
	retlw	125
	retlw	7
	retlw	127
	retlw	111
	retlw	119
	retlw	124
	retlw	57
	retlw	94
	retlw	121
	retlw	113

;********************************************************************************

;Start of program memory page 1
	ORG	2048
;Start of program memory page 2
	ORG	4096
;Start of program memory page 3
	ORG	6144

 END
