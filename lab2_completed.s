.text              # start a code segment (and we will also have data in it)
.global  _start    # export _start symbol for linker 
 
.equ      JTAG_UART_BASE, 0x10001000
.equ      DATA_OFFSET, 0
.equ      STATUS_OFFSET, 4
.equ      WSPACE_MASK, 0xFFFF
 
.equ	TIMER_STATUS, 0x10002000
.equ	TIMER_CONTROL, 0x10002004
.equ	TIMER_START_LO, 0x10002008
.equ	TIMER_START_HI, 0x1000200C
 
# mask/edge registers for pushbutton parallel port
 
.equ      BUTTON_MASK, 0x10000058
.equ      BUTTON_EDGE, 0x1000005C
 
# pattern corresponding to the bit assigned to button1 in the registers above
 
.equ      BUTTON1, 0x10000050  #todo
 
# data register for LED parallel port
 
.equ     LEDS, 		0x10000010

.equ	SevenSeg, 	0x10000020
 
#-----------------------------------------------------------------------------
# Define two branch instructions in specific locations at the start of memory
#-----------------------------------------------------------------------------
 
.org 0x0000 # this is the _reset_ address

_start:
br main # branch to actual start of main() routine 
 
.org 0x0020 # this is the _exception/interrupt_ address

br isr # branch to start of interrupt service routine 
# (rather than placing all of the service code here) 
 
#-----------------------------------------------------------------------------
# The actual program code (incl. service routine) can be placed immediately
# after the second branch above, or another .org directive could be used
# to place the program code at a desired address (e.g., 0x0080). It does not
# matter because the _start symbol defines where execution begins, and the
# branch at that location simply forces execution to continue where desired.
#-----------------------------------------------------------------------------

main:
    movia sp, 0x7FFFFC # initialize stack pointer
    movia r2, Lab2 
    call PrintString
	call Init # call hw/sw initialization subroutine
 
main_loop:
    movia r3, COUNT(r0)
    ldw r3, 0(r3)
    addi r3, r3, 1
    stw r3, COUNT(r0)
	br main_loop
end:
    break

#-----------------------------------------------------------------------------
# This subroutine should encompass preparation of I/O registers as well as
# special processor registers for recognition and processing of interrupt
# requests. Initialization of data variables in memory can also be done here.
#-----------------------------------------------------------------------------
 
Init: # make it modular -- save/restore registers
	subi sp, sp, 8
	stw r2, 4(sp)
	stw r3, 0(sp)
	
	movia	r3, SevenSeg
	movi	r2, HEXBITS
	ldw		r2, 0(r2)
	stwio	r2, 0(r3)
	
	movia r2, BUTTON1
    movia r3, 0xE
    stwio r3,8(r2)
    movia r3, 0xFFFF
    stwio r3,12(r2)
    movia r2,0x02
	
	movia r3, LEDS
    stwio r0, 0(r3)

	
	movia r3, 0x02FA
	srli r3, r3, 1
	movia r2, TIMER_START_HI
	
	stwio r3, 0(r2)
	movia r3, 0xF080

	movia r2, TIMER_START_LO
	
	srli r3, r3, 16
	stwio r3, 0(r2)
	movia r2, TIMER_CONTROL
	
	movi r3, 7
	stwio r3, 0(r2)
	movia r2, TIMER_STATUS
	stwio r0, 0(r2)
	
	movi r2, 0x3
	wrctl	ienable, r2
	
	movi r2, 0x1
	wrctl	status, r2
	
	
	ldw r2, 4(sp)
	ldw r3, 0(sp)
	addi sp, sp, 8
	ret
 
#-----------------------------------------------------------------------------
# The code for the interrupt service routine is below. Note that the branch
# instruction at 0x0020 is executed first upon recognition of interrupts,
# and that branch brings the flow of execution to the code below. Therefore,
# the actual code for this routine can be anywhere in memory for convenience.
# This template involves only hardware-generated interrupts. Therefore, the
# return-address adjustment on the ea register is performed unconditionally.
# Programs with software-generated interrupts must check for hardware sources
# to conditionally adjust the ea register (no adjustment for s/w interrupts).
#-----------------------------------------------------------------------------
 
isr:
	subi sp, sp, 12			
	stw r2, 8(sp)
	stw r3, 4(sp)		# save register values, except ea which
	stw ra, 0(sp)
						#  must be modified for hardware interrupts
	subi ea, ea, 4	# ea adjustment required for h/w interrupts
	
	rdctl r2, ipending		# body of interrupt service routine
							#   (use the proper approach for checking
check_timer:						#    the different interrupt sources)
	andi r3, r2, 0x1
	beq	r3, r0, check_btn
	movia r2, TIMER_STATUS
	stwio r0, 0(r2)
	call UpdateHexDisplay
	
	
check_btn:
	andi r3, r2, 0x2
	beq	r3, r0, end_isr
	movia r2, LEDS(r0)	
	ldwio r3, 0(r2)
	xori r3, r3, 1
	stwio r3, 0(r2)
	
	movia r2, BUTTON_EDGE
	movi r3, 0xFF
    stwio r3, 0(r2)

end_isr:
	ldw ra, 0(sp)
	ldw r3, 4(sp)
	ldw r2, 8(sp)
	addi sp, sp, 12
	
	eret		# interrupt service routines end _differently_

				# than subroutines; execution must return to

				# to point in main program where interrupt

				# request invoked service routine
isr_end:
    eret # interrupt service routines end _differently_
# than subroutines; execution must return to
# to point in main program where interrupt
# request invoked service routine
 
#------------------------------------------------------------------------------
# this is the subroutine for printing strings
# very modular
#------------------------------------------------------------------------------
 
PrintString:
    subi sp,sp, 16
    stw ra, 12(sp)
    stw r2, 8(sp)
    stw r3, 4(sp)
    stw r4, 0(sp)
    mov r3, r2
              
ps_loop:
    ldb r4, 0(r3)
              
ps_if:     
    bgt r4, r0, ps_else
 
ps_then:
    br ps_end_if
              
ps_else:
    mov r2, r4
    call PrintChar
    addi r3, r3, 1
    br ps_loop
              
ps_end_if:
    ldw ra, 12(sp)
    ldw r2, 8(sp)
    ldw r3, 4(sp)
    ldw r4, 0(sp)
    addi sp,sp,16
    ret
	
UpdateHexDisplay:
	subi sp,sp, 12
    stw r2, 8(sp)
    stw r3, 4(sp)
    stw r4, 0(sp)

	movia 	r4, SevenSeg
	ldwio	r2, 0(r4)
	movi	r3, 0xFF
	
	Hex_If:
		beq		r3, r2, Hex_Then
		br		Hex_Else
		
	Hex_Then:
		movia	r2, HEXBITS
		ldw		r2, 0(r2)
		br		Hex_EndIf
		
	Hex_Else:
		srli	r2, r2, 8
	
	Hex_EndIf:
	stwio	r2,0(r4)
	
    ldw r2, 8(sp)
    ldw r3, 4(sp)
    ldw r4, 0(sp)
	addi sp,sp, 12
	
	ret
#------------------------------------------------------------------------------
# this is the subroutine for printing characters
# very modular
#------------------------------------------------------------------------------
 
PrintChar:
    subi sp, sp, 8 # adust stack pointer down to reserve space
    stw r3, 4(sp) # save value of r3 so it can be a temp
    stw r4, 0(sp) # save value of r4 so it can be a temp
    movia r3, JTAG_UART_BASE # point to first memory-mapped I/O register
pc_loop:
    ldwio r4, STATUS_OFFSET(r3) #read bits from status register
    andhi r4, r4, WSPACE_MASK # mask off lower vits to isolate upper bits
    beq r4, r0, pc_loop # if upper bits are zero, loop again
    stwio r2, DATA_OFFSET(r3) # otherwise, write character to data register
    ldw r3, 4(sp) # restore value of r3 from stack
    ldw r4, 0(sp) # restore value of r4 from stack
    addi sp, sp, 8 # readust stack pointer up to deallocate space
    ret # return to calling routine

#-----------------------------------------------------------------------------
# Definitions for program data, incl. anything shared between main/isr code
#-----------------------------------------------------------------------------
 
.org		0x1000 # start should be fine for most small programs
 
COUNT:	.word 	0 #keep the main loop count in memory   
		.skip	8
		
HEXBITS: 	.word 0xFF000000
			.skip 8 
			
Lab2:	.asciz "ELEC 371 Lab 2\n" # define/reserve storage for program data
 
              .end
