 /**
 *  char_fsm.c  - character processor state machine version 2.0
 *  
 *  convert keyboard input to a stack of tokens  
 */

#include <propeller.h>
#include <stdio.h>
#include <stdlib.h>
#include "Pcon.h"
#include "char_fsm.h"
#include "typedefs.h"
// #include "cmd_fsm.h"

/****************************** globals ****************************/
TQ                          *head, *tail;
extern  char                input_buffer[];
extern  char                tbuf[];
extern  char                *input_buffer_ptr;
extern  uint8_t              cmd_state;
extern  uint8_t              char_state;

extern GLOBALS              *gptr;

/****************************** externals **************************/

// extern int          cmd_state,char_state;



// extern  struct {
//     int                 channel; 
//     int                 day; 
//     int                 hour; 
//     int                 minute; 
//     int                 key;
//     uint32_t            edit_buffer[_MAX_SCHEDULE_RECS];
//     uint32_t            clipboard_buffer[_MAX_SCHEDULE_RECS];

//     } edit;
/*********************** support functions **************************/
TQ *process_buffer(void)
{
    char        tb[_INPUT_BUFFER],*t_ptr,*start_char;        
    int         i;
    // printf("process buffer <%s>\n",input_buffer);
    input_buffer_ptr = input_buffer;
    t_ptr = tb;    
    start_char = input_buffer_ptr;
    head = '\0';
    tail = head;    
    while(*input_buffer_ptr != '\0')
    {
        if(*input_buffer_ptr==_QUOTE)
        {
            *t_ptr++ = *input_buffer_ptr++;            
            while((*input_buffer_ptr != _QUOTE)&&(*input_buffer_ptr != '\0'))
                *t_ptr++ = *input_buffer_ptr++;            
            *t_ptr++ = _QUOTE;
                *(++input_buffer_ptr)='\0';
                if(tail == '\0')
                {             
                    tail = (TQ *) malloc(sizeof(TQ));
                    head = tail;
                }
                else
                {
                    tail->next = (TQ *)malloc(sizeof(TQ));
                    tail = tail->next;
                }
                tail->tptr = malloc(input_buffer_ptr - start_char );
                memcpy(tail->tptr,start_char,input_buffer_ptr - start_char + 1);
                tail->next='\0';
                start_char = input_buffer_ptr;
                start_char++;         
        }          
        if(*input_buffer_ptr == _DELIMITER) 
        {
            *input_buffer_ptr='\0';
            if(tail == '\0')
            {             
                tail = (TQ *)malloc(sizeof(TQ));
                head = tail;
            }
            else
            {
                tail->next = (TQ *)malloc(sizeof(TQ));
                tail = tail->next;
            }
            tail->tptr = malloc(input_buffer_ptr - start_char );
            memcpy(tail->tptr,start_char,input_buffer_ptr - start_char + 1);
            tail->next='\0';
            start_char = input_buffer_ptr;
            start_char++;
        }
        *t_ptr++ = *input_buffer_ptr++;            
    }    
    for(i=0;i<_INPUT_BUFFER;i++) input_buffer[i] = '\0'; //clean out the buffer
    input_buffer_ptr = input_buffer;                     //reset the buffer pointer

    return head;
}
// char *pop()
// {
//     TQ                  *hold;
//     static char         tb[128];
//     char                *ptr1, *ptr2;
//     printf("pop called\n");
//     if(head == '\0')
//         return NULL;
//     ptr1 = head->tptr;
//     ptr2 = tb;
//     hold = head;
//     while(*ptr1 != '\0') *ptr2++ = *ptr1++;
//     *ptr2 = '\0';
//     head = head->next;     
//     free((void *)hold);
//     return tb;
// }
uint8_t char_type(char c)
{
    // printf ("char_type called char <%x>\n", c);
    switch(c)
    {
        case _ESC:
            return 0;
        case _SPACE:
            return 1;
        case _COLON:
            return 1;
        case _SLASH:
            return 1;
        case _COMMA:
            return 1;
        case _DELIMITER:
            return 1;            
        case _BS:
            return 2;
        case _QUOTE:
            return 3;
        case _CR:
            return 4;
    }
    return 5;    
}

/********** character input parser action routines ***********/

/* 0 -  do nothing */
int _p0(char *c)
{
    return 0;
}
/* 1 – clear all buffers, reset both state machines */
int _p1(char *c)
{
    char    dump[_MAX_TOKEN_SIZE];
    // cmd_state = 0;          //reset state
    while(pop_cmd_q(dump)); //clear out the command queue 

    printf("\n\nfsm reset\n");
    // reset_edit();           // clean out edit buffers
    input_buffer_ptr = input_buffer;
    *input_buffer_ptr++ = '?';
    *input_buffer_ptr++ = ' ';
    *input_buffer_ptr++ = '\0';
    process_buffer();
    return 0;
}
/* 2 – add char to buffer */                    
int _p2(char *c)
{
    *input_buffer_ptr++ = *c; 
    return 0;
}
/* 3 – remove char from buffer */               
int _p3(char *c)
{   
    *input_buffer_ptr-- = '\0';
    return 0;
}
/* 4 – add char to buffer, add delimiter to buffer */                 
int _p4(char *c)
{
    *input_buffer_ptr++ = _DELIMITER;
    return 0;
}
/* 5 -  add QUOTE to buffer, add char to buffer,  process buffer */              
int _p5(char *c)
{
    *input_buffer_ptr++ = _QUOTE;
    *input_buffer_ptr++ = *c;
    process_buffer();
    return 0;
}
/* 6 - add char to buffer,  process buffer */
int _p6(char *c)
{
    if(*(input_buffer_ptr-1) != _DELIMITER)
        *input_buffer_ptr++ = _DELIMITER; 
    *input_buffer_ptr++ = '\0'; 
    process_buffer();
    return 0;
} 
/* 7 – add delimiter to buffer, add quote to buffer */
int _p7(char *c)
{
    *input_buffer_ptr++ = _DELIMITER; 
    *input_buffer_ptr++ = _QUOTE; 
    return 0;
} 
/* 8  - add quote and delimiter to buffer */
int _p8(char *c)
{
    *input_buffer_ptr++ = _QUOTE;
    *input_buffer_ptr++ = _DELIMITER;
    return 0;
} 


/**************** character input parser fsm tables ******************/

/* fsm support functions */
uint8_t char_type(char);        //return a type code for the passed char
TQ *process_buffer(void);   //build token stack and clear input buffer

/* fsm functions */
int _p0(char *);   // do nothing 
int _p1(char *);   // clear all buffers, reset both state machines
int _p2(char *);   // add char to buffer
int _p3(char *);   // remove char from buffer
int _p4(char *);   // add char to buffer, add delimiter to buffer 
int _p5(char *);   // add QUOTE to buffer, add char to buffer,  process buffer
int _p6(char *);   // add char to buffer,  process buffer
int _p7(char *);   // add delimiter to buffer, add quote to buffer 
int _p8(char *);   // add quote and delimiter to buffer

/* character processor action table - initialized with fsm functions */
typedef int (*ACTION_PTR)(char *); 
ACTION_PTR char_action[_CHAR_TOKENS][_CHAR_STATES] = {     
/* ESC   */ {_p1, _p1, _p1, _p1},
/* DELIM */ {_p0, _p2, _p4, _p0},
/* BSPACE*/ {_p0, _p3, _p3, _p3},
/* QUOTE */ {_p2, _p8, _p7, _p2},
/* CR    */ {_p6, _p5, _p6, _p6},
/* other */ {_p2, _p2, _p2, _p2}};

/* character processor state transition table */
uint8_t char_new_state[_CHAR_TOKENS][_CHAR_STATES] ={
/* ESC   */ {0, 0, 0, 0},
/* DELIM */ {0, 1, 3, 3},
/* BSPACE*/ {0, 1, 2, 3},
/* QUOTE */ {1, 3, 1, 1},
/* CR    */ {0, 0, 0, 0},
/* other */ {2, 1, 2, 2}};

/************  character input parser state machine end  ***********/


void char_fsm(uint8_t c_type,uint8_t *state,char *c)
{
   // printf("\nchar_fsm called: token type <%d>, state <%d>, char <%c>, inputbuffer < %s>\n",
           // c_type, *state, *c, input_buffer);

    char_action[c_type][*state](c); 
    *state = char_new_state[c_type][*state];

    return;
}

/* test command queue return: 0 empty queue, -1 toke available */ 
int test_cmd_q()
{
    if(head == '\0')
        return 0;
    return -1;
}

/* pop token into buffer, return: 0 empty queue, -1 data placed in buffer  */ 
int pop_cmd_q(char *buf)
{
    TQ                  *hold;
    char                *ptr1, *ptr2;
    if(head == '\0')
    {
        *buf = '\0';    //set buffer to empty
        return 0;
    }
    // printf("pop_cmd_q called\n");
    ptr1 = head->tptr;
    ptr2 = buf;
    hold = head;
    while(*ptr1 != '\0') *ptr2++ = *ptr1++;
    *ptr2 = '\0';
    head = head->next;
    free(hold->tptr);     
    free(hold);

    return -1;
}

