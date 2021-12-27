	.text
.global add3
	.type   add3,@function
add3:

	# YOUR CODE HERE
	movq %rdi, %rax
	addq %rsi, %rax
	addq %rdx, %rax
	ret
	# END YOUR CODE
	
	.size   add3, .-add3
