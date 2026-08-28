.global _start
_start:
	bl main
	mov x8, #93
	svc #0
factorial:
	stp x29, x30, [sp, #-16]!
	mov x29, sp
	sub sp, sp, #16
	str x0, [x29, #-8]
	mov x0, #1
	str x0, [x29, #-16]
.Lstart1:
	ldr x0, [x29, #-8]
	str x0, [sp, #-16]!
	mov x0, #1
	ldr x1, [sp], #16
	cmp x1, x0
	b.gt .Ltrue3
	mov x0, #0
	b .Lend4
.Ltrue3:
	mov x0, #1
.Lend4:
	cmp x0, #0
	b.eq .Lend2
	ldr x0, [x29, #-16]
	str x0, [sp, #-16]!
	ldr x0, [x29, #-8]
	ldr x1, [sp], #16
	mul x0, x1, x0
	str x0, [x29, #-16]
	ldr x0, [x29, #-8]
	str x0, [sp, #-16]!
	mov x0, #1
	ldr x1, [sp], #16
	sub x0, x1, x0
	str x0, [x29, #-8]
	b .Lstart1
.Lend2:
	ldr x0, [x29, #-16]
	b .Lend0
.Lend0:
	add sp, sp, #16
	ldp x29, x30, [sp], #16
	ret
main:
	stp x29, x30, [sp, #-16]!
	mov x29, sp
	sub sp, sp, #0
	mov x0, #5
	str x0, [sp, #-16]!
	ldr x0, [sp], #16
	bl factorial
	b .Lend5
.Lend5:
	add sp, sp, #0
	ldp x29, x30, [sp], #16
	ret
