;Program compiled by Great Cow BASIC (0.98.<<>> 2021-<<>>-24 (Linux 64 bit) : Build 1005) for Microchip MPASM
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Chip Model: TINY13A
;Assembler header file
.INCLUDE "tn13def.inc"

;SREG bit names (for AVR Assembler compatibility, GCBASIC uses different names)
#define C 0
#define H 5
#define I 7
#define N 2
#define S 4
#define T 6
#define V 3
#define Z 1

;********************************************************************************

;Set aside memory locations for variables
.EQU	ADREADPORT=96
.EQU	LDR=97
.EQU	READAD=98

;********************************************************************************

;Register variables
.DEF	DELAYTEMP=r25
.DEF	DELAYTEMP2=r26
.DEF	SYSVALUECOPY=r21
.DEF	SYSWAITTEMP10US=r27
.DEF	SYSWAITTEMPMS=r29
.DEF	SYSWAITTEMPMS_H=r30

;********************************************************************************

;Alias variables
.EQU	SYSREADADBYTE=98

;********************************************************************************

;Vectors
;Interrupt vectors
.ORG	0
	rjmp	BASPROGRAMSTART ;Reset
.ORG	1
	reti	;INT0
.ORG	2
	reti	;PCINT0
.ORG	3
	reti	;TIM0_OVF
.ORG	4
	reti	;EE_RDY
.ORG	5
	reti	;ANA_COMP
.ORG	6
	reti	;TIM0_COMPA
.ORG	7
	reti	;TIM0_COMPB
.ORG	8
	reti	;WDT
.ORG	9
	reti	;ADC

;********************************************************************************

;Start of program memory page 0
.ORG	11
BASPROGRAMSTART:
;Initialise stack
	ldi	SysValueCopy,low(RAMEND)
	out	SPL, SysValueCopy
;Call initialisation routines
	rcall	INITSYS

;Start of the main program
SysDoLoop_S1:
	ldi	SysValueCopy,1
	sts	ADREADPORT,SysValueCopy
	rcall	FN_READAD4
	lds	SysValueCopy,SYSREADADBYTE
	sts	LDR,SysValueCopy
	ldi	SysWaitTempMS,100
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
	rjmp	SysDoLoop_S1
SysDoLoop_E1:
BASPROGRAMEND:
	sleep
	rjmp	BASPROGRAMEND

;********************************************************************************

Delay_10US:
D10US_START:
	ldi	DELAYTEMP,15
DelayUS0:
	dec	DELAYTEMP
	brne	DelayUS0
	dec	SysWaitTemp10US
	brne	D10US_START
	ret

;********************************************************************************

Delay_MS:
	inc	SysWaitTempMS_H
DMS_START:
	ldi	DELAYTEMP2,123
DMS_OUTER:
	ldi	DELAYTEMP,12
DMS_INNER:
	dec	DELAYTEMP
	brne	DMS_INNER
	dec	DELAYTEMP2
	brne	DMS_OUTER
	dec	SysWaitTempMS
	brne	DMS_START
	dec	SysWaitTempMS_H
	brne	DMS_START
	ret

;********************************************************************************

INITSYS:
	ldi	SysValueCopy,0
	out	PORTB,SysValueCopy
	ret

;********************************************************************************

FN_READAD4:
	lds	SysValueCopy,ADREADPORT
	out	ADMUX,SysValueCopy
	sbi	ADMUX,ADLAR
;ASM showdebug  'Bit(REFS1) does not exist, so assume 'VCC used as analog reference' REFS0=b'0'
	cbi	ADMUX,REFS0
	sbi	ADCSRA,ADPS2
	cbi	ADCSRA,ADPS1
	ldi	SysWaitTemp10US,2
	rcall	Delay_10US
	sbi	ADCSRA,ADEN
	sbi	ADCSRA,ADSC
SysWaitLoop1:
	sbic	ADCSRA,ADSC
	rjmp	SysWaitLoop1
	cbi	ADCSRA,ADEN
	in	SysValueCopy,ADCH
	sts	READAD,SysValueCopy
	ret

;********************************************************************************


