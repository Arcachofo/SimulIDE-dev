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
DOPWM                            EQU 33
PWMCHANNEL                       EQU 34
PWMDUR                           EQU 35
SOFTPWMCYCLES                    EQU 36
SOFTPWMDUTY                      EQU 37
SYSBYTETEMPA                     EQU 117
SYSBYTETEMPB                     EQU 121
SYSBYTETEMPX                     EQU 112
SYSDIVMULTA                      EQU 119
SYSDIVMULTA_H                    EQU 120
SYSFORLOOPSTEP0                  EQU 38
SYSFORLOOPSTEP0_H                EQU 39
SYSFORLOOPSTEP1                  EQU 40
SYSFORLOOPSTEP1_H                EQU 41
SYSINTEGERTEMPA                  EQU 117
SYSINTEGERTEMPA_H                EQU 118
SYSINTEGERTEMPB                  EQU 121
SYSINTEGERTEMPB_H                EQU 122
SYSTEMP1                         EQU 42
SYSTEMP2                         EQU 43
SYSTEMP2_H                       EQU 44
SYSTEMP3                         EQU 45

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
;Automatic pin direction setting
	banksel	TRISB
	bcf	TRISB,5
	bcf	TRISB,2

;Start of the main program
SysDoLoop_S1
	movlw	10
	banksel	CNT
	movwf	CNT
SysForLoop1
;#0 Init SysForLoopStep0
	movlw	10
	movwf	SysForLoopStep0
	clrf	SysForLoopStep0_H
	movlw	1
	movwf	PWMCHANNEL
	movf	CNT,W
	movwf	SOFTPWMDUTY
	movlw	10
	movwf	SOFTPWMCYCLES
	call	PWMOUT
	movlw	2
	movwf	PWMCHANNEL
	movf	CNT,W
	sublw	250
	movwf	SOFTPWMDUTY
	movlw	10
	movwf	SOFTPWMCYCLES
	call	PWMOUT
;Integer negative Step Handler in For-next statement
	btfss	SYSFORLOOPSTEP0_H,7
	goto	ELSE1_1
;#3n IF ( CNT - 250) } -SysForLoopStep0 THEN 
	movlw	250
	subwf	CNT,W
	movwf	SysTemp1
	comf	SysForLoopStep0,W
	movwf	SysTemp2
	comf	SysForLoopStep0_H,W
	movwf	SysTemp2_H
	incf	SysTemp2,F
	btfsc	STATUS,Z
	incf	SysTemp2_H,F
	movf	SysTemp1,W
	movwf	SysINTEGERTempA
	clrf	SysINTEGERTempA_H
	movf	SysTemp2,W
	movwf	SysINTEGERTempB
	movf	SysTemp2_H,W
	movwf	SysINTEGERTempB_H
	call	SYSCOMPLESSTHANINT
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF3
;Set LoopVar to LoopVar + StepValue where StepValue is a negative value
	movf	SysForLoopStep0,W
	addwf	CNT,F
	goto	SysForLoop1
;END IF
ENDIF3
	goto	ENDIF1
ELSE1_1
;Integer positive Step Handler in For-next statement
;#3p IF (250 - CNT) } SysForLoopStep0 THEN
	movf	CNT,W
	sublw	250
	movwf	SysTemp1
	movwf	SysINTEGERTempA
	clrf	SysINTEGERTempA_H
	movf	SysForLoopStep0,W
	movwf	SysINTEGERTempB
	movf	SysForLoopStep0_H,W
	movwf	SysINTEGERTempB_H
	call	SYSCOMPLESSTHANINT
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF4
;Set LoopVar to LoopVar + StepValue where StepValue is a positive value
	movf	SysForLoopStep0,W
	addwf	CNT,F
	goto	SysForLoop1
;END IF
ENDIF4
;END IF
ENDIF1
SysForLoopEnd1
	movlw	250
	movwf	CNT
SysForLoop2
;#0 Init SysForLoopStep1
	movlw	246
	movwf	SysForLoopStep1
	movlw	255
	movwf	SysForLoopStep1_H
	movlw	1
	movwf	PWMCHANNEL
	movf	CNT,W
	movwf	SOFTPWMDUTY
	movlw	10
	movwf	SOFTPWMCYCLES
	call	PWMOUT
	movlw	2
	movwf	PWMCHANNEL
	movf	CNT,W
	sublw	250
	movwf	SOFTPWMDUTY
	movlw	10
	movwf	SOFTPWMCYCLES
	call	PWMOUT
;Integer negative Step Handler in For-next statement
	btfss	SYSFORLOOPSTEP1_H,7
	goto	ELSE2_1
;#3n IF ( CNT - 10) } -SysForLoopStep1 THEN 
	movlw	10
	subwf	CNT,W
	movwf	SysTemp1
	comf	SysForLoopStep1,W
	movwf	SysTemp2
	comf	SysForLoopStep1_H,W
	movwf	SysTemp2_H
	incf	SysTemp2,F
	btfsc	STATUS,Z
	incf	SysTemp2_H,F
	movf	SysTemp1,W
	movwf	SysINTEGERTempA
	clrf	SysINTEGERTempA_H
	movf	SysTemp2,W
	movwf	SysINTEGERTempB
	movf	SysTemp2_H,W
	movwf	SysINTEGERTempB_H
	call	SYSCOMPLESSTHANINT
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF5
;Set LoopVar to LoopVar + StepValue where StepValue is a negative value
	movf	SysForLoopStep1,W
	addwf	CNT,F
	goto	SysForLoop2
;END IF
ENDIF5
	goto	ENDIF2
ELSE2_1
;Integer positive Step Handler in For-next statement
;#3p IF (10 - CNT) } SysForLoopStep1 THEN
	movf	CNT,W
	sublw	10
	movwf	SysTemp1
	movwf	SysINTEGERTempA
	clrf	SysINTEGERTempA_H
	movf	SysForLoopStep1,W
	movwf	SysINTEGERTempB
	movf	SysForLoopStep1_H,W
	movwf	SysINTEGERTempB_H
	call	SYSCOMPLESSTHANINT
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF6
;Set LoopVar to LoopVar + StepValue where StepValue is a positive value
	movf	SysForLoopStep1,W
	addwf	CNT,F
	goto	SysForLoop2
;END IF
ENDIF6
;END IF
ENDIF2
SysForLoopEnd2
	goto	SysDoLoop_S1
SysDoLoop_E1
BASPROGRAMEND
	sleep
	goto	BASPROGRAMEND

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

PWMOUT
	movlw	1
	movwf	PWMDUR
SysForLoop3
;Legacy method
	clrf	DOPWM
SysForLoop4
	incf	DOPWM,F
	movf	SOFTPWMDUTY,W
	subwf	DOPWM,W
	btfsc	STATUS, C
	goto	ELSE7_1
	decf	PWMCHANNEL,W
	btfsc	STATUS, Z
	bsf	PORTB,5
	movlw	2
	subwf	PWMCHANNEL,W
	btfsc	STATUS, Z
	bsf	PORTB,2
	goto	ENDIF7
ELSE7_1
	decf	PWMCHANNEL,W
	btfsc	STATUS, Z
	bcf	PORTB,5
	movlw	2
	subwf	PWMCHANNEL,W
	btfsc	STATUS, Z
	bcf	PORTB,2
ENDIF7
	movlw	255
	subwf	DOPWM,W
	btfss	STATUS, C
	goto	SysForLoop4
SysForLoopEnd4
;#4p Positive value Step Handler in For-next statement
	movf	PWMDUR,W
	subwf	SOFTPWMCYCLES,W
	movwf	SysTemp1
	movwf	SysBYTETempA
	clrf	SysBYTETempB
	call	SYSCOMPEQUAL
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF9
;Set LoopVar to LoopVar + StepValue where StepValue is a positive value
	incf	PWMDUR,F
	goto	SysForLoop3
;END IF
ENDIF9
SysForLoopEnd3
	return

;********************************************************************************

SYSCOMPEQUAL
	clrf	SYSBYTETEMPX
	movf	SYSBYTETEMPA, W
	subwf	SYSBYTETEMPB, W
	btfsc	STATUS, Z
	comf	SYSBYTETEMPX,F
	return

;********************************************************************************

SYSCOMPLESSTHANINT
	clrf	SYSBYTETEMPX
	btfss	SYSINTEGERTEMPA_H,7
	goto	ELSE14_1
	btfsc	SYSINTEGERTEMPB_H,7
	goto	ENDIF15
	comf	SYSBYTETEMPX,F
	return
ENDIF15
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
	goto	ENDIF14
ELSE14_1
	btfsc	SYSINTEGERTEMPB_H,7
	return
ENDIF14
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
;Start of program memory page 2
	ORG	4096
;Start of program memory page 3
	ORG	6144

 END
