; Compiler: Asem-51

ORG 0
start:
        clr P1.0
        acall delay
        acall delay

        setb P1.0
        acall delay
        acall delay
        sjmp start

delay:
        mov R7,#255
delay0:
        mov R2,#255
delay1: 
        djnz R2, delay1
        djnz R7, delay0
        ret
End