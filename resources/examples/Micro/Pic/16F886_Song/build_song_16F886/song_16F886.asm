;Program compiled by Great Cow BASIC (0.98.<<>> 2021-<<>>-24 (Linux 64 bit) : Build 1005) for Microchip MPASM
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=16F886, r=DEC
#include <P16F886.inc>
 __CONFIG _CONFIG1, _LVP_OFF & _FCMEN_ON & _CPD_OFF & _CP_OFF & _MCLRE_OFF & _WDTE_OFF & _FOSC_HS
 __CONFIG _CONFIG2, _WRT_OFF

;********************************************************************************

;Set aside memory locations for variables
CNT                              EQU 32
DELAYTEMP                        EQU 112
NOTE                             EQU 33
NOTE_H                           EQU 34
SYSBYTETEMPX                     EQU 112
SYSDIVLOOP                       EQU 116
SYSDIVMULTA                      EQU 119
SYSDIVMULTA_H                    EQU 120
SYSDIVMULTB                      EQU 123
SYSDIVMULTB_H                    EQU 124
SYSDIVMULTX                      EQU 114
SYSDIVMULTX_H                    EQU 115
SYSSTRINGA                       EQU 119
SYSTEMP1                         EQU 35
SYSTEMP1_H                       EQU 36
SYSTEMP2                         EQU 37
SYSWAITTEMP10US                  EQU 117
SYSWORDTEMPA                     EQU 117
SYSWORDTEMPA_H                   EQU 118
SYSWORDTEMPB                     EQU 121
SYSWORDTEMPB_H                   EQU 122
SYSWORDTEMPX                     EQU 112
SYSWORDTEMPX_H                   EQU 113
TONEDURATION                     EQU 38
TONEDURATION_H                   EQU 39
TONEFREQUENCY                    EQU 40
TONEFREQUENCY_H                  EQU 41
TONELOOP                         EQU 42
TONELOOP2                        EQU 44
TONELOOP2_H                      EQU 45
TONELOOP_H                       EQU 43
TONEPERIOD                       EQU 46
TONEPERIOD_H                     EQU 47

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
	call	INITSOUND

;Start of the main program
	banksel	TRISB
	bsf	TRISB,0
SysDoLoop_S1
HOLD
	banksel	PORTB
	btfsc	PORTB,0
	goto	HOLD
;Legacy method
	clrf	CNT
SysForLoop1
	incf	CNT,F
	movf	CNT,W
	movwf	SYSSTRINGA
	call	MARY
	movwf	NOTE
	movf	CNT,W
	movwf	SYSSTRINGA
	call	MARY_H
	movwf	NOTE_H
	movf	NOTE,W
	movwf	TONEFREQUENCY
	movf	NOTE_H,W
	movwf	TONEFREQUENCY_H
	movlw	50
	movwf	TONEDURATION
	clrf	TONEDURATION_H
	call	TONE
	movlw	32
	subwf	CNT,W
	btfss	STATUS, C
	goto	SysForLoop1
SysForLoopEnd1
	goto	SysDoLoop_S1
SysDoLoop_E1
BASPROGRAMEND
	sleep
	goto	BASPROGRAMEND

;********************************************************************************

Delay_10US
D10US_START
	movlw	12
	movwf	DELAYTEMP
DelayUS0
	decfsz	DELAYTEMP,F
	goto	DelayUS0
	decfsz	SysWaitTemp10US, F
	goto	D10US_START
	return

;********************************************************************************

INITSOUND
	banksel	TRISC
	bcf	TRISC,2
	banksel	STATUS
	return

;********************************************************************************

INITSYS
;asm showdebug _For_selected_frequency_-_the_external_oscillator_has_been_selected_by_compiler ChipMHz
;asm showdebug _Complete_the_chip_setup_of_BSR,ADCs,ANSEL_and_other_key_setup_registers_or_register_bits
	banksel	ADCON1
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
	clrf	PORTE
	return

;********************************************************************************

MARY
	movlw	33
	subwf	SysStringA, W
	btfsc	STATUS, C
	retlw	0
	movf	SysStringA, W
	addlw	low TABLEMARY
	movwf	SysStringA
	movlw	high TABLEMARY
	btfsc	STATUS, C
	addlw	1
	movwf	PCLATH
	movf	SysStringA, W
	movwf	PCL
TABLEMARY
	retlw	32
	retlw	42
	retlw	238
	retlw	184
	retlw	238
	retlw	42
	retlw	0
	retlw	42
	retlw	42
	retlw	238
	retlw	0
	retlw	238
	retlw	0
	retlw	238
	retlw	42
	retlw	147
	retlw	0
	retlw	147
	retlw	42
	retlw	238
	retlw	184
	retlw	238
	retlw	42
	retlw	0
	retlw	42
	retlw	0
	retlw	42
	retlw	238
	retlw	0
	retlw	238
	retlw	42
	retlw	238
	retlw	184

;********************************************************************************

MARY_H
	movlw	33
	subwf	SysStringA, W
	btfsc	STATUS, C
	retlw	0
	movf	SysStringA, W
	addlw	low TABLEMARY_H
	movwf	SysStringA
	movlw	high TABLEMARY_H
	btfsc	STATUS, C
	addlw	1
	movwf	PCLATH
	movf	SysStringA, W
	movwf	PCL
TABLEMARY_H
	retlw	0
	retlw	2
	retlw	1
	retlw	1
	retlw	1
	retlw	2
	retlw	0
	retlw	2
	retlw	2
	retlw	1
	retlw	0
	retlw	1
	retlw	0
	retlw	1
	retlw	2
	retlw	2
	retlw	0
	retlw	2
	retlw	2
	retlw	1
	retlw	1
	retlw	1
	retlw	2
	retlw	0
	retlw	2
	retlw	0
	retlw	2
	retlw	1
	retlw	0
	retlw	1
	retlw	2
	retlw	1
	retlw	1

;********************************************************************************

SYSCOMPEQUAL16
	clrf	SYSBYTETEMPX
	movf	SYSWORDTEMPA, W
	subwf	SYSWORDTEMPB, W
	btfss	STATUS, Z
	return
	movf	SYSWORDTEMPA_H, W
	subwf	SYSWORDTEMPB_H, W
	btfss	STATUS, Z
	return
	comf	SYSBYTETEMPX,F
	return

;********************************************************************************

SYSDIVSUB16
	movf	SYSWORDTEMPA,W
	movwf	SYSDIVMULTA
	movf	SYSWORDTEMPA_H,W
	movwf	SYSDIVMULTA_H
	movf	SYSWORDTEMPB,W
	movwf	SYSDIVMULTB
	movf	SYSWORDTEMPB_H,W
	movwf	SYSDIVMULTB_H
	clrf	SYSDIVMULTX
	clrf	SYSDIVMULTX_H
	movf	SYSDIVMULTB,W
	movwf	SysWORDTempA
	movf	SYSDIVMULTB_H,W
	movwf	SysWORDTempA_H
	clrf	SysWORDTempB
	clrf	SysWORDTempB_H
	call	SYSCOMPEQUAL16
	btfss	SysByteTempX,0
	goto	ENDIF8
	clrf	SYSWORDTEMPA
	clrf	SYSWORDTEMPA_H
	return
ENDIF8
	movlw	16
	movwf	SYSDIVLOOP
SYSDIV16START
	bcf	STATUS,C
	rlf	SYSDIVMULTA,F
	rlf	SYSDIVMULTA_H,F
	rlf	SYSDIVMULTX,F
	rlf	SYSDIVMULTX_H,F
	movf	SYSDIVMULTB,W
	subwf	SYSDIVMULTX,F
	movf	SYSDIVMULTB_H,W
	btfss	STATUS,C
	incfsz	SYSDIVMULTB_H,W
	subwf	SYSDIVMULTX_H,F
	bsf	SYSDIVMULTA,0
	btfsc	STATUS,C
	goto	ENDIF9
	bcf	SYSDIVMULTA,0
	movf	SYSDIVMULTB,W
	addwf	SYSDIVMULTX,F
	movf	SYSDIVMULTB_H,W
	btfsc	STATUS,C
	incfsz	SYSDIVMULTB_H,W
	addwf	SYSDIVMULTX_H,F
ENDIF9
	decfsz	SYSDIVLOOP, F
	goto	SYSDIV16START
	movf	SYSDIVMULTA,W
	movwf	SYSWORDTEMPA
	movf	SYSDIVMULTA_H,W
	movwf	SYSWORDTEMPA_H
	movf	SYSDIVMULTX,W
	movwf	SYSWORDTEMPX
	movf	SYSDIVMULTX_H,W
	movwf	SYSWORDTEMPX_H
	return

;********************************************************************************

TONE
	movf	TONEFREQUENCY,W
	movwf	SysWORDTempA
	movf	TONEFREQUENCY_H,W
	movwf	SysWORDTempA_H
	clrf	SysWORDTempB
	clrf	SysWORDTempB_H
	call	SYSCOMPEQUAL16
	btfsc	SysByteTempX,0
	return
	movlw	80
	movwf	SysWORDTempA
	movlw	195
	movwf	SysWORDTempA_H
	movf	TONEFREQUENCY,W
	movwf	SysWORDTempB
	movf	TONEFREQUENCY_H,W
	movwf	SysWORDTempB_H
	call	SYSDIVSUB16
	movf	SysWORDTempA,W
	movwf	TONEPERIOD
	movf	SysWORDTempA_H,W
	movwf	TONEPERIOD_H
	movf	TONEFREQUENCY,W
	movwf	SysWORDTempA
	movf	TONEFREQUENCY_H,W
	movwf	SysWORDTempA_H
	movlw	100
	movwf	SysWORDTempB
	clrf	SysWORDTempB_H
	call	SYSDIVSUB16
	movf	SysWORDTempA,W
	movwf	TONEFREQUENCY
	movf	SysWORDTempA_H,W
	movwf	TONEFREQUENCY_H
	movlw	1
	movwf	TONELOOP
	clrf	TONELOOP_H
SysForLoop2
	movlw	1
	movwf	TONELOOP2
	clrf	TONELOOP2_H
SysForLoop3
	bsf	PORTC,2
	movf	TONEPERIOD,W
	movwf	SysWaitTemp10US
	call	Delay_10US
	bcf	PORTC,2
	movf	TONEPERIOD,W
	movwf	SysWaitTemp10US
	call	Delay_10US
;#4p Positive value Step Handler in For-next statement
	movf	TONELOOP2,W
	subwf	TONEFREQUENCY,W
	movwf	SysTemp1
	movf	TONEFREQUENCY_H,W
	movwf	SysTemp1_H
	movf	TONELOOP2_H,W
	btfss	STATUS,C
	incfsz	TONELOOP2_H,W
	subwf	SysTemp1_H,F
	movf	SysTemp1,W
	movwf	SysWORDTempA
	movf	SysTemp1_H,W
	movwf	SysWORDTempA_H
	clrf	SysWORDTempB
	clrf	SysWORDTempB_H
	call	SYSCOMPEQUAL16
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF4
;Set LoopVar to LoopVar + StepValue where StepValue is a positive value
	incf	TONELOOP2,F
	btfsc	STATUS,Z
	incf	TONELOOP2_H,F
	goto	SysForLoop3
;END IF
ENDIF4
SysForLoopEnd3
;#4p Positive value Step Handler in For-next statement
	movf	TONELOOP,W
	subwf	TONEDURATION,W
	movwf	SysTemp1
	movf	TONEDURATION_H,W
	movwf	SysTemp1_H
	movf	TONELOOP_H,W
	btfss	STATUS,C
	incfsz	TONELOOP_H,W
	subwf	SysTemp1_H,F
	movf	SysTemp1,W
	movwf	SysWORDTempA
	movf	SysTemp1_H,W
	movwf	SysWORDTempA_H
	clrf	SysWORDTempB
	clrf	SysWORDTempB_H
	call	SYSCOMPEQUAL16
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF6
;Set LoopVar to LoopVar + StepValue where StepValue is a positive value
	incf	TONELOOP,F
	btfsc	STATUS,Z
	incf	TONELOOP_H,F
	goto	SysForLoop2
;END IF
ENDIF6
SysForLoopEnd2
	return

;********************************************************************************

;Start of program memory page 1
	ORG	2048
;Start of program memory page 2
	ORG	4096
;Start of program memory page 3
	ORG	6144

 END
