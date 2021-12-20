;Program compiled by Great Cow BASIC (0.98.<<>> 2021-<<>>-24 (Linux 64 bit) : Build 1005) for Microchip MPASM
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=16F876A, r=DEC
#include <P16F876A.inc>
 __CONFIG _CP_OFF & _WRT_OFF & _CPD_OFF & _LVP_OFF & _WDTE_OFF & _FOSC_HS

;********************************************************************************

;Set aside memory locations for variables
ADREADPORT                       EQU 32
DELAYTEMP                        EQU 112
DELAYTEMP2                       EQU 113
DISPCHAR                         EQU 33
DISPPORT                         EQU 34
DISPTEMP                         EQU 35
READAD                           EQU 36
SYSBYTETEMPA                     EQU 117
SYSBYTETEMPB                     EQU 121
SYSBYTETEMPX                     EQU 112
SYSDIVLOOP                       EQU 116
SYSSTRINGA                       EQU 119
SYSWAITTEMP10US                  EQU 117
SYSWAITTEMPMS                    EQU 114
SYSWAITTEMPMS_H                  EQU 115
VALUE                            EQU 37
_DISPDOT                         EQU 38

;********************************************************************************

;Alias variables
SYSREADADBYTE EQU 36

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
	banksel	TRISB
	clrf	TRISB
	bsf	TRISA,0
	bcf	TRISC,7
MAIN
	banksel	ADREADPORT
	clrf	ADREADPORT
	call	FN_READAD4
	movf	SYSREADADBYTE,W
	movwf	SysBYTETempA
	movlw	26
	movwf	SysBYTETempB
	call	SYSDIVSUB
	movf	SysBYTETempA,W
	movwf	VALUE
	movlw	1
	movwf	DISPPORT
	movf	VALUE,W
	movwf	DISPCHAR
	clrf	_DISPDOT
	call	DISPLAYVALUE
	movlw	10
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
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
	goto	ENDIF4
	movlw	128
	iorwf	DISPTEMP,F
ENDIF4
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

Delay_10US
D10US_START
	movlw	12
	movwf	DELAYTEMP
DelayUS0
	decfsz	DELAYTEMP,F
	goto	DelayUS0
	decfsz	SysWaitTemp10US, F
	goto	D10US_START
	return

;********************************************************************************

Delay_MS
	incf	SysWaitTempMS_H, F
DMS_START
	movlw	108
	movwf	DELAYTEMP2
DMS_OUTER
	movlw	11
	movwf	DELAYTEMP
DMS_INNER
	decfsz	DELAYTEMP, F
	goto	DMS_INNER
	decfsz	DELAYTEMP2, F
	goto	DMS_OUTER
	decfsz	SysWaitTempMS, F
	goto	DMS_START
	decfsz	SysWaitTempMS_H, F
	goto	DMS_START
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
	banksel	ADCON1
	bcf	ADCON1,PCFG3
	bsf	ADCON1,PCFG2
	bsf	ADCON1,PCFG1
	bcf	ADCON1,PCFG0
	movlw	7
	movwf	CMCON
	banksel	PORTA
	clrf	PORTA
	clrf	PORTB
	clrf	PORTC
	return

;********************************************************************************

FN_READAD4
	banksel	ADCON1
	bcf	ADCON1,ADFM
	bcf	ADCON1,PCFG3
	bcf	ADCON1,PCFG2
	bcf	ADCON1,PCFG1
	bcf	ADCON1,PCFG0
	bcf	ADCON1,ADCS2
	banksel	ADCON0
	bcf	ADCON0,ADCS1
	bsf	ADCON0,ADCS0
	bcf	ADCON0,CHS0
	bcf	ADCON0,CHS1
	bcf	ADCON0,CHS2
	btfsc	ADREADPORT,0
	bsf	ADCON0,CHS0
	btfsc	ADREADPORT,1
	bsf	ADCON0,CHS1
	btfsc	ADREADPORT,2
	bsf	ADCON0,CHS2
	bsf	ADCON0,ADON
	movlw	2
	movwf	SysWaitTemp10US
	call	Delay_10US
	bsf	ADCON0,GO_NOT_DONE
	nop
SysWaitLoop1
	btfsc	ADCON0,GO_NOT_DONE
	goto	SysWaitLoop1
	bcf	ADCON0,ADON
	banksel	ADCON1
	bcf	ADCON1,PCFG3
	bsf	ADCON1,PCFG2
	bsf	ADCON1,PCFG1
	bcf	ADCON1,PCFG0
	banksel	ADRESH
	movf	ADRESH,W
	movwf	READAD
	banksel	ADCON1
	bcf	ADCON1,ADFM
	banksel	STATUS
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

SYSDIVSUB
	movf	SYSBYTETEMPB, F
	btfsc	STATUS, Z
	return
	clrf	SYSBYTETEMPX
	movlw	8
	movwf	SYSDIVLOOP
SYSDIV8START
	bcf	STATUS, C
	rlf	SYSBYTETEMPA, F
	rlf	SYSBYTETEMPX, F
	movf	SYSBYTETEMPB, W
	subwf	SYSBYTETEMPX, F
	bsf	SYSBYTETEMPA, 0
	btfsc	STATUS, C
	goto	DIV8NOTNEG
	bcf	SYSBYTETEMPA, 0
	movf	SYSBYTETEMPB, W
	addwf	SYSBYTETEMPX, F
DIV8NOTNEG
	decfsz	SYSDIVLOOP, F
	goto	SYSDIV8START
	return

;********************************************************************************

;Start of program memory page 1
	ORG	2048
;Start of program memory page 2
	ORG	4096
;Start of program memory page 3
	ORG	6144

 END
