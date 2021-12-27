	.file	"func1.c"
	.text
	.globl	func1
	.type	func1, @function
func1:
.LFB0:
	.cfi_startproc
	leaq	0(%rdi,%rdi,8), %rax
	ret
	.cfi_endproc
.LFE0:
	.size	func1, .-func1
	.ident	"GCC: (GNU) 6.3.1 20170216 (Red Hat 6.3.1-3)"
	.section	.note.GNU-stack,"",@progbits
