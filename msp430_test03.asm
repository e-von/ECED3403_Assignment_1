;
; MSP430 Assembler Invalid Addressing Test
; This test shows that invalid addressing is detected by the assembler, and a
; diagnostic is printed into the diagnostics file, indicating the error or hinting
; at it.

	label1	equ	1
	label2	equ	64
	label3	equ	3
	stack

ORG 	0x1000

stack	bss 	255

	xor 	label1			; Missing an operand
	and	$0001, @R3		; Invalid DST
	cmp								; Missing operands
	add	stack, label6	; Undeclared label

LOOP	add	label1 ,#8	; Invalid DST
	cmp	label1, label2
	JGE	#1000						; Invalid offset
	cmp	label1, @R4+		; Invalid DST

	rrc	R2(label3)		; Incorrect index
	swpb	5label			; Invalid label
	rra	xor						; Usage of inst as operand
	sxt	!@#$					; Junk chars
	push	stack, R12
	call	LOOP
	reti	LOOP				; Reti is no-op

end
