;Program compiled by Great Cow BASIC (0.98.<<>> 2021-<<>>-24 (Linux 64 bit) : Build 1005) for Microchip MPASM
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=16F628A, r=DEC
#include <P16F628A.inc>
 __CONFIG _CP_OFF & _CPD_OFF & _LVP_OFF & _MCLRE_OFF & _WDTE_OFF & _INTRC_OSC_NOCLKOUT

;********************************************************************************

;Set aside memory locations for variables
DELAYTEMP                        EQU 112
DELAYTEMP2                       EQU 113
SAVEPCLATH                       EQU 32
SYSINTSTATESAVE0                 EQU 33
SYSSTATUS                        EQU 127
SYSW                             EQU 126
SYSWAITTEMPMS                    EQU 114
SYSWAITTEMPMS_H                  EQU 115

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
	btfss	INTCON,T0IE
	goto	NotT0IF
	btfss	INTCON,T0IF
	goto	NotT0IF
	call	INCCOUNTER
	bcf	INTCON,T0IF
	goto	INTERRUPTDONE
NotT0IF
;User Interrupt routine
INTERRUPTDONE
;Restore Context
;Restore system variables
	movf	SavePCLATH,W
	movwf	PCLATH
	swapf	SysSTATUS,W
	movwf	STATUS
	swapf	SysW,F
	swapf	SysW,W
	retfie

;********************************************************************************

;Start of program memory page 0
	ORG	27
BASPROGRAMSTART
;Call initialisation routines
	call	INITSYS
;Enable interrupts
	bsf	INTCON,GIE
	bsf	INTCON,PEIE
	bsf	SYSINTSTATESAVE0,0
;Automatic pin direction setting
	banksel	TRISB
	bcf	TRISB,5

;Start of the main program
	bsf	INTCON,T0IE
	movlw	7
	banksel	CMCON
	movwf	CMCON
	banksel	TRISB
	clrf	TRISB
	banksel	PORTB
	clrf	PORTB
	banksel	TRISA
	bsf	TRISA,4
	bsf	OPTION_REG,T0CS
	movlw	250
	banksel	TMR0
	movwf	TMR0
	btfss	SYSINTSTATESAVE0,0
	bcf	INTCON,GIE
	btfsc	SYSINTSTATESAVE0,0
	bsf	INTCON,GIE
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
	btfss	PORTB,0
	goto	ELSE1_1
	bcf	PORTB,0
	goto	ENDIF1
ELSE1_1
	bsf	PORTB,0
ENDIF1
	bcf	INTCON,T0IF
	movlw	250
	movwf	TMR0
	return

;********************************************************************************

INITSYS
;asm showdebug This code block sets the internal oscillator to ChipMHz
;asm showdebug _Complete_the_chip_setup_of_BSR,ADCs,ANSEL_and_other_key_setup_registers_or_register_bits
	movlw	7
	movwf	CMCON
	clrf	PORTA
	clrf	PORTB
	return

;********************************************************************************


 END
