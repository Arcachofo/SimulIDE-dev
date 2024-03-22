;Program compiled by Great Cow BASIC (0.98.<<>> 2021-<<>>-24 (Linux 64 bit) : Build 1005) for Microchip MPASM
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Chip Model: MEGA328P
;Assembler header file
.INCLUDE "m328pdef.inc"

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
.EQU	ADREADPORT=256
.EQU	COUNT=257
.EQU	READAD=258

;********************************************************************************

;Register variables
.DEF	DELAYTEMP=r25
.DEF	DELAYTEMP2=r26
.DEF	SYSBITTEST=r5
.DEF	SYSVALUECOPY=r21
.DEF	SYSWAITTEMP10US=r27
.DEF	SYSWAITTEMPMS=r29
.DEF	SYSWAITTEMPMS_H=r30
.DEF	SYSTEMP1=r0
.DEF	SYSTEMP2=r16

;********************************************************************************

;Alias variables
#define	SYSREADADBYTE	258

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
	reti	;PCINT0
.ORG	8
	reti	;PCINT1
.ORG	10
	reti	;PCINT2
.ORG	12
	reti	;WDT
.ORG	14
	reti	;TIMER2_COMPA
.ORG	16
	reti	;TIMER2_COMPB
.ORG	18
	reti	;TIMER2_OVF
.ORG	20
	reti	;TIMER1_CAPT
.ORG	22
	reti	;TIMER1_COMPA
.ORG	24
	reti	;TIMER1_COMPB
.ORG	26
	reti	;TIMER1_OVF
.ORG	28
	reti	;TIMER0_COMPA
.ORG	30
	reti	;TIMER0_COMPB
.ORG	32
	reti	;TIMER0_OVF
.ORG	34
	reti	;SPI_STC
.ORG	36
	reti	;USART_RX
.ORG	38
	reti	;USART_UDRE
.ORG	40
	reti	;USART_TX
.ORG	42
	reti	;ADC
.ORG	44
	reti	;EE_READY
.ORG	46
	reti	;ANALOG_COMP
.ORG	48
	reti	;TWI
.ORG	50
	reti	;SPM_READY

;********************************************************************************

;Start of program memory page 0
.ORG	52
BASPROGRAMSTART:
;Initialise stack
	ldi	SysValueCopy,high(RAMEND)
	out	SPH, SysValueCopy
	ldi	SysValueCopy,low(RAMEND)
	out	SPL, SysValueCopy
;Call initialisation routines
	rcall	INITSYS
;Automatic pin direction setting
	sbi	DDRB,0

;Start of the main program
SysDoLoop_S1:
	ldi	SysValueCopy,0
	sts	ADREADPORT,SysValueCopy
	rcall	FN_READAD4
	lds	SysTemp1,SYSREADADBYTE
	lsr	SysTemp1
	sts	COUNT,SysTemp1
	ldi	SysTemp2,89
	add	SysTemp1,SysTemp2
	sts	COUNT,SysTemp1
	sbi	PORTB,0
	lds	SysWaitTemp10US,COUNT
	rcall	Delay_10US
	cbi	PORTB,0
	ldi	SysWaitTempMS,20
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
	ldi	DELAYTEMP,52
DelayUS1:
	dec	DELAYTEMP
	brne	DelayUS1
	nop
	dec	SysWaitTemp10US
	brne	D10US_START
	ret

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
	out	PORTB,SysValueCopy
	ldi	SysValueCopy,0
	out	PORTC,SysValueCopy
	ldi	SysValueCopy,0
	out	PORTD,SysValueCopy
	ret

;********************************************************************************

FN_READAD4:
	lds	SysValueCopy,ADREADPORT
	sts	ADMUX,SysValueCopy
	sbr	SysValueCopy,1<<ADLAR
	sts	ADMUX,SysValueCopy
;ASM showdebug  'Bit(REFS1) does exist, so assume 'VCC used as analog reference' REFS0=b'1'
	sbr	SysValueCopy,1<<REFS0
	sts	ADMUX,SysValueCopy
	lds	SysValueCopy,ADCSRA
	sbr	SysValueCopy,1<<ADPS2
	sts	ADCSRA,SysValueCopy
	cbr	SysValueCopy,1<<ADPS1
	sts	ADCSRA,SysValueCopy
	ldi	SysWaitTemp10US,2
	rcall	Delay_10US
	lds	SysValueCopy,ADCSRA
	sbr	SysValueCopy,1<<ADEN
	sts	ADCSRA,SysValueCopy
	sbr	SysValueCopy,1<<ADSC
	sts	ADCSRA,SysValueCopy
SysWaitLoop1:
	lds	SysBitTest,ADCSRA
	sbrc	SysBitTest,ADSC
	rjmp	SysWaitLoop1
	lds	SysValueCopy,ADCSRA
	cbr	SysValueCopy,1<<ADEN
	sts	ADCSRA,SysValueCopy
	lds	SysValueCopy,ADCH
	sts	READAD,SysValueCopy
	ret

;********************************************************************************


