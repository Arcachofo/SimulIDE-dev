;Program compiled by Great Cow BASIC (1.00.00 2022-12-01 (Linux 64 bit) : Build 1199) for Microchip AVR Assembler
;Need help? 
;  See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;  Check the documentation and Help at http://gcbasic.sourceforge.net/help/,
;or, email us:
;   w_cholmondeley at users dot sourceforge dot net
;   evanvennn at users dot sourceforge dot net

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
.EQU	LEDSPEED=256	         ; 0x100
.EQU	PWMCOUNTER=257	         ; 0x101
.EQU	SAVESREG=258	         ; 0x102
.EQU	SAVESYSCALCTEMPA=259	         ; 0x103
.EQU	SAVESYSCALCTEMPB=260	         ; 0x104
.EQU	SAVESYSTEMP1=261	         ; 0x105
.EQU	SAVESYSVALUECOPY=262	         ; 0x106
.EQU	SYSFORLOOPSTEP1=263	         ; 0x107
.EQU	SYSFORLOOPSTEP1_H=264	         ; 0x108
.EQU	TMR0_TMP=265	         ; 0x109
.EQU	TMR1_TMP=266	         ; 0x10A
.EQU	TMR2_TMP=267	         ; 0x10B
.EQU	TMRNUMBER=268	         ; 0x10C
.EQU	TMRPOST=269	         ; 0x10D
.EQU	TMRPRES=270	         ; 0x10E

;********************************************************************************

;Register variables
.DEF	DELAYTEMP=r25
.DEF	DELAYTEMP2=r26
.DEF	SYSBITTEST=r5
.DEF	SYSBYTETEMPX=r0
.DEF	SYSCALCTEMPA=r22
.DEF	SYSCALCTEMPB=r28
.DEF	SYSDIVMULTA=r24
.DEF	SYSDIVMULTA_H=r25
.DEF	SYSINTEGERTEMPA=r22
.DEF	SYSINTEGERTEMPA_H=r23
.DEF	SYSINTEGERTEMPB=r28
.DEF	SYSINTEGERTEMPB_H=r29
.DEF	SYSVALUECOPY=r21
.DEF	SYSWAITTEMPMS=r29
.DEF	SYSWAITTEMPMS_H=r30
.DEF	SYSWAITTEMPS=r31
.DEF	SYSTEMP1=r1
.DEF	SYSTEMP2=r2
.DEF	SYSTEMP2_H=r3
.DEF	SYSTEMP3=r16
.DEF	SYSTEMP3_H=r17

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
	rjmp	IntTIMER2_OVF ;TIMER2_OVF
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
;Enable interrupts
	sei

;Start of the main program
	sbi	DDRB,5
	rcall	INITLEDCONTROL
SysDoLoop_S1:
	ldi	SysValueCopy,255
	sts	LEDSPEED,SysValueCopy
SysForLoop1:
	lds	SysTemp1,LEDSPEED
	inc	SysTemp1
	sts	LEDSPEED,SysTemp1
	ldi	SysWaitTempMS,25
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
	lds	SysCalcTempA,LEDSPEED
	cpi	SysCalcTempA,100
	brlo	SysForLoop1
SysForLoopEnd1:
	ldi	SysWaitTempS,1
	rcall	Delay_S
	ldi	SysValueCopy,100
	sts	LEDSPEED,SysValueCopy
SysForLoop2:
;Init SysForLoopStep1 :#0
	ldi	SysValueCopy,255
	sts	SysForLoopStep1,SysValueCopy
	sts	SysForLoopStep1_H,SysValueCopy
	ldi	SysWaitTempMS,25
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
	lds	SysBitTest,SYSFORLOOPSTEP1_H
	sbrs	SysBitTest,7
	rjmp	ELSE2_1
	lds	SysTemp2,LEDSPEED
	ldi	SysTemp3,0
	sub	SysTemp2,SysTemp3
	mov	SysTemp1,SysTemp2
	lds	SysTemp3,SysForLoopStep1
	lds	SysTemp3_H,SysForLoopStep1_H
	com	SysTemp3
	com	SysTemp3_H
	inc	SysTemp3
	brne	PC + 2
	inc	SysTemp3_H
	mov	SysTemp2,SysTemp3
	mov	SysTemp2_H,SysTemp3_H
	mov	SysINTEGERTempA,SysTemp1
	ldi	SysINTEGERTempA_H,0
	mov	SysINTEGERTempB,SysTemp2
	mov	SysINTEGERTempB_H,SysTemp2_H
	rcall	SYSCOMPLESSTHANINT
	com	SysByteTempX
	sbrs	SysByteTempX,0
	rjmp	ENDIF3
	lds	SysTemp1,LEDSPEED
	lds	SysTemp2,SysForLoopStep1
	add	SysTemp1,SysTemp2
	sts	LEDSPEED,SysTemp1
	rjmp	SysForLoop2
ENDIF3:
	rjmp	ENDIF2
ELSE2_1:
	ldi	SysTemp3,0
	lds	SysTemp2,LEDSPEED
	sub	SysTemp3,SysTemp2
	mov	SysTemp1,SysTemp3
	mov	SysINTEGERTempA,SysTemp1
	ldi	SysINTEGERTempA_H,0
	lds	SysINTEGERTempB,SysForLoopStep1
	lds	SysINTEGERTempB_H,SysForLoopStep1_H
	rcall	SYSCOMPLESSTHANINT
	com	SysByteTempX
	sbrs	SysByteTempX,0
	rjmp	ENDIF4
	lds	SysTemp1,LEDSPEED
	lds	SysTemp2,SysForLoopStep1
	add	SysTemp1,SysTemp2
	sts	LEDSPEED,SysTemp1
	rjmp	SysForLoop2
ENDIF4:
ENDIF2:
SysForLoopEnd2:
	ldi	SysWaitTempS,1
	rcall	Delay_S
	rjmp	SysDoLoop_S1
SysDoLoop_E1:
	rjmp	BASPROGRAMEND
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

Delay_S:
DS_START:
	ldi	SysWaitTempMS,232
	ldi	SysWaitTempMS_H,3
	rcall	Delay_MS
	dec	SysWaitTempS
	brne	DS_START
	ret

;********************************************************************************

INITLEDCONTROL:
	cbi	PORTB,5
	ldi	SysValueCopy,0
	sts	LEDSPEED,SysValueCopy
	ldi	SysValueCopy,0
	sts	PWMCOUNTER,SysValueCopy
	lds	SysValueCopy,TIMSK2
	sbr	SysValueCopy,1<<TOIE2
	sts	TIMSK2,SysValueCopy
	ldi	SysValueCopy,1
	sts	TMRPRES,SysValueCopy
	ldi	SysValueCopy,1
	sts	TMRPOST,SysValueCopy
	rcall	INITTIMER2182
	ldi	SysValueCopy,2
	sts	TMRNUMBER,SysValueCopy
	rjmp	STARTTIMER

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

INITTIMER2182:
	lds	SysValueCopy,TMRPOST
	sts	TMR2_TMP,SysValueCopy
	lds	SysCalcTempA,TMRPRES
	cpi	SysCalcTempA,2
	brne	ENDIF11
	ldi	SysValueCopy,7
	sts	TMR2_TMP,SysValueCopy
ENDIF11:
	ret

;********************************************************************************

IntTIMER2_OVF:
	rcall	SysIntContextSave
	rcall	PWMHANDLER
	cbi	TIFR2,TOV2
	rjmp	SysIntContextRestore

;********************************************************************************

PWMHANDLER:
	lds	SysCalcTempA,PWMCOUNTER
	lds	SysCalcTempB,LEDSPEED
	cp	SysCalcTempA,SysCalcTempB
	brsh	ELSE5_1
	sbi	PORTB,5
	rjmp	ENDIF5
ELSE5_1:
	cbi	PORTB,5
ENDIF5:
	lds	SysTemp1,PWMCOUNTER
	inc	SysTemp1
	sts	PWMCOUNTER,SysTemp1
	lds	SysCalcTempA,PWMCOUNTER
	cpi	SysCalcTempA,100
	brne	ENDIF6
	ldi	SysValueCopy,0
	sts	PWMCOUNTER,SysValueCopy
ENDIF6:
	ret

;********************************************************************************

STARTTIMER:
	lds	SysCalcTempA,TMRNUMBER
	tst	SysCalcTempA
	brne	ENDIF7
	ldi	SysTemp3,248
	in	SysTemp2,TCCR0B
	and	SysTemp2,SysTemp3
	mov	SysTemp1,SysTemp2
	lds	SysTemp2,TMR0_TMP
	or	SysTemp2,SysTemp1
	out	TCCR0B,SysTemp2
ENDIF7:
	lds	SysCalcTempA,TMRNUMBER
	cpi	SysCalcTempA,1
	brne	ENDIF8
	ldi	SysTemp3,248
	lds	SysTemp2,TCCR1B
	and	SysTemp2,SysTemp3
	mov	SysTemp1,SysTemp2
	lds	SysTemp2,TMR1_TMP
	or	SysTemp2,SysTemp1
	sts	TCCR1B,SysTemp2
ENDIF8:
	lds	SysCalcTempA,TMRNUMBER
	cpi	SysCalcTempA,2
	brne	ENDIF9
	ldi	SysTemp3,248
	lds	SysTemp2,TCCR2B
	and	SysTemp2,SysTemp3
	mov	SysTemp1,SysTemp2
	lds	SysTemp2,TMR2_TMP
	or	SysTemp2,SysTemp1
	sts	TCCR2B,SysTemp2
ENDIF9:
	lds	SysCalcTempA,TMRNUMBER
	cpi	SysCalcTempA,2
	brne	ENDIF10
	ldi	SysTemp3,248
	lds	SysTemp2,TCCR2A
	and	SysTemp2,SysTemp3
	mov	SysTemp1,SysTemp2
	lds	SysTemp2,TMR2_TMP
	or	SysTemp2,SysTemp1
	sts	TCCR2A,SysTemp2
ENDIF10:
	ret

;********************************************************************************

SYSCOMPLESSTHANINT:
	ldi	SysValueCopy,0
	mov	SYSBYTETEMPX,SysValueCopy
	sbrs	SYSINTEGERTEMPA_H,7
	rjmp	ELSE12_1
	sbrc	SYSINTEGERTEMPB_H,7
	rjmp	ENDIF13
	mov	SysTemp1,SYSBYTETEMPX
	com	SysTemp1
	mov	SYSBYTETEMPX,SysTemp1
	ret
ENDIF13:
	mov	SysTemp2,SYSINTEGERTEMPA
	mov	SysTemp2_H,SYSINTEGERTEMPA_H
	com	SysTemp2
	com	SysTemp2_H
	inc	SysTemp2
	brne	PC + 2
	inc	SysTemp2_H
	mov	SYSDIVMULTA,SysTemp2
	mov	SYSDIVMULTA_H,SysTemp2_H
	mov	SysTemp2,SYSINTEGERTEMPB
	mov	SysTemp2_H,SYSINTEGERTEMPB_H
	com	SysTemp2
	com	SysTemp2_H
	inc	SysTemp2
	brne	PC + 2
	inc	SysTemp2_H
	mov	SYSINTEGERTEMPA,SysTemp2
	mov	SYSINTEGERTEMPA_H,SysTemp2_H
	mov	SYSINTEGERTEMPB,SYSDIVMULTA
	mov	SYSINTEGERTEMPB_H,SYSDIVMULTA_H
	rjmp	ENDIF12
ELSE12_1:
	sbrc	SYSINTEGERTEMPB_H,7
	ret
ENDIF12:
	cp	SYSINTEGERTEMPB_H,SYSINTEGERTEMPA_H
	brlo	SCLTINTFALSE
	cp	SYSINTEGERTEMPA_H,SYSINTEGERTEMPB_H
	brlo	SCLTINTTRUE
	cp	SYSINTEGERTEMPA,SYSINTEGERTEMPB
	brlo	SCLTINTTRUE
	ret
SCLTINTTRUE:
	com	SYSBYTETEMPX
SCLTINTFALSE:
	ret

;********************************************************************************

SysIntContextRestore:
;Restore registers
	lds	SysCalcTempA,SaveSysCalcTempA
	lds	SysCalcTempB,SaveSysCalcTempB
	lds	SysTemp1,SaveSysTemp1
;Restore SREG
	lds	SysValueCopy,SaveSREG
	out	SREG,SysValueCopy
;Restore SysValueCopy
	lds	SysValueCopy,SaveSysValueCopy
	reti

;********************************************************************************

SysIntContextSave:
;Store SysValueCopy
	sts	SaveSysValueCopy,SysValueCopy
;Store SREG
	in	SysValueCopy,SREG
	sts	SaveSREG,SysValueCopy
;Store registers
	sts	SaveSysCalcTempA,SysCalcTempA
	sts	SaveSysCalcTempB,SysCalcTempB
	sts	SaveSysTemp1,SysTemp1
	ret

;********************************************************************************


