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
ADR                              EQU 32
ADR_H                            EQU 33
COMPORT                          EQU 34
DATARRAY                         EQU 466
DELAYTEMP                        EQU 112
DELAYTEMP2                       EQU 113
EEPADDR                          EQU 35
EEPADDR_H                        EQU 36
EEPDEV                           EQU 37
EEPROMVAL                        EQU 38
HI2CACKPOLLSTATE                 EQU 39
HI2CCURRENTMODE                  EQU 40
HI2CGETACK                       EQU 41
HI2CWAITMSSPTIMEOUT              EQU 42
HSERPRINTCRLFCOUNT               EQU 43
I2CBYTE                          EQU 44
OUTVALUETEMP                     EQU 45
PRINTLEN                         EQU 46
SERDATA                          EQU 47
SERPRINTVAL                      EQU 48
STRINGPOINTER                    EQU 49
SYSBYTETEMPA                     EQU 117
SYSBYTETEMPB                     EQU 121
SYSBYTETEMPX                     EQU 112
SYSCALCTEMPA                     EQU 117
SYSCALCTEMPX                     EQU 112
SYSDIVLOOP                       EQU 116
SYSPRINTDATAHANDLER              EQU 50
SYSPRINTDATAHANDLER_H            EQU 51
SYSPRINTTEMP                     EQU 52
SYSREPEATTEMP1                   EQU 53
SYSSTRINGA                       EQU 119
SYSSTRINGA_H                     EQU 120
SYSSTRINGB                       EQU 114
SYSSTRINGB_H                     EQU 115
SYSSTRINGLENGTH                  EQU 118
SYSSTRINGPARAM1                  EQU 477
SYSTEMP1                         EQU 54
SYSTEMP2                         EQU 55
SYSWAITTEMPMS                    EQU 114
SYSWAITTEMPMS_H                  EQU 115
SYSWORDTEMPA                     EQU 117
SYSWORDTEMPA_H                   EQU 118
SYSWORDTEMPB                     EQU 121
SYSWORDTEMPB_H                   EQU 122
XXX                              EQU 56

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
	call	INITUSART
	call	HI2CINIT

;Start of the main program
	movlw	12
	movwf	HI2CCURRENTMODE
	call	HI2CMODE
	banksel	TRISC
	bsf	TRISC,4
	bsf	TRISC,3
	bcf	TRISC,6
	bsf	TRISC,7
	movlw	244
	movwf	SysWaitTempMS
	movlw	1
	movwf	SysWaitTempMS_H
	banksel	STATUS
	call	Delay_MS
	movlw	2
	movwf	HSERPRINTCRLFCOUNT
	movlw	1
	movwf	COMPORT
	call	HSERPRINTCRLF
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
	movlw	1
	movwf	COMPORT
	call	HSERPRINT306
	movlw	2
	movwf	HSERPRINTCRLFCOUNT
	movlw	1
	movwf	COMPORT
	call	HSERPRINTCRLF
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
	movlw	1
	movwf	COMPORT
	call	HSERPRINT306
	movlw	1
	movwf	HSERPRINTCRLFCOUNT
	movlw	1
	movwf	COMPORT
	call	HSERPRINTCRLF
	clrf	XXX
;Legacy method
	clrf	ADR
	clrf	ADR_H
SysForLoop1
	incf	ADR,F
	btfsc	STATUS,Z
	incf	ADR_H,F
	incf	XXX,F
	movlw	160
	movwf	EEPDEV
	movf	ADR,W
	movwf	EEPADDR
	movf	ADR_H,W
	movwf	EEPADDR_H
	movf	XXX,W
	movwf	EEPROMVAL
	call	EEPROM_WR_BYTE1
	movf	XXX,W
	movwf	SERPRINTVAL
	movlw	1
	movwf	COMPORT
	call	HSERPRINT307
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
	movlw	1
	movwf	COMPORT
	call	HSERPRINT306
	movf	ADR,W
	movwf	SysWORDTempA
	movf	ADR_H,W
	movwf	SysWORDTempA_H
	movlw	10
	movwf	SysWORDTempB
	clrf	SysWORDTempB_H
	call	SYSCOMPLESSTHAN16
	btfsc	SysByteTempX,0
	goto	SysForLoop1
SysForLoopEnd1
	movlw	2
	movwf	HSERPRINTCRLFCOUNT
	movlw	1
	movwf	COMPORT
	call	HSERPRINTCRLF
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
	movlw	1
	movwf	COMPORT
	call	HSERPRINT306
	movlw	1
	movwf	HSERPRINTCRLFCOUNT
	movlw	1
	movwf	COMPORT
	call	HSERPRINTCRLF
	clrf	XXX
;Legacy method
	clrf	ADR
	clrf	ADR_H
SysForLoop2
	incf	ADR,F
	btfsc	STATUS,Z
	incf	ADR_H,F
	incf	XXX,F
	movlw	160
	movwf	EEPDEV
	movf	ADR,W
	movwf	EEPADDR
	movf	ADR_H,W
	movwf	EEPADDR_H
	call	EEPROM_RD_BYTE3
	movlw	low(DATARRAY)
	addwf	XXX,W
	movwf	FSR
	bankisel	DATARRAY
	movf	EEPROMVAL,W
	movwf	INDF
	movlw	low(DATARRAY)
	addwf	XXX,W
	movwf	FSR
	bankisel	DATARRAY
	movf	INDF,W
	movwf	SERPRINTVAL
	movlw	1
	movwf	COMPORT
	call	HSERPRINT307
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
	movlw	1
	movwf	COMPORT
	call	HSERPRINT306
	movf	ADR,W
	movwf	SysWORDTempA
	movf	ADR_H,W
	movwf	SysWORDTempA_H
	movlw	10
	movwf	SysWORDTempB
	clrf	SysWORDTempB_H
	call	SYSCOMPLESSTHAN16
	btfsc	SysByteTempX,0
	goto	SysForLoop2
SysForLoopEnd2
	movlw	1
	movwf	HSERPRINTCRLFCOUNT
	movlw	1
	movwf	COMPORT
	call	HSERPRINTCRLF
SysDoLoop_S1
	movlw	232
	movwf	SysWaitTempMS
	movlw	3
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

EEPROM_RD_BYTE3
;asm showdebug  `eeprom_rd_byte - Uses a WORD for eepAddr.  Intended for larger memory eeproms that require ADDRESS_H and ADDRESS (two bytes, therefore passed as a word ) to set addreess`
SysDoLoop_S3
	call	HI2CRESTART
	movlw	160
	movwf	I2CBYTE
	call	HI2CSEND
	movf	hi2cackpollstate,F
	btfss	STATUS,Z
	goto	SysDoLoop_S3
SysDoLoop_E3
	movf	EEPADDR_H,W
	movwf	I2CBYTE
	call	HI2CSEND
	movf	EEPADDR,W
	movwf	I2CBYTE
	call	HI2CSEND
	call	HI2CRESTART
	movlw	161
	movwf	I2CBYTE
	call	HI2CSEND
	clrf	HI2CGETACK
	call	HI2CRECEIVE
	movf	I2CBYTE,W
	movwf	EEPROMVAL
	goto	HI2CSTOP

;********************************************************************************

EEPROM_WR_BYTE1
;asm showdebug  `eeprom_wr_byte - Uses a WORD for eepAddr.  Intended for larger memory eeproms that require ADDRESS_H and ADDRESS (two bytes, therefore passed as a word ) to set addreess`
SysDoLoop_S2
	call	HI2CRESTART
	movlw	160
	movwf	I2CBYTE
	call	HI2CSEND
	movf	hi2cackpollstate,F
	btfss	STATUS,Z
	goto	SysDoLoop_S2
SysDoLoop_E2
	movf	EEPADDR_H,W
	movwf	I2CBYTE
	call	HI2CSEND
	movf	EEPADDR,W
	movwf	I2CBYTE
	call	HI2CSEND
	movf	EEPROMVAL,W
	movwf	I2CBYTE
	call	HI2CSEND
	goto	HI2CSTOP

;********************************************************************************

HI2CINIT
;asm showdebug  This method sets the variable `HI2CCurrentMode`, and, if required calls the method `SI2CInit` to set up new MSSP modules - aka K42s family chips
	clrf	HI2CCURRENTMODE
	return

;********************************************************************************

HI2CMODE
;asm showdebug  This method sets the variable `HI2CCurrentMode`, and, if required, sets the SSPCON1.bits
	banksel	SSPSTAT
	bsf	SSPSTAT,SMP
	banksel	SSPCON
	bsf	SSPCON,CKP
	bcf	SSPCON,WCOL
	movlw	12
	subwf	HI2CCURRENTMODE,W
	btfss	STATUS, Z
	goto	ENDIF7
	bsf	SSPCON,SSPM3
	bcf	SSPCON,SSPM2
	bcf	SSPCON,SSPM1
	bcf	SSPCON,SSPM0
	movlw	49
	banksel	SSPADD
	movwf	SSPADD
ENDIF7
	banksel	HI2CCURRENTMODE
	movf	HI2CCURRENTMODE,F
	btfss	STATUS, Z
	goto	ENDIF8
	bcf	SSPCON,SSPM3
	bsf	SSPCON,SSPM2
	bsf	SSPCON,SSPM1
	bcf	SSPCON,SSPM0
ENDIF8
	movlw	3
	subwf	HI2CCURRENTMODE,W
	btfss	STATUS, Z
	goto	ENDIF9
	bcf	SSPCON,SSPM3
	bsf	SSPCON,SSPM2
	bsf	SSPCON,SSPM1
	bsf	SSPCON,SSPM0
ENDIF9
	bsf	SSPCON,SSPEN
	return

;********************************************************************************

HI2CRECEIVE
	movf	HI2CCURRENTMODE,W
	sublw	10
	btfsc	STATUS, C
	goto	ELSE16_1
	btfss	HI2CGETACK,0
	goto	ELSE18_1
	banksel	SSPCON2
	bcf	SSPCON2,ACKDT
	goto	ENDIF18
ELSE18_1
	banksel	SSPCON2
	bsf	SSPCON2,ACKDT
ENDIF18
	bsf	SSPCON2,RCEN
	goto	ENDIF16
ELSE16_1
	banksel	SSPSTAT
	bsf	SSPSTAT,R_NOT_W
ENDIF16
	banksel	SSPCON
	bcf	SSPCON,WCOL
	bcf	SSPCON,SSPOV
SysWaitLoop3
	clrf	SysByteTempX
	banksel	SSPSTAT
	btfsc	SSPSTAT,BF
	comf	SysByteTempX,F
	movf	SysByteTempX,W
	banksel	SYSTEMP1
	movwf	SysTemp1
	clrf	SysByteTempX
	btfsc	PIR1,SSPIF
	comf	SysByteTempX,F
	movf	SysTemp1,W
	andwf	SysByteTempX,W
	movwf	SysTemp2
	btfss	SysTemp2,0
	goto	SysWaitLoop3
	movf	SSPBUF,W
	movwf	I2CBYTE
	bcf	PIR1,SSPIF
	banksel	SSPCON2
	bsf	SSPCON2,ACKEN
	bcf	SSPSTAT,BF
	banksel	STATUS
	call	HI2CWAITMSSP
	movf	HI2CCURRENTMODE,W
	sublw	10
	btfsc	STATUS, C
	goto	ELSE17_1
	banksel	SSPCON2
	bcf	SSPCON2,RCEN
	goto	ENDIF17
ELSE17_1
	banksel	SSPSTAT
	bcf	SSPSTAT,R_NOT_W
ENDIF17
	banksel	STATUS
	return

;********************************************************************************

HI2CRESTART
;asm showdebug  This method sets the registers and register bits to generate the I2C  RESTART signal
	movf	HI2CCURRENTMODE,W
	sublw	10
	btfsc	STATUS, C
	goto	ENDIF10
	banksel	SSPCON2
	bsf	SSPCON2,RSEN
	banksel	STATUS
	call	HI2CWAITMSSP
ENDIF10
	return

;********************************************************************************

HI2CSEND
;asm showdebug  This method sets the registers and register bits to send I2C data
RETRYHI2CSEND
	bcf	SSPCON,WCOL
	movf	I2CBYTE,W
	movwf	SSPBUF
	call	HI2CWAITMSSP
	banksel	SSPCON2
	btfss	SSPCON2,ACKSTAT
	goto	ELSE12_1
	movlw	255
	banksel	HI2CACKPOLLSTATE
	movwf	HI2CACKPOLLSTATE
	goto	ENDIF12
ELSE12_1
	banksel	HI2CACKPOLLSTATE
	clrf	HI2CACKPOLLSTATE
ENDIF12
	btfss	SSPCON,WCOL
	goto	ENDIF13
	movf	HI2CCURRENTMODE,W
	sublw	10
	btfsc	STATUS, C
	goto	RETRYHI2CSEND
ENDIF13
	movf	HI2CCURRENTMODE,W
	sublw	10
	btfsc	STATUS, C
	bsf	SSPCON,CKP
	return

;********************************************************************************

HI2CSTOP
	movf	HI2CCURRENTMODE,W
	sublw	10
	btfsc	STATUS, C
	goto	ELSE11_1
SysWaitLoop1
	banksel	SSPSTAT
	btfsc	SSPSTAT,R_NOT_W
	goto	SysWaitLoop1
	bsf	SSPCON2,PEN
	banksel	STATUS
	call	HI2CWAITMSSP
	goto	ENDIF11
ELSE11_1
SysWaitLoop2
	banksel	SSPSTAT
	btfss	SSPSTAT,P
	goto	SysWaitLoop2
ENDIF11
	banksel	STATUS
	return

;********************************************************************************

HI2CWAITMSSP
	clrf	HI2CWAITMSSPTIMEOUT
HI2CWAITMSSPWAIT
	incf	HI2CWAITMSSPTIMEOUT,F
	movlw	255
	subwf	HI2CWAITMSSPTIMEOUT,W
	btfsc	STATUS, C
	goto	ENDIF19
	btfss	PIR1,SSPIF
	goto	HI2CWAITMSSPWAIT
	bcf	PIR1,SSPIF
	return
	btfss	PIR1,SSPIF
	goto	HI2CWAITMSSPWAIT
	bcf	PIR1,SSPIF
	return
ENDIF19
	return

;********************************************************************************

HSERPRINT306
	movf	SysPRINTDATAHandler,W
	movwf	FSR
	bcf	STATUS, IRP
	btfsc	SysPRINTDATAHandler_H,0
	bsf	STATUS, IRP
	movf	INDF,W
	movwf	PRINTLEN
	movf	PRINTLEN,F
	btfsc	STATUS, Z
	goto	ENDIF3
	movlw	1
	movwf	SYSPRINTTEMP
SysForLoop3
	movf	SYSPRINTTEMP,W
	addwf	SysPRINTDATAHandler,W
	movwf	FSR
	bcf	STATUS, IRP
	btfsc	SysPRINTDATAHandler_H,0
	bsf	STATUS, IRP
	movf	INDF,W
	movwf	SERDATA
	call	HSERSEND296
;#4p Positive value Step Handler in For-next statement
	movf	SYSPRINTTEMP,W
	subwf	PRINTLEN,W
	movwf	SysTemp1
	movwf	SysBYTETempA
	clrf	SysBYTETempB
	call	SYSCOMPEQUAL
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF4
;Set LoopVar to LoopVar + StepValue where StepValue is a positive value
	incf	SYSPRINTTEMP,F
	goto	SysForLoop3
;END IF
ENDIF4
SysForLoopEnd3
ENDIF3
	return

;********************************************************************************

HSERPRINT307
	clrf	OUTVALUETEMP
	movlw	100
	subwf	SERPRINTVAL,W
	btfss	STATUS, C
	goto	ENDIF5
	movf	SERPRINTVAL,W
	movwf	SysBYTETempA
	movlw	100
	movwf	SysBYTETempB
	call	SYSDIVSUB
	movf	SysBYTETempA,W
	movwf	OUTVALUETEMP
	movf	SYSCALCTEMPX,W
	movwf	SERPRINTVAL
	movlw	48
	addwf	OUTVALUETEMP,W
	movwf	SERDATA
	call	HSERSEND296
ENDIF5
	movf	OUTVALUETEMP,W
	movwf	SysBYTETempB
	clrf	SysBYTETempA
	call	SYSCOMPLESSTHAN
	movf	SysByteTempX,W
	movwf	SysTemp1
	movf	SERPRINTVAL,W
	movwf	SysBYTETempA
	movlw	10
	movwf	SysBYTETempB
	call	SYSCOMPLESSTHAN
	comf	SysByteTempX,F
	movf	SysTemp1,W
	iorwf	SysByteTempX,W
	movwf	SysTemp2
	btfss	SysTemp2,0
	goto	ENDIF6
	movf	SERPRINTVAL,W
	movwf	SysBYTETempA
	movlw	10
	movwf	SysBYTETempB
	call	SYSDIVSUB
	movf	SysBYTETempA,W
	movwf	OUTVALUETEMP
	movf	SYSCALCTEMPX,W
	movwf	SERPRINTVAL
	movlw	48
	addwf	OUTVALUETEMP,W
	movwf	SERDATA
	call	HSERSEND296
ENDIF6
	movlw	48
	addwf	SERPRINTVAL,W
	movwf	SERDATA
	goto	HSERSEND296

;********************************************************************************

HSERPRINTCRLF
	movf	HSERPRINTCRLFCOUNT,W
	movwf	SysRepeatTemp1
	btfsc	STATUS,Z
	goto	SysRepeatLoopEnd1
SysRepeatLoop1
	movlw	13
	movwf	SERDATA
	call	HSERSEND296
	movlw	10
	movwf	SERDATA
	call	HSERSEND296
	decfsz	SysRepeatTemp1,F
	goto	SysRepeatLoop1
SysRepeatLoopEnd1
	return

;********************************************************************************

HSERSEND296
	decf	COMPORT,W
	btfss	STATUS, Z
	goto	ENDIF22
SysWaitLoop4
	btfss	PIR1,TXIF
	goto	SysWaitLoop4
SysWaitLoop5
	banksel	TXSTA
	btfss	TXSTA,TRMT
	goto	SysWaitLoop5
	banksel	SERDATA
	movf	SERDATA,W
	movwf	TXREG
	movlw	1
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
ENDIF22
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

INITUSART
;asm showdebug Values_calculated_in_the_script
;asm showdebug _SPBRGH_TEMP=_ SPBRGH_TEMP
;asm showdebug _SPBRGL_TEMP=_ SPBRGL_TEMP
;asm showdebug _BRG16_TEMP=_ BRG16_TEMP
;asm showdebug _BRGH_TEMP=_ BRGH_TEMP
	movlw	129
	banksel	SPBRG
	movwf	SPBRG
	bsf	TXSTA,BRGH
	bcf	TXSTA,SYNC
	bsf	TXSTA,TXEN
	banksel	RCSTA
	bsf	RCSTA,SPEN
	bsf	RCSTA,CREN
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

SYSCOMPLESSTHAN
	clrf	SYSBYTETEMPX
	bsf	STATUS, C
	movf	SYSBYTETEMPB, W
	subwf	SYSBYTETEMPA, W
	btfss	STATUS, C
	comf	SYSBYTETEMPX,F
	return

;********************************************************************************

SYSCOMPLESSTHAN16
	clrf	SYSBYTETEMPX
	movf	SYSWORDTEMPA_H,W
	subwf	SYSWORDTEMPB_H,W
	btfss	STATUS,C
	return
	movf	SYSWORDTEMPB_H,W
	subwf	SYSWORDTEMPA_H,W
	btfss	STATUS,C
	goto	SCLT16TRUE
	movf	SYSWORDTEMPB,W
	subwf	SYSWORDTEMPA,W
	btfsc	STATUS,C
	return
SCLT16TRUE
	comf	SYSBYTETEMPX,F
	return

;********************************************************************************

SYSDIVSUB
	movf	SYSBYTETEMPB, F
	btfsc	STATUS, Z
	return
	clrf	SYSBYTETEMPX
	movlw	8
	movwf	SYSDIVLOOP
SYSDIV8START
	bcf	STATUS, C
	rlf	SYSBYTETEMPA, F
	rlf	SYSBYTETEMPX, F
	movf	SYSBYTETEMPB, W
	subwf	SYSBYTETEMPX, F
	bsf	SYSBYTETEMPA, 0
	btfsc	STATUS, C
	goto	DIV8NOTNEG
	bcf	SYSBYTETEMPA, 0
	movf	SYSBYTETEMPB, W
	addwf	SYSBYTETEMPX, F
DIV8NOTNEG
	decfsz	SYSDIVLOOP, F
	goto	SYSDIV8START
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
	retlw	11
	retlw	72	;H
	retlw	87	;W
	retlw	32	; 
	retlw	73	;I
	retlw	50	;2
	retlw	67	;C
	retlw	32	; 
	retlw	77	;M
	retlw	111	;o
	retlw	100	;d
	retlw	101	;e


StringTable2
	retlw	17
	retlw	32	; 
	retlw	32	; 
	retlw	87	;W
	retlw	114	;r
	retlw	105	;i
	retlw	116	;t
	retlw	101	;e
	retlw	32	; 
	retlw	116	;t
	retlw	111	;o
	retlw	32	; 
	retlw	69	;E
	retlw	69	;E
	retlw	80	;P
	retlw	82	;R
	retlw	79	;O
	retlw	77	;M


StringTable3
	retlw	1
	retlw	32	; 


StringTable4
	retlw	18
	retlw	32	; 
	retlw	32	; 
	retlw	82	;R
	retlw	101	;e
	retlw	97	;a
	retlw	100	;d
	retlw	32	; 
	retlw	102	;f
	retlw	114	;r
	retlw	111	;o
	retlw	109	;m
	retlw	32	; 
	retlw	69	;E
	retlw	69	;E
	retlw	80	;P
	retlw	82	;R
	retlw	79	;O
	retlw	77	;M


;********************************************************************************

;Start of program memory page 1
	ORG	2048
;Start of program memory page 2
	ORG	4096
;Start of program memory page 3
	ORG	6144

 END
