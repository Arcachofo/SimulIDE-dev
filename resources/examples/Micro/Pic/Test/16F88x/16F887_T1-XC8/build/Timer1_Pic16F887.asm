;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 4.1.0 #12072 (Linux)
;--------------------------------------------------------
; PIC port for the 14-bit core
;--------------------------------------------------------
;	.file	"/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"
	list	p=16f887
	radix dec
	include "p16f887.inc"
;--------------------------------------------------------
; config word(s)
;--------------------------------------------------------
	__config _CONFIG1, 0x3ff2
;--------------------------------------------------------
; external declarations
;--------------------------------------------------------
	extern	_ANSELH
	extern	_ANSEL
	extern	_TRISC
	extern	_TRISB
	extern	_TMR1H
	extern	_TMR1L
	extern	_PORTC
	extern	_PORTB
	extern	_CM2CON0bits
	extern	_CM1CON0bits
	extern	_PIE1bits
	extern	_T1CONbits
	extern	_PIR1bits
	extern	_INTCONbits
	extern	__sdcc_gsinit_startup
;--------------------------------------------------------
; global declarations
;--------------------------------------------------------
	global	_main
	global	_myisr

	global PSAVE
	global SSAVE
	global WSAVE
	global STK12
	global STK11
	global STK10
	global STK09
	global STK08
	global STK07
	global STK06
	global STK05
	global STK04
	global STK03
	global STK02
	global STK01
	global STK00

sharebank udata_ovr 0x0070
PSAVE	res 1
SSAVE	res 1
WSAVE	res 1
STK12	res 1
STK11	res 1
STK10	res 1
STK09	res 1
STK08	res 1
STK07	res 1
STK06	res 1
STK05	res 1
STK04	res 1
STK03	res 1
STK02	res 1
STK01	res 1
STK00	res 1

;--------------------------------------------------------
; global definitions
;--------------------------------------------------------
;--------------------------------------------------------
; absolute symbol definitions
;--------------------------------------------------------
;--------------------------------------------------------
; compiler-defined variables
;--------------------------------------------------------
UDL_Timer1_Pic16F887_0	udata
r0x1003	res	1
r0x1001	res	1
___sdcc_saved_fsr	res	1
;--------------------------------------------------------
; initialized data
;--------------------------------------------------------
;--------------------------------------------------------
; initialized absolute data
;--------------------------------------------------------
;--------------------------------------------------------
; overlayable items in internal ram 
;--------------------------------------------------------
;	udata_ovr
;--------------------------------------------------------
; reset vector 
;--------------------------------------------------------
STARTUP	code 0x0000
	nop
	pagesel __sdcc_gsinit_startup
	goto	__sdcc_gsinit_startup
;--------------------------------------------------------
; interrupt and initialization code
;--------------------------------------------------------
c_interrupt	code	0x0004
__sdcc_interrupt:
;***
;  pBlock Stats: dbName = I
;***
;2 compiler assigned registers:
;   r0x1003
;   r0x1004
;; Starting pCode block
_myisr:
; 0 exit points
;	.line	44; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	void myisr(void) __interrupt 0 {
	MOVWF	WSAVE
	SWAPF	STATUS,W
	CLRF	STATUS
	MOVWF	SSAVE
	MOVF	PCLATH,W
	CLRF	PCLATH
	MOVWF	PSAVE
	MOVF	FSR,W
	BANKSEL	___sdcc_saved_fsr
	MOVWF	___sdcc_saved_fsr
;	.line	46; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	if( TMR1IF == 1 )
	BANKSEL	r0x1003
	CLRF	r0x1003
	BANKSEL	_PIR1bits
	BTFSS	_PIR1bits,0
	GOTO	_00001_DS_
	BANKSEL	r0x1003
	INCF	r0x1003,F
_00001_DS_:
	BANKSEL	r0x1003
	MOVF	r0x1003,W
	XORLW	0x01
	BTFSS	STATUS,2
	GOTO	_00107_DS_
;	.line	48; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	PORTC ^= 0b00001100;
	BANKSEL	_PORTC
	MOVF	_PORTC,W
	BANKSEL	r0x1003
	MOVWF	r0x1003
;;1	CLRF	r0x1004
	MOVLW	0x0c
	XORWF	r0x1003,F
	MOVF	r0x1003,W
	BANKSEL	_PORTC
	MOVWF	_PORTC
;	.line	51; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	TMR1IF = 0;
	BCF	_PIR1bits,0
_00107_DS_:
;	.line	53; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	}
	BANKSEL	___sdcc_saved_fsr
	MOVF	___sdcc_saved_fsr,W
	BANKSEL	FSR
	MOVWF	FSR
	MOVF	PSAVE,W
	MOVWF	PCLATH
	CLRF	STATUS
	SWAPF	SSAVE,W
	MOVWF	STATUS
	SWAPF	WSAVE,F
	SWAPF	WSAVE,W
END_OF_INTERRUPT:
	RETFIE	

;--------------------------------------------------------
; code
;--------------------------------------------------------
code_Timer1_Pic16F887	code
;***
;  pBlock Stats: dbName = M
;***
;has an exit
;2 compiler assigned registers:
;   r0x1001
;   r0x1002
;; Starting pCode block
S_Timer1_Pic16F887__main	code
_main:
; 2 exit points
;	.line	64; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	ANSEL  = 0x00;
	BANKSEL	_ANSEL
	CLRF	_ANSEL
;	.line	65; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	ANSELH = 0x00;
	CLRF	_ANSELH
;	.line	66; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	C1ON   = 0;
	BANKSEL	_CM1CON0bits
	BCF	_CM1CON0bits,7
;	.line	67; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	C2ON   = 0;
	BCF	_CM2CON0bits,7
;	.line	68; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	TRISB  = 0x00;
	BANKSEL	_TRISB
	CLRF	_TRISB
;	.line	69; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	PORTB  = 0x00;
	BANKSEL	_PORTB
	CLRF	_PORTB
;	.line	70; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	TRISC  = 0x00;
	BANKSEL	_TRISC
	CLRF	_TRISC
;	.line	71; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	PORTC  = 0x00;
	BANKSEL	_PORTC
	CLRF	_PORTC
;	.line	74; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	TMR1IF = 0;
	BCF	_PIR1bits,0
;	.line	75; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	TMR1H = 0;
	CLRF	_TMR1H
;	.line	76; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	TMR1L = 0;
	CLRF	_TMR1L
;	.line	78; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	TMR1CS  = 0;
	BCF	_T1CONbits,1
;	.line	79; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	T1CKPS0 = 1;
	BSF	_T1CONbits,4
;	.line	80; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	T1CKPS1 = 1;
	BSF	_T1CONbits,5
;	.line	81; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	TMR1IE  = 1;
	BANKSEL	_PIE1bits
	BSF	_PIE1bits,0
;	.line	83; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	PEIE   = 1;
	BANKSEL	_INTCONbits
	BSF	_INTCONbits,6
;	.line	84; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	GIE    = 1;            // General Interrupt Enable
	BSF	_INTCONbits,7
;	.line	85; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	TMR1ON = 1;
	BSF	_T1CONbits,0
_00116_DS_:
;	.line	89; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	if(TMR1H == 0x00 && TMR1L == 0x00){
	BANKSEL	_TMR1H
	MOVF	_TMR1H,W
	BTFSS	STATUS,2
	GOTO	_00116_DS_
	MOVF	_TMR1L,W
	BTFSS	STATUS,2
	GOTO	_00116_DS_
;	.line	90; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	PORTC ^= 0b00000011;
	MOVF	_PORTC,W
	BANKSEL	r0x1001
	MOVWF	r0x1001
;;1	CLRF	r0x1002
	MOVLW	0x03
	XORWF	r0x1001,F
	MOVF	r0x1001,W
	BANKSEL	_PORTC
	MOVWF	_PORTC
	GOTO	_00116_DS_
;	.line	93; "/mnt/software/simulide/tests/pic/pic_timers/Timer1_Pic16F887/Timer1_Pic16F887.c"	}
	RETURN	
; exit point of _main


;	code size estimation:
;	   66+   21 =    87 instructions (  216 byte)

	end
