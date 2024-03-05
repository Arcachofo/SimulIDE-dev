;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 4.1.0 #12072 (Linux)
;--------------------------------------------------------
; PIC port for the 14-bit core
;--------------------------------------------------------
;	.file	"/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"
	list	p=12f675
	radix dec
	include "p12f675.inc"
;--------------------------------------------------------
; config word(s)
;--------------------------------------------------------
	__config 0x3e07
;--------------------------------------------------------
; external declarations
;--------------------------------------------------------
	extern	_ANSEL
	extern	_TRISIO
	extern	_CMCON
	extern	_GPIO
	extern	_GPIObits
	extern	__sdcc_gsinit_startup
;--------------------------------------------------------
; global declarations
;--------------------------------------------------------
	global	_main
	global	_delay

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

sharebank udata_ovr 0x0020
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
UDL_test_12f675_0	udata
r0x1001	res	1
r0x1000	res	1
r0x1002	res	1
r0x1003	res	1
r0x1004	res	1
r0x1005	res	1
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
; code
;--------------------------------------------------------
code_test_12f675	code
;***
;  pBlock Stats: dbName = M
;***
;has an exit
;functions called:
;   _delay
;   _delay
;   _delay
;   _delay
;1 compiler assigned register :
;   STK00
;; Starting pCode block
S_test_12f675__main	code
_main:
; 2 exit points
;	.line	26; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	TRISIO = 0x08; //Poner todos los pines como salidas y activar entrada
	MOVLW	0x08
	BANKSEL	_TRISIO
	MOVWF	_TRISIO
;	.line	27; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	ANSEL  = 0x00; //desactivar convertidor A/D
	CLRF	_ANSEL
;	.line	28; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	CMCON  = 0x07; //Desabilitar comparadores
	MOVLW	0x07
	BANKSEL	_CMCON
	MOVWF	_CMCON
;	.line	29; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	GPIO   = 0x00; //Poner pines en bajo
	CLRF	_GPIO
_00142_DS_:
;	.line	33; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	if( GP3 ){
	BANKSEL	_GPIObits
	BTFSS	_GPIObits,3
	GOTO	_00139_DS_
;	.line	34; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	GPIO = 0x02;
	MOVLW	0x02
	MOVWF	_GPIO
;	.line	35; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	delay( 100 );
	MOVLW	0x64
	MOVWF	STK00
	MOVLW	0x00
	PAGESEL	_delay
	CALL	_delay
	PAGESEL	$
	GOTO	_00142_DS_
_00139_DS_:
;	.line	38; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	GPIO = 0x01;
	MOVLW	0x01
	BANKSEL	_GPIO
	MOVWF	_GPIO
;	.line	39; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	delay( 150 );
	MOVLW	0x96
	MOVWF	STK00
	MOVLW	0x00
	PAGESEL	_delay
	CALL	_delay
	PAGESEL	$
	GOTO	_00142_DS_
;	.line	42; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	}
	RETURN	
; exit point of _main

;***
;  pBlock Stats: dbName = C
;***
;has an exit
;7 compiler assigned registers:
;   r0x1000
;   STK00
;   r0x1001
;   r0x1002
;   r0x1003
;   r0x1004
;   r0x1005
;; Starting pCode block
S_test_12f675__delay	code
_delay:
; 2 exit points
;	.line	16; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	void delay( unsigned int tiempo )
	MOVWF	r0x1000
	MOVF	STK00,W
	MOVWF	r0x1001
;	.line	20; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	for( i=0; i<tiempo; i++ )
	CLRF	r0x1002
	CLRF	r0x1003
_00111_DS_:
	MOVF	r0x1000,W
	SUBWF	r0x1003,W
	BTFSS	STATUS,2
	GOTO	_00133_DS_
	MOVF	r0x1001,W
	SUBWF	r0x1002,W
_00133_DS_:
	BTFSC	STATUS,0
	GOTO	_00113_DS_
;;genSkipc:3307: created from rifx:0x7ffffffd7c20
;	.line	21; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	for( j=0; j<1275; j++ );
	MOVLW	0xfb
	MOVWF	r0x1004
	MOVLW	0x04
	MOVWF	r0x1005
_00109_DS_:
	MOVLW	0xff
	ADDWF	r0x1004,F
	BTFSS	STATUS,0
	DECF	r0x1005,F
	MOVF	r0x1005,W
	IORWF	r0x1004,W
	BTFSS	STATUS,2
	GOTO	_00109_DS_
;	.line	20; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	for( i=0; i<tiempo; i++ )
	INCF	r0x1002,F
	BTFSC	STATUS,2
	INCF	r0x1003,F
	GOTO	_00111_DS_
_00113_DS_:
;	.line	22; "/mnt/software/simulide/tests/pic/sdcc/test_12f675/sdcc_code/test_12f675.c"	}
	RETURN	
; exit point of _delay


;	code size estimation:
;	   53+    8 =    61 instructions (  138 byte)

	end
