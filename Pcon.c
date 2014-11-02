/**
 * This is the main pc program file.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <ctype.h> 		//isalnum, tolower
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include "tlpi_hdr.h"
#include "Pcon.h"
#include "typedefs.h"
#include "char_fsm.h"
#include "trace.h"

// #include "cmd_fsm.h"

/******************************** globals **************************************/
struct termios tp, save;
char input_buffer[_INPUT_BUFFER];
char tbuf[_TOKEN_BUFFER];
char *input_buffer_ptr;
uint8_t cmd_state;


/***************************** support routines ********************************/
/* display error message and abort */
//void error_exit(char *routine, char *call, char *what) {
	/* Restore original terminal settings */
//	if (tcsetattr(STDIN_FILENO, TCSANOW, &save) == -1) {
//		printf("\n\naborting program\ntcsetattr failed in error_exit\n");
//		abort();
//	}
//	fprintf(trace_file,"\n\naborting program\n%s failed in %s - %s\n\n", routine, call,what);
//	abort();
//	return;
//}
/* display error message and abort */

/* write system info to serial terminal */
void disp_sys(void) {
	printf("*** Pcon  %d.%d.%d ***\n\n", _major_version, _minor_version,
	_minor_revision);
//	printf("  schedule record size: %d bytes or %d uint32_t(s)\n",_BYTES_PER_SCHEDULE,_INTS_PER_SCHEDULE);
//	printf("  max schedule records per channel per day: %d\n",_MAX_SCHEDULE_RECS);
//	printf("  working set buffer: %d bytes\n", _BYTES_PER_WORKING_SET);
//	printf("  rtc - control block: %d, stack: %d\n",sizeof(RTC_CB),_STACK_SIZE_RTC);
//	printf("  dio - control block: %d, stack: %d\n",sizeof(DIO_CB),_STACK_SIZE_DIO);
//	printf("  max token size: %d\n",_MAX_TOKEN_SIZE);
	printf("  input buffer size: %d characters\n", _INPUT_BUFFER);
	return;
}

/********************************************************************/
/************************** start main  *****************************/
/********************************************************************/
int main(void) {
	char c;       								//character typed on keyboard
	int exit_flag;								//exit man loop if TRUE
	int	char_state;								//currect state of the character procesing fsm
	int			trace_flag = true;

	/************************ initializations ****************************/
	printf("\033\143"); //clear the terminal screen, perserve the scroll back
	disp_sys();	        				//display system info on serial terminal

	/* setup trace */
#ifdef _TRACE
	trace_flag = true;
#else
	trace_flag = false;
#endif
	if (trace_flag == true) {
		printf(" program trace active\n");
		if(trace_on(_TRACE_FILE_NAME,&trace_flag))
			abort();
		trace(_TRACE_FILE_NAME,"\n************************************************************************\nstart trace\n");
	}
	if (trace_flag == false)
		printf("program trace disabled\n");

	/* set up unbuffered io */
//	if (tcgetattr(STDIN_FILENO, &tp) == -1)	//Retrieve current terminal settings, turn echoing off
//		error_exit("tcsetattr", "main",
//				"bad return code, trying to retrieve terminal settings");
//	save = tp;								//So we can restore settings later
//	tp.c_lflag &= ~ECHO; 					//ECHO off, other bits unchanged
//	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp) == -1)	//reset terminal
//		error_exit("tcsetattr", "main",
//				"bad return code, trying to turn terminal echo off");


	system("stty -echo");
	system("/bin/stty raw");				// use system call to make terminal send all keystrokes directly to stdin



	/************************************************************/
	/**************** start main processing loop ****************/
	/************************************************************/

	input_buffer_ptr = input_buffer;    			//initialize input buffer
	cmd_state = 0;                     				//initialize the command processor fsm
	printf("\n> ");
	exit_flag = 1;
	trace(_TRACE_FILE_NAME,"main: starting main event loop");
	while (exit_flag){
		c = getchar();								//grab a character from the keyboard buffer
		if(isalnum(c)){
			trace(_TRACE_FILE_NAME,"Pcon: character entred is a alpha numeric");
			c = tolower(c);
	        fputc(c, stdout);       				// echo char
		}
		switch (c ) {
		case _ESC:
			exit_flag = 0;

			break;
		case _EOF:
		case _CR:
			exit_flag = 0;
			fputc(_CR, stdout);   		        	//second CR after uer input
			fputc(_NL, stdout);
			 char_fsm(char_type(c),&char_state,&c);  //cycle fsm
			break;
		case _BS:
			fputc(8, stdout);
			fputc(' ', stdout);
			fputc(8, stdout);
			break;
		default:
			break;
		}

	};

	/* Restore original terminal settings */
//	if (tcsetattr(STDIN_FILENO, TCSANOW, &save) == -1)
//		printf("error2\n");
	printf("\f\nnormal termination\n\n");
	return 0;
}
