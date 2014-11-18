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
char input_buffer[_INPUT_BUFFER];
char tbuf[_TOKEN_BUFFER];
char *input_buffer_ptr;
uint8_t cmd_state,char_state;

/***************************** support routines ********************************/
/* write system info to serial terminal */
void disp_sys(void) {
	printf("*** Pcon  %d.%d.%d ***\n\n", _major_version, _minor_version,
	_minor_revision);
	printf(" input buffer size: %d characters\n", _INPUT_BUFFER);
	return;
}

/********************************************************************/
/************************** start main  *****************************/
/********************************************************************/
int main(void) {
	int c;       							//character typed on keyboard
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
	setvbuf (stdout, NULL, _IONBF, BUFSIZ);	//turn off buffering for stdout

	/************************************************************/
	/**************** start main processing loop ****************/
	/************************************************************/

	input_buffer_ptr = input_buffer;    			//initialize input buffer pointer
	char_state = 0;									//initialize the character fsm
	cmd_state = 0;                     				//initialize the command processor fsm
//	printf("\n> ");
	fputc(_CR,stdout);
	fputc('>',stdout);
	fputc(' ',stdout);
/*
	fputc('x',stdout);
	sleep(1);
	fputc('x',stdout);
	sleep(1);
	fputc(8,stdout);
	sleep(1);
	fputc(' ',stdout);
	sleep(1);
	fputc(8,stdout);
*/
	exit_flag = 1;
	while (exit_flag){
		c = fgetc(stdin);			//grab a character from the keyboard buffer
		if(c == _ESC){
			exit_flag = 0;
			system("/bin/stty cooked");			//switch to buffered iput
			system("/bin/stty echo");					//turn on terminal echo
			printf("\nsystem reset\n");
			exit(1);
		}
//		printf("<%u>",c);
		if(c == 127){
			fputc(8,stdout);
			fputc(' ',stdout);
			fputc(8,stdout);
		}
		else fputc(c, stdout);       				// echo char
//		char_fsm(char_type(c),&char_state,&c);  //cycle fsm
	};

	system ("/bin/stty cooked");			//switch to buffered iput
	system("stty echo");					//turn on terminal echo
	printf("\f\nnormal termination\n\n");
	return 0;
}
