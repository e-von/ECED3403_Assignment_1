;
; MSP430 Assembler Instruction Test

; The purpose of this test is to efficiently prove that the assembler
; acknowledges all instructions and addressing modes, and can produce a valid
; s-record output.
; No errors are present in this code, nor does it accomplish some specific task.
;

; Single Instructions
label equ     20
      rrc     R12
      SwPb    label(R14)
      RRA     label2
      sxt     &0x3000
      push    @R12
      call    @R8+
      label2  reti

; Double Instructions
      mov     #-1, R5
      add.b   label, &label2
      addc.w  R5, R6
      subc    @R5+,label(R6)
LOOP  cmp     @R15, label
      DADD    &label2, &$1111

; Jump Instructions
      jne     0x200
      jz      LOOP

end 0x1100
