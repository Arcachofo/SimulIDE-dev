;Program compiled by Great Cow BASIC (0.98.<<>> 2021-<<>>-24 (Linux 64 bit) : Build 1005) for Microchip MPASM
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=16F877, r=DEC
#include <P16F877.inc>
 __CONFIG _WRT_OFF & _CPD_OFF & _LVP_OFF & _CP_OFF & _WDTE_OFF & _FOSC_HS

;********************************************************************************

;Set aside memory locations for variables
CHAR                             EQU 32
CHARCODE                         EQU 33
CHARCOL                          EQU 34
CHARCOLS                         EQU 36
CHARCOL_H                        EQU 35
CHARLOCX                         EQU 37
CHARLOCX_H                       EQU 38
CHARLOCY                         EQU 39
CHARLOCY_H                       EQU 40
CHARROW                          EQU 41
CHARROWS                         EQU 43
CHARROW_H                        EQU 42
COL                              EQU 44
CURRCHARCOL                      EQU 45
CURRCHARROW                      EQU 46
CURRCHARVAL                      EQU 47
CURRCOL                          EQU 48
CURRPAGE                         EQU 49
DELAYTEMP                        EQU 112
DELAYTEMP2                       EQU 113
DRAWLINE                         EQU 50
DRAWLINE_H                       EQU 51
GLCDBACKGROUND                   EQU 52
GLCDBACKGROUND_H                 EQU 53
GLCDBITNO                        EQU 54
GLCDCHANGE                       EQU 55
GLCDCOLOUR                       EQU 56
GLCDCOLOUR_H                     EQU 57
GLCDDATATEMP                     EQU 58
GLCDFNTDEFAULT                   EQU 59
GLCDFNTDEFAULTHEIGHT             EQU 60
GLCDFNTDEFAULTSIZE               EQU 61
GLCDFONTWIDTH                    EQU 62
GLCDFOREGROUND                   EQU 63
GLCDFOREGROUND_H                 EQU 64
GLCDPRINTLEN                     EQU 65
GLCDPRINTLOC                     EQU 66
GLCDPRINTLOC_H                   EQU 67
GLCDPRINT_STRING_COUNTER         EQU 68
GLCDREADBYTE_KS0108              EQU 69
GLCDTEMP                         EQU 70
GLCDTEMP_H                       EQU 71
GLCDX                            EQU 72
GLCDY                            EQU 73
GLCD_COUNT                       EQU 74
GLCD_YORDINATE                   EQU 75
GLCD_YORDINATE_H                 EQU 76
LCDBYTE                          EQU 77
LINECOLOUR                       EQU 78
LINECOLOUR_H                     EQU 79
LINEDIFFX                        EQU 80
LINEDIFFX_H                      EQU 81
LINEDIFFX_X2                     EQU 82
LINEDIFFX_X2_H                   EQU 83
LINEDIFFY                        EQU 84
LINEDIFFY_H                      EQU 85
LINEDIFFY_X2                     EQU 86
LINEDIFFY_X2_H                   EQU 87
LINEERR                          EQU 88
LINEERR_H                        EQU 89
LINESTEPX                        EQU 90
LINESTEPX_H                      EQU 91
LINESTEPY                        EQU 92
LINESTEPY_H                      EQU 93
LINEX1                           EQU 94
LINEX1_H                         EQU 95
LINEX2                           EQU 96
LINEX2_H                         EQU 97
LINEY1                           EQU 98
LINEY1_H                         EQU 99
LINEY2                           EQU 100
LINEY2_H                         EQU 101
PRINTLOCX                        EQU 102
PRINTLOCX_H                      EQU 103
PRINTLOCY                        EQU 104
PRINTLOCY_H                      EQU 105
ROW                              EQU 106
STR                              EQU 481
STRINGPOINTER                    EQU 107
SYSBITVAR0                       EQU 108
SYSBYTETEMPA                     EQU 117
SYSBYTETEMPB                     EQU 121
SYSBYTETEMPX                     EQU 112
SYSCALCTEMPA                     EQU 117
SYSCALCTEMPX                     EQU 112
SYSCALCTEMPX_H                   EQU 113
SYSCHARCOUNT                     EQU 109
SYSDIVLOOP                       EQU 116
SYSDIVMULTA                      EQU 119
SYSDIVMULTA_H                    EQU 120
SYSDIVMULTB                      EQU 123
SYSDIVMULTB_H                    EQU 124
SYSDIVMULTX                      EQU 114
SYSDIVMULTX_H                    EQU 115
SYSINTEGERTEMPA                  EQU 117
SYSINTEGERTEMPA_H                EQU 118
SYSINTEGERTEMPB                  EQU 121
SYSINTEGERTEMPB_H                EQU 122
SYSINTEGERTEMPX                  EQU 112
SYSINTEGERTEMPX_H                EQU 113
SYSLCDPRINTDATAHANDLER           EQU 110
SYSLCDPRINTDATAHANDLER_H         EQU 111
SYSREPEATTEMP1                   EQU 126
SYSSIGNBYTE                      EQU 125
SYSSTRDATA                       EQU 127
SYSSTRINGA                       EQU 119
SYSSTRINGA_H                     EQU 120
SYSSTRINGB                       EQU 114
SYSSTRINGB_H                     EQU 115
SYSSTRINGLENGTH                  EQU 118
SYSSTRINGPARAM1                  EQU 487
SYSTEMP1                         EQU 160
SYSTEMP1_H                       EQU 161
SYSTEMP2                         EQU 162
SYSTEMP2_H                       EQU 163
SYSVALTEMP                       EQU 164
SYSVALTEMP_H                     EQU 165
SYSWAITTEMPMS                    EQU 114
SYSWAITTEMPMS_H                  EQU 115
SYSWAITTEMPS                     EQU 116
SYSWAITTEMPUS                    EQU 117
SYSWAITTEMPUS_H                  EQU 118
SYSWORDTEMPA                     EQU 117
SYSWORDTEMPA_H                   EQU 118
SYSWORDTEMPB                     EQU 121
SYSWORDTEMPB_H                   EQU 122
SYSWORDTEMPX                     EQU 112
SYSWORDTEMPX_H                   EQU 113
XVAR                             EQU 166

;********************************************************************************

;Alias variables
SYSSTR_0 EQU 481

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
	call	INITGLCD_KS0108
	pagesel	$

;Start of the main program
START
	pagesel	GLCDCLS_KS0108
	call	GLCDCLS_KS0108
	pagesel	$
	clrf	PRINTLOCX
	clrf	PRINTLOCX_H
	movlw	10
	movwf	PRINTLOCY
	clrf	PRINTLOCY_H
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable1
	movwf	SysStringA
	movlw	high StringTable1
	movwf	SysStringA_H
	pagesel	SYSREADSTRING
	call	SYSREADSTRING
	pagesel	$
	movlw	low SYSSTRINGPARAM1
	movwf	SysLCDPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysLCDPRINTDATAHandler_H
	call	GLCDPRINT3
	movlw	1
	movwf	SysWaitTempS
	pagesel	Delay_S
	call	Delay_S
	pagesel	$
	clrf	PRINTLOCX
	clrf	PRINTLOCX_H
	movlw	10
	movwf	PRINTLOCY
	clrf	PRINTLOCY_H
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable2
	movwf	SysStringA
	movlw	high StringTable2
	movwf	SysStringA_H
	pagesel	SYSREADSTRING
	call	SYSREADSTRING
	pagesel	$
	movlw	low SYSSTRINGPARAM1
	movwf	SysLCDPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysLCDPRINTDATAHandler_H
	call	GLCDPRINT3
	movlw	18
	movwf	LINEX1
	clrf	LINEX1_H
	movlw	30
	movwf	LINEY1
	clrf	LINEY1_H
	movlw	28
	movwf	LINEX2
	clrf	LINEX2_H
	movlw	40
	movwf	LINEY2
	clrf	LINEY2_H
	movf	GLCDFOREGROUND,W
	movwf	LINECOLOUR
	movf	GLCDFOREGROUND_H,W
	movwf	LINECOLOUR_H
	call	BOX
;Legacy method
	movlw	14
	movwf	CHAR
SysForLoop1
	incf	CHAR,F
	movlw	17
	movwf	PRINTLOCX
	clrf	PRINTLOCX_H
	movlw	20
	movwf	PRINTLOCY
	clrf	PRINTLOCY_H
	movf	CHAR,W
	banksel	SYSVALTEMP
	movwf	SYSVALTEMP
	clrf	SYSVALTEMP_H
	banksel	STATUS
	call	FN_STR
	movlw	low STR
	movwf	SysLCDPRINTDATAHandler
	movlw	high STR
	movwf	SysLCDPRINTDATAHandler_H
	call	GLCDPRINT3
	movlw	20
	movwf	CHARLOCX
	clrf	CHARLOCX_H
	movlw	30
	movwf	CHARLOCY
	clrf	CHARLOCY_H
	movf	CHAR,W
	movwf	CHARCODE
	movf	GLCDFOREGROUND,W
	movwf	LINECOLOUR
	movf	GLCDFOREGROUND_H,W
	movwf	LINECOLOUR_H
	call	GLCDDRAWCHAR
	movlw	1
	movwf	SysWaitTempS
	pagesel	Delay_S
	call	Delay_S
	pagesel	$
	movlw	129
	subwf	CHAR,W
	btfss	STATUS, C
	goto	SysForLoop1
SysForLoopEnd1
	clrf	LINEX1
	clrf	LINEX1_H
	movlw	50
	movwf	LINEY1
	clrf	LINEY1_H
	movlw	127
	movwf	LINEX2
	clrf	LINEX2_H
	movlw	50
	movwf	LINEY2
	clrf	LINEY2_H
	movf	GLCDFOREGROUND,W
	movwf	LINECOLOUR
	movf	GLCDFOREGROUND_H,W
	movwf	LINECOLOUR_H
	call	_LINE
;Legacy method
	movlw	255
	banksel	XVAR
	movwf	XVAR
SysForLoop2
	incf	XVAR,F
	movf	XVAR,W
	banksel	GLCDX
	movwf	GLCDX
	movlw	63
	movwf	GLCDY
	movlw	1
	movwf	GLCDCOLOUR
	clrf	GLCDCOLOUR_H
	pagesel	PSET_KS0108
	call	PSET_KS0108
	pagesel	$
	movlw	80
	banksel	XVAR
	subwf	XVAR,W
	btfss	STATUS, C
	goto	SysForLoop2
SysForLoopEnd2
	movlw	10
	movwf	SysWaitTempS
	banksel	STATUS
	pagesel	Delay_S
	call	Delay_S
	pagesel	$
	goto	START
	goto	BASPROGRAMEND
BASPROGRAMEND
	sleep
	goto	BASPROGRAMEND

;********************************************************************************

BOX
	movf	LINEX1,W
	movwf	SysWORDTempB
	movf	LINEX1_H,W
	movwf	SysWORDTempB_H
	movf	LINEX2,W
	movwf	SysWORDTempA
	movf	LINEX2_H,W
	movwf	SysWORDTempA_H
	call	SYSCOMPLESSTHAN16
	btfss	SysByteTempX,0
	goto	ENDIF12
	movf	LINEX1,W
	movwf	GLCDTEMP
	movf	LINEX1_H,W
	movwf	GLCDTEMP_H
	movf	LINEX2,W
	movwf	LINEX1
	movf	LINEX2_H,W
	movwf	LINEX1_H
	movf	GLCDTEMP,W
	movwf	LINEX2
	movf	GLCDTEMP_H,W
	movwf	LINEX2_H
ENDIF12
	movf	LINEY1,W
	movwf	SysWORDTempB
	movf	LINEY1_H,W
	movwf	SysWORDTempB_H
	movf	LINEY2,W
	movwf	SysWORDTempA
	movf	LINEY2_H,W
	movwf	SysWORDTempA_H
	call	SYSCOMPLESSTHAN16
	btfss	SysByteTempX,0
	goto	ENDIF13
	movf	LINEY1,W
	movwf	GLCDTEMP
	movf	LINEY1_H,W
	movwf	GLCDTEMP_H
	movf	LINEY2,W
	movwf	LINEY1
	movf	LINEY2_H,W
	movwf	LINEY1_H
	movf	GLCDTEMP,W
	movwf	LINEY2
	movf	GLCDTEMP_H,W
	movwf	LINEY2_H
ENDIF13
	movf	LINEX1,W
	movwf	DRAWLINE
	movf	LINEX1_H,W
	movwf	DRAWLINE_H
SysForLoop8
	movf	DRAWLINE,W
	movwf	GLCDX
	movf	LINEY1,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	pagesel	PSET_KS0108
	call	PSET_KS0108
	pagesel	$
	movf	DRAWLINE,W
	movwf	GLCDX
	movf	LINEY2,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	pagesel	PSET_KS0108
	call	PSET_KS0108
	pagesel	$
;#4p Positive value Step Handler in For-next statement
	movf	DRAWLINE,W
	subwf	LINEX2,W
	banksel	SYSTEMP2
	movwf	SysTemp2
	banksel	LINEX2_H
	movf	LINEX2_H,W
	banksel	SYSTEMP2_H
	movwf	SysTemp2_H
	banksel	DRAWLINE_H
	movf	DRAWLINE_H,W
	btfss	STATUS,C
	incf	DRAWLINE_H,W
	btfsc	STATUS,Z
	goto	ENDIF15
	banksel	SYSTEMP2_H
	subwf	SysTemp2_H,F
ENDIF15
	banksel	SYSTEMP2
	movf	SysTemp2,W
	movwf	SysWORDTempA
	movf	SysTemp2_H,W
	movwf	SysWORDTempA_H
	clrf	SysWORDTempB
	clrf	SysWORDTempB_H
	banksel	STATUS
	pagesel	SYSCOMPEQUAL16
	call	SYSCOMPEQUAL16
	pagesel	$
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF14
;Set LoopVar to LoopVar + StepValue where StepValue is a positive value
	incf	DRAWLINE,F
	btfsc	STATUS,Z
	incf	DRAWLINE_H,F
	goto	SysForLoop8
;END IF
ENDIF14
SysForLoopEnd8
	movf	LINEY1,W
	movwf	DRAWLINE
	movf	LINEY1_H,W
	movwf	DRAWLINE_H
SysForLoop9
	movf	LINEX1,W
	movwf	GLCDX
	movf	DRAWLINE,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	pagesel	PSET_KS0108
	call	PSET_KS0108
	pagesel	$
	movf	LINEX2,W
	movwf	GLCDX
	movf	DRAWLINE,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	pagesel	PSET_KS0108
	call	PSET_KS0108
	pagesel	$
;#4p Positive value Step Handler in For-next statement
	movf	DRAWLINE,W
	subwf	LINEY2,W
	banksel	SYSTEMP2
	movwf	SysTemp2
	banksel	LINEY2_H
	movf	LINEY2_H,W
	banksel	SYSTEMP2_H
	movwf	SysTemp2_H
	banksel	DRAWLINE_H
	movf	DRAWLINE_H,W
	btfss	STATUS,C
	incf	DRAWLINE_H,W
	btfsc	STATUS,Z
	goto	ENDIF17
	banksel	SYSTEMP2_H
	subwf	SysTemp2_H,F
ENDIF17
	banksel	SYSTEMP2
	movf	SysTemp2,W
	movwf	SysWORDTempA
	movf	SysTemp2_H,W
	movwf	SysWORDTempA_H
	clrf	SysWORDTempB
	clrf	SysWORDTempB_H
	banksel	STATUS
	pagesel	SYSCOMPEQUAL16
	call	SYSCOMPEQUAL16
	pagesel	$
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF16
;Set LoopVar to LoopVar + StepValue where StepValue is a positive value
	incf	DRAWLINE,F
	btfsc	STATUS,Z
	incf	DRAWLINE_H,F
	goto	SysForLoop9
;END IF
ENDIF16
SysForLoopEnd9
	return

;********************************************************************************

GLCDCHARCOL3
	movlw	113
	subwf	SysStringA, W
	btfsc	STATUS, C
	retlw	0
	movf	SysStringA, W
	addlw	low TABLEGLCDCHARCOL3
	movwf	SysStringA
	movlw	high TABLEGLCDCHARCOL3
	btfsc	STATUS, C
	addlw	1
	movwf	PCLATH
	movf	SysStringA, W
	movwf	PCL
TABLEGLCDCHARCOL3
	retlw	112
	retlw	0
	retlw	16
	retlw	12
	retlw	10
	retlw	136
	retlw	34
	retlw	56
	retlw	32
	retlw	8
	retlw	32
	retlw	16
	retlw	16
	retlw	128
	retlw	128
	retlw	64
	retlw	4
	retlw	0
	retlw	0
	retlw	0
	retlw	40
	retlw	72
	retlw	70
	retlw	108
	retlw	0
	retlw	0
	retlw	0
	retlw	40
	retlw	16
	retlw	0
	retlw	16
	retlw	0
	retlw	64
	retlw	124
	retlw	0
	retlw	132
	retlw	130
	retlw	48
	retlw	78
	retlw	120
	retlw	6
	retlw	108
	retlw	12
	retlw	0
	retlw	0
	retlw	16
	retlw	40
	retlw	0
	retlw	4
	retlw	100
	retlw	248
	retlw	254
	retlw	124
	retlw	254
	retlw	254
	retlw	254
	retlw	124
	retlw	254
	retlw	0
	retlw	64
	retlw	254
	retlw	254
	retlw	254
	retlw	254
	retlw	124
	retlw	254
	retlw	124
	retlw	254
	retlw	76
	retlw	2
	retlw	126
	retlw	62
	retlw	126
	retlw	198
	retlw	14
	retlw	194
	retlw	0
	retlw	4
	retlw	0
	retlw	8
	retlw	128
	retlw	0
	retlw	64
	retlw	254
	retlw	112
	retlw	112
	retlw	112
	retlw	16
	retlw	16
	retlw	254
	retlw	0
	retlw	64
	retlw	254
	retlw	0
	retlw	248
	retlw	248
	retlw	112
	retlw	248
	retlw	16
	retlw	248
	retlw	144
	retlw	16
	retlw	120
	retlw	56
	retlw	120
	retlw	136
	retlw	24
	retlw	136
	retlw	0
	retlw	0
	retlw	0
	retlw	32
	retlw	120

;********************************************************************************

GLCDCHARCOL4
	movlw	113
	subwf	SysStringA, W
	btfsc	STATUS, C
	retlw	0
	movf	SysStringA, W
	addlw	low TABLEGLCDCHARCOL4
	movwf	SysStringA
	movlw	high TABLEGLCDCHARCOL4
	btfsc	STATUS, C
	addlw	1
	movwf	PCLATH
	movf	SysStringA, W
	movwf	PCL
TABLEGLCDCHARCOL4
	retlw	112
	retlw	254
	retlw	56
	retlw	10
	retlw	6
	retlw	204
	retlw	102
	retlw	124
	retlw	112
	retlw	4
	retlw	64
	retlw	16
	retlw	56
	retlw	136
	retlw	162
	retlw	112
	retlw	28
	retlw	0
	retlw	0
	retlw	14
	retlw	254
	retlw	84
	retlw	38
	retlw	146
	retlw	10
	retlw	56
	retlw	130
	retlw	16
	retlw	16
	retlw	160
	retlw	16
	retlw	192
	retlw	32
	retlw	162
	retlw	132
	retlw	194
	retlw	130
	retlw	40
	retlw	138
	retlw	148
	retlw	2
	retlw	146
	retlw	146
	retlw	108
	retlw	172
	retlw	40
	retlw	40
	retlw	130
	retlw	2
	retlw	146
	retlw	36
	retlw	146
	retlw	130
	retlw	130
	retlw	146
	retlw	18
	retlw	130
	retlw	16
	retlw	130
	retlw	128
	retlw	16
	retlw	128
	retlw	4
	retlw	8
	retlw	130
	retlw	18
	retlw	130
	retlw	18
	retlw	146
	retlw	2
	retlw	128
	retlw	64
	retlw	128
	retlw	40
	retlw	16
	retlw	162
	retlw	254
	retlw	8
	retlw	130
	retlw	4
	retlw	128
	retlw	2
	retlw	168
	retlw	144
	retlw	136
	retlw	136
	retlw	168
	retlw	252
	retlw	168
	retlw	16
	retlw	144
	retlw	128
	retlw	32
	retlw	130
	retlw	8
	retlw	16
	retlw	136
	retlw	40
	retlw	40
	retlw	16
	retlw	168
	retlw	124
	retlw	128
	retlw	64
	retlw	128
	retlw	80
	retlw	160
	retlw	200
	retlw	16
	retlw	0
	retlw	130
	retlw	16
	retlw	68

;********************************************************************************

GLCDCHARCOL5
	movlw	113
	subwf	SysStringA, W
	btfsc	STATUS, C
	retlw	0
	movf	SysStringA, W
	addlw	low TABLEGLCDCHARCOL5
	movwf	SysStringA
	movlw	high TABLEGLCDCHARCOL5
	btfsc	STATUS, C
	addlw	1
	movwf	PCLATH
	movf	SysStringA, W
	movwf	PCL
TABLEGLCDCHARCOL5
	retlw	112
	retlw	124
	retlw	124
	retlw	0
	retlw	0
	retlw	238
	retlw	238
	retlw	124
	retlw	168
	retlw	254
	retlw	254
	retlw	84
	retlw	84
	retlw	148
	retlw	148
	retlw	124
	retlw	124
	retlw	0
	retlw	158
	retlw	0
	retlw	40
	retlw	254
	retlw	16
	retlw	170
	retlw	6
	retlw	68
	retlw	68
	retlw	124
	retlw	124
	retlw	96
	retlw	16
	retlw	192
	retlw	16
	retlw	146
	retlw	254
	retlw	162
	retlw	138
	retlw	36
	retlw	138
	retlw	146
	retlw	226
	retlw	146
	retlw	146
	retlw	108
	retlw	108
	retlw	68
	retlw	40
	retlw	68
	retlw	162
	retlw	242
	retlw	34
	retlw	146
	retlw	130
	retlw	130
	retlw	146
	retlw	18
	retlw	146
	retlw	16
	retlw	254
	retlw	130
	retlw	40
	retlw	128
	retlw	24
	retlw	16
	retlw	130
	retlw	18
	retlw	162
	retlw	50
	retlw	146
	retlw	254
	retlw	128
	retlw	128
	retlw	112
	retlw	16
	retlw	224
	retlw	146
	retlw	130
	retlw	16
	retlw	130
	retlw	2
	retlw	128
	retlw	4
	retlw	168
	retlw	136
	retlw	136
	retlw	136
	retlw	168
	retlw	18
	retlw	168
	retlw	8
	retlw	250
	retlw	136
	retlw	80
	retlw	254
	retlw	240
	retlw	8
	retlw	136
	retlw	40
	retlw	40
	retlw	8
	retlw	168
	retlw	144
	retlw	128
	retlw	128
	retlw	96
	retlw	32
	retlw	160
	retlw	168
	retlw	108
	retlw	254
	retlw	108
	retlw	16
	retlw	66

;********************************************************************************

GLCDCHARCOL6
	movlw	113
	subwf	SysStringA, W
	btfsc	STATUS, C
	retlw	0
	movf	SysStringA, W
	addlw	low TABLEGLCDCHARCOL6
	movwf	SysStringA
	movlw	high TABLEGLCDCHARCOL6
	btfsc	STATUS, C
	addlw	1
	movwf	PCLATH
	movf	SysStringA, W
	movwf	PCL
TABLEGLCDCHARCOL6
	retlw	112
	retlw	56
	retlw	254
	retlw	12
	retlw	10
	retlw	204
	retlw	102
	retlw	124
	retlw	32
	retlw	4
	retlw	64
	retlw	56
	retlw	16
	retlw	162
	retlw	136
	retlw	112
	retlw	28
	retlw	0
	retlw	0
	retlw	14
	retlw	254
	retlw	84
	retlw	200
	retlw	68
	retlw	0
	retlw	130
	retlw	56
	retlw	16
	retlw	16
	retlw	0
	retlw	16
	retlw	0
	retlw	8
	retlw	138
	retlw	128
	retlw	146
	retlw	150
	retlw	254
	retlw	138
	retlw	146
	retlw	18
	retlw	146
	retlw	82
	retlw	0
	retlw	0
	retlw	130
	retlw	40
	retlw	40
	retlw	18
	retlw	130
	retlw	36
	retlw	146
	retlw	130
	retlw	68
	retlw	146
	retlw	18
	retlw	146
	retlw	16
	retlw	130
	retlw	126
	retlw	68
	retlw	128
	retlw	4
	retlw	32
	retlw	130
	retlw	18
	retlw	66
	retlw	82
	retlw	146
	retlw	2
	retlw	128
	retlw	64
	retlw	128
	retlw	40
	retlw	16
	retlw	138
	retlw	130
	retlw	32
	retlw	254
	retlw	4
	retlw	128
	retlw	8
	retlw	168
	retlw	136
	retlw	136
	retlw	144
	retlw	168
	retlw	2
	retlw	168
	retlw	8
	retlw	128
	retlw	122
	retlw	136
	retlw	128
	retlw	8
	retlw	8
	retlw	136
	retlw	40
	retlw	48
	retlw	8
	retlw	168
	retlw	128
	retlw	64
	retlw	64
	retlw	128
	retlw	80
	retlw	160
	retlw	152
	retlw	130
	retlw	0
	retlw	16
	retlw	32
	retlw	68

;********************************************************************************

GLCDDRAWCHAR
	movf	LINECOLOUR,W
	movwf	SysWORDTempA
	movf	LINECOLOUR_H,W
	movwf	SysWORDTempA_H
	movf	GLCDFOREGROUND,W
	movwf	SysWORDTempB
	movf	GLCDFOREGROUND_H,W
	movwf	SysWORDTempB_H
	pagesel	SYSCOMPEQUAL16
	call	SYSCOMPEQUAL16
	pagesel	$
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF5
	movlw	1
	movwf	GLCDBACKGROUND
	clrf	GLCDBACKGROUND_H
	clrf	GLCDFOREGROUND
	clrf	GLCDFOREGROUND_H
ENDIF5
	movlw	15
	subwf	CHARCODE,F
	clrf	CHARCOL
	clrf	CHARCOL_H
	movf	CHARCODE,W
	movwf	SysBYTETempA
	movlw	178
	movwf	SysBYTETempB
	pagesel	SYSCOMPLESSTHAN
	call	SYSCOMPLESSTHAN
	pagesel	$
	comf	SysByteTempX,F
	movf	SysByteTempX,W
	banksel	SYSTEMP1
	movwf	SysTemp1
	banksel	CHARCODE
	movf	CHARCODE,W
	movwf	SysBYTETempB
	movlw	202
	movwf	SysBYTETempA
	pagesel	SYSCOMPLESSTHAN
	call	SYSCOMPLESSTHAN
	pagesel	$
	comf	SysByteTempX,F
	banksel	SYSTEMP1
	movf	SysTemp1,W
	andwf	SysByteTempX,W
	movwf	SysTemp2
	btfss	SysTemp2,0
	goto	ENDIF6
	movlw	1
	banksel	CHARLOCY
	subwf	CHARLOCY,F
	movlw	0
	btfss	STATUS,C
	movlw	0 + 1
	subwf	CHARLOCY_H,F
ENDIF6
;Legacy method
	banksel	CURRCHARCOL
	clrf	CURRCHARCOL
SysForLoop4
	incf	CURRCHARCOL,F
SysSelect1Case1
	decf	CURRCHARCOL,W
	btfss	STATUS, Z
	goto	SysSelect1Case2
	movf	CHARCODE,W
	movwf	SYSSTRINGA
	call	GLCDCHARCOL3
	movwf	CURRCHARVAL
	goto	SysSelectEnd1
SysSelect1Case2
	movlw	2
	subwf	CURRCHARCOL,W
	btfss	STATUS, Z
	goto	SysSelect1Case3
	movf	CHARCODE,W
	movwf	SYSSTRINGA
	call	GLCDCHARCOL4
	movwf	CURRCHARVAL
	goto	SysSelectEnd1
SysSelect1Case3
	movlw	3
	subwf	CURRCHARCOL,W
	btfss	STATUS, Z
	goto	SysSelect1Case4
	movf	CHARCODE,W
	movwf	SYSSTRINGA
	call	GLCDCHARCOL5
	movwf	CURRCHARVAL
	goto	SysSelectEnd1
SysSelect1Case4
	movlw	4
	subwf	CURRCHARCOL,W
	btfss	STATUS, Z
	goto	SysSelect1Case5
	movf	CHARCODE,W
	movwf	SYSSTRINGA
	call	GLCDCHARCOL6
	movwf	CURRCHARVAL
	goto	SysSelectEnd1
SysSelect1Case5
	movlw	5
	subwf	CURRCHARCOL,W
	btfss	STATUS, Z
	goto	SysSelectEnd1
	movf	CHARCODE,W
	movwf	SYSSTRINGA
	pagesel	GLCDCHARCOL7
	call	GLCDCHARCOL7
	pagesel	$
	movwf	CURRCHARVAL
SysSelectEnd1
	clrf	CHARROW
	clrf	CHARROW_H
;Legacy method
	clrf	CURRCHARROW
SysForLoop5
	incf	CURRCHARROW,F
	clrf	CHARCOLS
	movlw	1
	movwf	COL
SysForLoop6
	clrf	CHARROWS
	movlw	1
	movwf	ROW
SysForLoop7
	btfss	CURRCHARVAL,0
	goto	ELSE7_1
	movf	CHARCOL,W
	addwf	CHARLOCX,W
	banksel	SYSTEMP1
	movwf	SysTemp1
	banksel	CHARCOLS
	movf	CHARCOLS,W
	banksel	SYSTEMP1
	addwf	SysTemp1,W
	banksel	GLCDX
	movwf	GLCDX
	movf	CHARROW,W
	addwf	CHARLOCY,W
	banksel	SYSTEMP1
	movwf	SysTemp1
	banksel	CHARROWS
	movf	CHARROWS,W
	banksel	SYSTEMP1
	addwf	SysTemp1,W
	banksel	GLCDY
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	pagesel	PSET_KS0108
	call	PSET_KS0108
	pagesel	$
	goto	ENDIF7
ELSE7_1
	movf	CHARCOL,W
	addwf	CHARLOCX,W
	banksel	SYSTEMP1
	movwf	SysTemp1
	banksel	CHARCOLS
	movf	CHARCOLS,W
	banksel	SYSTEMP1
	addwf	SysTemp1,W
	banksel	GLCDX
	movwf	GLCDX
	movf	CHARROW,W
	addwf	CHARLOCY,W
	banksel	SYSTEMP1
	movwf	SysTemp1
	banksel	CHARROWS
	movf	CHARROWS,W
	banksel	SYSTEMP1
	addwf	SysTemp1,W
	banksel	GLCDY
	movwf	GLCDY
	movf	GLCDBACKGROUND,W
	movwf	GLCDCOLOUR
	movf	GLCDBACKGROUND_H,W
	movwf	GLCDCOLOUR_H
	pagesel	PSET_KS0108
	call	PSET_KS0108
	pagesel	$
ENDIF7
	movf	GLCDFONTWIDTH,W
	movwf	SysBYTETempA
	movf	GLCDFNTDEFAULTSIZE,W
	movwf	SysBYTETempB
	pagesel	SYSMULTSUB
	call	SYSMULTSUB
	pagesel	$
	movf	SysBYTETempX,W
	banksel	SYSTEMP1
	movwf	SysTemp1
	banksel	CHARLOCX
	addwf	CHARLOCX,W
	movwf	GLCDX
	movf	CHARROW,W
	addwf	CHARLOCY,W
	banksel	SYSTEMP1
	movwf	SysTemp1
	banksel	CHARROWS
	movf	CHARROWS,W
	banksel	SYSTEMP1
	addwf	SysTemp1,W
	banksel	GLCDY
	movwf	GLCDY
	movf	GLCDBACKGROUND,W
	movwf	GLCDCOLOUR
	movf	GLCDBACKGROUND_H,W
	movwf	GLCDCOLOUR_H
	pagesel	PSET_KS0108
	call	PSET_KS0108
	pagesel	$
	incf	CHARROWS,F
;#4p Positive value Step Handler in For-next statement
	movf	ROW,W
	subwf	GLCDFNTDEFAULTSIZE,W
	banksel	SYSTEMP1
	movwf	SysTemp1
	movwf	SysBYTETempA
	clrf	SysBYTETempB
	banksel	STATUS
	pagesel	SYSCOMPEQUAL
	call	SYSCOMPEQUAL
	pagesel	$
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF8
;Set LoopVar to LoopVar + StepValue where StepValue is a positive value
	incf	ROW,F
	goto	SysForLoop7
;END IF
ENDIF8
SysForLoopEnd7
	incf	CHARCOLS,F
;#4p Positive value Step Handler in For-next statement
	movf	COL,W
	subwf	GLCDFNTDEFAULTSIZE,W
	banksel	SYSTEMP1
	movwf	SysTemp1
	movwf	SysBYTETempA
	clrf	SysBYTETempB
	banksel	STATUS
	pagesel	SYSCOMPEQUAL
	call	SYSCOMPEQUAL
	pagesel	$
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF9
;Set LoopVar to LoopVar + StepValue where StepValue is a positive value
	incf	COL,F
	goto	SysForLoop6
;END IF
ENDIF9
SysForLoopEnd6
	rrf	CURRCHARVAL,F
	movf	GLCDFNTDEFAULTSIZE,W
	addwf	CHARROW,F
	movlw	0
	btfsc	STATUS,C
	movlw	0 + 1
	addwf	CHARROW_H,F
	movlw	8
	subwf	CURRCHARROW,W
	btfss	STATUS, C
	goto	SysForLoop5
SysForLoopEnd5
	movf	GLCDFNTDEFAULTSIZE,W
	addwf	CHARCOL,F
	movlw	0
	btfsc	STATUS,C
	movlw	0 + 1
	addwf	CHARCOL_H,F
	movlw	5
	subwf	CURRCHARCOL,W
	btfss	STATUS, C
	goto	SysForLoop4
SysForLoopEnd4
	clrf	GLCDBACKGROUND
	clrf	GLCDBACKGROUND_H
	movlw	1
	movwf	GLCDFOREGROUND
	clrf	GLCDFOREGROUND_H
	return

;********************************************************************************

GLCDPRINT3
	movf	SysLCDPRINTDATAHandler,W
	movwf	FSR
	bcf	STATUS, IRP
	btfsc	SysLCDPRINTDATAHandler_H,0
	bsf	STATUS, IRP
	movf	INDF,W
	movwf	GLCDPRINTLEN
	movf	GLCDPRINTLEN,F
	btfsc	STATUS, Z
	return
	movf	PRINTLOCX,W
	movwf	GLCDPRINTLOC
	movf	PRINTLOCX_H,W
	movwf	GLCDPRINTLOC_H
	movlw	1
	movwf	GLCDPRINT_STRING_COUNTER
SysForLoop3
	movf	GLCDPRINTLOC,W
	movwf	CHARLOCX
	movf	GLCDPRINTLOC_H,W
	movwf	CHARLOCX_H
	movf	PRINTLOCY,W
	movwf	CHARLOCY
	movf	PRINTLOCY_H,W
	movwf	CHARLOCY_H
	movf	GLCDPRINT_STRING_COUNTER,W
	addwf	SysLCDPRINTDATAHandler,W
	movwf	FSR
	bcf	STATUS, IRP
	btfsc	SysLCDPRINTDATAHandler_H,0
	bsf	STATUS, IRP
	movf	INDF,W
	movwf	CHARCODE
	movf	GLCDFOREGROUND,W
	movwf	LINECOLOUR
	movf	GLCDFOREGROUND_H,W
	movwf	LINECOLOUR_H
	call	GLCDDRAWCHAR
	movf	GLCDFONTWIDTH,W
	movwf	SysBYTETempA
	movf	GLCDFNTDEFAULTSIZE,W
	movwf	SysBYTETempB
	pagesel	SYSMULTSUB
	call	SYSMULTSUB
	pagesel	$
	movf	SysBYTETempX,W
	banksel	SYSTEMP1
	movwf	SysTemp1
	banksel	GLCDPRINTLOC
	addwf	GLCDPRINTLOC,W
	banksel	SYSTEMP2
	movwf	SysTemp2
	banksel	GLCDPRINTLOC_H
	movf	GLCDPRINTLOC_H,W
	banksel	SYSTEMP2_H
	movwf	SysTemp2_H
	movlw	0
	btfsc	STATUS,C
	movlw	0 + 1
	addwf	SysTemp2_H,F
	movlw	1
	addwf	SysTemp2,W
	banksel	GLCDPRINTLOC
	movwf	GLCDPRINTLOC
	banksel	SYSTEMP2_H
	movf	SysTemp2_H,W
	banksel	GLCDPRINTLOC_H
	movwf	GLCDPRINTLOC_H
	movlw	0
	btfsc	STATUS,C
	movlw	0 + 1
	addwf	GLCDPRINTLOC_H,F
;#4p Positive value Step Handler in For-next statement
	movf	GLCDPRINT_STRING_COUNTER,W
	subwf	GLCDPRINTLEN,W
	banksel	SYSTEMP1
	movwf	SysTemp1
	movwf	SysBYTETempA
	clrf	SysBYTETempB
	banksel	STATUS
	pagesel	SYSCOMPEQUAL
	call	SYSCOMPEQUAL
	pagesel	$
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF4
;Set LoopVar to LoopVar + StepValue where StepValue is a positive value
	incf	GLCDPRINT_STRING_COUNTER,F
	goto	SysForLoop3
;END IF
ENDIF4
SysForLoopEnd3
	movf	GLCDPRINTLOC,W
	movwf	PRINTLOCX
	movf	GLCDPRINTLOC_H,W
	movwf	PRINTLOCX_H
	return

;********************************************************************************

INITGLCD_KS0108
	banksel	TRISE
	bcf	TRISE,0
	bcf	TRISE,1
	bcf	TRISE,2
	bcf	TRISC,0
	bcf	TRISC,1
	bcf	TRISC,2
	banksel	PORTC
	bcf	PORTC,2
	movlw	1
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	pagesel	Delay_MS
	call	Delay_MS
	pagesel	$
	bsf	PORTC,2
	movlw	1
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	pagesel	Delay_MS
	call	Delay_MS
	pagesel	$
	bsf	PORTC,0
	bsf	PORTC,1
	bcf	PORTE,0
	movlw	63
	movwf	LCDBYTE
	pagesel	GLCDWRITEBYTE_KS0108
	call	GLCDWRITEBYTE_KS0108
	pagesel	$
	movlw	192
	movwf	LCDBYTE
	pagesel	GLCDWRITEBYTE_KS0108
	call	GLCDWRITEBYTE_KS0108
	pagesel	$
	bcf	PORTC,0
	bcf	PORTC,1
	clrf	GLCDBACKGROUND
	clrf	GLCDBACKGROUND_H
	movlw	1
	movwf	GLCDFOREGROUND
	clrf	GLCDFOREGROUND_H
	movlw	5
	movwf	GLCDFONTWIDTH
	clrf	GLCDFNTDEFAULT
	movlw	1
	movwf	GLCDFNTDEFAULTSIZE
	movlw	7
	movwf	GLCDFNTDEFAULTHEIGHT
	pagesel	GLCDCLS_KS0108
	goto	GLCDCLS_KS0108

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
	clrf	PORTD
	clrf	PORTE
	return

;********************************************************************************

FN_STR
	clrf	SYSCHARCOUNT
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	16
	movwf	SysWORDTempB
	movlw	39
	movwf	SysWORDTempB_H
	banksel	STATUS
	call	SYSCOMPLESSTHAN16
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF71
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	16
	movwf	SysWORDTempB
	movlw	39
	movwf	SysWORDTempB_H
	banksel	STATUS
	pagesel	SYSDIVSUB16
	call	SYSDIVSUB16
	pagesel	$
	movf	SysWORDTempA,W
	movwf	SYSSTRDATA
	movf	SYSCALCTEMPX,W
	banksel	SYSVALTEMP
	movwf	SYSVALTEMP
	movf	SYSCALCTEMPX_H,W
	movwf	SYSVALTEMP_H
	banksel	SYSCHARCOUNT
	incf	SYSCHARCOUNT,F
	movlw	low(STR)
	addwf	SYSCHARCOUNT,W
	movwf	FSR
	bankisel	STR
	movlw	48
	addwf	SYSSTRDATA,W
	movwf	INDF
	goto	SYSVALTHOUSANDS
ENDIF71
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	232
	movwf	SysWORDTempB
	movlw	3
	movwf	SysWORDTempB_H
	banksel	STATUS
	call	SYSCOMPLESSTHAN16
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF72
SYSVALTHOUSANDS
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	232
	movwf	SysWORDTempB
	movlw	3
	movwf	SysWORDTempB_H
	banksel	STATUS
	pagesel	SYSDIVSUB16
	call	SYSDIVSUB16
	pagesel	$
	movf	SysWORDTempA,W
	movwf	SYSSTRDATA
	movf	SYSCALCTEMPX,W
	banksel	SYSVALTEMP
	movwf	SYSVALTEMP
	movf	SYSCALCTEMPX_H,W
	movwf	SYSVALTEMP_H
	banksel	SYSCHARCOUNT
	incf	SYSCHARCOUNT,F
	movlw	low(STR)
	addwf	SYSCHARCOUNT,W
	movwf	FSR
	bankisel	STR
	movlw	48
	addwf	SYSSTRDATA,W
	movwf	INDF
	goto	SYSVALHUNDREDS
ENDIF72
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	100
	movwf	SysWORDTempB
	clrf	SysWORDTempB_H
	banksel	STATUS
	call	SYSCOMPLESSTHAN16
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF73
SYSVALHUNDREDS
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	100
	movwf	SysWORDTempB
	clrf	SysWORDTempB_H
	banksel	STATUS
	pagesel	SYSDIVSUB16
	call	SYSDIVSUB16
	pagesel	$
	movf	SysWORDTempA,W
	movwf	SYSSTRDATA
	movf	SYSCALCTEMPX,W
	banksel	SYSVALTEMP
	movwf	SYSVALTEMP
	movf	SYSCALCTEMPX_H,W
	movwf	SYSVALTEMP_H
	banksel	SYSCHARCOUNT
	incf	SYSCHARCOUNT,F
	movlw	low(STR)
	addwf	SYSCHARCOUNT,W
	movwf	FSR
	bankisel	STR
	movlw	48
	addwf	SYSSTRDATA,W
	movwf	INDF
	goto	SYSVALTENS
ENDIF73
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	10
	movwf	SysWORDTempB
	clrf	SysWORDTempB_H
	banksel	STATUS
	call	SYSCOMPLESSTHAN16
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF74
SYSVALTENS
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	10
	movwf	SysWORDTempB
	clrf	SysWORDTempB_H
	banksel	STATUS
	pagesel	SYSDIVSUB16
	call	SYSDIVSUB16
	pagesel	$
	movf	SysWORDTempA,W
	movwf	SYSSTRDATA
	movf	SYSCALCTEMPX,W
	banksel	SYSVALTEMP
	movwf	SYSVALTEMP
	movf	SYSCALCTEMPX_H,W
	movwf	SYSVALTEMP_H
	banksel	SYSCHARCOUNT
	incf	SYSCHARCOUNT,F
	movlw	low(STR)
	addwf	SYSCHARCOUNT,W
	movwf	FSR
	bankisel	STR
	movlw	48
	addwf	SYSSTRDATA,W
	movwf	INDF
ENDIF74
	incf	SYSCHARCOUNT,F
	movlw	low(STR)
	addwf	SYSCHARCOUNT,W
	movwf	FSR
	bankisel	STR
	movlw	48
	banksel	SYSVALTEMP
	addwf	SYSVALTEMP,W
	movwf	INDF
	movf	SYSCALCTEMPX,W
	movwf	SYSVALTEMP
	movf	SYSCALCTEMPX_H,W
	movwf	SYSVALTEMP_H
	banksel	SYSCHARCOUNT
	movf	SYSCHARCOUNT,W
	banksel	SYSSTR_0
	movwf	SYSSTR_0
	banksel	STATUS
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

_LINE
	clrf	LINEDIFFX
	clrf	LINEDIFFX_H
	clrf	LINEDIFFY
	clrf	LINEDIFFY_H
	clrf	LINESTEPX
	clrf	LINESTEPX_H
	clrf	LINESTEPY
	clrf	LINESTEPY_H
	clrf	LINEDIFFX_X2
	clrf	LINEDIFFX_X2_H
	clrf	LINEDIFFY_X2
	clrf	LINEDIFFY_X2_H
	clrf	LINEERR
	clrf	LINEERR_H
	movf	LINEX1,W
	subwf	LINEX2,W
	movwf	LINEDIFFX
	movf	LINEX2_H,W
	movwf	LINEDIFFX_H
	movf	LINEX1_H,W
	btfss	STATUS,C
	incfsz	LINEX1_H,W
	subwf	LINEDIFFX_H,F
	movf	LINEY1,W
	subwf	LINEY2,W
	movwf	LINEDIFFY
	movf	LINEY2_H,W
	movwf	LINEDIFFY_H
	movf	LINEY1_H,W
	btfss	STATUS,C
	incfsz	LINEY1_H,W
	subwf	LINEDIFFY_H,F
	movf	LINEDIFFX,W
	movwf	SysINTEGERTempB
	movf	LINEDIFFX_H,W
	movwf	SysINTEGERTempB_H
	clrf	SysINTEGERTempA
	clrf	SysINTEGERTempA_H
	pagesel	SYSCOMPLESSTHANINT
	call	SYSCOMPLESSTHANINT
	pagesel	$
	btfss	SysByteTempX,0
	goto	ELSE46_1
	movlw	1
	movwf	LINESTEPX
	clrf	LINESTEPX_H
	goto	ENDIF46
ELSE46_1
	movlw	255
	movwf	LINESTEPX
	movwf	LINESTEPX_H
ENDIF46
	movf	LINEDIFFY,W
	movwf	SysINTEGERTempB
	movf	LINEDIFFY_H,W
	movwf	SysINTEGERTempB_H
	clrf	SysINTEGERTempA
	clrf	SysINTEGERTempA_H
	pagesel	SYSCOMPLESSTHANINT
	call	SYSCOMPLESSTHANINT
	pagesel	$
	btfss	SysByteTempX,0
	goto	ELSE47_1
	movlw	1
	movwf	LINESTEPY
	clrf	LINESTEPY_H
	goto	ENDIF47
ELSE47_1
	movlw	255
	movwf	LINESTEPY
	movwf	LINESTEPY_H
ENDIF47
	movf	LINESTEPX,W
	movwf	SysINTEGERTempA
	movf	LINESTEPX_H,W
	movwf	SysINTEGERTempA_H
	movf	LINEDIFFX,W
	movwf	SysINTEGERTempB
	movf	LINEDIFFX_H,W
	movwf	SysINTEGERTempB_H
	pagesel	SYSMULTSUBINT
	call	SYSMULTSUBINT
	pagesel	$
	movf	SysINTEGERTempX,W
	movwf	LINEDIFFX
	movf	SysINTEGERTempX_H,W
	movwf	LINEDIFFX_H
	movf	LINESTEPY,W
	movwf	SysINTEGERTempA
	movf	LINESTEPY_H,W
	movwf	SysINTEGERTempA_H
	movf	LINEDIFFY,W
	movwf	SysINTEGERTempB
	movf	LINEDIFFY_H,W
	movwf	SysINTEGERTempB_H
	pagesel	SYSMULTSUBINT
	call	SYSMULTSUBINT
	pagesel	$
	movf	SysINTEGERTempX,W
	movwf	LINEDIFFY
	movf	SysINTEGERTempX_H,W
	movwf	LINEDIFFY_H
	movf	LINEDIFFX,W
	movwf	SysINTEGERTempA
	movf	LINEDIFFX_H,W
	movwf	SysINTEGERTempA_H
	movlw	2
	movwf	SysINTEGERTempB
	clrf	SysINTEGERTempB_H
	pagesel	SYSMULTSUBINT
	call	SYSMULTSUBINT
	pagesel	$
	movf	SysINTEGERTempX,W
	movwf	LINEDIFFX_X2
	movf	SysINTEGERTempX_H,W
	movwf	LINEDIFFX_X2_H
	movf	LINEDIFFY,W
	movwf	SysINTEGERTempA
	movf	LINEDIFFY_H,W
	movwf	SysINTEGERTempA_H
	movlw	2
	movwf	SysINTEGERTempB
	clrf	SysINTEGERTempB_H
	pagesel	SYSMULTSUBINT
	call	SYSMULTSUBINT
	pagesel	$
	movf	SysINTEGERTempX,W
	movwf	LINEDIFFY_X2
	movf	SysINTEGERTempX_H,W
	movwf	LINEDIFFY_X2_H
	movf	LINEDIFFX,W
	movwf	SysINTEGERTempA
	movf	LINEDIFFX_H,W
	movwf	SysINTEGERTempA_H
	movf	LINEDIFFY,W
	movwf	SysINTEGERTempB
	movf	LINEDIFFY_H,W
	movwf	SysINTEGERTempB_H
	pagesel	SYSCOMPLESSTHANINT
	call	SYSCOMPLESSTHANINT
	pagesel	$
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ELSE48_1
	movf	LINEDIFFX,W
	subwf	LINEDIFFY_X2,W
	movwf	LINEERR
	movf	LINEDIFFY_X2_H,W
	movwf	LINEERR_H
	movf	LINEDIFFX_H,W
	btfss	STATUS,C
	incfsz	LINEDIFFX_H,W
	subwf	LINEERR_H,F
SysDoLoop_S1
	movf	linex1,W
	movwf	SysWORDTempA
	movf	linex1_H,W
	movwf	SysWORDTempA_H
	movf	linex2,W
	movwf	SysWORDTempB
	movf	linex2_H,W
	movwf	SysWORDTempB_H
	pagesel	SYSCOMPEQUAL16
	call	SYSCOMPEQUAL16
	pagesel	$
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	SysDoLoop_E1
	movf	LINEX1,W
	movwf	GLCDX
	movf	LINEY1,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	pagesel	PSET_KS0108
	call	PSET_KS0108
	pagesel	$
	movf	LINESTEPX,W
	addwf	LINEX1,F
	movf	LINESTEPX_H,W
	btfsc	STATUS,C
	incfsz	LINESTEPX_H,W
	addwf	LINEX1_H,F
	movf	LINEERR,W
	movwf	SysINTEGERTempA
	movf	LINEERR_H,W
	movwf	SysINTEGERTempA_H
	clrf	SysINTEGERTempB
	clrf	SysINTEGERTempB_H
	pagesel	SYSCOMPLESSTHANINT
	call	SYSCOMPLESSTHANINT
	pagesel	$
	btfss	SysByteTempX,0
	goto	ELSE49_1
	movf	LINEDIFFY_X2,W
	addwf	LINEERR,F
	movf	LINEDIFFY_X2_H,W
	btfsc	STATUS,C
	incfsz	LINEDIFFY_X2_H,W
	addwf	LINEERR_H,F
	goto	ENDIF49
ELSE49_1
	movf	LINEDIFFX_X2,W
	subwf	LINEDIFFY_X2,W
	banksel	SYSTEMP1
	movwf	SysTemp1
	banksel	LINEDIFFY_X2_H
	movf	LINEDIFFY_X2_H,W
	banksel	SYSTEMP1_H
	movwf	SysTemp1_H
	banksel	LINEDIFFX_X2_H
	movf	LINEDIFFX_X2_H,W
	btfss	STATUS,C
	incf	LINEDIFFX_X2_H,W
	btfsc	STATUS,Z
	goto	ENDIF56
	banksel	SYSTEMP1_H
	subwf	SysTemp1_H,F
ENDIF56
	banksel	SYSTEMP1
	movf	SysTemp1,W
	banksel	LINEERR
	addwf	LINEERR,F
	banksel	SYSTEMP1_H
	movf	SysTemp1_H,W
	btfsc	STATUS,C
	incf	SysTemp1_H,W
	btfsc	STATUS,Z
	goto	ENDIF57
	banksel	LINEERR_H
	addwf	LINEERR_H,F
ENDIF57
	banksel	LINESTEPY
	movf	LINESTEPY,W
	addwf	LINEY1,F
	movf	LINESTEPY_H,W
	btfsc	STATUS,C
	incfsz	LINESTEPY_H,W
	addwf	LINEY1_H,F
ENDIF49
	goto	SysDoLoop_S1
SysDoLoop_E1
	movf	LINEX1,W
	movwf	GLCDX
	movf	LINEY1,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	pagesel	PSET_KS0108
	call	PSET_KS0108
	pagesel	$
	goto	ENDIF48
ELSE48_1
	movf	LINEDIFFY,W
	subwf	LINEDIFFX_X2,W
	movwf	LINEERR
	movf	LINEDIFFX_X2_H,W
	movwf	LINEERR_H
	movf	LINEDIFFY_H,W
	btfss	STATUS,C
	incfsz	LINEDIFFY_H,W
	subwf	LINEERR_H,F
SysDoLoop_S2
	movf	liney1,W
	movwf	SysWORDTempA
	movf	liney1_H,W
	movwf	SysWORDTempA_H
	movf	liney2,W
	movwf	SysWORDTempB
	movf	liney2_H,W
	movwf	SysWORDTempB_H
	pagesel	SYSCOMPEQUAL16
	call	SYSCOMPEQUAL16
	pagesel	$
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	SysDoLoop_E2
	movf	LINEX1,W
	movwf	GLCDX
	movf	LINEY1,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	pagesel	PSET_KS0108
	call	PSET_KS0108
	pagesel	$
	movf	LINESTEPY,W
	addwf	LINEY1,F
	movf	LINESTEPY_H,W
	btfsc	STATUS,C
	incfsz	LINESTEPY_H,W
	addwf	LINEY1_H,F
	movf	LINEERR,W
	movwf	SysINTEGERTempA
	movf	LINEERR_H,W
	movwf	SysINTEGERTempA_H
	clrf	SysINTEGERTempB
	clrf	SysINTEGERTempB_H
	pagesel	SYSCOMPLESSTHANINT
	call	SYSCOMPLESSTHANINT
	pagesel	$
	btfss	SysByteTempX,0
	goto	ELSE50_1
	movf	LINEDIFFX_X2,W
	addwf	LINEERR,F
	movf	LINEDIFFX_X2_H,W
	btfsc	STATUS,C
	incfsz	LINEDIFFX_X2_H,W
	addwf	LINEERR_H,F
	goto	ENDIF50
ELSE50_1
	movf	LINEDIFFY_X2,W
	subwf	LINEDIFFX_X2,W
	banksel	SYSTEMP1
	movwf	SysTemp1
	banksel	LINEDIFFX_X2_H
	movf	LINEDIFFX_X2_H,W
	banksel	SYSTEMP1_H
	movwf	SysTemp1_H
	banksel	LINEDIFFY_X2_H
	movf	LINEDIFFY_X2_H,W
	btfss	STATUS,C
	incf	LINEDIFFY_X2_H,W
	btfsc	STATUS,Z
	goto	ENDIF62
	banksel	SYSTEMP1_H
	subwf	SysTemp1_H,F
ENDIF62
	banksel	SYSTEMP1
	movf	SysTemp1,W
	banksel	LINEERR
	addwf	LINEERR,F
	banksel	SYSTEMP1_H
	movf	SysTemp1_H,W
	btfsc	STATUS,C
	incf	SysTemp1_H,W
	btfsc	STATUS,Z
	goto	ENDIF63
	banksel	LINEERR_H
	addwf	LINEERR_H,F
ENDIF63
	banksel	LINESTEPX
	movf	LINESTEPX,W
	addwf	LINEX1,F
	movf	LINESTEPX_H,W
	btfsc	STATUS,C
	incfsz	LINESTEPX_H,W
	addwf	LINEX1_H,F
ENDIF50
	goto	SysDoLoop_S2
SysDoLoop_E2
	movf	LINEX1,W
	movwf	GLCDX
	movf	LINEY1,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	pagesel	PSET_KS0108
	call	PSET_KS0108
	pagesel	$
ENDIF48
	return

;********************************************************************************

;Start of program memory page 1
	ORG	2048
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

GLCDCHARCOL7
	movlw	113
	subwf	SysStringA, W
	btfsc	STATUS, C
	retlw	0
	movf	SysStringA, W
	addlw	low TABLEGLCDCHARCOL7
	movwf	SysStringA
	movlw	high TABLEGLCDCHARCOL7
	btfsc	STATUS, C
	addlw	1
	movwf	PCLATH
	movf	SysStringA, W
	movwf	PCL
TABLEGLCDCHARCOL7
	retlw	112
	retlw	16
	retlw	0
	retlw	10
	retlw	6
	retlw	136
	retlw	34
	retlw	56
	retlw	62
	retlw	8
	retlw	32
	retlw	16
	retlw	16
	retlw	128
	retlw	128
	retlw	64
	retlw	4
	retlw	0
	retlw	0
	retlw	0
	retlw	40
	retlw	36
	retlw	196
	retlw	160
	retlw	0
	retlw	0
	retlw	0
	retlw	40
	retlw	16
	retlw	0
	retlw	16
	retlw	0
	retlw	4
	retlw	124
	retlw	0
	retlw	140
	retlw	98
	retlw	32
	retlw	114
	retlw	96
	retlw	14
	retlw	108
	retlw	60
	retlw	0
	retlw	0
	retlw	0
	retlw	40
	retlw	16
	retlw	12
	retlw	124
	retlw	248
	retlw	108
	retlw	68
	retlw	56
	retlw	130
	retlw	2
	retlw	244
	retlw	254
	retlw	0
	retlw	2
	retlw	130
	retlw	128
	retlw	254
	retlw	254
	retlw	124
	retlw	12
	retlw	188
	retlw	140
	retlw	100
	retlw	2
	retlw	126
	retlw	62
	retlw	126
	retlw	198
	retlw	14
	retlw	134
	retlw	0
	retlw	64
	retlw	0
	retlw	8
	retlw	128
	retlw	0
	retlw	240
	retlw	112
	retlw	64
	retlw	254
	retlw	48
	retlw	4
	retlw	120
	retlw	240
	retlw	0
	retlw	0
	retlw	0
	retlw	0
	retlw	240
	retlw	240
	retlw	112
	retlw	16
	retlw	248
	retlw	16
	retlw	64
	retlw	64
	retlw	248
	retlw	56
	retlw	120
	retlw	136
	retlw	120
	retlw	136
	retlw	0
	retlw	0
	retlw	0
	retlw	16
	retlw	120

;********************************************************************************

GLCDCLS_KS0108
	clrf	GLCD_YORDINATE
	clrf	GLCD_YORDINATE_H
	bsf	PORTC,0
	bcf	PORTC,1
;Legacy method
	clrf	GLCD_COUNT
SysForLoop10
	incf	GLCD_COUNT,F
;Legacy method
	movlw	255
	movwf	CURRPAGE
SysForLoop11
	incf	CURRPAGE,F
	bcf	PORTE,0
	movlw	184
	iorwf	CURRPAGE,W
	movwf	LCDBYTE
	call	GLCDWRITEBYTE_KS0108
;Legacy method
	movlw	255
	movwf	CURRCOL
SysForLoop12
	incf	CURRCOL,F
	bcf	PORTE,0
	movlw	64
	iorwf	CURRCOL,W
	movwf	LCDBYTE
	call	GLCDWRITEBYTE_KS0108
	bsf	PORTE,0
	clrf	LCDBYTE
	call	GLCDWRITEBYTE_KS0108
	movlw	63
	subwf	CURRCOL,W
	btfss	STATUS, C
	goto	SysForLoop12
SysForLoopEnd12
	movlw	7
	subwf	CURRPAGE,W
	btfss	STATUS, C
	goto	SysForLoop11
SysForLoopEnd11
	bcf	PORTC,0
	bsf	PORTC,1
	movlw	2
	subwf	GLCD_COUNT,W
	btfss	STATUS, C
	goto	SysForLoop10
SysForLoopEnd10
	bcf	PORTC,0
	bcf	PORTC,1
	return

;********************************************************************************

FN_GLCDREADBYTE_KS0108
	banksel	TRISD
	bsf	TRISD,7
	bsf	TRISD,6
	bsf	TRISD,5
	bsf	TRISD,4
	bsf	TRISD,3
	bsf	TRISD,2
	bsf	TRISD,1
	bsf	TRISD,0
	banksel	PORTE
	bsf	PORTE,1
	bsf	PORTE,2
	goto	$+1
	goto	$+1
	bcf	GLCDREADBYTE_KS0108,7
	btfsc	PORTD,7
	bsf	GLCDREADBYTE_KS0108,7
	bcf	GLCDREADBYTE_KS0108,6
	btfsc	PORTD,6
	bsf	GLCDREADBYTE_KS0108,6
	bcf	GLCDREADBYTE_KS0108,5
	btfsc	PORTD,5
	bsf	GLCDREADBYTE_KS0108,5
	bcf	GLCDREADBYTE_KS0108,4
	btfsc	PORTD,4
	bsf	GLCDREADBYTE_KS0108,4
	bcf	GLCDREADBYTE_KS0108,3
	btfsc	PORTD,3
	bsf	GLCDREADBYTE_KS0108,3
	bcf	GLCDREADBYTE_KS0108,2
	btfsc	PORTD,2
	bsf	GLCDREADBYTE_KS0108,2
	bcf	GLCDREADBYTE_KS0108,1
	btfsc	PORTD,1
	bsf	GLCDREADBYTE_KS0108,1
	bcf	GLCDREADBYTE_KS0108,0
	btfsc	PORTD,0
	bsf	GLCDREADBYTE_KS0108,0
	bcf	PORTE,2
	goto	$+1
	goto	$+1
	return

;********************************************************************************

GLCDWRITEBYTE_KS0108
	bcf	SYSBITVAR0,0
	btfsc	PORTE,0
	bsf	SYSBITVAR0,0
	bcf	SYSBITVAR0,1
	btfsc	PORTC,1
	bsf	SYSBITVAR0,1
	btfsc	PORTC,0
	bcf	PORTC,1
	bcf	PORTE,0
SysWaitLoop1
	call	FN_GLCDREADBYTE_KS0108
	btfsc	GLCDREADBYTE_KS0108,7
	goto	SysWaitLoop1
	bcf	PORTE,0
	btfsc	SYSBITVAR0,0
	bsf	PORTE,0
	bcf	PORTC,1
	btfsc	SYSBITVAR0,1
	bsf	PORTC,1
	bcf	PORTE,1
	banksel	TRISD
	bcf	TRISD,0
	bcf	TRISD,1
	bcf	TRISD,2
	bcf	TRISD,3
	bcf	TRISD,4
	bcf	TRISD,5
	bcf	TRISD,6
	bcf	TRISD,7
	banksel	PORTD
	bcf	PORTD,7
	btfsc	LCDBYTE,7
	bsf	PORTD,7
	bcf	PORTD,6
	btfsc	LCDBYTE,6
	bsf	PORTD,6
	bcf	PORTD,5
	btfsc	LCDBYTE,5
	bsf	PORTD,5
	bcf	PORTD,4
	btfsc	LCDBYTE,4
	bsf	PORTD,4
	bcf	PORTD,3
	btfsc	LCDBYTE,3
	bsf	PORTD,3
	bcf	PORTD,2
	btfsc	LCDBYTE,2
	bsf	PORTD,2
	bcf	PORTD,1
	btfsc	LCDBYTE,1
	bsf	PORTD,1
	bcf	PORTD,0
	btfsc	LCDBYTE,0
	bsf	PORTD,0
	goto	$+1
	goto	$+1
	bsf	PORTE,2
	goto	$+1
	goto	$+1
	bcf	PORTE,2
	goto	$+1
	goto	$+1
	return

;********************************************************************************

PSET_KS0108
	btfsc	GLCDX,6
	goto	ENDIF21
	bsf	PORTC,1
	bcf	PORTC,0
ENDIF21
	btfss	GLCDX,6
	goto	ENDIF22
	bsf	PORTC,0
	movlw	64
	subwf	GLCDX,F
	bcf	PORTC,1
ENDIF22
	movf	GLCDY,W
	movwf	SysBYTETempA
	movlw	8
	movwf	SysBYTETempB
	call	SYSDIVSUB
	movf	SysBYTETempA,W
	movwf	CURRPAGE
	bcf	PORTE,0
	movlw	184
	iorwf	CURRPAGE,W
	movwf	LCDBYTE
	call	GLCDWRITEBYTE_KS0108
	bcf	PORTE,0
	movlw	64
	iorwf	GLCDX,W
	movwf	LCDBYTE
	call	GLCDWRITEBYTE_KS0108
	bsf	PORTE,0
	call	FN_GLCDREADBYTE_KS0108
	movf	GLCDREADBYTE_KS0108,W
	movwf	GLCDDATATEMP
	bsf	PORTE,0
	call	FN_GLCDREADBYTE_KS0108
	movf	GLCDREADBYTE_KS0108,W
	movwf	GLCDDATATEMP
	movlw	7
	andwf	GLCDY,W
	movwf	GLCDBITNO
	btfsc	GLCDCOLOUR,0
	goto	ELSE23_1
	movlw	254
	movwf	GLCDCHANGE
	bsf	STATUS,C
	goto	ENDIF23
ELSE23_1
	movlw	1
	movwf	GLCDCHANGE
	bcf	STATUS,C
ENDIF23
	movf	GLCDBITNO,W
	movwf	SysRepeatTemp1
	btfsc	STATUS,Z
	goto	SysRepeatLoopEnd1
SysRepeatLoop1
	rlf	GLCDCHANGE,F
	decfsz	SysRepeatTemp1,F
	goto	SysRepeatLoop1
SysRepeatLoopEnd1
	btfsc	GLCDCOLOUR,0
	goto	ELSE24_1
	movf	GLCDDATATEMP,W
	andwf	GLCDCHANGE,W
	movwf	GLCDDATATEMP
	goto	ENDIF24
ELSE24_1
	movf	GLCDDATATEMP,W
	iorwf	GLCDCHANGE,W
	movwf	GLCDDATATEMP
ENDIF24
	bcf	PORTE,0
	movlw	64
	iorwf	GLCDX,W
	movwf	LCDBYTE
	call	GLCDWRITEBYTE_KS0108
	bsf	PORTE,0
	movf	GLCDDATATEMP,W
	movwf	LCDBYTE
	call	GLCDWRITEBYTE_KS0108
	bcf	PORTC,0
	bcf	PORTC,1
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

SYSCOMPLESSTHAN
	clrf	SYSBYTETEMPX
	bsf	STATUS, C
	movf	SYSBYTETEMPB, W
	subwf	SYSBYTETEMPA, W
	btfss	STATUS, C
	comf	SYSBYTETEMPX,F
	return

;********************************************************************************

SYSCOMPLESSTHANINT
	clrf	SYSBYTETEMPX
	btfss	SYSINTEGERTEMPA_H,7
	goto	ELSE68_1
	btfsc	SYSINTEGERTEMPB_H,7
	goto	ENDIF69
	comf	SYSBYTETEMPX,F
	return
ENDIF69
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
	goto	ENDIF68
ELSE68_1
	btfsc	SYSINTEGERTEMPB_H,7
	return
ENDIF68
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
	goto	ENDIF78
	clrf	SYSWORDTEMPA
	clrf	SYSWORDTEMPA_H
	return
ENDIF78
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
	goto	ENDIF79
	bcf	SYSDIVMULTA,0
	movf	SYSDIVMULTB,W
	addwf	SYSDIVMULTX,F
	movf	SYSDIVMULTB_H,W
	btfsc	STATUS,C
	incfsz	SYSDIVMULTB_H,W
	addwf	SYSDIVMULTX_H,F
ENDIF79
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

SYSMULTSUB16
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
MUL16LOOP
	btfss	SYSDIVMULTB,0
	goto	ENDIF75
	movf	SYSDIVMULTA,W
	addwf	SYSDIVMULTX,F
	movf	SYSDIVMULTA_H,W
	btfsc	STATUS,C
	incfsz	SYSDIVMULTA_H,W
	addwf	SYSDIVMULTX_H,F
ENDIF75
	bcf	STATUS,C
	rrf	SYSDIVMULTB_H,F
	rrf	SYSDIVMULTB,F
	bcf	STATUS,C
	rlf	SYSDIVMULTA,F
	rlf	SYSDIVMULTA_H,F
	movf	SYSDIVMULTB,W
	movwf	SysWORDTempB
	movf	SYSDIVMULTB_H,W
	movwf	SysWORDTempB_H
	clrf	SysWORDTempA
	clrf	SysWORDTempA_H
	pagesel	SYSCOMPLESSTHAN16
	call	SYSCOMPLESSTHAN16
	pagesel	$
	btfsc	SysByteTempX,0
	goto	MUL16LOOP
	movf	SYSDIVMULTX,W
	movwf	SYSWORDTEMPX
	movf	SYSDIVMULTX_H,W
	movwf	SYSWORDTEMPX_H
	return

;********************************************************************************

SYSMULTSUBINT
	movf	SYSINTEGERTEMPA_H,W
	xorwf	SYSINTEGERTEMPB_H,W
	movwf	SYSSIGNBYTE
	btfss	SYSINTEGERTEMPA_H,7
	goto	ENDIF65
	comf	SYSINTEGERTEMPA,F
	comf	SYSINTEGERTEMPA_H,F
	incf	SYSINTEGERTEMPA,F
	btfsc	STATUS,Z
	incf	SYSINTEGERTEMPA_H,F
ENDIF65
	btfss	SYSINTEGERTEMPB_H,7
	goto	ENDIF66
	comf	SYSINTEGERTEMPB,F
	comf	SYSINTEGERTEMPB_H,F
	incf	SYSINTEGERTEMPB,F
	btfsc	STATUS,Z
	incf	SYSINTEGERTEMPB_H,F
ENDIF66
	call	SYSMULTSUB16
	btfss	SYSSIGNBYTE,7
	goto	ENDIF67
	comf	SYSINTEGERTEMPX,F
	comf	SYSINTEGERTEMPX_H,F
	incf	SYSINTEGERTEMPX,F
	btfsc	STATUS,Z
	incf	SYSINTEGERTEMPX_H,F
ENDIF67
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
	retlw	5
	retlw	72	;H
	retlw	101	;e
	retlw	108	;l
	retlw	108	;l
	retlw	111	;o


StringTable2
	retlw	8
	retlw	65	;A
	retlw	83	;S
	retlw	67	;C
	retlw	73	;I
	retlw	73	;I
	retlw	32	; 
	retlw	35	;#
	retlw	58	;:


;********************************************************************************

;Start of program memory page 2
	ORG	4096
;Start of program memory page 3
	ORG	6144

 END
