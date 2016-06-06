;
; MSP430 Assembler Directive Test
; This test is to show that all directives for the MSP430 are supported with
; in legal usage situations.
;

ascii "This is a directive test. Let's make LC odd"
align
label1  bss  4
label2  byte 2

label3  equ  3
org     0x3334
word    $1234

end 0x1000
