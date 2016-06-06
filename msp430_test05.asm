;
; MSP430 Comprehensive Test
; This is a test which makes use of code that could be encountered in realistic
; assembly usage. No errors should occur and a valid s-record should be emitted.
; A mix of directives, instructions and addressing modes are tested.
; Some code in this test was based on online materials.
;

COUNT equ 2000
      bss 2
PIN3  equ 65500       ; No reason for this. Declating the lacking a physical pin
PIN4  equ 65501           ; Same as above. Just a declaration for use later

mov.w #0x0280, SP         ; Initializing a Stack Pointer, alias for R2
bis.b #0x01, &PIN3        ; OR 0000 00001 with pin 3
bis.b #$20, &PIN4         ; OR 0010 0000  with pin 4

LOOP  xor.b   #1, &PIN4   ; XOR 1 with pin 4, start of loop
      mov.w   #5000, R15  ; mov 5000 to R15, to introduce a delay followingly
L1    sub     #1, R15     ; decrement R15 by 1
      jnz L1              ; jump to L1 if negative
      jmp LOOP            ; else jump to LOOP

L2    mov     3000, R14   ; The commands below are similar to loop above
      cmp     COUNT, R14
L3    sub     #50, R14
      jnz     L3
      jmp     L4

L4    ascii   "The end."
      end
