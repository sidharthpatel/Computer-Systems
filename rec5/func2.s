	.file	"func2.c"
	.text
	.globl	func2
	.type	func2, @function
func2:
.LFB0:
	.cfi_startproc
	imulq	%rsi, %rdi
	leaq	(%rdx,%rcx), %rax
	addq	%rdi, %rax
	ret
	.cfi_endproc
.LFE0:
	.size	func2, .-func2
	.ident	"GCC: (GNU) 6.3.1 20170216 (Red Hat 6.3.1-3)"
	.section	.note.GNU-stack,"",@progbits
