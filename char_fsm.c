
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
    sprintf(trace_buf, "process_buffer: called <%s>", input_buffer);
	trace(_TRACE_FILE_NAME,trace_buf);
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
			trace(_TRACE_FILE_NAME,"process_buffer: found a quote");
#endif
			*t_ptr++ = *input_buffer_ptr++;
			while ((*input_buffer_ptr != _QUOTE) && (*input_buffer_ptr != '\0'))
				*t_ptr++ = *input_buffer_ptr++;
			*t_ptr++ = _QUOTE;
#ifdef _TRACE
			trace(_TRACE_FILE_NAME,"process_buffer: found a second quote");
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
			trace(_TRACE_FILE_NAME,"process_buffer: found a delimiter");
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
	}
	for (i = 0; i < _INPUT_BUFFER; i++)					//clean out input buffer
		input_buffer[i] = '\0';
	input_buffer_ptr = input_buffer;					//reset pointer
	return head;
}
/*
char *pop() {
	TQ *hold;
	static char tb[128];
	char *ptr1, *ptr2;

	if (head == '\0')
		return NULL;
	ptr1 = head->tptr;
	ptr2 = tb;
	hold = head;
	while (*ptr1 != '\0')
		*ptr2++ = *ptr1++;
	*ptr2 = '\0';
	head = head->next;
	free(hold);
	return tb;
}
*/

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
int nop(char *c) {
	return 0;
}
/* 1 – clear all buffers, reset both state machines */
int char_esc(char *c) {
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
int add(char *c) {
#ifdef _TRACE
	trace(_TRACE_FILE_NAME," add: adding character to buffer");
#endif
	*input_buffer_ptr++ = *c;
	return 0;
}
/* remove char from buffer */
int del(char *c) {
#ifdef _TRACE
	trace(_TRACE_FILE_NAME," del: removing character from buffer");
#endif
	input_buffer_ptr--;
	return 0;
}

/*  add char to buffer, add delimiter to buffer */
int dlm(char *c) {
#ifdef _TRACE
	trace(_TRACE_FILE_NAME," dlm: add character and entered dilimitor to buffer");
#endif
	*input_buffer_ptr++ = *c;
//	*input_buffer_ptr++ = _DELIMITER;
	return 0;
}
/* process buffer */
int cr(char *c) {
#ifdef _TRACE
	trace(_TRACE_FILE_NAME," cr: add character and dilimitor to buffer");
#endif
//	if(char_type(*input_buffer_ptr)!=0) *input_buffer_ptr++ = ' ';
	*input_buffer_ptr++ = '\0';
	process_buffer();
#ifdef _TRACE
    sprintf(trace_buf, "process_buffer: input buffer after being processed <%s>", input_buffer);
	trace(_TRACE_FILE_NAME,trace_buf);
	char	bbb[128];
	while(pop_cmd_q(bbb)){
		sprintf(trace_buf, "                pop token queue <%s>", bbb);
		trace(_TRACE_FILE_NAME,trace_buf);
	}
#endif
	return 0;
}
/* 5 -  add QUOTE to buffer, add char to buffer,  process buffer */
int char_add_quote_char_process(char *c) {
	*input_buffer_ptr++ = _QUOTE;
	*input_buffer_ptr++ = *c;
	return 0;
}


/* 7 – add delimiter to buffer, add quote to buffer */
int char_delim_add(char *c) {
	*input_buffer_ptr++ = _QUOTE;
	return 0;
}

/* 8  - add char to buffer,  process buffer */
int char_eof_process(char *c) {
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
int nop(char *);
int del(char *);
int add(char *);
int dlm(char *);
int cr(char *);


/* character processor action table - initialized with fsm fuctions */
typedef int (*ACTION_PTR)(char *);
ACTION_PTR char_action[_CHAR_TOKENS][_CHAR_STATES] = {
/* DELIMITOR */{ nop, add, dlm, nop },
/*     QUOTE */{ nop, add, add, nop },
/*        BS */{ del, del, del, del },
/*        CR */{ nop, add,  cr,  cr },
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

void char_fsm(int c_type, int *state, char *c) {
//	char			buf[128];


#ifdef _TRACE
    sprintf(trace_buf, "char_fsm: called with - c_type %d, state %d , char<%u>\n          buffer <%s>\n", c_type,*state,*c,input_buffer);
    trace(_TRACE_FILE_NAME,trace_buf);
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

