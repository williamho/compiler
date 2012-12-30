
	.comm x,4,4

	.section	.rodata
.LC0:
	.string	"global variable x is %d\012\012"
.LC1:
	.string	"after leaving function, global variable x is still %d\012\012"
.LC2:
	.string	"the function returnval() returned %d\012\012"
.LC3:
	.string	"in function, local var x is %d\012"
.LC4:
	.string	"changing values via pointer indirection\012"
.LC5:
	.string	"in function, local var x is now %d\012\012"
.LC6:
	.string	"fibonacci using a while-loop:\012%d %d "
.LC7:
	.string	"%d "
.LC8:
	.string	"\012\012"
.LC9:
	.string	"example of continues and breaks in for-loop:\012"
.LC10:
	.string	". "
.LC11:
	.string	"%d "
.LC12:
	.string	"\012\012"
.LC13:
	.string	"this function returns the value %d\012"
.LC14:
	.string	"left/right shift example\012"
.LC15:
	.string	"%d in binary is: "
.LC16:
	.string	"%d"
.LC17:
	.string	"\012\012"

	.text
	.globl	main
	.type	main, @function
main:
	pushl %ebp
	movl %esp, %ebp
	subl $40, %esp
.BB1.1:
	# x = MOV 50
	movl $50, %eax
	movl %eax, x

	# ARGBEGIN 2
	subl $12, %esp

	# ARG .LC0
	movl $.LC0, %eax
	movl %eax, 0(%esp)

	# ARG x
	movl x, %eax
	movl %eax, 4(%esp)

	# %T1 = CALL printf
	call printf
	movl %eax, -4(%ebp)

	# %T2 = CALL pointers
	call pointers
	movl %eax, -8(%ebp)

	# ARGBEGIN 2
	subl $12, %esp

	# ARG .LC1
	movl $.LC1, %eax
	movl %eax, 0(%esp)

	# ARG x
	movl x, %eax
	movl %eax, 4(%esp)

	# %T3 = CALL printf
	call printf
	movl %eax, -12(%ebp)

	# %T4 = CALL fibonacci
	call fibonacci
	movl %eax, -16(%ebp)

	# %T5 = CALL continues_and_breaks
	call continues_and_breaks
	movl %eax, -20(%ebp)

	# ARGBEGIN 2
	subl $12, %esp

	# ARG .LC2
	movl $.LC2, %eax
	movl %eax, 0(%esp)

	# %T6 = CALL returnval
	call returnval
	movl %eax, -24(%ebp)

	# x = MOV %T6
	movl -24(%ebp), %eax
	movl %eax, x

	# ARG %T6
	movl -24(%ebp), %eax
	movl %eax, 4(%esp)

	# %T7 = CALL printf
	call printf
	movl %eax, -28(%ebp)

	# %T8 = CALL dec2bin
	call dec2bin
	movl %eax, -32(%ebp)

	# RETURN 1
	movl $1, %eax
	leave
	ret

	leave
	ret
.LFE0:
	.size main, .-main
	.text
	.globl	pointers
	.type	pointers, @function
pointers:
	pushl %ebp
	movl %esp, %ebp
	subl $32, %esp
.BB2.1:
	# %T1 = MOV 0
	movl $0, %eax
	movl %eax, -4(%ebp)

	# ARGBEGIN 2
	subl $12, %esp

	# ARG .LC3
	movl $.LC3, %eax
	movl %eax, 0(%esp)

	# ARG %T1
	movl -4(%ebp), %eax
	movl %eax, 4(%esp)

	# %T2 = CALL printf
	call printf
	movl %eax, -8(%ebp)

	# ARGBEGIN 1
	subl $8, %esp

	# ARG .LC4
	movl $.LC4, %eax
	movl %eax, 0(%esp)

	# %T3 = CALL printf
	call printf
	movl %eax, -12(%ebp)

	# %T5 = LEA %T1
	leal -4(%ebp), %edx
	movl %edx, -20(%ebp)

	# %T4 = MOV %T5
	movl -20(%ebp), %eax
	movl %eax, -16(%ebp)

	# STORE 1234,%T4
	movl -16(%ebp), %eax
	movl $1234, %edx
	movl %edx, (%eax)

	# ARGBEGIN 2
	subl $12, %esp

	# ARG .LC5
	movl $.LC5, %eax
	movl %eax, 0(%esp)

	# ARG %T1
	movl -4(%ebp), %eax
	movl %eax, 4(%esp)

	# %T6 = CALL printf
	call printf
	movl %eax, -24(%ebp)

	# RETURN 1
	movl $1, %eax
	leave
	ret

	leave
	ret
.LFE1:
	.size pointers, .-pointers
	.text
	.globl	fibonacci
	.type	fibonacci, @function
fibonacci:
	pushl %ebp
	movl %esp, %ebp
	subl $216, %esp
.BB3.1:
	# %T17 = LEA %T16
	leal -64(%ebp), %edx
	movl %edx, -68(%ebp)

	# %T19 = MUL 0,4
	movl $4, %eax
	movl $0, %edx
	imull %edx
	movl %eax, -76(%ebp)

	# %T18 = ADD %T17,%T19
	movl -76(%ebp), %eax
	movl -68(%ebp), %edx
	addl %eax, %edx
	movl %edx, -72(%ebp)

	# %T20 = LEA %T16
	leal -64(%ebp), %edx
	movl %edx, -80(%ebp)

	# %T22 = MUL 1,4
	movl $4, %eax
	movl $1, %edx
	imull %edx
	movl %eax, -88(%ebp)

	# %T21 = ADD %T20,%T22
	movl -88(%ebp), %eax
	movl -80(%ebp), %edx
	addl %eax, %edx
	movl %edx, -84(%ebp)

	# STORE 1,%T21
	movl -84(%ebp), %eax
	movl $1, %edx
	movl %edx, (%eax)

	# STORE 1,%T18
	movl -72(%ebp), %eax
	movl $1, %edx
	movl %edx, (%eax)

	# ARGBEGIN 3
	subl $16, %esp

	# ARG .LC6
	movl $.LC6, %eax
	movl %eax, 0(%esp)

	# %T23 = LEA %T16
	leal -64(%ebp), %edx
	movl %edx, -92(%ebp)

	# %T24 = LOAD %T23
	movl -92(%ebp), %eax
	movl (%eax), %edx
	movl %edx, -96(%ebp)

	# ARG %T24
	movl -96(%ebp), %eax
	movl %eax, 4(%esp)

	# %T25 = LEA %T16
	leal -64(%ebp), %edx
	movl %edx, -100(%ebp)

	# %T27 = MUL 1,4
	movl $4, %eax
	movl $1, %edx
	imull %edx
	movl %eax, -108(%ebp)

	# %T26 = ADD %T25,%T27
	movl -108(%ebp), %eax
	movl -100(%ebp), %edx
	addl %eax, %edx
	movl %edx, -104(%ebp)

	# %T28 = LOAD %T26
	movl -104(%ebp), %eax
	movl (%eax), %edx
	movl %edx, -112(%ebp)

	# ARG %T28
	movl -112(%ebp), %eax
	movl %eax, 8(%esp)

	# %T29 = CALL printf
	call printf
	movl %eax, -116(%ebp)

	# %T30 = MOV 2
	movl $2, %eax
	movl %eax, -120(%ebp)

.BB3.2:
	# CMP %T30,15
	movl -120(%ebp), %eax
	cmpl $15, %eax

	# BRLT .BB3.5,.BB3.6
	jl .BB3.5
	jmp .BB3.6

.BB3.3:
	# %T32 = LEA %T16
	leal -64(%ebp), %edx
	movl %edx, -128(%ebp)

	# %T34 = MUL %T30,4
	movl $4, %eax
	movl -120(%ebp), %edx
	imull %edx
	movl %eax, -136(%ebp)

	# %T33 = ADD %T32,%T34
	movl -136(%ebp), %eax
	movl -128(%ebp), %edx
	addl %eax, %edx
	movl %edx, -132(%ebp)

	# %T35 = LEA %T16
	leal -64(%ebp), %edx
	movl %edx, -140(%ebp)

	# %T36 = SUB %T30,1
	movl $1, %eax
	movl -120(%ebp), %edx
	subl %eax, %edx
	movl %edx, -144(%ebp)

	# %T38 = MUL %T36,4
	movl $4, %eax
	movl -144(%ebp), %edx
	imull %edx
	movl %eax, -152(%ebp)

	# %T37 = ADD %T35,%T38
	movl -152(%ebp), %eax
	movl -140(%ebp), %edx
	addl %eax, %edx
	movl %edx, -148(%ebp)

	# %T39 = LOAD %T37
	movl -148(%ebp), %eax
	movl (%eax), %edx
	movl %edx, -156(%ebp)

	# %T40 = LEA %T16
	leal -64(%ebp), %edx
	movl %edx, -160(%ebp)

	# %T41 = SUB %T30,2
	movl $2, %eax
	movl -120(%ebp), %edx
	subl %eax, %edx
	movl %edx, -164(%ebp)

	# %T43 = MUL %T41,4
	movl $4, %eax
	movl -164(%ebp), %edx
	imull %edx
	movl %eax, -172(%ebp)

	# %T42 = ADD %T40,%T43
	movl -172(%ebp), %eax
	movl -160(%ebp), %edx
	addl %eax, %edx
	movl %edx, -168(%ebp)

	# %T44 = LOAD %T42
	movl -168(%ebp), %eax
	movl (%eax), %edx
	movl %edx, -176(%ebp)

	# %T45 = ADD %T39,%T44
	movl -176(%ebp), %eax
	movl -156(%ebp), %edx
	addl %eax, %edx
	movl %edx, -180(%ebp)

	# STORE %T45,%T33
	movl -132(%ebp), %eax
	movl -180(%ebp), %edx
	movl %edx, (%eax)

	# ARGBEGIN 2
	subl $12, %esp

	# ARG .LC7
	movl $.LC7, %eax
	movl %eax, 0(%esp)

	# %T46 = LEA %T16
	leal -64(%ebp), %edx
	movl %edx, -184(%ebp)

	# %T47 = MOV %T30
	movl -120(%ebp), %eax
	movl %eax, -188(%ebp)

	# INC %T30
	incl -120(%ebp)

	# %T49 = MUL %T47,4
	movl $4, %eax
	movl -188(%ebp), %edx
	imull %edx
	movl %eax, -196(%ebp)

	# %T48 = ADD %T46,%T49
	movl -196(%ebp), %eax
	movl -184(%ebp), %edx
	addl %eax, %edx
	movl %edx, -192(%ebp)

	# %T50 = LOAD %T48
	movl -192(%ebp), %eax
	movl (%eax), %edx
	movl %edx, -200(%ebp)

	# ARG %T50
	movl -200(%ebp), %eax
	movl %eax, 4(%esp)

	# %T51 = CALL printf
	call printf
	movl %eax, -204(%ebp)

	# BR .BB3.2
	jmp .BB3.2

.BB3.4:
	# ARGBEGIN 1
	subl $8, %esp

	# ARG .LC8
	movl $.LC8, %eax
	movl %eax, 0(%esp)

	# %T52 = CALL printf
	call printf
	movl %eax, -208(%ebp)

	# RETURN 1
	movl $1, %eax
	leave
	ret

.BB3.5:
	# %T31 = MOV 1
	movl $1, %eax
	movl %eax, -124(%ebp)

	# BR .BB3.7
	jmp .BB3.7

.BB3.6:
	# %T31 = MOV 0
	movl $0, %eax
	movl %eax, -124(%ebp)

	# BR .BB3.7
	jmp .BB3.7

.BB3.7:
	# CMP %T31,0
	movl -124(%ebp), %eax
	cmpl $0, %eax

	# BRNE .BB3.3,.BB3.4
	jne .BB3.3
	jmp .BB3.4

	leave
	ret
.LFE2:
	.size fibonacci, .-fibonacci
	.text
	.globl	continues_and_breaks
	.type	continues_and_breaks, @function
continues_and_breaks:
	pushl %ebp
	movl %esp, %ebp
	subl $52, %esp
.BB4.1:
	# ARGBEGIN 1
	subl $8, %esp

	# ARG .LC9
	movl $.LC9, %eax
	movl %eax, 0(%esp)

	# %T1 = CALL printf
	call printf
	movl %eax, -4(%ebp)

	# %T2 = MOV 16
	movl $16, %eax
	movl %eax, -8(%ebp)

.BB4.2:
	# CMP %T2,0
	movl -8(%ebp), %eax
	cmpl $0, %eax

	# BRGE .BB4.6,.BB4.7
	jge .BB4.6
	jmp .BB4.7

.BB4.3:
	# CMP %T2,8
	movl -8(%ebp), %eax
	cmpl $8, %eax

	# BRGT .BB4.11,.BB4.12
	jg .BB4.11
	jmp .BB4.12

.BB4.4:
	# %T10 = MOV %T2
	movl -8(%ebp), %eax
	movl %eax, -40(%ebp)

	# DEC %T2
	decl -8(%ebp)

	# BR .BB4.2
	jmp .BB4.2

.BB4.5:
	# ARGBEGIN 1
	subl $8, %esp

	# ARG .LC12
	movl $.LC12, %eax
	movl %eax, 0(%esp)

	# %T11 = CALL printf
	call printf
	movl %eax, -44(%ebp)

	# RETURN 1
	movl $1, %eax
	leave
	ret

.BB4.6:
	# %T3 = MOV 1
	movl $1, %eax
	movl %eax, -12(%ebp)

	# BR .BB4.8
	jmp .BB4.8

.BB4.7:
	# %T3 = MOV 0
	movl $0, %eax
	movl %eax, -12(%ebp)

	# BR .BB4.8
	jmp .BB4.8

.BB4.8:
	# CMP %T3,0
	movl -12(%ebp), %eax
	cmpl $0, %eax

	# BRNE .BB4.3,.BB4.5
	jne .BB4.3
	jmp .BB4.5

.BB4.9:
	# ARGBEGIN 1
	subl $8, %esp

	# ARG .LC10
	movl $.LC10, %eax
	movl %eax, 0(%esp)

	# %T7 = CALL printf
	call printf
	movl %eax, -28(%ebp)

	# BR .BB4.4
	jmp .BB4.4

	# BR .BB4.10
	jmp .BB4.10

.BB4.10:
	# CMP %T2,3
	movl -8(%ebp), %eax
	cmpl $3, %eax

	# BRLT .BB4.16,.BB4.17
	jl .BB4.16
	jmp .BB4.17

.BB4.11:
	# %T4 = MOV 1
	movl $1, %eax
	movl %eax, -16(%ebp)

	# BR .BB4.13
	jmp .BB4.13

.BB4.12:
	# %T4 = MOV 0
	movl $0, %eax
	movl %eax, -16(%ebp)

	# BR .BB4.13
	jmp .BB4.13

.BB4.13:
	# %T5 = MOD %T2,2
	movl $0, %edx
	movl -8(%ebp), %eax
	movl $2, %ecx
	idivl %ecx
	movl %edx, -20(%ebp)

	# %T6 = LOGAND %T4,%T5
	movl -16(%ebp), %edx
	movl -20(%ebp), %ecx
	testl %edx, %edx
	setne %al
	xorl %edx, %edx
	testl %ecx, %ecx
	setne %dl
	andl %edx, %eax
	andl $1, %eax
	movl %eax, -24(%ebp)

	# CMP %T6,0
	movl -24(%ebp), %eax
	cmpl $0, %eax

	# BRNE .BB4.9,.BB4.10
	jne .BB4.9
	jmp .BB4.10

.BB4.14:
	# BR .BB4.5
	jmp .BB4.5

	# BR .BB4.15
	jmp .BB4.15

.BB4.15:
	# ARGBEGIN 2
	subl $12, %esp

	# ARG .LC11
	movl $.LC11, %eax
	movl %eax, 0(%esp)

	# ARG %T2
	movl -8(%ebp), %eax
	movl %eax, 4(%esp)

	# %T9 = CALL printf
	call printf
	movl %eax, -36(%ebp)

	# BR .BB4.4
	jmp .BB4.4

.BB4.16:
	# %T8 = MOV 1
	movl $1, %eax
	movl %eax, -32(%ebp)

	# BR .BB4.18
	jmp .BB4.18

.BB4.17:
	# %T8 = MOV 0
	movl $0, %eax
	movl %eax, -32(%ebp)

	# BR .BB4.18
	jmp .BB4.18

.BB4.18:
	# CMP %T8,0
	movl -32(%ebp), %eax
	cmpl $0, %eax

	# BRNE .BB4.14,.BB4.15
	jne .BB4.14
	jmp .BB4.15

	leave
	ret
.LFE3:
	.size continues_and_breaks, .-continues_and_breaks
	.text
	.globl	returnval
	.type	returnval, @function
returnval:
	pushl %ebp
	movl %esp, %ebp
	subl $16, %esp
.BB5.1:
	# ARGBEGIN 2
	subl $12, %esp

	# ARG .LC13
	movl $.LC13, %eax
	movl %eax, 0(%esp)

	# %T1 = MOV 21475
	movl $21475, %eax
	movl %eax, -4(%ebp)

	# ARG 21475
	movl $21475, %eax
	movl %eax, 4(%esp)

	# %T2 = CALL printf
	call printf
	movl %eax, -8(%ebp)

	# RETURN %T1
	movl -4(%ebp), %eax
	leave
	ret

	# RETURN 1
	movl $1, %eax
	leave
	ret

	leave
	ret
.LFE4:
	.size returnval, .-returnval
	.text
	.globl	dec2bin
	.type	dec2bin, @function
dec2bin:
	pushl %ebp
	movl %esp, %ebp
	subl $48, %esp
.BB6.1:
	# ARGBEGIN 1
	subl $8, %esp

	# ARG .LC14
	movl $.LC14, %eax
	movl %eax, 0(%esp)

	# %T1 = CALL printf
	call printf
	movl %eax, -4(%ebp)

	# ARGBEGIN 2
	subl $12, %esp

	# ARG .LC15
	movl $.LC15, %eax
	movl %eax, 0(%esp)

	# ARG x
	movl x, %eax
	movl %eax, 4(%esp)

	# %T2 = CALL printf
	call printf
	movl %eax, -8(%ebp)

	# %T4 = SHL 1,16
	movl %ebx, %edx
	movl $1, %ebx
	movl $16, %ecx
	sall %cl, %ebx
	movl %ebx, -16(%ebp)
	movl %edx, %ebx

	# %T3 = MOV %T4
	movl -16(%ebp), %eax
	movl %eax, -12(%ebp)

.BB6.2:
	# CMP %T3,0
	movl -12(%ebp), %eax
	cmpl $0, %eax

	# BRGT .BB6.6,.BB6.7
	jg .BB6.6
	jmp .BB6.7

.BB6.3:
	# ARGBEGIN 2
	subl $12, %esp

	# ARG .LC16
	movl $.LC16, %eax
	movl %eax, 0(%esp)

	# %T6 = AND %T3,x
	movl -12(%ebp), %eax
	andl x, %eax
	movl %eax, -24(%ebp)

	# CMP %T6,0
	movl -24(%ebp), %eax
	cmpl $0, %eax

	# BRNE .BB6.9,.BB6.10
	jne .BB6.9
	jmp .BB6.10

.BB6.4:
	# %T9 = SHR %T3,1
	movl %ebx, %edx
	movl -12(%ebp), %ebx
	movl $1, %ecx
	sarl %cl, %ebx
	movl %ebx, -36(%ebp)
	movl %edx, %ebx

	# %T3 = MOV %T9
	movl -36(%ebp), %eax
	movl %eax, -12(%ebp)

	# BR .BB6.2
	jmp .BB6.2

.BB6.5:
	# ARGBEGIN 1
	subl $8, %esp

	# ARG .LC17
	movl $.LC17, %eax
	movl %eax, 0(%esp)

	# %T10 = CALL printf
	call printf
	movl %eax, -40(%ebp)

	# RETURN 1
	movl $1, %eax
	leave
	ret

.BB6.6:
	# %T5 = MOV 1
	movl $1, %eax
	movl %eax, -20(%ebp)

	# BR .BB6.8
	jmp .BB6.8

.BB6.7:
	# %T5 = MOV 0
	movl $0, %eax
	movl %eax, -20(%ebp)

	# BR .BB6.8
	jmp .BB6.8

.BB6.8:
	# CMP %T5,0
	movl -20(%ebp), %eax
	cmpl $0, %eax

	# BRNE .BB6.3,.BB6.5
	jne .BB6.3
	jmp .BB6.5

.BB6.9:
	# %T7 = MOV 1
	movl $1, %eax
	movl %eax, -28(%ebp)

	# BR .BB6.11
	jmp .BB6.11

.BB6.10:
	# %T7 = MOV 0
	movl $0, %eax
	movl %eax, -28(%ebp)

	# BR .BB6.11
	jmp .BB6.11

.BB6.11:
	# ARG %T7
	movl -28(%ebp), %eax
	movl %eax, 4(%esp)

	# %T8 = CALL printf
	call printf
	movl %eax, -32(%ebp)

	# BR .BB6.4
	jmp .BB6.4

	leave
	ret
.LFE5:
	.size dec2bin, .-dec2bin

