;Program compiled by Great Cow BASIC (0.98.02 2018-05-16 (Linux))
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Chip Model: MEGA168
;Assembler header file
.INCLUDE "m168def.inc"

;SREG bit names (for AVR Assembler compatibility, GCBASIC uses different names)
#define C 0
#define H 5
#define I 7
#define N 2
#define S 4
#define T 6
#define V 3
#define Z 1

;********************************************************************************

;Set aside memory locations for variables
.EQU	CC_X=280
.EQU	CHARLINE=281
.EQU	C_X=282
.EQU	C_Y=283
.EQU	D_X=284
.EQU	D_Y=285
.EQU	GLCDCHAR=286
.EQU	GLCDCOMDAT=287
.EQU	GLCDDAT=288
.EQU	GLCDINDEX=289
.EQU	GLCDP_X=290
.EQU	GLCDP_Y=291
.EQU	STRINGPOINTER=292
.EQU	SYSGLCDSTRDATHANDLER=293
.EQU	SYSGLCDSTRDATHANDLER_H=294
.EQU	SYSREPEATTEMP1=295
.EQU	SYSREPEATTEMP2=296
.EQU	SYSREPEATTEMP3=297
.EQU	SYSREPEATTEMP3_H=298
.EQU	SYSREPEATTEMP4=299
.EQU	SYSSTRINGPARAM1=256
.EQU	SYSSTRINGPARAM2=268

;********************************************************************************

;Register variables
.DEF	DELAYTEMP=r25
.DEF	DELAYTEMP2=r26
.DEF	SYSBITTEST=r5
.DEF	SYSBYTETEMPA=r22
.DEF	SYSBYTETEMPB=r28
.DEF	SYSBYTETEMPX=r0
.DEF	SYSCALCTEMPA=r22
.DEF	SYSCALCTEMPB=r28
.DEF	SYSCALCTEMPX=r0
.DEF	SYSREADA=r30
.DEF	SYSREADA_H=r31
.DEF	SYSSTRINGA=r26
.DEF	SYSSTRINGA_H=r27
.DEF	SYSSTRINGB=r28
.DEF	SYSSTRINGB_H=r29
.DEF	SYSSTRINGLENGTH=r25
.DEF	SYSVALUECOPY=r21
.DEF	SYSWAITTEMPMS=r29
.DEF	SYSWAITTEMPMS_H=r30
.DEF	SYSWAITTEMPS=r31
.DEF	SYSTEMP1=r1
.DEF	SYSTEMP1_H=r2
.DEF	SYSTEMP2=r16
.DEF	SYSTEMP3=r17

;********************************************************************************

;Vectors
;Interrupt vectors
.ORG	0
	rjmp	BASPROGRAMSTART ;Reset
.ORG	2
	reti	;INT0
.ORG	4
	reti	;INT1
.ORG	6
	reti	;PCINT0
.ORG	8
	reti	;PCINT1
.ORG	10
	reti	;PCINT2
.ORG	12
	reti	;WDT
.ORG	14
	reti	;TIMER2_COMPA
.ORG	16
	reti	;TIMER2_COMPB
.ORG	18
	reti	;TIMER2_OVF
.ORG	20
	reti	;TIMER1_CAPT
.ORG	22
	reti	;TIMER1_COMPA
.ORG	24
	reti	;TIMER1_COMPB
.ORG	26
	reti	;TIMER1_OVF
.ORG	28
	reti	;TIMER0_COMPA
.ORG	30
	reti	;TIMER0_COMPB
.ORG	32
	reti	;TIMER0_OVF
.ORG	34
	reti	;SPI_STC
.ORG	36
	reti	;USART_RX
.ORG	38
	reti	;USART_UDRE
.ORG	40
	reti	;USART_TX
.ORG	42
	reti	;ADC
.ORG	44
	reti	;EE_READY
.ORG	46
	reti	;ANALOG_COMP
.ORG	48
	reti	;TWI
.ORG	50
	reti	;SPM_READY

;********************************************************************************

;Start of program memory page 0
.ORG	52
BASPROGRAMSTART:
;Initialise stack
	ldi	SysValueCopy,high(RAMEND)
	out	SPH, SysValueCopy
	ldi	SysValueCopy,low(RAMEND)
	out	SPL, SysValueCopy
;Call initialisation routines
	rcall	INITSYS
;Automatic pin direction setting
	sbi	DDRB,5
	sbi	DDRB,4
	sbi	DDRB,3
	sbi	DDRB,2
	sbi	DDRB,0
	sbi	DDRB,7
	sbi	DDRB,6

;Start of the main program
;Source:F1L5S0I5
;Source:F1L6S0I6
;Source:F1L7S0I7
;Source:F1L8S0I8
;Source:F1L9S0I9
;Source:F1L10S0I10
;Source:F1L12S0I12
;Source:F1L114S4I1
	cbi	PORTB,3
;Source:F1L115S4I2
	cbi	PORTB,2
;Source:F1L13S0I13
	rcall	INITGLCD
;Source:F1L15S0I15
	ldi	SysValueCopy,3
	sts	SysRepeatTemp1,SysValueCopy
SysRepeatLoop1:
;Source:F1L16S0I16
;Source:F1L104S2I1
	cbi	PORTB,3
;Source:F1L105S2I2
	sbi	PORTB,2
;Source:F1L17S0I17
	rcall	GLCDCLEAR
;Source:F1L18S0I18
	ldi	SysWaitTempMS,244
	ldi	SysWaitTempMS_H,1
	rcall	Delay_MS
;Source:F1L20S0I20
;Source:F1L109S3I1
	sbi	PORTB,3
;Source:F1L110S3I2
	cbi	PORTB,2
;Source:F1L21S0I21
	rcall	GLCDCLEAR
;Source:F1L22S0I22
	ldi	SysWaitTempMS,244
	ldi	SysWaitTempMS_H,1
	rcall	Delay_MS
;Source:F1L24S0I24
;Source:F1L104S2I1
	cbi	PORTB,3
;Source:F1L105S2I2
	sbi	PORTB,2
;Source:F1L25S0I25
	rcall	GLCDCLEAR
;Source:F1L26S0I26
	rcall	GLCDDRAWFRAME
;Source:F1L27S0I27
	ldi	SysValueCopy,1
	sts	GLCDP_X,SysValueCopy
	ldi	SysValueCopy,1
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
;Source:F1L28S0I28
	ldi	SysStringB,low(SYSSTRINGPARAM1)
	ldi	SysStringB_H,high(SYSSTRINGPARAM1)
	ldi	SysReadA,low(StringTable2<<1)
	ldi	SysReadA_H,high(StringTable2<<1)
	rcall	SysReadString
	ldi	SysValueCopy,low(SYSSTRINGPARAM1)
	sts	SysGLCDSTRDATHandler,SysValueCopy
	ldi	SysValueCopy,high(SYSSTRINGPARAM1)
	sts	SysGLCDSTRDATHandler_H,SysValueCopy
	rcall	GLCDPRINT9
;Source:F1L29S0I29
	ldi	SysWaitTempMS,244
	ldi	SysWaitTempMS_H,1
	rcall	Delay_MS
;Source:F1L31S0I31
;Source:F1L109S3I1
	sbi	PORTB,3
;Source:F1L110S3I2
	cbi	PORTB,2
;Source:F1L32S0I32
	rcall	GLCDCLEAR
;Source:F1L33S0I33
	rcall	GLCDDRAWFRAME
;Source:F1L34S0I34
	rcall	GLCDDRAWFRAME
;Source:F1L35S0I35
	ldi	SysValueCopy,1
	sts	GLCDP_X,SysValueCopy
	ldi	SysValueCopy,1
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
;Source:F1L36S0I36
	ldi	SysStringB,low(SYSSTRINGPARAM1)
	ldi	SysStringB_H,high(SYSSTRINGPARAM1)
	ldi	SysReadA,low(StringTable3<<1)
	ldi	SysReadA_H,high(StringTable3<<1)
	rcall	SysReadString
	ldi	SysValueCopy,low(SYSSTRINGPARAM1)
	sts	SysGLCDSTRDATHandler,SysValueCopy
	ldi	SysValueCopy,high(SYSSTRINGPARAM1)
	sts	SysGLCDSTRDATHandler_H,SysValueCopy
	rcall	GLCDPRINT9
;Source:F1L37S0I37
	ldi	SysWaitTempMS,244
	ldi	SysWaitTempMS_H,1
	rcall	Delay_MS
;Source:F1L38S0I38
	lds	SysTemp1,SysRepeatTemp1
	dec	SysTemp1
	sts	SysRepeatTemp1,SysTemp1
	breq	PC + 2
	rjmp	SysRepeatLoop1
SysRepeatLoopEnd1:
;Source:F1L39S0I39
	ldi	SysWaitTempS,1
	rcall	Delay_S
;Source:F1L41S0I41
	ldi	SysValueCopy,0
	sts	CC_X,SysValueCopy
;Source:F1L42S0I42
	ldi	SysValueCopy,0
	sts	C_X,SysValueCopy
;Source:F1L43S0I43
	ldi	SysValueCopy,0
	sts	C_Y,SysValueCopy
;Source:F1L44S0I44
	ldi	SysValueCopy,0
	sts	D_X,SysValueCopy
;Source:F1L45S0I45
	ldi	SysValueCopy,0
	sts	D_Y,SysValueCopy
;Source:F1L47S0I47
SysDoLoop_S1:
;Source:F1L48S0I48
	sbi	PORTB,0
;Source:F1L49S0I49
	rcall	ANIMATE
;Source:F1L50S0I50
	ldi	SysWaitTempMS,200
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
;Source:F1L52S0I52
	cbi	PORTB,0
;Source:F1L53S0I53
	rcall	ANIMATE
;Source:F1L54S0I54
	ldi	SysWaitTempMS,200
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
;Source:F1L55S0I55
	rjmp	SysDoLoop_S1
SysDoLoop_E1:
;Source:F2L11S0I11
;Source:F2L12S0I12
;Source:F2L14S0I14
;Source:F2L15S0I15
;Source:F3L146S0I146
;Source:F3L147S0I147
;Source:F3L151S0I151
;Source:F3L157S0I157
;Source:F3L162S0I162
;Source:F3L164S0I164
;Source:F3L165S0I165
;Source:F3L166S0I166
;Source:F3L167S0I167
;Source:F3L169S0I169
;Source:F3L172S0I172
;Source:F3L173S0I173
;Source:F3L174S0I174
;Source:F3L175S0I175
;Source:F3L179S0I179
;Source:F3L180S0I180
;Source:F3L181S0I181
;Source:F3L182S0I182
;Source:F3L183S0I183
;Source:F3L184S0I184
;Source:F3L185S0I185
;Source:F3L186S0I186
;Source:F3L187S0I187
;Source:F3L188S0I188
;Source:F3L189S0I189
;Source:F3L190S0I190
;Source:F3L191S0I191
;Source:F3L192S0I192
;Source:F3L193S0I193
;Source:F3L194S0I194
;Source:F3L195S0I195
;Source:F3L196S0I196
;Source:F3L197S0I197
;Source:F3L198S0I198
;Source:F3L199S0I199
;Source:F3L200S0I200
;Source:F3L201S0I201
;Source:F3L202S0I202
;Source:F3L203S0I203
;Source:F3L204S0I204
;Source:F3L205S0I205
;Source:F3L206S0I206
;Source:F3L207S0I207
;Source:F3L208S0I208
;Source:F3L209S0I209
;Source:F3L210S0I210
;Source:F3L211S0I211
;Source:F3L212S0I212
;Source:F3L213S0I213
;Source:F3L215S0I215
;Source:F3L216S0I216
;Source:F3L217S0I217
;Source:F3L218S0I218
;Source:F3L219S0I219
;Source:F3L220S0I220
;Source:F3L221S0I221
;Source:F3L222S0I222
;Source:F3L223S0I223
;Source:F3L224S0I224
;Source:F3L225S0I225
;Source:F3L226S0I226
;Source:F3L227S0I227
;Source:F3L228S0I228
;Source:F3L229S0I229
;Source:F3L230S0I230
;Source:F3L231S0I231
;Source:F3L232S0I232
;Source:F3L233S0I233
;Source:F3L234S0I234
;Source:F3L235S0I235
;Source:F3L236S0I236
;Source:F3L237S0I237
;Source:F3L238S0I238
;Source:F3L239S0I239
;Source:F3L240S0I240
;Source:F3L241S0I241
;Source:F3L242S0I242
;Source:F3L243S0I243
;Source:F3L244S0I244
;Source:F3L245S0I245
;Source:F3L246S0I246
;Source:F3L247S0I247
;Source:F3L248S0I248
;Source:F3L249S0I249
;Source:F3L252S0I252
;Source:F3L253S0I253
;Source:F3L254S0I254
;Source:F3L255S0I255
;Source:F3L256S0I256
;Source:F3L257S0I257
;Source:F3L258S0I258
;Source:F3L259S0I259
;Source:F3L260S0I260
;Source:F3L261S0I261
;Source:F3L262S0I262
;Source:F3L263S0I263
;Source:F3L264S0I264
;Source:F3L265S0I265
;Source:F3L267S0I267
;Source:F3L268S0I268
;Source:F3L269S0I269
;Source:F3L270S0I270
;Source:F3L271S0I271
;Source:F3L272S0I272
;Source:F3L273S0I273
;Source:F3L274S0I274
;Source:F3L275S0I275
;Source:F3L276S0I276
;Source:F3L277S0I277
;Source:F3L278S0I278
;Source:F3L279S0I279
;Source:F3L280S0I280
;Source:F3L281S0I281
;Source:F3L282S0I282
;Source:F3L283S0I283
;Source:F3L284S0I284
;Source:F3L285S0I285
;Source:F3L286S0I286
;Source:F3L287S0I287
;Source:F3L288S0I288
;Source:F3L289S0I289
;Source:F3L290S0I290
;Source:F3L291S0I291
;Source:F3L292S0I292
;Source:F3L293S0I293
;Source:F3L294S0I294
;Source:F3L295S0I295
;Source:F3L296S0I296
;Source:F3L297S0I297
;Source:F3L298S0I298
;Source:F3L299S0I299
;Source:F3L300S0I300
;Source:F3L301S0I301
;Source:F3L302S0I302
;Source:F3L305S0I305
;Source:F3L306S0I306
;Source:F3L307S0I307
;Source:F3L308S0I308
;Source:F3L309S0I309
;Source:F3L310S0I310
;Source:F3L311S0I311
;Source:F3L312S0I312
;Source:F3L313S0I313
;Source:F3L314S0I314
;Source:F3L315S0I315
;Source:F3L316S0I316
;Source:F3L317S0I317
;Source:F3L318S0I318
;Source:F3L319S0I319
;Source:F3L320S0I320
;Source:F3L321S0I321
;Source:F3L322S0I322
;Source:F3L323S0I323
;Source:F3L324S0I324
;Source:F3L325S0I325
;Source:F3L326S0I326
;Source:F3L327S0I327
;Source:F3L328S0I328
;Source:F3L329S0I329
;Source:F3L330S0I330
;Source:F3L331S0I331
;Source:F3L332S0I332
;Source:F3L333S0I333
;Source:F3L334S0I334
;Source:F3L335S0I335
;Source:F3L336S0I336
;Source:F3L337S0I337
;Source:F3L338S0I338
;Source:F3L339S0I339
;Source:F3L2063S0I38
;Source:F3L2064S0I39
;Source:F3L2065S0I40
;Source:F3L2066S0I41
;Source:F3L2068S0I43
;Source:F3L2069S0I44
;Source:F4L77S0I77
;Source:F4L78S0I78
;Source:F4L79S0I79
;Source:F4L80S0I80
;Source:F4L82S0I82
;Source:F4L1841S0I1706
;Source:F4L1974S0I51
;Source:F4L4037S0I160
;Source:F5L58S0I58
;Source:F5L59S0I59
;Source:F5L60S0I60
;Source:F5L63S0I63
;Source:F5L64S0I64
;Source:F5L67S0I67
;Source:F5L69S0I69
;Source:F5L118S0I118
;Source:F6L149S0I84
;Source:F7L25S0I25
;Source:F7L26S0I26
;Source:F7L54S0I23
;Source:F8L41S0I41
;Source:F8L42S0I42
;Source:F8L43S0I43
;Source:F8L44S0I44
;Source:F8L45S0I45
;Source:F8L46S0I46
;Source:F8L47S0I47
;Source:F8L49S0I49
;Source:F8L52S0I52
;Source:F8L53S0I53
;Source:F8L54S0I54
;Source:F8L247S0I21
;Source:F10L149S0I149
;Source:F10L152S0I152
;Source:F10L153S0I153
;Source:F10L156S0I156
;Source:F10L157S0I157
;Source:F10L159S0I159
;Source:F10L160S0I160
;Source:F10L162S0I162
;Source:F10L164S0I164
;Source:F10L165S0I165
;Source:F10L166S0I166
;Source:F10L167S0I167
;Source:F10L169S0I169
;Source:F10L170S0I170
;Source:F10L171S0I171
;Source:F10L173S0I173
;Source:F10L177S0I177
;Source:F10L179S0I179
;Source:F10L180S0I180
;Source:F10L181S0I181
;Source:F10L182S0I182
;Source:F10L185S0I185
;Source:F10L186S0I186
;Source:F10L188S0I188
;Source:F10L189S0I189
;Source:F10L190S0I190
;Source:F10L192S0I192
;Source:F10L193S0I193
;Source:F10L195S0I195
;Source:F10L196S0I196
;Source:F10L199S0I199
;Source:F10L200S0I200
;Source:F10L203S0I203
;Source:F10L207S0I207
;Source:F10L208S0I208
;Source:F10L328S0I9
;Source:F10L329S0I10
;Source:F11L34S0I34
;Source:F11L35S0I35
;Source:F11L36S0I36
;Source:F11L37S0I37
;Source:F11L38S0I38
;Source:F11L39S0I39
;Source:F11L40S0I40
;Source:F11L41S0I41
;Source:F12L202S0I202
;Source:F12L224S0I224
;Source:F12L228S0I228
;Source:F12L262S0I262
;Source:F12L263S0I263
;Source:F12L264S0I264
;Source:F12L318S0I318
;Source:F12L319S0I319
;Source:F12L320S0I320
;Source:F12L321S0I321
;Source:F12L322S0I322
;Source:F12L323S0I323
;Source:F12L325S0I325
;Source:F12L326S0I326
;Source:F12L327S0I327
;Source:F12L328S0I328
;Source:F12L329S0I329
;Source:F12L330S0I330
;Source:F12L332S0I332
;Source:F12L333S0I333
;Source:F12L334S0I334
;Source:F12L335S0I335
;Source:F12L336S0I336
;Source:F12L337S0I337
;Source:F12L339S0I339
;Source:F12L340S0I340
;Source:F12L341S0I341
;Source:F12L342S0I342
;Source:F12L343S0I343
;Source:F12L344S0I344
;Source:F12L346S0I346
;Source:F12L347S0I347
;Source:F12L348S0I348
;Source:F12L349S0I349
;Source:F12L350S0I350
;Source:F12L351S0I351
;Source:F12L353S0I353
;Source:F12L354S0I354
;Source:F12L355S0I355
;Source:F12L356S0I356
;Source:F12L357S0I357
;Source:F12L358S0I358
;Source:F12L363S0I363
;Source:F12L364S0I364
;Source:F12L365S0I365
;Source:F12L367S0I367
;Source:F12L368S0I368
;Source:F12L369S0I369
;Source:F12L370S0I370
;Source:F12L372S0I372
;Source:F12L374S0I374
;Source:F12L376S0I376
;Source:F12L377S0I377
;Source:F12L378S0I378
;Source:F12L379S0I379
;Source:F12L380S0I380
;Source:F12L381S0I381
;Source:F12L382S0I382
;Source:F12L383S0I383
;Source:F12L384S0I384
;Source:F12L385S0I385
;Source:F12L386S0I386
;Source:F12L387S0I387
;Source:F12L388S0I388
;Source:F12L389S0I389
;Source:F12L390S0I390
;Source:F12L391S0I391
;Source:F12L393S0I393
;Source:F12L394S0I394
;Source:F12L395S0I395
;Source:F12L396S0I396
;Source:F12L397S0I397
;Source:F12L398S0I398
;Source:F12L399S0I399
;Source:F12L400S0I400
;Source:F12L401S0I401
;Source:F12L402S0I402
;Source:F12L403S0I403
;Source:F12L404S0I404
;Source:F12L405S0I405
;Source:F12L406S0I406
;Source:F12L407S0I407
;Source:F12L408S0I408
;Source:F12L412S0I412
;Source:F12L413S0I413
;Source:F12L414S0I414
;Source:F12L415S0I415
;Source:F12L416S0I416
;Source:F12L417S0I417
;Source:F12L418S0I418
;Source:F12L419S0I419
;Source:F12L420S0I420
;Source:F12L421S0I421
;Source:F12L422S0I422
;Source:F12L423S0I423
;Source:F12L424S0I424
;Source:F12L425S0I425
;Source:F12L426S0I426
;Source:F12L427S0I427
;Source:F12L431S0I431
;Source:F12L432S0I432
;Source:F12L433S0I433
;Source:F12L434S0I434
;Source:F12L435S0I435
;Source:F12L436S0I436
;Source:F12L437S0I437
;Source:F12L438S0I438
;Source:F12L439S0I439
;Source:F12L440S0I440
;Source:F12L441S0I441
;Source:F12L442S0I442
;Source:F12L443S0I443
;Source:F12L503S0I503
;Source:F12L504S0I504
;Source:F12L506S0I506
;Source:F12L507S0I507
;Source:F12L509S0I509
;Source:F12L510S0I510
;Source:F12L512S0I512
;Source:F12L513S0I513
;Source:F12L515S0I515
;Source:F12L516S0I516
;Source:F12L518S0I518
;Source:F12L519S0I519
;Source:F12L531S0I531
;Source:F12L532S0I532
;Source:F12L533S0I533
;Source:F12L534S0I534
;Source:F12L535S0I535
;Source:F12L536S0I536
;Source:F12L537S0I537
;Source:F12L538S0I538
;Source:F12L541S0I541
;Source:F12L542S0I542
;Source:F12L543S0I543
;Source:F12L544S0I544
;Source:F12L547S0I547
;Source:F12L548S0I548
;Source:F12L549S0I549
;Source:F12L550S0I550
;Source:F12L553S0I553
;Source:F12L554S0I554
;Source:F12L555S0I555
;Source:F12L556S0I556
;Source:F12L559S0I559
;Source:F12L560S0I560
;Source:F12L561S0I561
;Source:F12L562S0I562
;Source:F12L673S0I673
;Source:F12L674S0I674
;Source:F12L675S0I675
;Source:F12L676S0I676
;Source:F12L679S0I679
;Source:F12L680S0I680
;Source:F12L681S0I681
;Source:F12L682S0I682
;Source:F13L57S0I57
;Source:F13L58S0I58
;Source:F13L59S0I59
;Source:F13L60S0I60
;Source:F13L63S0I63
;Source:F13L64S0I64
;Source:F13L65S0I65
;Source:F13L66S0I66
;Source:F13L67S0I67
;Source:F13L70S0I70
;Source:F14L42S0I42
;Source:F14L43S0I43
;Source:F14L44S0I44
;Source:F14L45S0I45
;Source:F14L46S0I46
;Source:F14L50S0I50
;Source:F14L51S0I51
;Source:F14L52S0I52
;Source:F14L53S0I53
;Source:F14L56S0I56
;Source:F14L57S0I57
;Source:F14L58S0I58
;Source:F14L59S0I59
;Source:F14L60S0I60
;Source:F14L61S0I61
;Source:F14L62S0I62
;Source:F14L63S0I63
;Source:F14L64S0I64
;Source:F14L66S0I66
;Source:F14L495S0I11
;Source:F15L57S0I57
;Source:F15L64S0I64
;Source:F15L66S0I66
;Source:F15L67S0I67
;Source:F15L68S0I68
;Source:F15L69S0I69
;Source:F15L70S0I70
;Source:F15L71S0I71
;Source:F15L72S0I72
;Source:F16L22S0I22
;Source:F16L25S0I25
;Source:F17L298S0I60
;Source:F18L88S0I88
;Source:F18L91S0I91
;Source:F18L94S0I94
;Source:F19L136S0I136
;Source:F19L137S0I137
;Source:F19L138S0I138
;Source:F19L139S0I139
;Source:F19L143S0I143
;Source:F19L144S0I144
;Source:F19L145S0I145
;Source:F19L146S0I146
;Source:F19L148S0I148
;Source:F19L149S0I149
;Source:F19L150S0I150
;Source:F19L155S0I155
;Source:F19L157S0I157
;Source:F19L158S0I158
;Source:F20L61S0I61
;Source:F20L89S0I89
;Source:F20L92S0I92
;Source:F20L93S0I93
;Source:F20L95S0I95
;Source:F20L96S0I96
;Source:F20L97S0I97
;Source:F20L98S0I98
;Source:F20L99S0I99
;Source:F20L101S0I101
;Source:F20L102S0I102
;Source:F20L103S0I103
;Source:F20L104S0I104
;Source:F20L384S0I19
;Source:F20L470S0I56
;Source:F20L686S0I16
;Source:F20L687S0I17
;Source:F20L688S0I18
;Source:F20L690S0I20
;Source:F20L691S0I21
;Source:F20L692S0I22
;Source:F20L693S0I23
;Source:F20L694S0I24
;Source:F20L695S0I25
;Source:F20L696S0I26
;Source:F20L697S0I27
;Source:F20L698S0I28
;Source:F20L700S0I30
;Source:F20L701S0I31
;Source:F20L702S0I32
;Source:F20L703S0I33
;Source:F20L704S0I34
;Source:F20L705S0I35
;Source:F20L706S0I36
;Source:F20L707S0I37
;Source:F20L708S0I38
;Source:F20L709S0I39
;Source:F20L711S0I41
;Source:F20L713S0I43
;Source:F20L714S0I44
;Source:F20L715S0I45
;Source:F20L717S0I47
;Source:F21L42S0I42
;Source:F21L43S0I43
;Source:F21L44S0I44
;Source:F21L45S0I45
;Source:F21L46S0I46
;Source:F21L47S0I47
;Source:F21L48S0I48
;Source:F21L49S0I49
;Source:F21L50S0I50
;Source:F21L51S0I51
;Source:F21L52S0I52
;Source:F21L53S0I53
;Source:F21L57S0I57
;Source:F21L58S0I58
BASPROGRAMEND:
	sleep
	rjmp	BASPROGRAMEND

;********************************************************************************

ANIMATE:
;Source:F1L58S1I1
	ldi	SysCalcTempA,78
	lds	SysCalcTempB,C_X
	cp	SysCalcTempA,SysCalcTempB
	brsh	ELSE1_1
;Source:F1L59S1I2
;Source:F1L109S3I1
	sbi	PORTB,3
;Source:F1L110S3I2
	cbi	PORTB,2
;Source:F1L60S1I3
	rjmp	ENDIF1
ELSE1_1:
;Source:F1L61S1I4
;Source:F1L104S2I1
	cbi	PORTB,3
;Source:F1L105S2I2
	sbi	PORTB,2
;Source:F1L62S1I5
ENDIF1:
;Source:F1L63S1I6
	lds	SysValueCopy,CC_X
	sts	GLCDP_X,SysValueCopy
	lds	SysValueCopy,C_Y
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
;Source:F1L64S1I7
	ldi	SysStringB,low(SYSSTRINGPARAM2)
	ldi	SysStringB_H,high(SYSSTRINGPARAM2)
	ldi	SysReadA,low(StringTable4<<1)
	ldi	SysReadA_H,high(StringTable4<<1)
	rcall	SysReadString
	ldi	SysValueCopy,low(SYSSTRINGPARAM2)
	sts	SysGLCDSTRDATHandler,SysValueCopy
	ldi	SysValueCopy,high(SYSSTRINGPARAM2)
	sts	SysGLCDSTRDATHandler_H,SysValueCopy
	rcall	GLCDPRINT9
;Source:F1L66S1I9
	lds	SysCalcTempA,D_X
	tst	SysCalcTempA
	brne	ELSE2_1
;Source:F1L67S1I10
	lds	SysTemp1,C_X
	ldi	SysTemp2,6
	add	SysTemp1,SysTemp2
	sts	C_X,SysTemp1
;Source:F1L68S1I11
	rjmp	ENDIF2
ELSE2_1:
;Source:F1L69S1I12
	lds	SysTemp1,C_X
	ldi	SysTemp2,6
	sub	SysTemp1,SysTemp2
	sts	C_X,SysTemp1
;Source:F1L70S1I13
ENDIF2:
;Source:F1L72S1I15
	lds	SysCalcTempA,D_Y
	tst	SysCalcTempA
	brne	ELSE3_1
;Source:F1L73S1I16
	lds	SysTemp1,C_Y
	inc	SysTemp1
	sts	C_Y,SysTemp1
;Source:F1L74S1I17
	rjmp	ENDIF3
ELSE3_1:
;Source:F1L75S1I18
	lds	SysTemp1,C_Y
	dec	SysTemp1
	sts	C_Y,SysTemp1
;Source:F1L76S1I19
ENDIF3:
;Source:F1L78S1I21
	lds	SysCalcTempA,C_Y
	cpi	SysCalcTempA,5
	brne	ENDIF4
;Source:F1L79S1I22
	ldi	SysValueCopy,1
	sts	D_Y,SysValueCopy
;Source:F1L80S1I23
ENDIF4:
;Source:F1L81S1I24
	lds	SysCalcTempA,C_Y
	tst	SysCalcTempA
	brne	ENDIF5
;Source:F1L82S1I25
	ldi	SysValueCopy,0
	sts	D_Y,SysValueCopy
;Source:F1L83S1I26
ENDIF5:
;Source:F1L85S1I28
	lds	SysCalcTempA,C_X
	cpi	SysCalcTempA,162
	brne	ENDIF6
;Source:F1L86S1I29
	ldi	SysValueCopy,1
	sts	D_X,SysValueCopy
;Source:F1L87S1I30
ENDIF6:
;Source:F1L88S1I31
	lds	SysCalcTempA,C_X
	tst	SysCalcTempA
	brne	ENDIF7
;Source:F1L89S1I32
	ldi	SysValueCopy,0
	sts	D_X,SysValueCopy
;Source:F1L90S1I33
ENDIF7:
;Source:F1L92S1I35
	ldi	SysCalcTempA,78
	lds	SysCalcTempB,C_X
	cp	SysCalcTempA,SysCalcTempB
	brsh	ELSE8_1
;Source:F1L93S1I36
	lds	SysTemp1,C_X
	ldi	SysTemp2,84
	sub	SysTemp1,SysTemp2
	sts	CC_X,SysTemp1
;Source:F1L94S1I37
;Source:F1L109S3I1
	sbi	PORTB,3
;Source:F1L110S3I2
	cbi	PORTB,2
;Source:F1L95S1I38
	rjmp	ENDIF8
ELSE8_1:
;Source:F1L96S1I39
	lds	SysValueCopy,C_X
	sts	CC_X,SysValueCopy
;Source:F1L97S1I40
;Source:F1L104S2I1
	cbi	PORTB,3
;Source:F1L105S2I2
	sbi	PORTB,2
;Source:F1L98S1I41
ENDIF8:
;Source:F1L99S1I42
	lds	SysValueCopy,CC_X
	sts	GLCDP_X,SysValueCopy
	lds	SysValueCopy,C_Y
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
;Source:F1L100S1I43
	ldi	SysStringB,low(SYSSTRINGPARAM2)
	ldi	SysStringB_H,high(SYSSTRINGPARAM2)
	ldi	SysReadA,low(StringTable5<<1)
	ldi	SysReadA_H,high(StringTable5<<1)
	rcall	SysReadString
	ldi	SysValueCopy,low(SYSSTRINGPARAM2)
	sts	SysGLCDSTRDATHandler,SysValueCopy
	ldi	SysValueCopy,high(SYSSTRINGPARAM2)
	sts	SysGLCDSTRDATHandler_H,SysValueCopy
	rjmp	GLCDPRINT9

;********************************************************************************

ASCII_TABLE_H:
	cpi	SysStringA, 241
	brlo	PC + 3
	clr	SysByteTempX
	ret
	ldi	SysReadA, low(TableASCII_TABLE_H<<1)
	ldi	SysReadA_H, high(TableASCII_TABLE_H<<1)
	add	SysReadA, SysStringA
	brcc	PC + 2
	inc	SysReadA_H
	lpm
	ret
TableASCII_TABLE_H:
	.DB	240,127,9,9,9,6,62,65,81,33,94,127,9,25,41,70,70,73,73,73,49,1,1,127,1,1,63,64
	.DB	64,64,63,31,32,64,32,31,63,64,56,64,63,99,20,8,20,99,7,8,112,8,7,97,81,73,69,67
	.DB	0,127,65,65,0,2,4,8,16,32,0,65,65,127,0,4,2,1,2,4,64,64,64,64,64,0,1,2,4,0,32,84
	.DB	84,84,120,127,72,68,68,56,56,68,68,68,32,56,68,68,72,127,56,84,84,84,24,8,126,9
	.DB	1,2,12,82,82,82,62,127,8,4,4,120,0,68,125,64,0,32,64,68,61,0,127,16,40,68,0,0
	.DB	65,127,64,0,124,4,24,4,120,124,8,4,4,120,56,68,68,68,56,124,20,20,20,8,8,20,20,24
	.DB	124,124,8,4,4,8,72,84,84,84,32,4,63,68,64,32,60,64,64,32,124,28,32,64,32,28,60,64
	.DB	48,64,60,68,40,16,40,68,12,80,80,80,60,68,100,84,76,68,0,8,54,65,0,0,0,127,0,0
	.DB	0,65,54,8,0,16,8,8,16,8,0,6,9,9,6

;********************************************************************************

ASCII_TABLE_L:
	cpi	SysStringA, 241
	brlo	PC + 3
	clr	SysByteTempX
	ret
	ldi	SysReadA, low(TableASCII_TABLE_L<<1)
	ldi	SysReadA_H, high(TableASCII_TABLE_L<<1)
	add	SysReadA, SysStringA
	brcc	PC + 2
	inc	SysReadA_H
	lpm
	ret
TableASCII_TABLE_L:
	.DB	240,0,0,0,0,0,0,0,95,0,0,0,7,0,7,0,20,127,20,127,20,36,42,127,42,18,35,19,8,100
	.DB	98,54,73,85,34,80,0,5,3,0,0,0,28,34,65,0,0,65,34,28,0,20,8,62,8,20,8,8,62,8,8,0
	.DB	80,48,0,0,8,8,8,8,8,0,96,96,0,0,32,16,8,4,2,62,81,73,69,62,0,66,127,64,0,66,97,81
	.DB	73,70,33,65,69,75,49,24,20,18,127,16,39,69,69,69,57,60,74,73,73,48,1,113,9,5,3,54
	.DB	73,73,73,54,6,73,73,41,30,0,54,54,0,0,0,86,54,0,0,8,20,34,65,0,20,20,20,20,20,0
	.DB	65,34,20,8,2,1,81,9,6,50,73,121,65,62,126,17,17,17,126,127,73,73,73,54,62,65,65,65
	.DB	34,127,65,65,34,28,127,73,73,73,65,127,9,9,9,1,62,65,73,73,122,127,8,8,8,127,0,65
	.DB	127,65,0,32,64,65,63,1,127,8,20,34,65,127,64,64,64,64,127,2,12,2,127,127,4,8,16,127
	.DB	62,65,65,65,62

;********************************************************************************

Delay_MS:
	inc	SysWaitTempMS_H
DMS_START:
	ldi	DELAYTEMP2,254
DMS_OUTER:
	ldi	DELAYTEMP,20
DMS_INNER:
	dec	DELAYTEMP
	brne	DMS_INNER
	dec	DELAYTEMP2
	brne	DMS_OUTER
	dec	SysWaitTempMS
	brne	DMS_START
	dec	SysWaitTempMS_H
	brne	DMS_START
	ret

;********************************************************************************

Delay_S:
DS_START:
	ldi	SysWaitTempMS,232
	ldi	SysWaitTempMS_H,3
	rcall	Delay_MS
	dec	SysWaitTempS
	brne	DS_START
	ret

;********************************************************************************

GLCDCHARACTER:
;Source:F2L63S8I2
	lds	SysCalcTempA,GLCDCHAR
	cpi	SysCalcTempA,80
	brsh	ELSE17_1
;Source:F2L64S8I3
	lds	SysTemp2,GLCDCHAR
	ldi	SysTemp3,32
	sub	SysTemp2,SysTemp3
	mov	SysTemp1,SysTemp2
	mov	SysBYTETempA,SysTemp1
	ldi	SysBYTETempB,5
	mul	SysByteTempA,SysByteTempB
	sts	GLCDCHAR,SysByteTempX
;Source:F2L65S8I4
	ldi	SysValueCopy,5
	sts	SysRepeatTemp4,SysValueCopy
SysRepeatLoop4:
;Source:F2L66S8I5
	lds	SysTemp1,GLCDCHAR
	inc	SysTemp1
	sts	GLCDCHAR,SysTemp1
;Source:F2L67S8I6
	lds	SYSSTRINGA,GLCDCHAR
	rcall	ASCII_TABLE_L
	sts	CHARLINE,SysByteTempX
;Source:F2L68S8I7
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	lds	SysValueCopy,CHARLINE
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	lds	SysValueCopy,GLCDDAT
	sts	CHARLINE,SysValueCopy
;Source:F2L69S8I8
	lds	SysTemp1,SysRepeatTemp4
	dec	SysTemp1
	sts	SysRepeatTemp4,SysTemp1
	brne	SysRepeatLoop4
SysRepeatLoopEnd4:
;Source:F2L70S8I9
	rjmp	ENDIF17
ELSE17_1:
;Source:F2L71S8I10
	lds	SysTemp2,GLCDCHAR
	ldi	SysTemp3,80
	sub	SysTemp2,SysTemp3
	mov	SysTemp1,SysTemp2
	mov	SysBYTETempA,SysTemp1
	ldi	SysBYTETempB,5
	mul	SysByteTempA,SysByteTempB
	sts	GLCDCHAR,SysByteTempX
;Source:F2L72S8I11
	ldi	SysValueCopy,5
	sts	SysRepeatTemp4,SysValueCopy
SysRepeatLoop5:
;Source:F2L73S8I12
	lds	SysTemp1,GLCDCHAR
	inc	SysTemp1
	sts	GLCDCHAR,SysTemp1
;Source:F2L74S8I13
	lds	SYSSTRINGA,GLCDCHAR
	rcall	ASCII_TABLE_H
	sts	CHARLINE,SysByteTempX
;Source:F2L75S8I14
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	lds	SysValueCopy,CHARLINE
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	lds	SysValueCopy,GLCDDAT
	sts	CHARLINE,SysValueCopy
;Source:F2L76S8I15
	lds	SysTemp1,SysRepeatTemp4
	dec	SysTemp1
	sts	SysRepeatTemp4,SysTemp1
	brne	SysRepeatLoop5
SysRepeatLoopEnd5:
;Source:F2L77S8I16
ENDIF17:
;Source:F2L78S8I17
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,0
	sts	GLCDDAT,SysValueCopy
	rjmp	GLCDWRITE

;********************************************************************************

GLCDCLEAR:
;Source:F2L56S7I1
	ldi	SysValueCopy,247
	sts	SysRepeatTemp3,SysValueCopy
	ldi	SysValueCopy,2
	sts	SysRepeatTemp3_H,SysValueCopy
SysRepeatLoop3:
;Source:F2L57S7I2
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,0
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
;Source:F2L58S7I3
	lds	SysTemp1,SysRepeatTemp3
	lds	SysTemp1_H,SysRepeatTemp3_H
	dec	SysTemp1
	sts	SysRepeatTemp3,SysTemp1
	brne	SysRepeatLoop3
	dec	SysTemp1_H
	sts	SysRepeatTemp3_H,SysTemp1_H
	brne	SysRepeatLoop3
SysRepeatLoopEnd3:
	ret

;********************************************************************************

GLCDDRAWFRAME:
;Source:F2L136S15I2
	ldi	SysValueCopy,255
	sts	GLCDINDEX,SysValueCopy
SysForLoop2:
	lds	SysTemp1,GLCDINDEX
	inc	SysTemp1
	sts	GLCDINDEX,SysTemp1
;Source:F2L137S15I3
	lds	SysValueCopy,GLCDINDEX
	sts	GLCDP_X,SysValueCopy
	ldi	SysValueCopy,0
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
;Source:F2L138S15I4
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,1
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
;Source:F2L139S15I5
	lds	SysCalcTempA,GLCDINDEX
	cpi	SysCalcTempA,83
	brlo	SysForLoop2
SysForLoopEnd2:
;Source:F2L140S15I6
	ldi	SysValueCopy,255
	sts	GLCDINDEX,SysValueCopy
SysForLoop3:
	lds	SysTemp1,GLCDINDEX
	inc	SysTemp1
	sts	GLCDINDEX,SysTemp1
;Source:F2L141S15I7
	lds	SysValueCopy,GLCDINDEX
	sts	GLCDP_X,SysValueCopy
	ldi	SysValueCopy,5
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
;Source:F2L142S15I8
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,128
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
;Source:F2L143S15I9
	lds	SysCalcTempA,GLCDINDEX
	cpi	SysCalcTempA,83
	brlo	SysForLoop3
SysForLoopEnd3:
;Source:F2L144S15I10
	ldi	SysValueCopy,255
	sts	GLCDINDEX,SysValueCopy
SysForLoop4:
	lds	SysTemp1,GLCDINDEX
	inc	SysTemp1
	sts	GLCDINDEX,SysTemp1
;Source:F2L145S15I11
	ldi	SysValueCopy,83
	sts	GLCDP_X,SysValueCopy
	lds	SysValueCopy,GLCDINDEX
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
;Source:F2L146S15I12
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,255
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
;Source:F2L147S15I13
	lds	SysCalcTempA,GLCDINDEX
	cpi	SysCalcTempA,5
	brlo	SysForLoop4
SysForLoopEnd4:
;Source:F2L148S15I14
	ldi	SysValueCopy,255
	sts	GLCDINDEX,SysValueCopy
SysForLoop5:
	lds	SysTemp1,GLCDINDEX
	inc	SysTemp1
	sts	GLCDINDEX,SysTemp1
;Source:F2L149S15I15
	ldi	SysValueCopy,0
	sts	GLCDP_X,SysValueCopy
	lds	SysValueCopy,GLCDINDEX
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
;Source:F2L150S15I16
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,255
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
;Source:F2L151S15I17
	lds	SysCalcTempA,GLCDINDEX
	cpi	SysCalcTempA,5
	brlo	SysForLoop5
SysForLoopEnd5:
	ret

;********************************************************************************

GLCDGOTOXY:
;Source:F2L121S13I2
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysTemp2,128
	lds	SysTemp1,GLCDP_X
	or	SysTemp1,SysTemp2
	sts	GLCDDAT,SysTemp1
	rcall	GLCDWRITE
;Source:F2L122S13I3
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysTemp2,64
	lds	SysTemp1,GLCDP_Y
	or	SysTemp1,SysTemp2
	sts	GLCDDAT,SysTemp1
	rjmp	GLCDWRITE

;********************************************************************************

;Overloaded signature: STRING:
GLCDPRINT9:
;Source:F2L83S9I2
	ldi	SysValueCopy,0
	sts	GLCDINDEX,SysValueCopy
	lds	SysStringA,SysGLCDSTRDATHandler
	lds	SysStringA_H,SysGLCDSTRDATHandler_H
	ld	SysCalcTempA,X
	cpi	SysCalcTempA,1
	brlo	SysForLoopEnd1
SysForLoop1:
	lds	SysTemp1,GLCDINDEX
	inc	SysTemp1
	sts	GLCDINDEX,SysTemp1
;Source:F2L84S9I3
	lds	SysTemp1,SysGLCDSTRDATHandler
	lds	SysTemp2,GLCDINDEX
	add	SysTemp1,SysTemp2
	mov	SysStringA,SysTemp1
	lds	SysTemp1,SysGLCDSTRDATHandler_H
	ldi	SysTemp2,0
	adc	SysTemp1,SysTemp2
	mov	SysStringA_H,SysTemp1
	ld	SysValueCopy,X
	sts	GLCDCHAR,SysValueCopy
	rcall	GLCDCHARACTER
	lds	SysTemp1,SysGLCDSTRDATHandler
	lds	SysTemp2,GLCDINDEX
	add	SysTemp1,SysTemp2
	mov	SysStringA,SysTemp1
	lds	SysTemp1,SysGLCDSTRDATHandler_H
	ldi	SysTemp2,0
	adc	SysTemp1,SysTemp2
	mov	SysStringA_H,SysTemp1
	lds	SysValueCopy,GLCDCHAR
	st	X,SysValueCopy
;Source:F2L85S9I4
	lds	SysStringA,SysGLCDSTRDATHandler
	lds	SysStringA_H,SysGLCDSTRDATHandler_H
	lds	SysCalcTempA,GLCDINDEX
	ld	SysCalcTempB,X
	cp	SysCalcTempA,SysCalcTempB
	brlo	SysForLoop1
SysForLoopEnd1:
	ret

;********************************************************************************

GLCDWRITE:
;Source:F2L35S6I2
	lds	SysCalcTempA,GLCDCOMDAT
	cpi	SysCalcTempA,1
	brne	ELSE9_1
;Source:F2L36S6I3
	sbi	PORTB,6
;Source:F2L37S6I4
	rjmp	ENDIF9
ELSE9_1:
;Source:F2L38S6I5
	cbi	PORTB,6
;Source:F2L39S6I6
ENDIF9:
;Source:F2L40S6I7
	ldi	SysValueCopy,8
	sts	SysRepeatTemp2,SysValueCopy
SysRepeatLoop2:
;Source:F2L41S6I8
	cbi	PORTB,4
;Source:F2L43S6I10
	lds	SysBitTest,GLCDDAT
	sbrs	SysBitTest,7
	rjmp	ELSE10_1
;Source:F2L44S6I11
	sbi	PORTB,5
;Source:F2L45S6I12
	rjmp	ENDIF10
ELSE10_1:
;Source:F2L46S6I13
	cbi	PORTB,5
;Source:F2L47S6I14
ENDIF10:
;Source:F2L48S6I15
	lds	SysValueCopy,GLCDDAT
	rol	SysValueCopy
	sts	GLCDDAT,SysValueCopy
;Source:F2L50S6I17
	sbi	PORTB,4
;Source:F2L52S6I19
	lds	SysTemp1,SysRepeatTemp2
	dec	SysTemp1
	sts	SysRepeatTemp2,SysTemp1
	brne	SysRepeatLoop2
SysRepeatLoopEnd2:
	ret

;********************************************************************************

INITGLCD:
;Source:F2L20S5I2
	cbi	PORTB,7
;Source:F2L21S5I3
	ldi	SysWaitTempMS,100
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
;Source:F2L22S5I4
	sbi	PORTB,7
;Source:F2L24S5I6
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,33
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
;Source:F2L25S5I7
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,191
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
;Source:F2L26S5I8
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,4
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
;Source:F2L27S5I9
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,20
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
;Source:F2L28S5I10
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,12
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
;Source:F2L29S5I11
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,32
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
;Source:F2L30S5I12
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,12
	sts	GLCDDAT,SysValueCopy
	rjmp	GLCDWRITE

;********************************************************************************

INITSYS:
;Source:F13L909S179I818
	ldi	SysValueCopy,0
	out	PORTB,SysValueCopy
;Source:F13L912S179I821
	ldi	SysValueCopy,0
	out	PORTC,SysValueCopy
;Source:F13L915S179I824
	ldi	SysValueCopy,0
	out	PORTD,SysValueCopy
	ret

;********************************************************************************

SYSREADSTRING:
;Source:F13L1148S181I2
;Source:F13L1149S181I3
;Source:F13L1280S181I134
;Source:F13L1284S181I138
	lpm
;Source:F13L1285S181I139
	mov	SYSCALCTEMPA, SYSCALCTEMPX
;Source:F13L1286S181I140
	inc	SYSREADA
	brne	PC + 2
	inc	SYSREADA_H
;Source:F13L1287S181I141
	st	Y+, SYSCALCTEMPA
;Source:F13L1288S181I142
	rjmp	SYSSTRINGREADCHECK
SYSREADSTRINGPART:
;Source:F13L1292S181I146
	lpm
;Source:F13L1293S181I147
	mov	SYSCALCTEMPA, SYSCALCTEMPX
;Source:F13L1294S181I148
	inc	SYSREADA
	brne	PC + 2
	inc	SYSREADA_H
;Source:F13L1295S181I149
	add	SYSSTRINGLENGTH, SYSCALCTEMPA
SYSSTRINGREADCHECK:
;Source:F13L1300S181I154
	cpi	SYSCALCTEMPA, 0
;Source:F13L1301S181I155
	brne	SYSSTRINGREAD
;Source:F13L1302S181I156
	ret
SYSSTRINGREAD:
;Source:F13L1309S181I163
	lpm
;Source:F13L1310S181I164
	inc	SYSREADA
	brne	PC + 2
	inc	SYSREADA_H
;Source:F13L1311S181I165
	st	Y+, SYSCALCTEMPX
;Source:F13L1313S181I167
	dec	SYSCALCTEMPA
;Source:F13L1314S181I168
	brne	SYSSTRINGREAD
	ret

;********************************************************************************

SysStringTables:

StringTable2:
.DB	11,32,32,68,105,115,112,108,97,121,32,49


StringTable3:
.DB	11,32,32,68,105,115,112,108,97,121,32,50


StringTable4:
.DB	1,32


StringTable5:
.DB	1,42


;********************************************************************************


