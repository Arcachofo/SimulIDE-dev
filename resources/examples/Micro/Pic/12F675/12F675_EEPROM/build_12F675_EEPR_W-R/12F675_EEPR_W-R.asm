;Program compiled by Great Cow BASIC (0.98.<<>> 2021-<<>>-24 (Linux 64 bit) : Build 1005) for Microchip MPASM
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=12F675, r=DEC
#include <P12F675.inc>
 __CONFIG _CPD_OFF & _CP_OFF & _MCLRE_OFF & _WDTE_OFF & _FOSC_HS

;********************************************************************************

;Set aside memory locations for variables
ADDR                             EQU 37
DATA                             EQU 38
DELAYTEMP                        EQU 32
DELAYTEMP2                       EQU 33
SYSWAITTEMPMS                    EQU 34
SYSWAITTEMPMS_H                  EQU 35
SYSWAITTEMPS                     EQU 36

;********************************************************************************

;Alias variables
EEADDRESS EQU 155
EEDATAVALUE EQU 154

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
;Legacy method
	movlw	255
	movwf	ADDR
SysForLoop1
	incf	ADDR,F
	movf	ADDR,W
	banksel	EEADDRESS
	movwf	EEADDRESS
	movf	ADDR,W
	movwf	EEDATAVALUE
	banksel	STATUS
	call	EPWRITE
	movlw	10
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	movlw	20
	subwf	ADDR,W
	btfss	STATUS, C
	goto	SysForLoop1
SysForLoopEnd1
	movlw	2
	movwf	SysWaitTempS
	call	Delay_S
;Legacy method
	movlw	255
	movwf	ADDR
SysForLoop2
	incf	ADDR,F
	movf	ADDR,W
	banksel	EEADDRESS
	movwf	EEADDRESS
	banksel	STATUS
	call	SYSEPREAD
	banksel	EEDATAVALUE
	movf	EEDATAVALUE,W
	movwf	DATA
	movlw	244
	movwf	SysWaitTempMS
	movlw	1
	movwf	SysWaitTempMS_H
	banksel	STATUS
	call	Delay_MS
	movlw	25
	subwf	ADDR,W
	btfss	STATUS, C
	goto	SysForLoop2
SysForLoopEnd2
SysDoLoop_S1
	movlw	1
	movwf	SysWaitTempS
	call	Delay_S
	goto	SysDoLoop_S1
SysDoLoop_E1
BASPROGRAMEND
	sleep
	goto	BASPROGRAMEND

;********************************************************************************

Delay_MS
	incf	SysWaitTempMS_H, F
DMS_START
	movlw	227
	movwf	DELAYTEMP2
DMS_OUTER
	movlw	6
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

Delay_S
DS_START
	movlw	232
	movwf	SysWaitTempMS
	movlw	3
	movwf	SysWaitTempMS_H
	call	Delay_MS
	decfsz	SysWaitTempS, F
	goto	DS_START
	return

;********************************************************************************

EPWRITE
	banksel	EECON1
	bsf	EECON1,WREN
	movlw	85
	movwf	EECON2
	movlw	170
	movwf	EECON2
	bsf	EECON1,WR
	bcf	EECON1,WREN
SysWaitLoop1
	btfsc	EECON1,WR
	goto	SysWaitLoop1
	bcf	EECON1,WREN
	banksel	STATUS
	return

;********************************************************************************

INITSYS
;asm showdebug _For_selected_frequency_-_the_external_oscillator_has_been_selected_by_compiler ChipMHz
	call	1023
	banksel	OSCCAL
	movwf	OSCCAL
;asm showdebug _Complete_the_chip_setup_of_BSR,ADCs,ANSEL_and_other_key_setup_registers_or_register_bits
	banksel	ADCON0
	bcf	ADCON0,ADFM
	bcf	ADCON0,ADON
	banksel	ANSEL
	clrf	ANSEL
	movlw	7
	banksel	CMCON
	movwf	CMCON
	clrf	GPIO
	return

;********************************************************************************

SYSEPREAD
	banksel	EECON1
	bsf	EECON1,RD
	banksel	STATUS
	return

;********************************************************************************


 END
