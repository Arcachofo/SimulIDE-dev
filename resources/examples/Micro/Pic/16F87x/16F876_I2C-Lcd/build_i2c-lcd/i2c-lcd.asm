;Program compiled by Great Cow BASIC (0.98.<<>> 2021-<<>>-24 (Linux 64 bit) : Build 1005) for Microchip MPASM
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=16F876, r=DEC
#include <P16F876.inc>
 __CONFIG _WRT_OFF & _CPD_OFF & _LVP_OFF & _CP_OFF & _WDTE_OFF & _FOSC_HS

;********************************************************************************

;Set aside memory locations for variables
DELAYTEMP                        EQU 112
DELAYTEMP2                       EQU 113
I2CACK                           EQU 32
I2CACKPOLLSTATE                  EQU 33
I2CBYTE                          EQU 34
I2CMATCH                         EQU 35
I2CSTATE                         EQU 36
I2C_LCD_BYTE                     EQU 37
LCDBYTE                          EQU 38
LCDCOLUMN                        EQU 39
LCDCRSR                          EQU 40
LCDLINE                          EQU 41
LCDTEMP                          EQU 42
LCD_BACKLIGHT                    EQU 43
LCD_I2C_ADDRESS_CURRENT          EQU 44
LCD_STATE                        EQU 45
PRINTLEN                         EQU 46
STRINGPOINTER                    EQU 47
SYSBYTETEMPA                     EQU 117
SYSBYTETEMPB                     EQU 121
SYSBYTETEMPX                     EQU 112
SYSCALCTEMPA                     EQU 117
SYSLCDTEMP                       EQU 48
SYSPRINTDATAHANDLER              EQU 49
SYSPRINTDATAHANDLER_H            EQU 50
SYSPRINTTEMP                     EQU 51
SYSREPEATTEMP1                   EQU 52
SYSREPEATTEMP2                   EQU 53
SYSREPEATTEMP3                   EQU 54
SYSSTRINGA                       EQU 119
SYSSTRINGA_H                     EQU 120
SYSSTRINGB                       EQU 114
SYSSTRINGB_H                     EQU 115
SYSSTRINGLENGTH                  EQU 118
SYSSTRINGPARAM1                  EQU 479
SYSTEMP1                         EQU 55
SYSTEMP2                         EQU 56
SYSWAITTEMP10US                  EQU 117
SYSWAITTEMPMS                    EQU 114
SYSWAITTEMPMS_H                  EQU 115
SYSWAITTEMPS                     EQU 116
SYSWAITTEMPUS                    EQU 117
SYSWAITTEMPUS_H                  EQU 118

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
	call	INITI2C
	call	INITLCD

;Start of the main program
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable1
	movwf	SysStringA
	movlw	high StringTable1
	movwf	SysStringA_H
	call	SYSREADSTRING
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H
	call	PRINT106
	movlw	1
	movwf	SysWaitTempS
	call	Delay_S
SysDoLoop_S1
	call	CLS
	movlw	3
	movwf	SysWaitTempS
	call	Delay_S
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable2
	movwf	SysStringA
	movlw	high StringTable2
	movwf	SysStringA_H
	call	SYSREADSTRING
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H
	call	PRINT106
	movlw	1
	movwf	LCDLINE
	clrf	LCDCOLUMN
	call	LOCATE
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable3
	movwf	SysStringA
	movlw	high StringTable3
	movwf	SysStringA_H
	call	SYSREADSTRING
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H
	call	PRINT106
	movlw	3
	movwf	SysWaitTempS
	call	Delay_S
	call	CLS
	clrf	LCDLINE
	clrf	LCDCOLUMN
	call	LOCATE
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable4
	movwf	SysStringA
	movlw	high StringTable4
	movwf	SysStringA_H
	call	SYSREADSTRING
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H
	call	PRINT106
	movlw	1
	movwf	LCDLINE
	clrf	LCDCOLUMN
	call	LOCATE
	movlw	10
	movwf	LCDCRSR
	call	LCDCURSOR
	movlw	3
	movwf	SysWaitTempS
	call	Delay_S
	call	CLS
	movlw	13
	movwf	LCDCRSR
	call	LCDCURSOR
	clrf	LCDLINE
	clrf	LCDCOLUMN
	call	LOCATE
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable5
	movwf	SysStringA
	movlw	high StringTable5
	movwf	SysStringA_H
	call	SYSREADSTRING
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H
	call	PRINT106
	movlw	3
	movwf	SysWaitTempS
	call	Delay_S
	call	CLS
	clrf	LCDLINE
	clrf	LCDCOLUMN
	call	LOCATE
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable6
	movwf	SysStringA
	movlw	high StringTable6
	movwf	SysStringA_H
	call	SYSREADSTRING
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H
	call	PRINT106
	movlw	1
	movwf	LCDLINE
	clrf	LCDCOLUMN
	call	LOCATE
	movlw	9
	movwf	LCDCRSR
	call	LCDCURSOR
	movlw	3
	movwf	SysWaitTempS
	call	Delay_S
	call	CLS
	clrf	LCDLINE
	clrf	LCDCOLUMN
	call	LOCATE
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable7
	movwf	SysStringA
	movlw	high StringTable7
	movwf	SysStringA_H
	call	SYSREADSTRING
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H
	call	PRINT106
	movlw	14
	movwf	LCDCRSR
	call	LCDCURSOR
	movlw	3
	movwf	SysWaitTempS
	call	Delay_S
	clrf	LCDLINE
	clrf	LCDCOLUMN
	call	LOCATE
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable8
	movwf	SysStringA
	movlw	high StringTable8
	movwf	SysStringA_H
	call	SYSREADSTRING
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H
	call	PRINT106
	movlw	1
	movwf	LCDLINE
	clrf	LCDCOLUMN
	call	LOCATE
	movlw	10
	movwf	LCDCRSR
	call	LCDCURSOR
	movlw	9
	movwf	LCDCRSR
	call	LCDCURSOR
	movlw	3
	movwf	SysWaitTempS
	call	Delay_S
	clrf	LCDLINE
	clrf	LCDCOLUMN
	call	LOCATE
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable9
	movwf	SysStringA
	movlw	high StringTable9
	movwf	SysStringA_H
	call	SYSREADSTRING
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H
	call	PRINT106
	movlw	1
	movwf	LCDLINE
	clrf	LCDCOLUMN
	call	LOCATE
	movlw	13
	movwf	LCDCRSR
	call	LCDCURSOR
	movlw	14
	movwf	LCDCRSR
	call	LCDCURSOR
	movlw	3
	movwf	SysWaitTempS
	call	Delay_S
	call	CLS
	clrf	LCDLINE
	movlw	4
	movwf	LCDCOLUMN
	call	LOCATE
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable10
	movwf	SysStringA
	movlw	high StringTable10
	movwf	SysStringA_H
	call	SYSREADSTRING
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H
	call	PRINT106
	movlw	1
	movwf	LCDLINE
	movlw	4
	movwf	LCDCOLUMN
	call	LOCATE
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable11
	movwf	SysStringA
	movlw	high StringTable11
	movwf	SysStringA_H
	call	SYSREADSTRING
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H
	call	PRINT106
	movlw	244
	movwf	SysWaitTempMS
	movlw	1
	movwf	SysWaitTempMS_H
	call	Delay_MS
	movlw	10
	movwf	SysRepeatTemp1
SysRepeatLoop1
	movlw	11
	movwf	LCDCRSR
	call	LCDCURSOR
	movlw	244
	movwf	SysWaitTempMS
	movlw	1
	movwf	SysWaitTempMS_H
	call	Delay_MS
	movlw	12
	movwf	LCDCRSR
	call	LCDCURSOR
	movlw	244
	movwf	SysWaitTempMS
	movlw	1
	movwf	SysWaitTempMS_H
	call	Delay_MS
	decfsz	SysRepeatTemp1,F
	goto	SysRepeatLoop1
SysRepeatLoopEnd1
	call	CLS
	clrf	LCDLINE
	clrf	LCDCOLUMN
	call	LOCATE
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable12
	movwf	SysStringA
	movlw	high StringTable12
	movwf	SysStringA_H
	call	SYSREADSTRING
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H
	call	PRINT106
	movlw	1
	movwf	LCDLINE
	clrf	LCDCOLUMN
	call	LOCATE
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable13
	movwf	SysStringA
	movlw	high StringTable13
	movwf	SysStringA_H
	call	SYSREADSTRING
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H
	call	PRINT106
	movlw	2
	movwf	SysWaitTempS
	call	Delay_S
	movlw	11
	movwf	LCDCRSR
	call	LCDCURSOR
	clrf	LCDTEMP
	call	LCDBACKLIGHT
	movlw	5
	movwf	SysWaitTempS
	call	Delay_S
	movlw	1
	movwf	LCDTEMP
	call	LCDBACKLIGHT
	call	CLS
	clrf	LCDLINE
	clrf	LCDCOLUMN
	call	LOCATE
	movlw	12
	movwf	LCDCRSR
	call	LCDCURSOR
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable14
	movwf	SysStringA
	movlw	high StringTable14
	movwf	SysStringA_H
	call	SYSREADSTRING
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H
	call	PRINT106
	movlw	3
	movwf	SysWaitTempS
	call	Delay_S
	goto	SysDoLoop_S1
SysDoLoop_E1
	goto	BASPROGRAMEND
BASPROGRAMEND
	sleep
	goto	BASPROGRAMEND

;********************************************************************************

CLS
	bcf	SYSLCDTEMP,1
	movlw	1
	movwf	LCDBYTE
	call	LCDNORMALWRITEBYTE
	movlw	4
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	movlw	128
	movwf	LCDBYTE
	call	LCDNORMALWRITEBYTE
	movlw	66
	movwf	DELAYTEMP
DelayUS1
	decfsz	DELAYTEMP,F
	goto	DelayUS1
	nop
	return

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

I2CRESTART
	goto	I2CSTART

;********************************************************************************

I2CSEND
	banksel	TRISC
	bcf	TRISC,3
	banksel	PORTC
	bcf	PORTC,3
	goto	$+1
	goto	$+1
	movlw	8
	movwf	SysRepeatTemp3
SysRepeatLoop3
	btfss	I2CBYTE,7
	goto	ELSE28_1
	banksel	TRISC
	bsf	TRISC,4
	goto	ENDIF28
ELSE28_1
	banksel	TRISC
	bcf	TRISC,4
	banksel	PORTC
	bcf	PORTC,4
ENDIF28
	banksel	I2CBYTE
	rlf	I2CBYTE,F
	banksel	TRISC
	bsf	TRISC,3
SysWaitLoop1
	banksel	PORTC
	btfss	PORTC,3
	goto	SysWaitLoop1
	goto	$+1
	goto	$+1
	banksel	TRISC
	bcf	TRISC,3
	banksel	PORTC
	bcf	PORTC,3
	goto	$+1
	goto	$+1
	decfsz	SysRepeatTemp3,F
	goto	SysRepeatLoop3
SysRepeatLoopEnd3
	movlw	2
	movwf	DELAYTEMP
DelayUS3
	decfsz	DELAYTEMP,F
	goto	DelayUS3
	nop
	banksel	TRISC
	bsf	TRISC,4
	goto	$+1
	goto	$+1
	bsf	TRISC,3
SysWaitLoop2
	banksel	PORTC
	btfss	PORTC,3
	goto	SysWaitLoop2
	btfss	PORTC,4
	goto	ELSE29_1
	clrf	I2CACK
	clrf	I2CACKPOLLSTATE
	goto	ENDIF29
ELSE29_1
	movlw	255
	movwf	I2CACK
	movlw	255
	movwf	I2CACKPOLLSTATE
ENDIF29
	banksel	TRISC
	bcf	TRISC,3
	banksel	PORTC
	bcf	PORTC,3
	goto	$+1
	goto	$+1
	banksel	TRISC
	bcf	TRISC,4
	banksel	PORTC
	bcf	PORTC,4
	movlw	2
	movwf	DELAYTEMP
DelayUS4
	decfsz	DELAYTEMP,F
	goto	DelayUS4
	nop
	return

;********************************************************************************

I2CSTART
	banksel	TRISC
	bsf	TRISC,4
	bsf	TRISC,3
	goto	$+1
	goto	$+1
	bcf	TRISC,4
	banksel	PORTC
	bcf	PORTC,4
	goto	$+1
	goto	$+1
	banksel	TRISC
	bcf	TRISC,3
	banksel	PORTC
	bcf	PORTC,3
	return

;********************************************************************************

I2CSTOP
	banksel	TRISC
	bcf	TRISC,3
	banksel	PORTC
	bcf	PORTC,3
	banksel	TRISC
	bcf	TRISC,4
	banksel	PORTC
	bcf	PORTC,4
	goto	$+1
	goto	$+1
	banksel	TRISC
	bsf	TRISC,3
	goto	$+1
	goto	$+1
	bsf	TRISC,4
	goto	$+1
	goto	$+1
	banksel	STATUS
	return

;********************************************************************************

INITI2C
	banksel	TRISC
	bsf	TRISC,4
	bsf	TRISC,3
	banksel	I2CMATCH
	clrf	I2CMATCH
	return

;********************************************************************************

INITI2CLCD
	movlw	15
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	clrf	I2C_LCD_BYTE
	movlw	3
	movwf	LCDBYTE
	call	LCDNORMALWRITEBYTE
	movlw	5
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	movlw	3
	movwf	LCDBYTE
	call	LCDNORMALWRITEBYTE
	movlw	1
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	movlw	3
	movwf	LCDBYTE
	call	LCDNORMALWRITEBYTE
	movlw	1
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	movlw	3
	movwf	LCDBYTE
	call	LCDNORMALWRITEBYTE
	movlw	1
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	movlw	2
	movwf	LCDBYTE
	call	LCDNORMALWRITEBYTE
	movlw	1
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	movlw	40
	movwf	LCDBYTE
	call	LCDNORMALWRITEBYTE
	movlw	1
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	movlw	12
	movwf	LCDBYTE
	call	LCDNORMALWRITEBYTE
	movlw	1
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	movlw	1
	movwf	LCDBYTE
	call	LCDNORMALWRITEBYTE
	movlw	15
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	movlw	6
	movwf	LCDBYTE
	call	LCDNORMALWRITEBYTE
	movlw	1
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	goto	CLS

;********************************************************************************

INITLCD
;asm showdebug  `LCD_IO selected is ` LCD_IO
;asm showdebug  `LCD_Speed is FAST`
;asm showdebug  `OPTIMAL is set to ` OPTIMAL
;asm showdebug  `LCD_Speed is set to ` LCD_Speed
	call	INITI2C
	movlw	1
	movwf	LCD_BACKLIGHT
	movlw	2
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	movlw	2
	movwf	SysRepeatTemp2
SysRepeatLoop2
	movlw	78
	movwf	LCD_I2C_ADDRESS_CURRENT
	call	INITI2CLCD
	decfsz	SysRepeatTemp2,F
	goto	SysRepeatLoop2
SysRepeatLoopEnd2
	movlw	12
	movwf	LCD_STATE
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

LCDBACKLIGHT
	movf	LCDTEMP,F
	btfsc	STATUS, Z
	clrf	LCD_BACKLIGHT
	decf	LCDTEMP,W
	btfss	STATUS, Z
	goto	ENDIF27
	movlw	1
	movwf	LCD_BACKLIGHT
ENDIF27
	bcf	SYSLCDTEMP,1
	clrf	LCDBYTE
	goto	LCDNORMALWRITEBYTE

;********************************************************************************

LCDCURSOR
	bcf	SYSLCDTEMP,1
	decf	LCDCRSR,W
	btfss	STATUS, Z
	goto	ENDIF17
	movlw	12
	iorwf	LCD_STATE,W
	movwf	LCDTEMP
ENDIF17
	movlw	12
	subwf	LCDCRSR,W
	btfss	STATUS, Z
	goto	ENDIF18
	movlw	12
	iorwf	LCD_STATE,W
	movwf	LCDTEMP
ENDIF18
	movf	LCDCRSR,F
	btfss	STATUS, Z
	goto	ENDIF19
	movlw	11
	andwf	LCD_STATE,W
	movwf	LCDTEMP
ENDIF19
	movlw	11
	subwf	LCDCRSR,W
	btfss	STATUS, Z
	goto	ENDIF20
	movlw	11
	andwf	LCD_STATE,W
	movwf	LCDTEMP
ENDIF20
	movlw	10
	subwf	LCDCRSR,W
	btfss	STATUS, Z
	goto	ENDIF21
	movlw	10
	iorwf	LCD_STATE,W
	movwf	LCDTEMP
ENDIF21
	movlw	13
	subwf	LCDCRSR,W
	btfss	STATUS, Z
	goto	ENDIF22
	movlw	13
	andwf	LCD_STATE,W
	movwf	LCDTEMP
ENDIF22
	movlw	9
	subwf	LCDCRSR,W
	btfss	STATUS, Z
	goto	ENDIF23
	movlw	9
	iorwf	LCD_STATE,W
	movwf	LCDTEMP
ENDIF23
	movlw	9
	subwf	LCDCRSR,W
	btfss	STATUS, Z
	goto	ENDIF24
	movlw	9
	iorwf	LCD_STATE,W
	movwf	LCDTEMP
ENDIF24
	movlw	14
	subwf	LCDCRSR,W
	btfss	STATUS, Z
	goto	ENDIF25
	movlw	14
	andwf	LCD_STATE,W
	movwf	LCDTEMP
ENDIF25
	movf	LCDTEMP,W
	movwf	LCDBYTE
	call	LCDNORMALWRITEBYTE
	movf	LCDTEMP,W
	movwf	LCD_STATE
	return

;********************************************************************************

LCDNORMALWRITEBYTE
	btfss	SYSLCDTEMP,1
	goto	ELSE4_1
	bsf	I2C_LCD_BYTE,0
	goto	ENDIF4
ELSE4_1
	bcf	I2C_LCD_BYTE,0
ENDIF4
	bcf	I2C_LCD_BYTE,1
	bcf	I2C_LCD_BYTE,3
	btfsc	LCD_BACKLIGHT,0
	bsf	I2C_LCD_BYTE,3
	call	I2CRESTART
	movf	LCD_I2C_ADDRESS_CURRENT,W
	movwf	I2CBYTE
	call	I2CSEND
	bcf	I2C_LCD_BYTE,7
	btfsc	LCDBYTE,7
	bsf	I2C_LCD_BYTE,7
	bcf	I2C_LCD_BYTE,6
	btfsc	LCDBYTE,6
	bsf	I2C_LCD_BYTE,6
	bcf	I2C_LCD_BYTE,5
	btfsc	LCDBYTE,5
	bsf	I2C_LCD_BYTE,5
	bcf	I2C_LCD_BYTE,4
	btfsc	LCDBYTE,4
	bsf	I2C_LCD_BYTE,4
	bsf	I2C_LCD_BYTE,2
	movf	I2C_LCD_BYTE,W
	movwf	I2CBYTE
	call	I2CSEND
	bcf	I2C_LCD_BYTE,2
	movf	I2C_LCD_BYTE,W
	movwf	I2CBYTE
	call	I2CSEND
	bcf	I2C_LCD_BYTE,7
	btfsc	LCDBYTE,3
	bsf	I2C_LCD_BYTE,7
	bcf	I2C_LCD_BYTE,6
	btfsc	LCDBYTE,2
	bsf	I2C_LCD_BYTE,6
	bcf	I2C_LCD_BYTE,5
	btfsc	LCDBYTE,1
	bsf	I2C_LCD_BYTE,5
	bcf	I2C_LCD_BYTE,4
	btfsc	LCDBYTE,0
	bsf	I2C_LCD_BYTE,4
	bsf	I2C_LCD_BYTE,2
	movf	I2C_LCD_BYTE,W
	movwf	I2CBYTE
	call	I2CSEND
	bcf	I2C_LCD_BYTE,2
	movf	I2C_LCD_BYTE,W
	movwf	I2CBYTE
	call	I2CSEND
	call	I2CSTOP
	movlw	12
	movwf	LCD_STATE
	movlw	13
	movwf	DELAYTEMP
DelayUS2
	decfsz	DELAYTEMP,F
	goto	DelayUS2
	btfsc	SYSLCDTEMP,1
	goto	ENDIF5
	movlw	16
	subwf	LCDBYTE,W
	btfsc	STATUS, C
	goto	ENDIF6
	movf	LCDBYTE,W
	sublw	7
	btfsc	STATUS, C
	goto	ENDIF7
	movf	LCDBYTE,W
	movwf	LCD_STATE
ENDIF7
ENDIF6
ENDIF5
	return

;********************************************************************************

LOCATE
	bcf	SYSLCDTEMP,1
	movf	LCDLINE,W
	sublw	1
	btfsc	STATUS, C
	goto	ENDIF1
	movlw	2
	subwf	LCDLINE,F
	movlw	20
	addwf	LCDCOLUMN,F
ENDIF1
	movf	LCDLINE,W
	movwf	SysBYTETempA
	movlw	64
	movwf	SysBYTETempB
	call	SYSMULTSUB
	movf	LCDCOLUMN,W
	addwf	SysBYTETempX,W
	movwf	SysTemp1
	movlw	128
	iorwf	SysTemp1,W
	movwf	LCDBYTE
	call	LCDNORMALWRITEBYTE
	movlw	5
	movwf	SysWaitTemp10US
	goto	Delay_10US

;********************************************************************************

PRINT106
	movf	SysPRINTDATAHandler,W
	movwf	FSR
	bcf	STATUS, IRP
	btfsc	SysPRINTDATAHandler_H,0
	bsf	STATUS, IRP
	movf	INDF,W
	movwf	PRINTLEN
	movf	PRINTLEN,F
	btfsc	STATUS, Z
	return
	bsf	SYSLCDTEMP,1
	movlw	1
	movwf	SYSPRINTTEMP
SysForLoop1
	movf	SYSPRINTTEMP,W
	addwf	SysPRINTDATAHandler,W
	movwf	FSR
	bcf	STATUS, IRP
	btfsc	SysPRINTDATAHandler_H,0
	bsf	STATUS, IRP
	movf	INDF,W
	movwf	LCDBYTE
	call	LCDNORMALWRITEBYTE
;#4p Positive value Step Handler in For-next statement
	movf	SYSPRINTTEMP,W
	subwf	PRINTLEN,W
	movwf	SysTemp1
	movwf	SysBYTETempA
	clrf	SysBYTETempB
	call	SYSCOMPEQUAL
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF3
;Set LoopVar to LoopVar + StepValue where StepValue is a positive value
	incf	SYSPRINTTEMP,F
	goto	SysForLoop1
;END IF
ENDIF3
SysForLoopEnd1
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

SYSMULTSUB
	clrf	SYSBYTETEMPX
MUL8LOOP
	movf	SYSBYTETEMPA, W
	btfsc	SYSBYTETEMPB, 0
	addwf	SYSBYTETEMPX, F
	bcf	STATUS, C
	rrf	SYSBYTETEMPB, F
	bcf	STATUS, C
	rlf	SYSBYTETEMPA, F
	movf	SYSBYTETEMPB, F
	btfss	STATUS, Z
	goto	MUL8LOOP
	return

;********************************************************************************

SYSREADSTRING
	movf	SYSSTRINGB, W
	movwf	FSR
	bcf	STATUS, IRP
	btfsc	SYSSTRINGB_H, 0
	bsf	STATUS, IRP
	call	SYSSTRINGTABLES
	movwf	SYSCALCTEMPA
	movwf	INDF
	addwf	SYSSTRINGB, F
	goto	SYSSTRINGREADCHECK
SYSREADSTRINGPART
	movf	SYSSTRINGB, W
	movwf	FSR
	bcf	STATUS, IRP
	btfsc	SYSSTRINGB_H, 0
	bsf	STATUS, IRP
	call	SYSSTRINGTABLES
	movwf	SYSCALCTEMPA
	addwf	SYSSTRINGLENGTH,F
	addwf	SYSSTRINGB,F
SYSSTRINGREADCHECK
	movf	SYSCALCTEMPA,F
	btfsc	STATUS,Z
	return
SYSSTRINGREAD
	call	SYSSTRINGTABLES
	incf	FSR, F
	movwf	INDF
	decfsz	SYSCALCTEMPA, F
	goto	SYSSTRINGREAD
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
	retlw	15
	retlw	71	;G
	retlw	114	;r
	retlw	101	;e
	retlw	97	;a
	retlw	116	;t
	retlw	32	; 
	retlw	67	;C
	retlw	111	;o
	retlw	119	;w
	retlw	32	; 
	retlw	66	;B
	retlw	97	;a
	retlw	115	;s
	retlw	105	;i
	retlw	99	;c


StringTable2
	retlw	10
	retlw	83	;S
	retlw	84	;T
	retlw	65	;A
	retlw	82	;R
	retlw	84	;T
	retlw	32	; 
	retlw	84	;T
	retlw	69	;E
	retlw	83	;S
	retlw	84	;T


StringTable3
	retlw	10
	retlw	68	;D
	retlw	73	;I
	retlw	83	;S
	retlw	80	;P
	retlw	76	;L
	retlw	65	;A
	retlw	89	;Y
	retlw	32	; 
	retlw	79	;O
	retlw	78	;N


StringTable4
	retlw	9
	retlw	67	;C
	retlw	117	;u
	retlw	114	;r
	retlw	115	;s
	retlw	111	;o
	retlw	114	;r
	retlw	32	; 
	retlw	79	;O
	retlw	78	;N


StringTable5
	retlw	10
	retlw	67	;C
	retlw	117	;u
	retlw	114	;r
	retlw	115	;s
	retlw	111	;o
	retlw	114	;r
	retlw	32	; 
	retlw	79	;O
	retlw	70	;F
	retlw	70	;F


StringTable6
	retlw	8
	retlw	70	;F
	retlw	76	;L
	retlw	65	;A
	retlw	83	;S
	retlw	72	;H
	retlw	32	; 
	retlw	79	;O
	retlw	78	;N


StringTable7
	retlw	9
	retlw	70	;F
	retlw	76	;L
	retlw	65	;A
	retlw	83	;S
	retlw	72	;H
	retlw	32	; 
	retlw	79	;O
	retlw	70	;F
	retlw	70	;F


StringTable8
	retlw	15
	retlw	67	;C
	retlw	85	;U
	retlw	82	;R
	retlw	83	;S
	retlw	82	;R
	retlw	32	; 
	retlw	38	;&
	retlw	32	; 
	retlw	70	;F
	retlw	76	;L
	retlw	83	;S
	retlw	72	;H
	retlw	32	; 
	retlw	79	;O
	retlw	78	;N


StringTable9
	retlw	16
	retlw	67	;C
	retlw	85	;U
	retlw	82	;R
	retlw	83	;S
	retlw	82	;R
	retlw	32	; 
	retlw	38	;&
	retlw	32	; 
	retlw	70	;F
	retlw	76	;L
	retlw	83	;S
	retlw	72	;H
	retlw	32	; 
	retlw	79	;O
	retlw	70	;F
	retlw	70	;F


StringTable10
	retlw	8
	retlw	70	;F
	retlw	108	;l
	retlw	97	;a
	retlw	115	;s
	retlw	104	;h
	retlw	105	;i
	retlw	110	;n
	retlw	103	;g


StringTable11
	retlw	7
	retlw	68	;D
	retlw	105	;i
	retlw	115	;s
	retlw	112	;p
	retlw	108	;l
	retlw	97	;a
	retlw	121	;y


StringTable12
	retlw	16
	retlw	68	;D
	retlw	73	;I
	retlw	83	;S
	retlw	80	;P
	retlw	76	;L
	retlw	65	;A
	retlw	89	;Y
	retlw	32	; 
	retlw	38	;&
	retlw	32	; 
	retlw	66	;B
	retlw	65	;A
	retlw	67	;C
	retlw	75	;K
	retlw	76	;L
	retlw	46	;.


StringTable13
	retlw	9
	retlw	70	;F
	retlw	79	;O
	retlw	82	;R
	retlw	32	; 
	retlw	53	;5
	retlw	32	; 
	retlw	83	;S
	retlw	69	;E
	retlw	67	;C


StringTable14
	retlw	8
	retlw	69	;E
	retlw	78	;N
	retlw	68	;D
	retlw	32	; 
	retlw	84	;T
	retlw	69	;E
	retlw	83	;S
	retlw	84	;T


;********************************************************************************

;Start of program memory page 1
	ORG	2048
;Start of program memory page 2
	ORG	4096
;Start of program memory page 3
	ORG	6144

 END
