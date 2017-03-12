section .bss
	BFTAB:	resb	30000
section .text
global _start
extern putchar
extern getchar

_start:
	push	rbp
	mov	rbp, BFTAB
	add	byte [rbp], 4
	call	getchar
	mov	[rbp], al
	movzx	rdi, byte [rbp]
	call	putchar
	mov	eax, 0
	pop	rbp
	ret
