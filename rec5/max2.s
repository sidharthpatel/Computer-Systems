	.text
.global max2
	.type   max2,@function
max2:

	# YOUR CODE HERE
	movq $0,%rax
	movq %rdi, %rax
	cmpq %rdi, %rax
  ret
	# END YOUR CODE
	
	.size   max2, .-max2
