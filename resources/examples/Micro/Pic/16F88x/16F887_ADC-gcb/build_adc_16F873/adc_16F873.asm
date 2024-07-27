;Program compiled by Great Cow BASIC (0.98.<<>> 2021-<<>>-24 (Linux 64 bit) : Build 1005) for Microchip MPASM
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=16F873, r=DEC
#include <P16F873.inc>
 __CONFIG _WRT_OFF & _CPD_OFF & _LVP_OFF & _CP_OFF & _WDTE_OFF & _FOSC_HS

;********************************************************************************

;Set aside memory locations for variables
ADREADPORT                       EQU 32
AD_VAL                           EQU 33
AD_VAL_H                         EQU 34
DELAYTEMP                        EQU 112
DELAYTEMP2                       EQU 113
READAD                           EQU 35
SYSBYTETEMPX                     EQU 112
SYSDIVMULTA                      EQU 119
SYSDIVMULTA_H                    EQU 120
SYSINTEGERTEMPA                  EQU 117
SYSINTEGERTEMPA_H                EQU 118
SYSINTEGERTEMPB                  EQU 121
SYSINTEGERTEMPB_H                EQU 122
SYSTEMP1                         EQU 36
SYSWAITTEMP10US                  EQU 117
SYSWAITTEMPMS                    EQU 114
SYSWAITTEMPMS_H                  EQU 115
VALUE                            EQU 37

;********************************************************************************

;Alias variables
SYSREADADBYTE EQU 35

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
	banksel	TRISB
	bcf	TRISB,7
	bcf	TRISB,6
SysDoLoop_S1
	banksel	ADREADPORT
	clrf	ADREADPORT
	call	FN_READAD4
	movf	SYSREADADBYTE,W
	movwf	AD_VAL
	clrf	AD_VAL_H
	movf	AD_VAL,W
	movwf	VALUE
	movf	AD_VAL,W
	movwf	SysINTEGERTempB
	movf	AD_VAL_H,W
	movwf	SysINTEGERTempB_H
	movlw	125
	movwf	SysINTEGERTempA
	clrf	SysINTEGERTempA_H
	call	SYSCOMPLESSTHANINT
	btfsc	SysByteTempX,0
	bsf	PORTB,7
	movf	AD_VAL,W
	movwf	SysINTEGERTempA
	movf	AD_VAL_H,W
	movwf	SysINTEGERTempA_H
	movlw	125
	movwf	SysINTEGERTempB
	clrf	SysINTEGERTempB_H
	call	SYSCOMPLESSTHANINT
	btfsc	SysByteTempX,0
	bcf	PORTB,7
	bsf	PORTB,6
	movlw	200
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	bcf	PORTB,6
	movlw	200
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	goto	SysDoLoop_S1
SysDoLoop_E1
BASPROGRAMEND
	sleep
	goto	BASPROGRAMEND

;********************************************************************************

Delay_10US
D10US_START
	movlw	15
	movwf	DELAYTEMP
DelayUS0
	decfsz	DELAYTEMP,F
	goto	DelayUS0
	nop
	decfsz	SysWaitTemp10US, F
	goto	D10US_START
	return

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

SYSCOMPLESSTHANINT
	clrf	SYSBYTETEMPX
	btfss	SYSINTEGERTEMPA_H,7
	goto	ELSE6_1
	btfsc	SYSINTEGERTEMPB_H,7
	goto	ENDIF7
	comf	SYSBYTETEMPX,F
	return
ENDIF7
	comf	SYSINTEGERTEMPA,W
	movwf	SYSDIVMULTA
	comf	SYSINTEGERTEMPA_H,W
	movwf	SYSDIVMULTA_H
	incf	SYSDIVMULTA,F
	btfsc	STATUS,Z
	incf	SYSDIVMULTA_H,F
	comf	SYSINTEGERTEMPB,W
	movwf	SYSINTEGERTEMPA
	comf	SYSINTEGERTEMPB_H,W
	movwf	SYSINTEGERTEMPA_H
	incf	SYSINTEGERTEMPA,F
	btfsc	STATUS,Z
	incf	SYSINTEGERTEMPA_H,F
	movf	SYSDIVMULTA,W
	movwf	SYSINTEGERTEMPB
	movf	SYSDIVMULTA_H,W
	movwf	SYSINTEGERTEMPB_H
	goto	ENDIF6
ELSE6_1
	btfsc	SYSINTEGERTEMPB_H,7
	return
ENDIF6
	movf	SYSINTEGERTEMPA_H,W
	subwf	SYSINTEGERTEMPB_H,W
	btfss	STATUS,C
	return
	movf	SYSINTEGERTEMPB_H,W
	subwf	SYSINTEGERTEMPA_H,W
	btfss	STATUS,C
	goto	SCLTINTTRUE
	movf	SYSINTEGERTEMPB,W
	subwf	SYSINTEGERTEMPA,W
	btfsc	STATUS,C
	return
SCLTINTTRUE
	comf	SYSBYTETEMPX,F
	return

;********************************************************************************

;Start of program memory page 1
	ORG	2048

 END
