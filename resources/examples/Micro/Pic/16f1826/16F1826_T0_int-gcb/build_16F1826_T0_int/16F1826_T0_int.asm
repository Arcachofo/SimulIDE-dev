;Program compiled by Great Cow BASIC (0.98.<<>> 2021-<<>>-24 (Linux 64 bit) : Build 1005) for Microchip MPASM
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=16F1826, r=DEC
#include <P16F1826.inc>
 __CONFIG _CONFIG1, _FCMEN_ON & _CLKOUTEN_OFF & _CPD_OFF & _CP_OFF & _MCLRE_OFF & _WDTE_OFF & _FOSC_INTOSC
 __CONFIG _CONFIG2, _LVP_OFF & _PLLEN_OFF & _WRT_OFF

;********************************************************************************

;Set aside memory locations for variables
COMPORT                          EQU 32
DELAYTEMP                        EQU 112
DELAYTEMP2                       EQU 113
HSERPRINTCRLFCOUNT               EQU 33
PRINTLEN                         EQU 34
SAVEFSR0H                        EQU 35
SAVEFSR0L                        EQU 36
SAVEPCLATH                       EQU 37
SAVESYSBYTETEMPA                 EQU 38
SAVESYSBYTETEMPB                 EQU 39
SAVESYSBYTETEMPX                 EQU 40
SAVESYSTEMP1                     EQU 41
SERDATA                          EQU 42
STRINGPOINTER                    EQU 43
SYSBSR                           EQU 44
SYSBYTETEMPA                     EQU 117
SYSBYTETEMPB                     EQU 121
SYSBYTETEMPX                     EQU 112
SYSINTSTATESAVE0                 EQU 45
SYSPRINTDATAHANDLER              EQU 46
SYSPRINTDATAHANDLER_H            EQU 47
SYSPRINTTEMP                     EQU 48
SYSREPEATTEMP1                   EQU 49
SYSSTATUS                        EQU 127
SYSSTRINGA                       EQU 119
SYSSTRINGA_H                     EQU 120
SYSTEMP1                         EQU 50
SYSTEMP2                         EQU 51
SYSW                             EQU 126
SYSWAITTEMPMS                    EQU 114
SYSWAITTEMPMS_H                  EQU 115

;********************************************************************************

;Alias variables
AFSR0 EQU 4
AFSR0_H EQU 5

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
	movf	BSR,W
	banksel	STATUS
	movwf	SysBSR
;Store system variables
	movf	FSR0L,W
	movwf	SaveFSR0L
	movf	FSR0H,W
	movwf	SaveFSR0H
	movf	SysTemp1,W
	movwf	SaveSysTemp1
	movf	SysByteTempA,W
	movwf	SaveSysByteTempA
	movf	SysByteTempB,W
	movwf	SaveSysByteTempB
	movf	SysByteTempX,W
	movwf	SaveSysByteTempX
	movf	PCLATH,W
	movwf	SavePCLATH
	clrf	PCLATH
;On Interrupt handlers
	btfss	INTCON,TMR0IE
	goto	NotTMR0IF
	btfss	INTCON,TMR0IF
	goto	NotTMR0IF
	call	READTMR0
	bcf	INTCON,TMR0IF
	goto	INTERRUPTDONE
NotTMR0IF
;User Interrupt routine
INTERRUPTDONE
;Restore Context
;Restore system variables
	movf	SaveFSR0L,W
	movwf	FSR0L
	movf	SaveFSR0H,W
	movwf	FSR0H
	movf	SaveSysTemp1,W
	movwf	SysTemp1
	movf	SaveSysByteTempA,W
	movwf	SysByteTempA
	movf	SaveSysByteTempB,W
	movwf	SysByteTempB
	movf	SaveSysByteTempX,W
	movwf	SysByteTempX
	movf	SavePCLATH,W
	movwf	PCLATH
	movf	SysBSR,W
	movwf	BSR
	swapf	SysSTATUS,W
	movwf	STATUS
	swapf	SysW,F
	swapf	SysW,W
	retfie

;********************************************************************************

;Start of program memory page 0
	ORG	54
BASPROGRAMSTART
;Call initialisation routines
	call	INITSYS
	call	INITUSART
;Enable interrupts
	bsf	INTCON,GIE
	bsf	INTCON,PEIE
	bsf	SYSINTSTATESAVE0,0
;Automatic pin direction setting
	banksel	TRISB
	bcf	TRISB,5

;Start of the main program
	banksel	SYSINTSTATESAVE0
	btfss	SYSINTSTATESAVE0,0
	bcf	INTCON,GIE
	btfsc	SYSINTSTATESAVE0,0
	bsf	INTCON,GIE
	banksel	OPTION_REG
	bcf	OPTION_REG,TMR0CS
	bcf	OPTION_REG,PSA
	bcf	OPTION_REG,PS0
	bcf	OPTION_REG,PS1
	bcf	OPTION_REG,PS2
	bsf	INTCON,TMR0IE
	bcf	INTCON,TMR0IF
	bsf	TRISB,1
	bcf	TRISB,2
	movlw	low StringTable1
	banksel	SYSPRINTDATAHANDLER
	movwf	SysPRINTDATAHandler
	movlw	(high StringTable1) | 128
	movwf	SysPRINTDATAHandler_H
	movlw	1
	movwf	COMPORT
	call	HSERPRINT299
	movlw	1
	movwf	HSERPRINTCRLFCOUNT
	movlw	1
	movwf	COMPORT
	call	HSERPRINTCRLF
	bsf	INTCON,TMR0IE
SysDoLoop_S1
	movlw	50
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	goto	SysDoLoop_S1
SysDoLoop_E1
	goto	BASPROGRAMEND
BASPROGRAMEND
	sleep
	goto	BASPROGRAMEND

;********************************************************************************

Delay_MS
	incf	SysWaitTempMS_H, F
DMS_START
	movlw	108
	movwf	DELAYTEMP2
DMS_OUTER
	movlw	11
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

HSERPRINT299
	movf	SysPRINTDATAHandler,W
	movwf	AFSR0
	movf	SysPRINTDATAHandler_H,W
	movwf	AFSR0_H
	movf	INDF0,W
	movwf	PRINTLEN
	movf	PRINTLEN,F
	btfsc	STATUS, Z
	goto	ENDIF1
	movlw	1
	movwf	SYSPRINTTEMP
SysForLoop1
	movf	SYSPRINTTEMP,W
	addwf	SysPRINTDATAHandler,W
	movwf	AFSR0
	movlw	0
	addwfc	SysPRINTDATAHandler_H,W
	movwf	AFSR0_H
	movf	INDF0,W
	movwf	SERDATA
	call	HSERSEND289
;#4p Positive value Step Handler in For-next statement
	movf	SYSPRINTTEMP,W
	subwf	PRINTLEN,W
	movwf	SysTemp1
	movwf	SysBYTETempA
	clrf	SysBYTETempB
	call	SYSCOMPEQUAL
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF2
;Set LoopVar to LoopVar + StepValue where StepValue is a positive value
	incf	SYSPRINTTEMP,F
	goto	SysForLoop1
;END IF
ENDIF2
SysForLoopEnd1
ENDIF1
	return

;********************************************************************************

HSERPRINTCRLF
	movf	HSERPRINTCRLFCOUNT,W
	movwf	SysRepeatTemp1
	btfsc	STATUS,Z
	goto	SysRepeatLoopEnd1
SysRepeatLoop1
	movlw	13
	movwf	SERDATA
	call	HSERSEND289
	movlw	10
	movwf	SERDATA
	call	HSERSEND289
	decfsz	SysRepeatTemp1,F
	goto	SysRepeatLoop1
SysRepeatLoopEnd1
	return

;********************************************************************************

HSERSEND289
	decf	COMPORT,W
	btfss	STATUS, Z
	goto	ENDIF3
SysWaitLoop1
	btfss	PIR1,TXIF
	goto	SysWaitLoop1
SysWaitLoop2
	banksel	TXSTA
	btfss	TXSTA,TRMT
	goto	SysWaitLoop2
	banksel	SERDATA
	movf	SERDATA,W
	banksel	TXREG
	movwf	TXREG
ENDIF3
	banksel	STATUS
	return

;********************************************************************************

INITSYS
;asm showdebug This code block sets the internal oscillator to ChipMHz
;asm showdebug OSCCON type is 105 'Bit(SPLLEN) Or Bit(IRCF3) And NoBit(INTSRC) and ifdef Bit(IRCF3)
	banksel	OSCCON
	bsf	OSCCON,IRCF3
	bsf	OSCCON,IRCF2
	bsf	OSCCON,IRCF1
	bsf	OSCCON,IRCF0
	bcf	OSCCON,SPLLEN
;asm showdebug _Complete_the_chip_setup_of_BSR,ADCs,ANSEL_and_other_key_setup_registers_or_register_bits
	bcf	ADCON1,ADFM
	bcf	ADCON0,ADON
	banksel	ANSELA
	clrf	ANSELA
	clrf	ANSELB
	banksel	CM2CON0
	bcf	CM2CON0,C2ON
	bcf	CM1CON0,C1ON
	banksel	PORTA
	clrf	PORTA
	clrf	PORTB
	return

;********************************************************************************

INITUSART
;asm showdebug Values_calculated_in_the_script
;asm showdebug _SPBRGH_TEMP=_ SPBRGH_TEMP
;asm showdebug _SPBRGL_TEMP=_ SPBRGL_TEMP
;asm showdebug _BRG16_TEMP=_ BRG16_TEMP
;asm showdebug _BRGH_TEMP=_ BRGH_TEMP
	movlw	159
	banksel	SPBRG
	movwf	SPBRG
	movlw	1
	movwf	SP1BRGH
	movlw	159
	movwf	SP1BRGL
	bsf	BAUDCON,BRG16
	bsf	TXSTA,BRGH
	bcf	TXSTA,SYNC
	bsf	TXSTA,TXEN
	bsf	RCSTA,SPEN
	bsf	RCSTA,CREN
	banksel	STATUS
	return

;********************************************************************************

READTMR0
	banksel	LATB
	bsf	LATB,RB5
	bcf	INTCON,TMR0IF
	movlw	low StringTable2
	banksel	SYSPRINTDATAHANDLER
	movwf	SysPRINTDATAHandler
	movlw	(high StringTable2) | 128
	movwf	SysPRINTDATAHandler_H
	movlw	1
	movwf	COMPORT
	call	HSERPRINT299
	movlw	1
	movwf	HSERPRINTCRLFCOUNT
	movlw	1
	movwf	COMPORT
	call	HSERPRINTCRLF
	banksel	LATB
	bcf	LATB,RB5
	banksel	STATUS
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

SysStringTables
	movf	SysStringA_H,W
	movwf	PCLATH
	movf	SysStringA,W
	incf	SysStringA,F
	btfsc	STATUS,Z
	incf	SysStringA_H,F
	movwf	PCL

StringTable1
	retlw	32
	retlw	84	;T
	retlw	101	;e
	retlw	115	;s
	retlw	116	;t
	retlw	32	; 
	retlw	49	;1
	retlw	54	;6
	retlw	70	;F
	retlw	49	;1
	retlw	56	;8
	retlw	50	;2
	retlw	54	;6
	retlw	32	; 
	retlw	85	;U
	retlw	83	;S
	retlw	65	;A
	retlw	82	;R
	retlw	84	;T
	retlw	32	; 
	retlw	67	;C
	retlw	111	;o
	retlw	109	;m
	retlw	109	;m
	retlw	117	;u
	retlw	110	;n
	retlw	105	;i
	retlw	99	;c
	retlw	97	;a
	retlw	116	;t
	retlw	105	;i
	retlw	111	;o
	retlw	110	;n


StringTable2
	retlw	13
	retlw	84	;T
	retlw	77	;M
	retlw	82	;R
	retlw	48	;0
	retlw	32	; 
	retlw	73	;I
	retlw	110	;n
	retlw	116	;t
	retlw	101	;e
	retlw	114	;r
	retlw	117	;u
	retlw	112	;p
	retlw	116	;t


;********************************************************************************


 END
