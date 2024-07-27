
ORG 0000H         ; starting address for the microcontroller 
LJMP MAIN         ; used to bypass the memory which store the interrupt service routine 

ORG  0003H        ; memory location which holds the interrupt service routine for INT0
    SETB  P1.0    ; Set LED pin ( turn on LED )
    CLR  TCON.1   ; Clear INT0 flag
    ACALL delay
    RETI          ; used to exit the interrupt subroutine 
    
ORG 0030H; Memory location where the program code (i.e MAIN) is kept 
MAIN: 
    MOV P3, #0FFH       ; P0 as input
    MOV IE, #10000001B  ; enables interrupts p3.3 edge triggered INT0

    MOV R2,#130         ; moves 130 into r2 which acts as a counter 
    
AGAIN:
    CLR  P1.0     ; Clear LED pin ( turn on LED )
    SJMP AGAIN    ; decrements value in r2 and jumps to again label till value !=0 

delay:
    mov R3,#10    ; 2 clock cycles (call) = 2
    mov R2,#0     ; 2 clock cycles (call) = 2
    mov R1,#0     ; 2 clock cycles (loading) = 2
  d1: 
    djnz R1,d1    ; 2 * 256 clock cycles *225 = 115200
    djnz R2,d1    ; 2 * 225 clock cycles = 450
    djnz R3,d1    ; 2 * 225 clock cycles = 450
    ret           ; 2 clock cycles (return) = 2

END
