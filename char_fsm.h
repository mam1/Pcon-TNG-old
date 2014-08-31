/**************************************************/
/*  char_fsm.h version 0.0                        */
/*  header file for char_fsm.c                    */
/*                                                */
/**************************************************/
#ifndef CHAR_FSM_H_
#define CHAR_FSM_H_

/* data structures */
typedef struct node
{
    struct node     *next;
    char            *tptr;
}TQ;

/* external fuctions */
void char_fsm(uint8_t ,uint8_t *,char *);	//cycle the character fsm
// void cmd_fsm(char *,int *);
int pop_cmd_q(char *);				//pop a token off the command q
int test_cmd_q();					//test for empty q 
TQ *process_buffer(void);			//tokenize the input buffer, create command q

/* internal fuctions */
uint8_t char_type(char);				//return char type code


#endif