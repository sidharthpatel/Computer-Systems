	.file	"rec5.c"
	.section	.rodata
	.align 8
.LC0:
	.string	"error - not enough arguments.\n\n\tusage: ./rec4 funcname arg1 arg2 arg3 ...\n"
.LC1:
	.string	"%ld"
.LC2:
	.string	"mul_20"
.LC3:
	.string	"%ld\n"
.LC4:
	.string	"add3"
.LC5:
	.string	"max2"
.LC6:
	.string	"sumUpTo"
.LC7:
	.string	"collatzLength"
.LC8:
	.string	"fact"
.LC9:
	.string	"caller"
.LC10:
	.string	"func2"
.LC11:
	.string	"func1"
.LC12:
	.string	"func3"
	.align 8
.LC13:
	.string	"error - unrecognized command '%s'.\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$144, %rsp
	movl	%edi, -132(%rbp)
	movq	%rsi, -144(%rbp)
	cmpl	$2, -132(%rbp)
	jg	.L2
	movl	$.LC0, %edi
	call	puts
	movl	$1, %eax
	jmp	.L15
.L2:
	movq	-144(%rbp), %rax
	addq	$16, %rax
	movq	(%rax), %rax
	leaq	-48(%rbp), %rdx
	movl	$.LC1, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	__isoc99_sscanf
	movq	-144(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	$10, %edx
	movq	%rax, %rsi
	movl	$.LC2, %edi
	call	strncmp
	testl	%eax, %eax
	jne	.L4
	movq	-48(%rbp), %rax
	movq	%rax, %rdi
	call	mul_20
	movq	%rax, %rsi
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	jmp	.L5
.L4:
	movq	-8(%rbp), %rax
	movl	$10, %edx
	movq	%rax, %rsi
	movl	$.LC4, %edi
	call	strncmp
	testl	%eax, %eax
	jne	.L6
	movq	-48(%rbp), %rax
	movq	%rax, -16(%rbp)
	movq	-144(%rbp), %rax
	addq	$24, %rax
	movq	(%rax), %rax
	leaq	-56(%rbp), %rdx
	movl	$.LC1, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	__isoc99_sscanf
	movq	-144(%rbp), %rax
	addq	$32, %rax
	movq	(%rax), %rax
	leaq	-64(%rbp), %rdx
	movl	$.LC1, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	__isoc99_sscanf
	movq	-64(%rbp), %rdx
	movq	-56(%rbp), %rcx
	movq	-16(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	add3
	movq	%rax, %rsi
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	jmp	.L5
.L6:
	movq	-8(%rbp), %rax
	movl	$10, %edx
	movq	%rax, %rsi
	movl	$.LC5, %edi
	call	strncmp
	testl	%eax, %eax
	jne	.L7
	movq	-48(%rbp), %rax
	movq	%rax, -24(%rbp)
	movq	-144(%rbp), %rax
	addq	$24, %rax
	movq	(%rax), %rax
	leaq	-72(%rbp), %rdx
	movl	$.LC1, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	__isoc99_sscanf
	movq	-72(%rbp), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	max2
	movq	%rax, %rsi
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	jmp	.L5
.L7:
	movq	-8(%rbp), %rax
	movl	$10, %edx
	movq	%rax, %rsi
	movl	$.LC6, %edi
	call	strncmp
	testl	%eax, %eax
	jne	.L8
	movq	-48(%rbp), %rax
	movq	%rax, %rdi
	call	sumUpTo
	movq	%rax, %rsi
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	jmp	.L5
.L8:
	movq	-8(%rbp), %rax
	movl	$15, %edx
	movq	%rax, %rsi
	movl	$.LC7, %edi
	call	strncmp
	testl	%eax, %eax
	jne	.L9
	movq	-48(%rbp), %rax
	movq	%rax, %rdi
	call	collatzLength
	movq	%rax, %rsi
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	jmp	.L5
.L9:
	movq	-8(%rbp), %rax
	movl	$10, %edx
	movq	%rax, %rsi
	movl	$.LC8, %edi
	call	strncmp
	testl	%eax, %eax
	jne	.L10
	movq	-48(%rbp), %rax
	movq	%rax, %rdi
	call	fact
	movq	%rax, %rsi
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	jmp	.L5
.L10:
	movq	-8(%rbp), %rax
	movl	$10, %edx
	movq	%rax, %rsi
	movl	$.LC9, %edi
	call	strncmp
	testl	%eax, %eax
	jne	.L11
	movq	-48(%rbp), %rax
	movq	%rax, -32(%rbp)
	movq	-144(%rbp), %rax
	addq	$24, %rax
	movq	(%rax), %rax
	leaq	-80(%rbp), %rdx
	movl	$.LC1, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	__isoc99_sscanf
	movq	-80(%rbp), %rdx
	movq	-32(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	caller
	movq	%rax, %rsi
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	jmp	.L5
.L11:
	movq	-8(%rbp), %rax
	movl	$10, %edx
	movq	%rax, %rsi
	movl	$.LC10, %edi
	call	strncmp
	testl	%eax, %eax
	jne	.L12
	movq	-48(%rbp), %rax
	movq	%rax, -40(%rbp)
	movq	-144(%rbp), %rax
	addq	$24, %rax
	movq	(%rax), %rax
	leaq	-88(%rbp), %rdx
	movl	$.LC1, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	__isoc99_sscanf
	movq	-144(%rbp), %rax
	addq	$32, %rax
	movq	(%rax), %rax
	leaq	-96(%rbp), %rdx
	movl	$.LC1, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	__isoc99_sscanf
	movq	-144(%rbp), %rax
	addq	$40, %rax
	movq	(%rax), %rax
	leaq	-104(%rbp), %rdx
	movl	$.LC1, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	__isoc99_sscanf
	movq	-104(%rbp), %rcx
	movq	-96(%rbp), %rdx
	movq	-88(%rbp), %rsi
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	call	func2
	movq	%rax, %rsi
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	jmp	.L5
.L12:
	movq	-8(%rbp), %rax
	movl	$10, %edx
	movq	%rax, %rsi
	movl	$.LC11, %edi
	call	strncmp
	testl	%eax, %eax
	jne	.L13
	movq	-48(%rbp), %rax
	movq	%rax, %rdi
	call	func1
	movq	%rax, %rsi
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	jmp	.L5
.L13:
	movq	-8(%rbp), %rax
	movl	$10, %edx
	movq	%rax, %rsi
	movl	$.LC12, %edi
	call	strncmp
	testl	%eax, %eax
	jne	.L14
	movq	-48(%rbp), %rax
	movq	%rax, -112(%rbp)
	movq	-144(%rbp), %rax
	addq	$24, %rax
	movq	(%rax), %rax
	leaq	-120(%rbp), %rdx
	movl	$.LC1, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	__isoc99_sscanf
	movq	-144(%rbp), %rax
	addq	$32, %rax
	movq	(%rax), %rax
	leaq	-128(%rbp), %rdx
	movl	$.LC1, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	__isoc99_sscanf
	movq	-128(%rbp), %rdx
	leaq	-120(%rbp), %rcx
	leaq	-112(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	func3
	movq	%rax, %rsi
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	jmp	.L5
.L14:
	movq	-144(%rbp), %rax
	addq	$8, %rax
	movq	(%rax), %rax
	movq	%rax, %rsi
	movl	$.LC13, %edi
	movl	$0, %eax
	call	printf
	movl	$2, %eax
	jmp	.L15
.L5:
	movl	$0, %eax
.L15:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-39)"
	.section	.note.GNU-stack,"",@progbits
