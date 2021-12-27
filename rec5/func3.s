	.file	"func3.c"
	.text
	.globl	func3
	.type	func3, @function
func3:
.LFB0:
	.cfi_startproc
	movq	(%rdi), %rcx
	movq	(%rsi), %rax
	addq	$1, %rax
	movq	%rax, (%rdi)
	leaq	(%rdx,%rcx,4), %rax
	movq	%rax, (%rsi)
	ret
	.cfi_endproc
.LFE0:
	.size	func3, .-func3
	.ident	"GCC: (GNU) 6.3.1 20170216 (Red Hat 6.3.1-3)"
	.section	.note.GNU-stack,"",@progbits
