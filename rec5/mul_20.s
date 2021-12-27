	.text
.global mul_20
	.type   mul_20,@function
mul_20:

	# YOUR CODE HERE
	movq %rdi, %rax
	shlq $2, %rdi
	shlq $4, %rax
	add %rdi, %rax
	ret
	# END YOUR CODE
	
	.size   mul_20, .-mul_20
