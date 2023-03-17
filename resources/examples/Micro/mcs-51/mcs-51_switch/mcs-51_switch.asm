   org 0 ; Reset vector
   sjmp Start
   org 30H     ; Code starts here
   
Start:
   mov P3,#0FFH    ; P3 as input
while:
   jnb P3.1,off    ; If the switch is on
   clr P1.0        ; Clear LED pin ( turn off LED )
   sjmp on
off:
   setb P1.0       ; Set LED pin ( turn on LED )
on:
   sjmp while      ; do it again ( Forever loop )
end
