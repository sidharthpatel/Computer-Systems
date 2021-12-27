	.text
.global fact
	.type   fact,@function
fact:

	# YOUR CODE HERE
  movq $0, %rax
  ret
	# END YOUR CODE
	
	.size   fact, .-fact
