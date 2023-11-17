#include "nios2_control.h"
#include "chario.h"
#include "adc.h"

/* place additional #define macros here */


/* define global program variables here */


/* place additional functions here */



/*-----------------------------------------------------------------*/

/* this routine is called from the_exception() in exception_handler.c */

void interrupt_handler(void);

/*-----------------------------------------------------------------*/

#ifndef _NIOS2_CONTROL_H_
#define _NIOS2_CONTROL_H_


// NIOS2_CONTOL HEADER
#define NIOS2_WRITE_STATUS(value)  (__builtin_wrctl (0, value))

#define NIOS2_READ_IENABLE()	   (__builtin_rdctl (3))

#define NIOS2_WRITE_IENABLE(value) (__builtin_wrctl (3, value))

#define NIOS2_READ_IPENDING()	   (__builtin_rdctl (4))


#endif /* _NIOS2_CONTROL_H_ */



/*-----------------------------------------------------------------*/
// TIMER HEADER
#ifndef _TIMER_H_
#define _TIMER_H_

// TIMER 1
#define TIMER1_STATUS	((volatile unsigned int *) 0x10004020)
#define TIMER1_CONTROL	((volatile unsigned int *) 0x10004024)
#define TIMER1_START_LO	((volatile unsigned int *) 0x10004028)
#define TIMER1_START_HI	((volatile unsigned int *) 0x1000402C)
#define TIMER1_SNAP_LO	((volatile unsigned int *) 0x10004030)
#define TIMER1_SNAP_HI	((volatile unsigned int *) 0x10004034)

// TIMER 2
#define TIMER2_STATUS	((volatile unsigned int *) 0x10004040)
#define TIMER2_CONTROL	((volatile unsigned int *) 0x10004044)
#define TIMER2_START_LO	((volatile unsigned int *) 0x10004048)
#define TIMER2_START_HI	((volatile unsigned int *) 0x1000404C)
#define TIMER2_SNAP_LO	((volatile unsigned int *) 0x10004050)
#define TIMER2_SNAP_HI	((volatile unsigned int *) 0x10004054)


/* define a bit pattern reflecting the position of the timeout (TO) bit
   in the timer status register */
#define TIMER_TO_BIT 0x1

#endif /* _TIMER_H_ */


/*-----------------------------------------------------------------*/
// LEDS HEADER
#ifndef _LEDS_H_
#define _LEDS_H_

/* define pointer macro for accessing the LED interface data register */

#define LEDS ((volatile unsigned int *) 0x10000010)

#endif /* _LEDS_H_ */


/*-----------------------------------------------------------------*/
// SEVEN SEGMENT DISPLAY HEADER

#define SEVEN_SEG ((volatile unsigned int*) 0x10000020)

// BIT PATTERNS FOR 0, 1, 2, 3 IN HEX DISPLAY
#define HEX0 0x3F
#define HEX1 0x6
#define HEX2 0x5B
#define HEX3 0x4F

// BIT PATTERNS FOR A, b, C, d
#define HEXA 0x77 
#define HEXB 0x7C
#define HEXC 0x39
#define HEXD 0x5E


/*-----------------------------------------------------------------*/
// JTAG UART HEADER
#define JTAG ((volatile unsigned int*) 0x10001000)
#define JTAG_STATUS ((volatile unsigned int*) 10001004)


/*-----------------------------------------------------------------*/
// SWITCHES HEADER

#define SWITCHES ((volatile unsigned int*) 0x10000040)





/*-----------------------------------------------------------------*/
/*             start of application-specific code                  */
/*-----------------------------------------------------------------*/

/* place additional #define macros here */


/* define global program variables here */

// tracker for led pattern
int ledCount = 0;

// global variables for timer flags
int timer1_flag = 0;
int timer2_flag = 0;



void interrupt_handler(void)
{
	
	/* read current value in ipending register */

	/* do one or more checks for different sources using ipending value */

        /* remember to clear interrupt sources */
		
	unsigned int ipending;
	
	ipending = NIOS2_READ_IPENDING();
	
	// check timer 1 interrupt
	if (ipending & 0x4000){
		
		// CLEAR TIMER INTERRUPT
		*TIMER1_STATUS = 0;
		
        // set new led pattern
        if(ledCount == 0){
            *LEDS = 0b1000000001;
            ledCount++;
        }
        else if(ledCount == 1){
            *LEDS = 0b0100000010;
            ledCount++;
        }
        else if(ledCount == 2){
            *LEDS = 0b0010000100;
            ledCount++;
        }
        else if(ledCount == 3){
            *LEDS = 0b0001001000;
            ledCount++;
        }
        else if(ledCount == 4){
            *LEDS = 0b0000110000;
            ledCount = 0;
        }
        
        // set timer 1 flag
        timer1_flag = 1;
        
	}

	// check timer 2 interrupt
    if (ipending & 0x8000){
        
        // CLEAR TIMER INTERRUPT
        *TIMER2_STATUS = 0;
        
        // set timer flag
        timer2_flag = 1;
    }
}

void Init (void)
{	
	// initialize all displays to off
	*SEVEN_SEG = HEX0;
	*LEDS = 0x0;
	
	// INITIALIZE INITIAL BIT PATTERN FOR LEDS
	*LEDS = 0x300;
	
	// timers 1 and 2 initialization
    // timer int every 0.2 s
	*TIMER1_START_HI = 0x0098;
	*TIMER1_START_LO = 0x9680;
	*TIMER1_STATUS = TIMER_TO_BIT;
	*TIMER1_CONTROL = 0x7;
	
    // timer int every 0.5 s
	*TIMER2_START_HI = 0x017D;
	*TIMER2_START_LO = 0x7840;
	*TIMER2_STATUS = TIMER_TO_BIT;
	*TIMER2_CONTROL = 0x7;
	
	// enable timers 1 and 2
	NIOS2_WRITE_IENABLE(0xC000);
	
	NIOS2_WRITE_STATUS(0x1);
    
    // initialize leds with first pattern
    *LEDS = 0b1000000001;
    
    
    // ADC initialization
    InitADC(2, 2);
    
    // CHECK IF ANYTHING ELSE NEEDS TO BE DONE
    
    
}


/* place additional functions here */



int main (void)
{
	Init ();	/* perform software/hardware initialization */

    // used to determine if leftmost is true
    int ch = GetChar();
    
	int leftMost = 0;
	
    if(ch == 'l' || ch == 'L'){
        leftMost = 1;
    }
    
    // welcome msg
    PrintString("\nELEC 371 Lab 4 by Allan, Ivan, Sere\n\nA/D result divided by 16: 0x?");
    
    
    // additional variables for main loop
    unsigned int switches = 0;
    unsigned int ADC = 0;
    
    // main loop
	while (1)
	{
        
        // check timer 2 flag
        if(timer2_flag == 1){
            
            //rest flag
            timer2_flag = 0;
            
            // do sum here
            switches = *SWITCHES;
            switches = switches & 0x7F;
            
            if(leftMost == 0){
                //right
                *SEVEN_SEG = switches;
            }
            else{
                //left
                switches = switches << 24;
                *SEVEN_SEG = switches;
            }
            
            
        }
        
        // check timer 1 flag
        if(timer1_flag == 1){
            
            // reset flag
            timer1_flag = 0;
        
            //do sum here
            ADC = ADConvert();
            ADC = ADC/16;
            
            // delete
            PrintChar('\b');
            
            // print hex digit
            PrintHexDigit(ADC);
        }
	}

	return 0;	/* never reached, but main() must return a value */
}

/*-----------------------------------------------------------------*/
/*              end of application-specific code                   */
/*-----------------------------------------------------------------*/