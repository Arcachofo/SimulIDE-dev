   org 0 ; Reset vector
   sjmp Start
   org 30H     ; Code starts here
   
Start:
   mov P3,#0FFH    ; P3 as input
   
while:
   jnb P3.1,off    ; If the switch is on
   clr P1.0     ; Clear LED pin ( turn off LED )
   sjmp on
   
off:
   setb P1.0     ; Set LED pin ( turn on LED )
   
on:
   sjmp while    ; do it again ( Forever loop )
   
end


;----------------------------------------------------
Again:    
    Mov p3, #0ffh  ; initialize port P3 as input port

Loop:   
    Mov C, P3.1
    ;Mov a, P0.1       ; get the data in acc
    ;Cjne a, #0FFh, dat  ; compare it with FFh
    Sjmp loop           ; if same keep looping

Dat:      
    acall multi;                    if different call subroutine
    Sjmp again                     ; again start polling

Multi:
    Mov b,#10d                 ; load 10d in register B
    Mul ab                         ; multiply it with received data
    Mov p1, a                    ; send the result to P1
    Ret                               ;return to main program     
;----------------------------------------------------


    MOV P0,#83H ; Initializing push button switches and initializing LED in OFF state.
    
READSW: 
    MOV A,P0    ; Moving the port value to Accumulator.
    RRC A       ; Checking the vale of Port 0 to know if switch 1 is ON or not
    JC NXT      ; If switch 1 is OFF then jump to NXT to check if switch 2 is ON
    CLR P0.7    ; Turn ON LED because Switch 1 is ON
    SJMP READSW ; Read switch status again.
    
NXT: 
    RRC A       ; Checking the value of Port 0 to know if switch 2 is ON or not
    JC READSW   ; Jumping to READSW to check status of switch 1 again (provided switch 2 is OFF)
    SETB P0.7   ; Turning OFF LED because Switch 2 is ON
    SJMP READSW ; Jumping to READSW to read status of switch 1 again.
    
END

