;Program compiled by Great Cow BASIC (1.00.00 2022-12-01 (Linux 64 bit) : Build 1199) for Microchip MPASM/MPLAB-X Assembler
;Need help? 
;  See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;  Check the documentation and Help at http://gcbasic.sourceforge.net/help/,
;or, email us:
;   w_cholmondeley at users dot sourceforge dot net
;   evanvennn at users dot sourceforge dot net

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=12F683, r=DEC
#include <P12F683.inc>
 __CONFIG _FCMEN_ON & _CPD_OFF & _CP_OFF & _MCLRE_OFF & _WDTE_OFF & _INTRC_OSC_NOCLKOUT

;********************************************************************************

;Set aside memory locations for variables
DELAYTEMP                        EQU     112          ; 0x70
DELAYTEMP2                       EQU     113          ; 0x71
SYSWAITTEMPMS                    EQU     114          ; 0x72
SYSWAITTEMPMS_H                  EQU     115          ; 0x73

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

;Start of the main program
	banksel	TRISIO
	bcf	TRISIO,2
	movlw	175
	movwf	VRCON
MAIN
	movlw	19
	banksel	CMCON0
	movwf	CMCON0
	movlw	100
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	goto	MAIN
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
	banksel	ADCON0
	bcf	ADCON0,ADFM
	bcf	ADCON0,ADON
	banksel	ANSEL
	clrf	ANSEL
	movlw	7
	banksel	CMCON0
	movwf	CMCON0
	clrf	GPIO
	return

;********************************************************************************


 END
