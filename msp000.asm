;
; MSP-430 assembler code example
; ECED 3403
; 5 May 2016
;
	ORG	0x1000
	MOV	#1,R4    	; R4 = 1
	SWPB	R4      	; Swap bytes
Loop	add	@R7,Data(R5)	; Data[R5] = Data[R5] + [R7]
	ADD	Data(R5),R4	; R4 = R4 + Data[R5]
	ADDC	#1,R5		; R5 = R5 +1 + C
;
	org	0x2000
Data	bss	4       	; 4 bytes associated with Data
;
	END 0x3000
