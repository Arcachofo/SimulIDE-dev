;Program compiled by Great Cow BASIC (1.00.00 2022-12-01 (Linux 64 bit) : Build 1199) for Microchip MPASM/MPLAB-X Assembler
;Need help? 
;  See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;  Check the documentation and Help at http://gcbasic.sourceforge.net/help/,
;or, email us:
;   w_cholmondeley at users dot sourceforge dot net
;   evanvennn at users dot sourceforge dot net

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=16F883, r=DEC
#include <P16F883.inc>
 __CONFIG _CONFIG1, _LVP_OFF & _FCMEN_ON & _CPD_OFF & _CP_OFF & _MCLRE_OFF & _WDTE_OFF & _FOSC_HS
 __CONFIG _CONFIG2, _WRT_OFF

;********************************************************************************

;Set aside memory locations for variables
COMPORT                          EQU      32          ; 0x20
HSERRECEIVE                      EQU      33          ; 0x21
INCHAR                           EQU      34          ; 0x22
SERDATA                          EQU      35          ; 0x23

;********************************************************************************

;Alias variables
SYSHSERRECEIVEBYTE EQU 33

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

;Start of the main program
	banksel	TRISB
	clrf	TRISB
	bcf	TRISC,6
	bsf	TRISC,7
SysDoLoop_S1
	banksel	STATUS
	call	FN_HSERRECEIVE294
	movf	SYSHSERRECEIVEBYTE,W
	movwf	INCHAR
	movwf	PORTB
	goto	SysDoLoop_S1
SysDoLoop_E1
BASPROGRAMEND
	sleep
	goto	BASPROGRAMEND

;********************************************************************************

FN_HSERRECEIVE294
	movlw	1
	movwf	COMPORT
	call	HSERRECEIVE298
	movf	SERDATA,W
	movwf	HSERRECEIVE
	return

;********************************************************************************

HSERRECEIVE298
HSERRECEIVE1HANDLER
SysWaitLoop1
	btfss	PIR1,RCIF
	goto	SysWaitLoop1
	movf	RCREG,W
	movwf	SERDATA
	btfss	RCSTA,OERR
	goto	ENDIF1
	bcf	RCSTA,CREN
	bsf	RCSTA,CREN
ENDIF1
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

INITUSART
	movlw	2
	banksel	SPBRGH
	movwf	SPBRGH
	banksel	BAUDCTL
	bsf	BAUDCTL,BRG16
	banksel	TXSTA
	bsf	TXSTA,BRGH
	bcf	TXSTA,SYNC
	bsf	TXSTA,TXEN
	banksel	RCSTA
	bsf	RCSTA,SPEN
	bsf	RCSTA,CREN
	return

;********************************************************************************

;Start of program memory page 1
	ORG	2048

 END
