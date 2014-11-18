
#include <stdio.h>
#include <string.h> //memset, memcpy
#include <ctype.h> //isalnum, tolower
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include "Pcon.h"
#include "char_fsm.h"
#include "cmd_fsm.h"
#include "trace.h"

/***************************** globals ******************************/
TQ *head, *tail;
#ifdef _TRACE
	char			trace_buf[128];
#endif
/***************************** externals ************************/
extern char input_buffer[_INPUT_BUFFER], *input_buffer_ptr;
extern int cmd_state, char_state;
extern struct {
	int channel;
	int day;
	int hour;
	int minute;
	int key;
	uint32_t edit_buffer[_MAX_SCHEDULE_RECS];
	uint32_t clipboard_buffer[_MAX_SCHEDULE_RECS];

} edit;
/********************** support functions ****************************/
TQ *process_buffer(void) {
#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"process_buffer", char_state, input_buffer, "called");
#endif
	char tb[_INPUT_BUFFER], *t_ptr, *start_char;        //,*end_char;
	int i;
	input_buffer_ptr = input_buffer;					//set pointer to start of buffer
	t_ptr = tb;											//set pointer to temporary buffer
	start_char = input_buffer_ptr;
	head = '\0';										//initialize head pointer
	tail = head;
	while (*input_buffer_ptr != '\0') {
		if (*input_buffer_ptr == _QUOTE) {
#ifdef _TRACE
			trace(_TRACE_FILE_NAME,"process_buffer",char_state,input_buffer,"found a quote");
#endif
			*t_ptr++ = *input_buffer_ptr++;
			while ((*input_buffer_ptr != _QUOTE) && (*input_buffer_ptr != '\0'))
				*t_ptr++ = *input_buffer_ptr++;
			*t_ptr++ = _QUOTE;
#ifdef _TRACE
			trace(_TRACE_FILE_NAME,"process_buffer",char_state,input_buffer,"found a second quote");
#endif
			*(++input_buffer_ptr) = '\0';
			if (tail == '\0') {
				tail = malloc(sizeof(TQ));
				head = tail;
			} else {
				tail->next = malloc(sizeof(TQ));
				tail = tail->next;
			}
			tail->tptr = malloc(input_buffer_ptr - start_char);
			memcpy(tail->tptr, start_char, input_buffer_ptr - start_char + 1);
			tail->next = '\0';
			start_char = input_buffer_ptr;
			start_char++;
		}

		if(char_type(*input_buffer_ptr)==0) {					//test for a delimiter
#ifdef _TRACE
			trace(_TRACE_FILE_NAME,"process_buffer",char_state,input_buffer,"found a delimiter");
#endif
			*input_buffer_ptr = '\0';
			if (tail == '\0') {
				tail = malloc(sizeof(TQ));
				head = tail;
			} else {
				tail->next = malloc(sizeof(TQ));
				tail = tail->next;
			}
			tail->tptr = malloc(input_buffer_ptr - start_char);
			memcpy(tail->tptr, start_char, input_buffer_ptr - start_char + 1);
			tail->next = '\0';
			start_char = input_buffer_ptr;
			start_char++;
		}

		*t_ptr++ = *input_buffer_ptr++;
#ifdef _TRACE
		trace(_TRACE_FILE_NAME,"process_buffer",char_state,input_buffer,"character added to temp buffer");
#endif
	}
#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"process_buffer",char_state,input_buffer,"done processing, clean up");
#endif
	for (i = 0; i < _INPUT_BUFFER; i++)					//clean out input buffer
		input_buffer[i] = '\0';
	input_buffer_ptr = input_buffer;					//reset pointer
	return head;
}

int char_type(char c) {
	switch (c) {
	case _COMMA:
	case _SPACE:
	case _COLON:
	case _SLASH:
		return 0;
	case _QUOTE:
		return 1;
	case _BS:
		return 2;
	case _CR:
		return 3;
	}
	return 4;
}

/********************************************/
/** character input parser action routines **/
/********************************************/

/* 0 -  do nothing */
int nop(uint8_t *c) {
	return 0;
}
/* 1 – clear all buffers, reset both state machines */
int char_esc(uint8_t *c) {
#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"esc",char_state,input_buffer,"terminating program");
#endif
	abort();
	char dump[_TOKEN_BUFFER];
	cmd_state = 0;          //reset state
	while (pop_cmd_q(dump))
		; //clear out the command queue 

	printf("\n\nfsm reset\n");
//    reset_edit();           // clean out edit buffers
	input_buffer_ptr = input_buffer;
	*input_buffer_ptr++ = '?';
	*input_buffer_ptr++ = ' ';
	*input_buffer_ptr++ = '\0';

	process_buffer();
	return 0;
}
/* add char to buffer */
int add(uint8_t *c) {
#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"add",char_state,input_buffer,"adding character to buffer");

#endif
	*input_buffer_ptr++ = *c;
	return 0;
}
/* remove char from buffer */
int del(uint8_t *c) {
#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"add",char_state,input_buffer,"removing character from buffer");
#endif
	input_buffer_ptr--;
	return 0;
}

/*  add char to buffer */
int dlm(uint8_t *c) {
#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"dlm",char_state,input_buffer,"add delimiter to buffer");
#endif
	*input_buffer_ptr++ = *c;
	return 0;
}
/* process buffer */
int cr(uint8_t *c) {
#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"cr",char_state,input_buffer,"process buffer");
#endif
	if(char_type(*input_buffer_ptr)!=0) *input_buffer_ptr++ = ' ';
	*input_buffer_ptr++ = '\0';
	process_buffer();
#ifdef _TRACE
	system ("/bin/stty cooked");			//switch to buffered iput
	system("stty echo");					//turn on terminal echo
	char	bbb[128];
	printf("\npopping command queue\n");
	while(pop_cmd_q(bbb)) printf("<%s>\n",bbb);
#endif
	system("stty -echo");					//turn off terminal echo
	system("/bin/stty raw");				// use system call to make terminal send all keystrokes directly to stdin
	return 0;
}

int cr2(uint8_t *c) {
#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"cr2",char_state,input_buffer,"process buffer");
#endif
//	if(char_state == 2) *input_buffer_ptr++ = ' ';
//	if(char_type(*input_buffer_ptr)!=0) *input_buffer_ptr++ = ' ';
	*input_buffer_ptr++ = '\0';
	process_buffer();
#ifdef _TRACE
	system ("/bin/stty cooked");			//switch to buffered iput
	system("stty echo");					//turn on terminal echo
	char	bbb[128];
	printf("\npopping command queue\n");
	while(pop_cmd_q(bbb)) printf("<%s>\n",bbb);
#endif
	system("stty -echo");					//turn off terminal echo
	system("/bin/stty raw");				// use system call to make terminal send all keystrokes directly to stdin
	return 0;
}
/* 5 -  add QUOTE to buffer, add char to buffer,  process buffer */
int crq(uint8_t *c) {
#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"crq",char_state,input_buffer,"add quote, process buffer");
#endif
	*input_buffer_ptr++ = _QUOTE;
	*input_buffer_ptr++ = *c;
	return 0;
}


/* 7 – add delimiter to buffer, add quote to buffer */
int char_delim_add(uint8_t *c) {
	*input_buffer_ptr++ = _QUOTE;
	return 0;
}

/* 8  - add char to buffer,  process buffer */
int char_eof_process(uint8_t *c) {
	*input_buffer_ptr++ = '\0';
	process_buffer();
	return 0;
}

/***************************************/
/**  character input parser fsm start **/
/***************************************/

/* fsm support functions */
int char_type(char);
TQ *process_buffer(void);

/* fsm fuctions */
int nop(uint8_t *);
int del(uint8_t *);
int add(uint8_t *);
int dlm(uint8_t *);
int cr(uint8_t *);
int cr2(uint8_t *);
int crq(uint8_t *);

/* character processor action table - initialized with fsm fuctions */
typedef int (*ACTION_PTR)(uint8_t *);
ACTION_PTR char_action[_CHAR_TOKENS][_CHAR_STATES] = {
/* DELIMITOR */{ nop, add, dlm, nop },
/*     QUOTE */{ add, add, add, nop },
/*        BS */{ del, del, del, del },
/*        CR */{ nop, crq,  cr, cr2 },
/*     OTHER */{ add, add, add, add }};

/* character processor state transition table */
int char_new_state[_CHAR_TOKENS][_CHAR_STATES] = {

/* DELIMITOR */{ 0, 1, 3, 3},
/*     QUOTE */{ 1, 0, 1, 1},
/*        BS */{ 0, 1, 2, 3},
/*        CR */{ 0, 0, 0, 0},
/*     OTHER */{ 2, 1, 2, 2} };

/*****************************************************/
/****  character input parser state machine end  *****/
/*****************************************************/

void char_fsm(int c_type, int *state, uint8_t *c) {
//	char			buf[128];


#ifdef _TRACE
    sprintf(trace_buf, "called with - c_type %d, char<%u>", c_type,*c);
	trace(_TRACE_FILE_NAME,"char_fsm",*state,input_buffer,trace_buf);
#endif
	char_action[c_type][*state](c);
	*state = char_new_state[c_type][*state];

	return;
}

/* test command queue return: 0 empty queue, -1 toke available */
int test_cmd_q() {
	if (head == '\0')
		return 0;
	return -1;
}

/* pop token into buffer, return: 0 empty queue, -1 data placed in buffer  */
int pop_cmd_q(char *buf) {
//#ifdef _TRACE
//	trace(_TRACE_FILE_NAME,"pop_cmd_q: called");
//#endif
	TQ *hold;
	char *ptr1, *ptr2;

	if (head == '\0') {
		*buf = '\0';    //set buffer to empty
		return 0;
	}
	ptr1 = head->tptr;
	ptr2 = buf;
	hold = head;
	while (*ptr1 != '\0')
		*ptr2++ = *ptr1++;
	*ptr2 = '\0';
	head = head->next;
	free(hold->tptr);
	free(hold);

	return -1;
}

