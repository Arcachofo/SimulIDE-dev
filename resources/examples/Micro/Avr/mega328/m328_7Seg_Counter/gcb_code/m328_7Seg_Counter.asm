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
.EQU	DISPCHAR=256
.EQU	DISPPORT=257
.EQU	DISPTEMP=258
.EQU	TEMP=259
.EQU	_DISPDOT=260

;********************************************************************************

;Register variables
.DEF	DELAYTEMP=r25
.DEF	DELAYTEMP2=r26
.DEF	SYSBITTEST=r5
.DEF	SYSBYTETEMPX=r0
.DEF	SYSCALCTEMPA=r22
.DEF	SYSCALCTEMPB=r28
.DEF	SYSREADA=r30
.DEF	SYSREADA_H=r31
.DEF	SYSSTRINGA=r26
.DEF	SYSVALUECOPY=r21
.DEF	SYSWAITTEMPMS=r29
.DEF	SYSWAITTEMPMS_H=r30
.DEF	SYSTEMP1=r1
.DEF	SYSTEMP2=r16

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
	rcall	INITSEVENSEG

;Start of the main program
	ldi	SysValueCopy,255
	out	DDRB,SysValueCopy
	sbi	DDRD,1
	ldi	SysValueCopy,0
	sts	TEMP,SysValueCopy
MAIN:
	ldi	SysValueCopy,1
	sts	DISPPORT,SysValueCopy
	lds	SysValueCopy,TEMP
	sts	DISPCHAR,SysValueCopy
	ldi	SysValueCopy,0
	sts	_DISPDOT,SysValueCopy
	rcall	DISPLAYVALUE
	ldi	SysWaitTempMS,250
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
	ldi	SysWaitTempMS,250
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
	lds	SysTemp1,TEMP
	inc	SysTemp1
	sts	TEMP,SysTemp1
	ldi	SysCalcTempA,9
	lds	SysCalcTempB,TEMP
	cp	SysCalcTempA,SysCalcTempB
	brsh	ENDIF1
	ldi	SysWaitTempMS,250
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
	ldi	SysWaitTempMS,250
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
	ldi	SysValueCopy,0
	sts	TEMP,SysValueCopy
ENDIF1:
	rjmp	MAIN
	rjmp	BASPROGRAMEND
BASPROGRAMEND:
	sleep
	rjmp	BASPROGRAMEND

;********************************************************************************

DISPLAYVALUE:
	lds	SysTemp1,DISPCHAR
	inc	SysTemp1
	mov	SYSSTRINGA,SysTemp1
	rcall	SEVENSEGDISPDIGIT
	sts	DISPTEMP,SysByteTempX
	lds	SysBitTest,_DISPDOT
	sbrs	SysBitTest,0
	rjmp	ENDIF2
	ldi	SysTemp2,128
	lds	SysTemp1,DISPTEMP
	or	SysTemp1,SysTemp2
	sts	DISPTEMP,SysTemp1
ENDIF2:
	cbi	PORTB,0
	cbi	PORTB,1
	cbi	PORTB,2
	cbi	PORTB,3
	cbi	PORTB,4
	cbi	PORTB,5
	cbi	PORTB,6
	cbi	PORTD,1
	lds	SysCalcTempA,DISPPORT
	cpi	SysCalcTempA,1
	brne	ENDIF3
	sbi	PORTD,1
ENDIF3:
	lds	SysBitTest,DISPTEMP
	sbrc	SysBitTest,0
	sbi	PORTB,0
	lds	SysBitTest,DISPTEMP
	sbrc	SysBitTest,1
	sbi	PORTB,1
	lds	SysBitTest,DISPTEMP
	sbrc	SysBitTest,2
	sbi	PORTB,2
	lds	SysBitTest,DISPTEMP
	sbrc	SysBitTest,3
	sbi	PORTB,3
	lds	SysBitTest,DISPTEMP
	sbrc	SysBitTest,4
	sbi	PORTB,4
	lds	SysBitTest,DISPTEMP
	sbrc	SysBitTest,5
	sbi	PORTB,5
	lds	SysBitTest,DISPTEMP
	sbrc	SysBitTest,6
	sbi	PORTB,6
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

INITSEVENSEG:
	sbi	DDRB,0
	sbi	DDRB,1
	sbi	DDRB,2
	sbi	DDRB,3
	sbi	DDRB,4
	sbi	DDRB,5
	sbi	DDRB,6
	sbi	DDRD,1
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

SEVENSEGDISPDIGIT:
	cpi	SysStringA, 17
	brlo	PC + 3
	clr	SysByteTempX
	ret
	ldi	SysReadA, low(TABLESEVENSEGDISPDIGIT<<1)
	ldi	SysReadA_H, high(TABLESEVENSEGDISPDIGIT<<1)
	add	SysReadA, SysStringA
	brcc	PC + 2
	inc	SysReadA_H
	lpm
	ret
TABLESEVENSEGDISPDIGIT:
	.DB	16,63,6,91,79,102,109,125,7,127,111,119,124,57,94,121,113

;********************************************************************************


