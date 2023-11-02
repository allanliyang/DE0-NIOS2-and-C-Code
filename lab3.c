/*==================================================================

	Single file C code used for the CPUlator sim.

  	The .h header file contents are positioned at the beginning of
   	this template file, and the exception_handler.c file contents
   	are positioned at the end of this template file. There are
   	comments with "----" dividers and original-file identification
   	to aid in understanding the different portions.

   	In the middle of this template file is the portion that can be
   	customized for a particular application. For interrupt support,
   	there is an empty interrupt_handler() function. There is an
   	empty Init() function and a simple main() function. Other
   	functions can be added as necessary in front of main().

==================================================================*/

/*-----------------------------------------------------------------*/
/*  Not all of the possible control-register accesses are covered by
   	the macros below. The ones most relevant for ELEC 371 laboratory work
   	are included. If access to the other control registers is required,
   	additional macros could easily be introduced, based on the existing ones.
*/
// nio2_control.h
// SPECIAL INSTRUCTIONS TO WRITE TO CONTROL REGISTERS
#ifndef _NIOS2_CONTROL_H_
#define _NIOS2_CONTROL_H_


#define NIOS2_WRITE_STATUS(value)  (__builtin_wrctl (0, value))

#define NIOS2_READ_IENABLE()	   (__builtin_rdctl (3))

#define NIOS2_WRITE_IENABLE(value) (__builtin_wrctl (3, value))

#define NIOS2_READ_IPENDING()	   (__builtin_rdctl (4))


#endif /* _NIOS2_CONTROL_H_ */

	
/*-----------------------------------------------------------------*/
// timer.h
// equivalent implementation of the timer header
	
#ifndef _TIMER_H_
#define _TIMER_H_


/* define pointer macros for accessing the timer interface registers */

#define TIMER_STATUS	((volatile unsigned int *) 0x10002000)

#define TIMER_CONTROL	((volatile unsigned int *) 0x10002004)

#define TIMER_START_LO	((volatile unsigned int *) 0x10002008)

#define TIMER_START_HI	((volatile unsigned int *) 0x1000200C)

#define TIMER_SNAP_LO	((volatile unsigned int *) 0x10002010)

#define TIMER_SNAP_HI	((volatile unsigned int *) 0x10002014)

/* bit pattern reflecting the position of the timeout(TO) bit 
	in the timer status register */

#define TIMER_TO_BIT 0x1

#endif /* _TIMER_H_ */


/*-----------------------------------------------------------------*/
// LEDS header
// equivalent implementation of the LEDs header
	
#ifndef _LEDS_H_
#define _LEDS_H_


/* define pointer macro for accessing the LED interface data register */

#define LEDS	((volatile unsigned int *) 0x10000010)

#endif /* _LEDS_H_ */

	

/*-----------------------------------------------------------------*/
/*             start of application-specific code                  */
/*-----------------------------------------------------------------*/

/* place additional #define macros here */
#define BUTTON_DATA ((volatile unsigned int*) 0x10000050)

/* define global program variables here */
int flag;

void interrupt_handler(void)
{
	unsigned int ipending;


	/* read current value in ipending register */

	/* do one or more checks for different sources using ipending value */

        /* remember to clear interrupt sources */
}

// initial setup function
void Init (void)
{
	/* initialize software variables */
	flag = 0;
	
	/* set up each hardware interface */
	
	// STORE TIMER HI AND LO VALUES INTO MEM0RY
	*TIMER_START_HI = 25000000 >> 16;
	*TIMER_START_LO = 25000000 & 0xFFFF;
	*TIMER_CONTROL = 0x7;
	
	// CLEAR EXTRANEOUS INTERRUPT
	*TIMER_STATUS = TIMER_TO_BIT;
	
	
	/* set up ienable */
	
	// WRITE VALUE IN IENABLE TO "ienable" REGISTER
	NIOS2_WRITE_IENABLE(0x3);
	
	/* enable global recognition of interrupts in procr. status reg. */
	NIOS2_WRITE_STATUS(0x1);
	
}


/* place additional functions here */


int main (void)
{
	
	Init();	//perform initialization
	
	//main loop of program
	while (1)
	{
		
		
		
	}

	return 0;	/* never reached, but main() must return a value */
}

/*-----------------------------------------------------------------*/
/*              end of application-specific code                   */
/*-----------------------------------------------------------------*/



/*-----------------------------------------------------------------*/
/* 
   exception_handler.c

   This file is a portion of the original code supplied by Altera.

   It has been adapted by N. Manjikian for use in ELEC 371 laboratory work.

   Various unnecessary or extraneous elements have been excluded. For
   example, declarations in C for external functions called from asm()
   instructions are not required because any reference to external names
   in asm() instructions is embedded directly in the output written to
   the assembly-language .s file without any other checks by the C compiler.

   There is one particularly important change: on _reset_, the jump must be
   to the >> _start << location in order to properly initialize the stack
   pointer and to perform other crucial initialization tasks that ensure
   proper C semantics for variable initialization are enforced. The Altera
   version of the code jumped to main(), which will _not_ perform these
   crucial initialization tasks correctly.

   Finally, a reference to control register 'ctl4' in the asm() sequence
   has been replaced with the more meaningful alias 'ipending' for clarity.

   Other than the changes described above, the file contents have also been
   reformatted to fit in 80 columns of text, and comments have been edited.
*/


/* The assembly language code below handles processor reset */
void the_reset (void) __attribute__ ((section (".reset")));

/*****************************************************************************
 * Reset code. By giving the code a section attribute with the name ".reset" *
 * we allow the linker program to locate this code at the proper reset vector*
 * address. This code jumps to _startup_ code for C program, _not_ main().   *
 *****************************************************************************/

void the_reset (void)
{
  asm (".set noat");         /* the .set commands are included to prevent */
  asm (".set nobreak");      /* warning messages from the assembler */
  asm ("movia r2, _start");  /* jump to the C language _startup_ code */
  asm ("jmp r2");            /* (_not_ main, as in the original Altera file) */
}

/* The assembly language code below handles exception processing. This
 * code should not be modified; instead, the C language code in the normal
 * function interrupt_handler() [which is called from the code below]
 * can be modified as needed for a given application.
 */

void the_exception (void) __attribute__ ((section (".exceptions")));

/*****************************************************************************
 * Exceptions code. By giving the code a section attribute with the name     *
 * ".exceptions" we allow the linker program to locate this code at the      *
 * proper exceptions vector address. This code calls the interrupt handler   *
 * and later returns from the exception to the main program.                 *
 *****************************************************************************/

void the_exception (void)
{
  asm (".set noat");         /* the .set commands are included to prevent */
  asm (".set nobreak");      /* warning messages from the assembler */
  asm ("subi sp, sp, 128");
  asm ("stw  et, 96(sp)");
  asm ("rdctl et, ipending"); /* changed 'ctl4' to 'ipending' for clarity */
  asm ("beq  et, r0, SKIP_EA_DEC");   /* Not a hardware interrupt, */
  asm ("subi ea, ea, 4");             /* so decrement ea by one instruction */ 
  asm ("SKIP_EA_DEC:");
  asm ("stw	r1,  4(sp)"); /* Save all registers */
  asm ("stw	r2,  8(sp)");
  asm ("stw	r3,  12(sp)");
  asm ("stw	r4,  16(sp)");
  asm ("stw	r5,  20(sp)");
  asm ("stw	r6,  24(sp)");
  asm ("stw	r7,  28(sp)");
  asm ("stw	r8,  32(sp)");
  asm ("stw	r9,  36(sp)");
  asm ("stw	r10, 40(sp)");
  asm ("stw	r11, 44(sp)");
  asm ("stw	r12, 48(sp)");
  asm ("stw	r13, 52(sp)");
  asm ("stw	r14, 56(sp)");
  asm ("stw	r15, 60(sp)");
  asm ("stw	r16, 64(sp)");
  asm ("stw	r17, 68(sp)");
  asm ("stw	r18, 72(sp)");
  asm ("stw	r19, 76(sp)");
  asm ("stw	r20, 80(sp)");
  asm ("stw	r21, 84(sp)");
  asm ("stw	r22, 88(sp)");
  asm ("stw	r23, 92(sp)");
  asm ("stw	r25, 100(sp)"); /* r25 = bt (r24 = et, saved above) */
  asm ("stw	r26, 104(sp)"); /* r26 = gp */
  /* skip saving r27 because it is sp, and there is no point in saving sp */
  asm ("stw	r28, 112(sp)"); /* r28 = fp */
  asm ("stw	r29, 116(sp)"); /* r29 = ea */
  asm ("stw	r30, 120(sp)"); /* r30 = ba */
  asm ("stw	r31, 124(sp)"); /* r31 = ra */
  asm ("addi	fp,  sp, 128"); /* frame pointer adjustment */

  asm ("call	interrupt_handler"); /* call normal function */

  asm ("ldw	r1,  4(sp)"); /* Restore all registers */
  asm ("ldw	r2,  8(sp)");
  asm ("ldw	r3,  12(sp)");
  asm ("ldw	r4,  16(sp)");
  asm ("ldw	r5,  20(sp)");
  asm ("ldw	r6,  24(sp)");
  asm ("ldw	r7,  28(sp)");
  asm ("ldw	r8,  32(sp)");
  asm ("ldw	r9,  36(sp)");
  asm ("ldw	r10, 40(sp)");
  asm ("ldw	r11, 44(sp)");
  asm ("ldw	r12, 48(sp)");
  asm ("ldw	r13, 52(sp)");
  asm ("ldw	r14, 56(sp)");
  asm ("ldw	r15, 60(sp)");
  asm ("ldw	r16, 64(sp)");
  asm ("ldw	r17, 68(sp)");
  asm ("ldw	r18, 72(sp)");
  asm ("ldw	r19, 76(sp)");
  asm ("ldw	r20, 80(sp)");
  asm ("ldw	r21, 84(sp)");
  asm ("ldw	r22, 88(sp)");
  asm ("ldw	r23, 92(sp)");
  asm ("ldw	r24, 96(sp)");
  asm ("ldw	r25, 100(sp)");
  asm ("ldw	r26, 104(sp)");
  /* skip r27 because it is sp, and we did not save this on the stack */
  asm ("ldw	r28, 112(sp)");
  asm ("ldw	r29, 116(sp)");
  asm ("ldw	r30, 120(sp)");
  asm ("ldw	r31, 124(sp)");

  asm ("addi	sp,  sp, 128");

  asm ("eret"); /* return from exception */

  /* Note that the C compiler will still generate the 'standard'
     end-of-normal-function code with a normal return-from-subroutine
     instruction. But with the above eret instruction embedded
     in the final output from the compiler, that end-of-function code
     will never be executed.
   */ 
}