/* CODE FOR GETCHAR AND ECHO */

.text
.global _start
_start:

.org	0x0000

.equ	JTAG_UART_BASE,		0x10001000 	#address of first JTAG UART register
.equ	DATA_OFFSET,		0			#offset of JTAG UART data register
.equ	STATUS_OFFSET,		4			#offset of JTAG UART status register
.equ	WSPACE_MASK,		0xFFFF		#used in AND operation to check status
.equ	SWITCH,				0x10000040	#mem addr for switch
	
movia sp, 0x007FFFFC #start stack from highest memory address in SDRAM

main:
	
	#initialize stack pointer
	subi	sp, sp, 12
	stw		r2, 8(sp)
	stw		r3, 4(sp)
	stw		r4, 0(sp)
	
	#gets val of switch (0 or 1) and loads into r2
	call SwitchSetting
	
	#calculates ASCII value of 0 or 1 for switch
	addi	r2, r2, '0'
	
	#prints value of r2
	call	PrintChar
	
	loop:
		
		#restores true value of 0 or 1 in r2
		subi	r2, r2, '0'
		#temporarily stores r2 in r4
		mov		r4, r2
		
		if:
			#gets new switch value in r2
			call 	SwitchSetting
			#checks if switch has changed, ends loop if state is the same
			beq		r2, r4, end_if
			#stores new switch value in r4
			mov		r4, r2
		then:
			#moves backspace character into r2
			movi	r2, '\b'
			#prints backspace character to delete old value in JTAG UART
			call PrintChar
			
			#moves stored switch value back to r2
			mov		r2, r4
			#calculates ASCII value of 0 or 1 for switch
			addi	r2, r2, '0'
			#prints new switch setting
			call PrintChar
		end_if:
	
			br loop
	
	ldw		r2, 8(sp)
	ldw		r3, 4(sp)
	ldw		r4, 0(sp)
	addi	sp, sp, 12
	
	br		main
	
	
/* PRINT CHAR SUBROUTINE */
PrintChar:
	subi	sp,	sp,	4
	stw		r3,	4(sp)
	stw		r4,	0(sp)
	movia	r3,	JTAG_UART_BASE

pc_loop:
	ldwio	r4, STATUS_OFFSET(r3)
	andhi	r4,	r4,	WSPACE_MASK
	beq		r4,	r0,	pc_loop
	
	stwio	r2,	DATA_OFFSET(r3) #NEED TO GIVE PRINTCHAR A VALUE FOR R2 !!!
	
	#ldw		r3,	4(sp)
	ldw		r4,	0(sp)
	addi	sp, sp,	4
	ret	
/* PRINT CHAR SUBROUTINE END */

SwitchSetting:

	subi 	sp, sp, 4
	stw		r3, 0(sp)
	
	movia	r3, SWITCH
	ldwio	r2, 0(r3)
	andi 	r2, r2, 0x1
	
	ldw		r3, 0(sp)
	addi 	sp, sp, 4
	ret
	
	
_end:
	br	_end
	
.end

.org	0x1000
CH:		.byte '*'
