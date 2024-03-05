;Program compiled by Great Cow BASIC (1.00.00 2022-12-01 (Linux 64 bit) : Build 1199) for Microchip AVR Assembler
;Need help? 
;  See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;  Check the documentation and Help at http://gcbasic.sourceforge.net/help/,
;or, email us:
;   w_cholmondeley at users dot sourceforge dot net
;   evanvennn at users dot sourceforge dot net

;********************************************************************************

;Chip Model: MEGA32
;Assembler header file
.INCLUDE "m32def.inc"

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

;Register variables
.DEF	DELAYTEMP=r25
.DEF	DELAYTEMP2=r26
.DEF	SYSVALUECOPY=r21
.DEF	SYSWAITTEMPMS=r29
.DEF	SYSWAITTEMPMS_H=r30

;********************************************************************************

;Vectors
;Interrupt vectors
.ORG	0
	rjmp	BASPROGRAMSTART ;Reset
.ORG	2
	reti	;INT0
.ORG	4
	reti	;INT1
.ORG	6
	reti	;INT2
.ORG	8
	reti	;TIMER2_COMP
.ORG	10
	reti	;TIMER2_OVF
.ORG	12
	reti	;TIMER1_CAPT
.ORG	14
	reti	;TIMER1_COMPA
.ORG	16
	reti	;TIMER1_COMPB
.ORG	18
	reti	;TIMER1_OVF
.ORG	20
	reti	;TIMER0_COMP
.ORG	22
	reti	;TIMER0_OVF
.ORG	24
	reti	;SPI_STC
.ORG	26
	reti	;USART_RXC
.ORG	28
	reti	;USART_UDRE
.ORG	30
	reti	;USART_TXC
.ORG	32
	reti	;ADC
.ORG	34
	reti	;EE_RDY
.ORG	36
	reti	;ANA_COMP
.ORG	38
	reti	;TWI
.ORG	40
	reti	;SPM_RDY

;********************************************************************************

;Start of program memory page 0
.ORG	42
BASPROGRAMSTART:
;Initialise stack
	ldi	SysValueCopy,high(RAMEND)
	out	SPH, SysValueCopy
	ldi	SysValueCopy,low(RAMEND)
	out	SPL, SysValueCopy
;Call initialisation routines
	rcall	INITSYS

;Start of the main program
	ldi	SysValueCopy,0
	out	ADMUX,SysValueCopy
	ldi	SysValueCopy,255
	out	DDRB,SysValueCopy
	ldi	SysValueCopy,255
	out	DDRD,SysValueCopy
SysDoLoop_S1:
	ldi	SysValueCopy,199
	out	ADCSRA,SysValueCopy
SysDoLoop_S2:
	sbic	ADCSRA,ADSC
	rjmp	SysDoLoop_S2
SysDoLoop_E2:
	in	SysValueCopy,ADCL
	out	PORTB,SysValueCopy
	in	SysValueCopy,ADCH
	out	PORTD,SysValueCopy
	ldi	SysWaitTempMS,100
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
	rjmp	SysDoLoop_S1
SysDoLoop_E1:
BASPROGRAMEND:
	sleep
	rjmp	BASPROGRAMEND

;********************************************************************************

Delay_MS:
	inc	SysWaitTempMS_H
DMS_START:
	ldi	DELAYTEMP2,254
DMS_OUTER:
	ldi	DELAYTEMP,20
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
	out	PORTA,SysValueCopy
	ldi	SysValueCopy,0
	out	PORTB,SysValueCopy
	ldi	SysValueCopy,0
	out	PORTC,SysValueCopy
	ldi	SysValueCopy,0
	out	PORTD,SysValueCopy
	ret

;********************************************************************************


