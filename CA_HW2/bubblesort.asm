.data
str1: .asciiz "How many numbers of intergers:\n"
str2: .asciiz "Input integers:\n"
str3: .asciiz "Sorting Result:\n"
str4: .asciiz " "
		
		.text
		.globl main
main:

		addi $sp,$sp,-20	   #make room on stack for 5 reg
		sw	 $ra,16($sp)	   #save $ra on stack
		sw	 $s3,12($sp)	   #save $s3 on stack
		sw	 $s2, 8($sp)	   #save $s2 on stack
		sw	 $s1, 4($sp)	   #save $s1 on stack
		sw	 $s0, 0($sp)	   #save $s0 on stach

		#print "input the number of integers"
		li	$v0,4			   #print_string syscall code = 4	 
		la	$a0,str1 		   #store the address of strg1 in $a0
		syscall

		#read the number of input
		li	$v0,5 	   		   #read_int syscall code = 5, input stored in $v0
		syscall

		#store the number of input
		move $s0,$v0		   #store the input in $s0 
		
		#creat an sized array according to the input, the address in $v0
	    mul   $t0, $s0, 4      # multiply $s0 by 4 since MIPS is byte address and store into $t0
        move  $a0, $t0         # allocate the size of the array to $a0
        li    $v0, 9           # sbrk,now,$v0 has the address of allocated memory
        syscall

		move $s1,$v0		   #cuz syscall use $v0, move it to $s1 to keep it safe
		
creat_array: #creat an array with input number from user
		
		#print "Input integers:\n"
		li $v0,4 	 #print_string syscall code = 4	 
		la $a0,str2  #store the address of strg2 in $a0
		syscall

		
		li $t0,0 	 # $t0 set as a counter 
		li $t1,0 	 # offset for the address of array
		move $t2,$s0 # move the size into $t2 to keep $s0 safe
		move $t3,$s1 # move the address of array into $t3 to keep $s1 safe
		
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


#SORTING

		li $s2,0 			   # i = 0
for1tst: 					   # for ( i = 0 ; i < n ; i++ )

		bge  $s2,$s0,endfor1   # if i>=n jump out the loop		
		addi $s3,$s2,-1		   # j = i - 1
		
for2tst:					   # for ( j = i - 1; j >= 0 && v[j] > v[j+1]; j -= 1 ) swap(v,j)

		blt $s3,0,endfor2      # if j <= 0 end loop
		mul $t0,$s3,4		   # t0 = j * 4
		add $t1,$s1,$t0		   # t1 = s1(base address) + t0 (offset)
		lw  $t2,0($t1)		   # t2 = v[j]
		lw  $t3,4($t1)		   # t3 = v[j+1]
		ble $t2,$t3,endfor2    # if v[j] <= v[j+1] end loop
		jal swap			   # do swap
		
		addi $s3,$s3,-1  	   # j -= 1
		j	 for2tst		

endfor2:

		addi $s2,$s2,1		   # i = i + 1
		j    for1tst		   # jump back to fisrt loop

endfor1:		
		
		#print "\nSorting Result:\n"
		li $v0,4		#print_string syscall code = 4	 
		la $a0,str3		#store the address of strg3 in $a0
		syscall

		li $t0,0 	    # $t0 set as a counter 
		li $t1,0 	    # offset for the address of array
		move $t2,$s0    # move the size into $t2 to keep $s0 safe
		move $t3,$s1    # move the address of array into $t3 to keep $s1 safe

Output:
		
		beq $t0,$t2,end_loop2	#if counter = the number of input go to end_loop1	
		
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

		lw $s0, 0($sp)	#restore $s0 from stack
		lw $s1, 4($sp)	#restore $s1 from stack
		lw $s2, 8($sp)	#restore $s2 from stack
		lw $s3,12($sp)	#restore $s3 from stack
		lw $ra,16($sp)	#restore $ra from stack
		addi $sp,$sp,20 #restore stack pointer

		li $v0,10
		syscall
		
swap:

		mul $t0,$s3,4		 # t0 = j * 4
		add $t1,$s1,$t0		 # t1 = s1 (base address) + t0 (offset)
		
		lw  $t2,0($t1)		 # t2 = v[j]
		lw  $t3,4($t1)		 # t3 = v[j+1]
		
		sw  $t3,0($t1)		 # v[j] and v[j+1] exchange
		sw  $t2,4($t1)
		
		jr  $ra

