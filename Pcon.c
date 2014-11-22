/**
 * This is the main Pcon.c program file
 * BeagleBone Black implementation
 */
#include <stdio.h>
#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <ctype.h> 		//isalnum, tolower
#include <stdlib.h>
#include <string.h>
#include "Pcon.h"
#include "typedefs.h"
#include "char_fsm.h"
#include "trace.h"

// #include "cmd_fsm.h"

/******************************** globals **************************************/
int	trace_flag;			//control program trace
char work_buffer[_INPUT_BUFFER], *work_buffer_ptr;
char tbuf[_TOKEN_BUFFER];

uint8_t cmd_state,char_state;

/***************************** support routines ********************************/
/* write system info to stdout */
void disp_sys(void) {
	printf("*** Pcon  %d.%d.%d ***\n\n", _major_version, _minor_version,
	_minor_revision);
	printf(" input buffer size: %d characters\n", _INPUT_BUFFER);
	return;
}
/* prompt for user input */
void prompt(void){
	fputc(_CR,stdout);
	fputc('>',stdout);
	fputc(' ',stdout);
	return;
}

/********************************************************************/
/************************** start main  *****************************/
/********************************************************************/
int main(void) {
	char c;       			//character typed on keyboard
	int exit_flag;			//exit man loop if TRUE
	int	char_state;			//currect state of the character procesing fsm
	int i;

	/************************ initializations ****************************/
	printf("\033\143"); //clear the terminal screen, perserve the scroll back
	disp_sys();	        //display system info on serial terminal

	/* setup trace */
#ifdef _TRACE
	trace_flag = true;
#else
	trace_flag = false;
#endif
	if (trace_flag == true) {
		printf(" program trace active,");
		if(trace_on(_TRACE_FILE_NAME,&trace_flag)){
			printf("trace_on returned error\n");
			trace_flag = false;
		}
	}
	if (trace_flag == false)
		printf(" program trace disabled\n");

	/* set up unbuffered io */
	system("stty -echo");					//turn off terminal echo
	system("/bin/stty raw");				// use system call to make terminal send all keystrokes directly to stdin

	/************************************************************/
	/**************** start main processing loop ****************/
	/************************************************************/

//	input_buffer_ptr = input_buffer;    //initialize input buffer pointer
	work_buffer_ptr = work_buffer;    	//initialize work buffer pointer
	char_state = 0;						//initialize the character fsm
	cmd_state = 0;                     	//initialize the command processor fsm
	prompt();
	exit_flag = 1;
#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"Pcon",char_state,NULL,"starting main event loop\n",trace_flag);
#endif
	while (exit_flag){
		c = getchar();			//grab a character from the keyboard buffer
		switch (c) {
/* ESC */  case _ESC:
#ifdef _TRACE
			trace(_TRACE_FILE_NAME,"Pcon",char_state,work_buffer,"ecape entered",trace_flag);
#endif
			exit_flag = 0;
			system("/bin/stty cooked");			//switch to buffered iput
			system("stty echo");				//turn on terminal echo
			printf("\nsystem reset\n");
			exit(1);
			break;


/* CR */	case _CR:
#ifdef _TRACE
			trace(_TRACE_FILE_NAME,"Pcon",char_state,work_buffer,"character entered is a _CR",trace_flag);
#endif
			fputc(_CR, stdout);
			fputc(_NL, stdout);
//			if(char_type(*(work_buffer_ptr-1))>1 )		//the character before the CR is not a delimiter or QUOTE
//				*work_buffer_ptr++ = ' ';
			*work_buffer_ptr = c;
//			for (i = 0; i < _INPUT_BUFFER; i++)					//clean out input buffer
//				input_buffer[i] = '\0';
//			input_buffer_ptr = input_buffer;					//reset pointer

			work_buffer_ptr = work_buffer;
			reset_char_fsm();
			while(*work_buffer_ptr != '\0'){
				char_fsm(char_type(*work_buffer_ptr),&char_state,work_buffer_ptr++);  //cycle fsm

			}
			for (i = 0; i < _INPUT_BUFFER; i++)					//clean out work buffer
				work_buffer[i] = '\0';
			work_buffer_ptr = work_buffer;					//reset pointer
			break;
/* DEL */   case _DEL:
#ifdef _TRACE
			trace(_TRACE_FILE_NAME,"Pcon",char_state,work_buffer,"character entered is a _BS",trace_flag);
#endif
			fputc(_BS,stdout);
			fputc(' ',stdout);
			fputc(_BS,stdout);
			*work_buffer_ptr-- = '\0';
			*work_buffer_ptr = '\0';
#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"Pcon",char_state,work_buffer,"remove charater from input buffer",trace_flag);
#endif
			break;


/* OTHER */ default:
			fputc(c, stdout);       				// echo char
			*work_buffer_ptr++ = c;
#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"Pcon",char_state,work_buffer,"add charater to work buffer",trace_flag);
#endif
		}

	};

	system("/bin/stty cooked");			//switch to buffered iput
	system("/bin/stty echo");			//turn on terminal echo
	printf("\f\nnormal termination\n\n");
	return 0;
}
