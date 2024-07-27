;Program compiled by Great Cow BASIC (1.00.00 2022-12-01 (Linux 64 bit) : Build 1199) for Microchip MPASM/MPLAB-X Assembler
;Need help? 
;  See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;  Check the documentation and Help at http://gcbasic.sourceforge.net/help/,
;or, email us:
;   w_cholmondeley at users dot sourceforge dot net
;   evanvennn at users dot sourceforge dot net

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=16F876, r=DEC
#include <P16F876.inc>
 __CONFIG _WRT_OFF & _CPD_OFF & _LVP_OFF & _CP_OFF & _WDTE_OFF & _FOSC_HS

;********************************************************************************

;Set aside memory locations for variables
DATA                             EQU      32          ; 0x20
DELAYTEMP                        EQU     112          ; 0x70
I2C_WRITE                        EQU      33          ; 0x21
INDEX                            EQU      34          ; 0x22
SYSWAITTEMPUS                    EQU     117          ; 0x75
SYSWAITTEMPUS_H                  EQU     118          ; 0x76

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
	banksel	TRISC
	bsf	TRISC,4
	bsf	TRISC,3
	movlw	40
	banksel	SSPCON
	movwf	SSPCON
	banksel	SSPCON2
	clrf	SSPCON2
	movlw	19
	movwf	SSPADD
SysDoLoop_S1
	banksel	STATUS
	call	I2C_BEGIN
	movlw	78
	movwf	DATA
	call	FN_I2C_WRITE
	movf	INDEX,W
	movwf	DATA
	call	FN_I2C_WRITE
	movlw	165
	movwf	DATA
	call	FN_I2C_WRITE
	movlw	65
	movwf	DATA
	call	FN_I2C_WRITE
	incf	INDEX,F
	call	I2C_END
	goto	SysDoLoop_S1
SysDoLoop_E1
	goto	BASPROGRAMEND
BASPROGRAMEND
	sleep
	goto	BASPROGRAMEND

;********************************************************************************

I2C_BEGIN
	call	I2C_HOLD
	banksel	SSPCON2
	bsf	SSPCON2,SEN
	banksel	STATUS
	return

;********************************************************************************

I2C_END
	call	I2C_HOLD
	banksel	SSPCON2
	bsf	SSPCON2,PEN
	banksel	STATUS
	return

;********************************************************************************

I2C_HOLD
	movlw	66
	movwf	DELAYTEMP
DelayUS1
	decfsz	DELAYTEMP,F
	goto	DelayUS1
	nop
	return

;********************************************************************************

FN_I2C_WRITE
	call	I2C_HOLD
	movf	DATA,W
	movwf	SSPBUF
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

;Start of program memory page 1
	ORG	2048
;Start of program memory page 2
	ORG	4096
;Start of program memory page 3
	ORG	6144

 END
