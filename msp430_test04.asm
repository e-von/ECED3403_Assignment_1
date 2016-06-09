;
; MSP430 Assembler Boundary Testing
; This test will look at how the assembler detects and warns about using values
; beyong the machine's limits. This will include location counter testing, value
; storage tests and usage of negative numbers in cases where only positives are
; accepted
;

org 65510

label0  equ 0xFFFF
label1  equ 0xFFFFF ; Maximum accepted 16 bits
label2  equ $FF00
longlabelnamesnotaddedtosymboltbl equ 5

data0 bss   10      ; Increment LC by 10
data1 bss   30      ; LC is not incremented
data2 bss   10      ; LC now is incremented that we are incrementing below max

data3 byte 256      ; Invalid max byte
data4 byte  -1      ; Invalid negative byte

data5 word $fffff   ; Invalid max word
data5 word -1       ; Invalid negative word

stack bss -1111     ; Invalid negative bss

swpb  label0          ; LC = 65530 + 4
mov   label0, label1  ; Cannot increment LC, will store the new one for diagnostics
and   label1, label2  ; Cannot increment LC
add   &data1, label1  ; LC not incremented

end -100000         ; There should be no limitations on the end address
