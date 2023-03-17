; Compiler: Asem-51

; Program: Software Delays
; Author: Diego Souza (original example by from Prof. Vooi Yap)

; Generating delays via software
; Reference: https://forum.allaboutcircuits.com/threads/delay-calculation-for-8051.52052/

; Crystal frequency: 11.0592 MHz
; The 8051 uses 1/12 of oscilator frequency. So the frequency is: 
; 11.0592 / 12 = 921.6 kHz 
; The cycle execution time becomes 1/f = 1/921.6kHz = 1.085uS per cycle. So the
; Desired time = Total Cycles * 1.085 us

; If we are using an 11.0592 MHz xtal, we will need to consume 
; approx 921 instruction cycles per millisecond of delay.
; 921 * 1.085 us = 0.999285 ms
 
; We could write a routine to consume 92,100 cyc, but that would only 
; generate a delay of 100mSec. The delay subroutine below consumes 
; 922 cyc for each count in R7, so we have a subroutine that can delay 
; between 1 and 255 mSec.

ORG 00H  ; Assembly Starts from 0000H.
Back:
        Clr P1.0     ; Set Port1 bit 0 to zero
        mov R7,#250  ; 250 mSec (we may change this)
        acall delay  ; Call delay subroutine
        acall delay  ; Call delay subroutine
        ;acall delay  ; Call delay subroutine
        ;acall delay  ; Call delay subroutine

        Setb P1.0    ; Set Port1 bit 0 to one
        mov R7,#250  ; 250 mSec (we may change this)
        acall delay  ; Call delay subroutine
        acall delay  ; Call delay subroutine
        acall delay  ; Call delay subroutine
        acall delay  ; Call delay subroutine
        Sjmp Back    ; Restart the loop

delay:  
        mov R2,#230     ; 2 cycles
delay1: 
        nop             ; 1 cycles
        nop             ; 1 cycles
        djnz R2, delay1 ; 2 cycles consume 230x4 + 2 instr cycles = 922 cycles
        djnz R7, delay  ; 922 cycles (which is equal to 1 ms) * number of counts in R7  
        ret             ; Return from subroutine

        End ; End of program