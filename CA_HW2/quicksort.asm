.data
str1: .asciiz "How many numbers of intergers:\n"
str2: .asciiz "Input integers:\n"
str3: .asciiz "Sorting Result:\n"
str4: .asciiz " "
str5: .asciiz "test"
		
		.text
		.globl main
main:
	    #print "input the number of integers"
		li	$v0,4			   #print_string syscall code = 4	 
		la	$a0,str1 		   #store the address of strg1 in $a0
		syscall

		#read the number of input
		li	$v0,5 	   		   #read_int syscall code = 5, input stored in $v0
		syscall

		#store the number of input
		move  $s0,$v0		   #store the input in $s0 
		
		#creat an sized array according to the input, the address in $v0
	    mul   $t0, $s0, 4      # multiply $s0 by 4 since MIPS is byte address and store into $t0
        move  $a0, $t0         # allocate the size of the array to $a0
        li    $v0, 9           # sbrk,now,$v0 has the address of allocated memory
        syscall

		move $s1,$v0		   #cuz syscall use $v0, move it to $s1 to keep it safe
		
creat_array: #creat an array with input number from user
		
		#print "Input integers:\n"
		li $v0,4 	 		   #print_string syscall code = 4	 
		la $a0,str2 		   #store the address of strg2 in $a0
		syscall

		
		li   $t0,0 		   	   # $t0 set as a counter 
		li   $t1,0 	 		   # offset for the address of array
		move $t2,$s0 		   # move the size into $t2 to keep $s0 safe
		move $t3,$s1 		   # move the address of array into $t3 to keep $s1 safe
		
Input:  #load input into array

        beq $t0,$t2,end_loop1 #if counter = the number of input go to end_loop1

		#input the integer
		li $v0,5     		  #read_int syscall code = 5, input stored in $v0
		syscall		
		
		add	 $t4,$t3,$t1	  #t4 = t3(base address) + t1(offset)
		sw	 $v0,0($t4) 	  #store input to array 
		
		addi $t1,$t1,4 		  #offset  + 4
		addi $t0,$t0,1 		  #counter + 1
		
		j Input				  #jump back to Input

end_loop1: #no code
		
		# if the input number only 1,jump directly to result
		li  $t0,1
		beq $s0,$t0,Result 
		
		li   $s2,0			  # s2(left)  = 0
		addi $t0,$s0,-1		  # t0  = number - 1
		move $s3,$t0		  # s3(right)  = number - 1
		
		jal  quicksort		  #start sorting

Result:

		#print "\nSorting Result:\n"
		li $v0,4		#print_string syscall code = 4	 
		la $a0,str3		#store the address of strg3 in $a0
		syscall

		li $t0,0 	    # $t0 set as a counter 
		li $t1,0 	    # offset for the address of array
		move $t2,$s0    # move the size into $t2 to keep $s0 safe
		move $t3,$s1    # move the address of array into $t3 to keep $s1 safe

Output:
		
		beq  $t0,$t2,end_loop2	#if counter = the number of input go to end_loop1	
		
		add	 $t4,$t3,$t1 		#t4 = t3(base address) + t1(offset)
		lw 	 $a0,0($t4)  		#load word from array into a0
		li	 $v0,1		 		#print_int syscall code = 1
		syscall
		
		#print " "
		li	$v0,4
		la	$a0,str4
		syscall
		
		addi $t1,$t1,4 			#offset  + 4
		addi $t0,$t0,1 			#counter + 1 
		
		j Output	   			#jump back to Output
		
end_loop2:

		li $v0,10
		syscall				
   
quicksort:
		
		#to keep the parameter 
		addi $sp,$sp,-20
		sw   $s2, 16($sp)	  #store left
		sw   $s3, 12($sp)	  #store right
		sw   $a2,  8($sp)	  #store previous left 
		sw   $a3,  4($sp)	  #store previous right	
		sw   $ra,  0($sp)	  #store $ra
		
		bge	 $s2,$s3,j_exit    # if left >= right,go to j_exit 
		add  $t0,$s2,$s3	   # left + right
		div  $t0,$t0,2		   # (left+right)/2
		mul  $t0,$t0,4		   # *4
		add  $t0,$s1,$t0	   # pivot = the medium number,t0 = pivot address
		lw	 $s4,0($t0)        # load pivot number into s4
		move $t1,$s2		   # i = left
		move $t2,$s3		   # j = right

		j    while1			   # jump to while, start partition
		
quicksort_left:

		addi $t1,$t1,-1		   # i-1
		move $a2,$s2		   # keep left in a2 for quicksort_right use 
		move $a3,$s3		   # keep right in a3 for quicksort_right use
		move $s3,$t1		   # i-1 store into right
		jal	 quicksort		   # jump to quicksort to sort the left side of pivot and record the address for quicksort_right
		
quicksort_right:

		addi $t2,$t2,1		   # j+1
		move $s2,$t2		   # left  = j+1
		move $s3,$a3		   # a3 (the right from quicksort_left) store into s3
		jal  quicksort		   # jump to quicksort to sort the right side of pivot
		j    j_exit
		
while1:
		bge  $t1,$t2,quicksort_left		# if i >= j,go to quicksort_left
		
while2:
		mul  $t3,$t1,4			# t3 = t1(i) * 4
		add  $t4,$s1,$t3		# t4 = s1 (base address) + t3 (offset)
		lw	 $s5,0($t4)			# load word a[i] to s5
		bge  $s5,$s4,while3		# if s5 (a[i]) >= s4 (pivot),go to next while3
		addi $t1,$t1,1          # i++
		j    while2
		
while3:
		mul  $t5,$t2,4			# t5 = t2(j) * 4
		add  $t6,$s1,$t5		# t6 = s1 (base address) + t5 (offset)
		lw   $s6,0($t6)		    # load word a[j] to s6
		ble  $s6,$s4,pre_swap   # if s6 (a[j]) <= s4 (pivot),go to pre_swap
		addi $t2,$t2,-1         # j--
		j    while3
		
pre_swap:		

		blt $t1,$t2,makesure   	# if i < j,go to makesure
		j	while1				# jump to while		
		
makesure: #this function is prevent the value store in a[i] and a[j] from exchange for fear that it may cause infinite loop

		bne $s5,$s6,swap		# if a[i] != a[j], jump to swap
		
		# if t1(i) < t0(the place of pivot), then i++, in order to skip the same number with pivot and move on to the next number
		slt $t8,$t1,$t0			
		add $t1,$t1,$t8
		
		# if t2(j) > t0(the place of pivot), then j--, in order to skip the same number with pivot and move on to the next number
		slt $t9,$t0,$t2
		sub $t2,$t2,$t9 
		j   while1
	
swap:
        #to exchange a[i] and a[j]
		lw $t8,0($t4)
		lw $t9,0($t6)
		
		sw $t9,0($t4)		 
		sw $t8,0($t6)
		
		j while1	# after swapping, go back to while1
		
j_exit:
		#restore the parameter from previous loop to let the next loop use
	
		lw   $ra,  0($sp)
		lw   $a3,  4($sp)
		lw   $a2,  8($sp)
		lw   $s3, 12($sp)
		lw   $s2, 16($sp)		
		addi $sp,$sp,20
		
		jr $ra
		


		

