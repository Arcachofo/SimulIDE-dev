;Program compiled by Great Cow BASIC (0.98.<<>> 2021-<<>>-24 (Linux 64 bit) : Build 1005) for Microchip MPASM
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=16F887, r=DEC
#include <P16F887.inc>
 __CONFIG _CONFIG1, _LVP_OFF & _FCMEN_ON & _CPD_OFF & _CP_OFF & _MCLRE_OFF & _WDTE_OFF & _INTOSCIO
 __CONFIG _CONFIG2, _WRT_OFF

;********************************************************************************

;Set aside memory locations for variables
COUNT                            EQU 32
DELAYTEMP                        EQU 112
DELAYTEMP2                       EQU 113
SAVEPCLATH                       EQU 33
SYSSTATUS                        EQU 127
SYSW                             EQU 126
SYSWAITTEMPMS                    EQU 114
SYSWAITTEMPMS_H                  EQU 115
TMRNUMBER                        EQU 34
TMRPRES                          EQU 35
TMRSOURCE                        EQU 36

;********************************************************************************

;Vectors
	ORG	0
	pagesel	BASPROGRAMSTART
	goto	BASPROGRAMSTART
	ORG	4
INTERRUPT

;********************************************************************************

;Save Context
	movwf	SysW
	swapf	STATUS,W
	movwf	SysSTATUS
	banksel	STATUS
;Store system variables
	movf	PCLATH,W
	movwf	SavePCLATH
	clrf	PCLATH
;On Interrupt handlers
	banksel	PIE1
	btfss	PIE1,TMR1IE
	goto	NotTMR1IF
	banksel	PIR1
	btfss	PIR1,TMR1IF
	goto	NotTMR1IF
	call	INCCOUNTER
	bcf	PIR1,TMR1IF
	goto	INTERRUPTDONE
NotTMR1IF
;User Interrupt routine
INTERRUPTDONE
;Restore Context
;Restore system variables
	banksel	SAVEPCLATH
	movf	SavePCLATH,W
	movwf	PCLATH
	swapf	SysSTATUS,W
	movwf	STATUS
	swapf	SysW,F
	swapf	SysW,W
	retfie

;********************************************************************************

;Start of program memory page 0
	ORG	33
BASPROGRAMSTART
;Call initialisation routines
	call	INITSYS
;Enable interrupts
	bsf	INTCON,GIE
	bsf	INTCON,PEIE

;Start of the main program
	clrf	COUNT
	banksel	TRISB
	clrf	TRISB
	bsf	PIE1,TMR1IE
	movlw	1
	banksel	TMRSOURCE
	movwf	TMRSOURCE
	clrf	TMRPRES
	call	INITTIMER1
	movlw	1
	movwf	TMRNUMBER
	call	CLEARTIMER
	movlw	60
	movwf	TMR1H
	movlw	199
	movwf	TMR1L
	movlw	1
	movwf	TMRNUMBER
	call	STARTTIMER
SysDoLoop_S1
	bcf	PORTB,5
	movlw	244
	movwf	SysWaitTempMS
	movlw	1
	movwf	SysWaitTempMS_H
	call	Delay_MS
	bsf	PORTB,5
	movlw	244
	movwf	SysWaitTempMS
	movlw	1
	movwf	SysWaitTempMS_H
	call	Delay_MS
	goto	SysDoLoop_S1
SysDoLoop_E1
BASPROGRAMEND
	sleep
	goto	BASPROGRAMEND

;********************************************************************************

CLEARTIMER
	movf	TMRNUMBER,F
	btfsc	STATUS, Z
	clrf	TMR0
	decf	TMRNUMBER,W
	btfss	STATUS, Z
	goto	ENDIF6
	clrf	TMR1H
	clrf	TMR1L
ENDIF6
	movlw	2
	subwf	TMRNUMBER,W
	btfsc	STATUS, Z
	clrf	TMR2
	return

;********************************************************************************

Delay_MS
	incf	SysWaitTempMS_H, F
DMS_START
	movlw	142
	movwf	DELAYTEMP2
DMS_OUTER
	movlw	1
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

INCCOUNTER
	movlw	60
	movwf	TMR1H
	movlw	199
	movwf	TMR1L
	incf	COUNT,F
	movlw	10
	subwf	COUNT,W
	btfss	STATUS, Z
	goto	ENDIF1
	clrf	COUNT
	btfsc	PORTB,4
	goto	ELSE2_1
	bsf	PORTB,4
	goto	ENDIF2
ELSE2_1
	bcf	PORTB,4
ENDIF2
ENDIF1
	return

;********************************************************************************

INITSYS
;asm showdebug This code block sets the internal oscillator to ChipMHz
;asm showdebug 'OSCCON type is 103 - This part does not have Bit HFIOFS @ ifndef Bit(HFIOFS)
	movlw	143
	banksel	OSCCON
	andwf	OSCCON,F
	movlw	96
	iorwf	OSCCON,F
	movlw	143
	andwf	OSCCON,F
	bsf	OSCCON,IRCF2
	bsf	OSCCON,IRCF1
	bcf	OSCCON,IRCF0
;asm showdebug _Complete_the_chip_setup_of_BSR,ADCs,ANSEL_and_other_key_setup_registers_or_register_bits
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
	clrf	PORTD
	clrf	PORTE
	return

;********************************************************************************

INITTIMER1
	movf	TMRPRES,F
	btfsc	STATUS, Z
	goto	ENDIF8
	movlw	16
	subwf	TMRPRES,W
	btfsc	STATUS, Z
	goto	ENDIF14
	movlw	32
	subwf	TMRPRES,W
	btfsc	STATUS, Z
	goto	ENDIF15
	movlw	48
	subwf	TMRPRES,W
	btfss	STATUS, Z
	clrf	TMRPRES
ENDIF15
ENDIF14
ENDIF8
	btfsc	T1CON,TMR1ON
	bsf	TMRPRES,0
	decf	TMRSOURCE,W
	btfss	STATUS, Z
	goto	ENDIF10
	bcf	TMRPRES,1
	bcf	TMRPRES,3
ENDIF10
	movlw	2
	subwf	TMRSOURCE,W
	btfss	STATUS, Z
	goto	ENDIF11
	bsf	TMRPRES,1
	bcf	TMRPRES,3
ENDIF11
	movlw	3
	subwf	TMRSOURCE,W
	btfss	STATUS, Z
	goto	ENDIF12
	bsf	TMRPRES,1
	bsf	TMRPRES,3
ENDIF12
	movlw	4
	subwf	TMRSOURCE,W
	btfsc	STATUS, Z
	bcf	TMRPRES,3
	movf	TMRPRES,W
	movwf	T1CON
	return

;********************************************************************************

STARTTIMER
	decf	TMRNUMBER,W
	btfsc	STATUS, Z
	bsf	T1CON,TMR1ON
	movlw	2
	subwf	TMRNUMBER,W
	btfsc	STATUS, Z
	bsf	T2CON,TMR2ON
	return

;********************************************************************************

;Start of program memory page 1
	ORG	2048
;Start of program memory page 2
	ORG	4096
;Start of program memory page 3
	ORG	6144

 END
