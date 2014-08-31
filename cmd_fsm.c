/*
*	cmd_fsm.c
*/
#include <propeller.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
// #include <string.h>

#include "Pcon.h"
#include "typedefs.h"
#include "char_fsm.h"
#include "cmd_fsm.h"

/****************************** globals ****************************/

/* key word list */
static char    *keyword[] = {
/*  0 */    "INT",
/*  1 */    "STR",
/*  2 */    "OTHER",
/*  3 */    "EMPTY",
/*  4 */    "?",
/*  5 */	"reboot"  };

/***************************** externals ***************************/
extern char                tbuf[];
extern char                input_buffer[];
extern GLOBALS      *gptr;

/****************  command parser state machine tables  ************/


/*cmd processor functions */
int c_0(GLOBALS *,int,int *,char *); /* prompt if needed else do nothing */
int c_1(GLOBALS *,int,int *,char *); /* display all valid commands for the current state */  
int c_2(GLOBALS *,int,int *,char *); /* display invalid command message, dump token statck */  
int c_3(GLOBALS *,int,int *,char *); /* reboot */  

/* cmd processor action table - initialized with fsm functions */
CMD_ACTION_PTR cmd_action[sizeof(keyword)/_BYTES_PER_INT][_CMD_STATES] = {
/*                STA _0 */ 
/*  0  INT      */  { c_2},
/*  1  STR      */  { c_2},
/*  2  OTHER    */  { c_2},
/*  3  EMPTY    */  { c_0},
/*  4  ?        */  { c_1},
/*  5  reboot   */	{ c_3}};

/* cmd processor state transition table */
uint8_t cmd_new_state[sizeof(keyword)/_BYTES_PER_INT][_CMD_STATES] ={
/*                   0 */
/*  0  INT      */  {0},
/*  1  STR      */  {0},
/*  2  OTHER    */  {0},
/*  3  EMPTY    */  {0},
/*  4  ?        */  {0},
/*  5  reboot   */	{0}};

/****************************** externals **************************/
extern uint8_t             cmd_state;	
/************************** support functions **********************/

int is_valid_int(const char *str)
{   
   if (*str == '-')     //negative numbers
      ++str;   
   if (!*str)           //empty string or just "-"
      return 0;   
   while (*str)         //check for non-digit chars in the rest of the string
   {
      if ((*str > 47) && (*str < 58))
     	++str;
      else
        return 0;   
	}
   return -1;
}
/* return the type of the token 0=INT, 1=STR, 2=OTHER, 3=EMPTY, >3=command number */
uint8_t cmd_type(char *c)
{
    int     i;
    /*test for an empty command que */
    if((*c==NULL) || (*c==' '))
        return 3;					//empty que
    /* test for a keyword */
    for(i=4;i<(sizeof(keyword)/_BYTES_PER_INT);i++)
    {   
        if(strlen(c) == strlen(keyword[i]))
            if(strncmp(c,keyword[i],strlen(c))==0)
                return i;			//return command number
    }
    /* test for a quoted string*/
    if(*c==_QUOTE)
        return 1;
    /* test for a integer */
    if(is_valid_int(c))
    {
        return 0;
    }
    /* unrecognized token */
    return 2;            
}
/**************** command parser action routines *******************/
/* do nothing except prompt if needed */
int c_0(GLOBALS *g, int tt, int *n, char *s)
{
    if(test_cmd_q()== 0)
    	printf("> ");
    return 0;
}
/* display all valid commands for the current state */
int c_1(GLOBALS *g, int tt, int *n, char *s)
{
    int     i;
    printf("  state %d, ", cmd_state);
// /*
//     if((cmd_state > 1) && (cmd_state < 6))
//         printf("editing channel %d, ",edit.channel);
//     else if(cmd_state > 2)
//         printf("editing channel %d schedule, ",edit.channel);
// */
//     printf("channel <%d> day <%d> hour <%d> minute <%d>\n",edit.channel,edit.day,edit.hour,edit.minute);

    printf("valid commands:");


    for(i=0; i<(sizeof(keyword)/_BYTES_PER_INT);i++)
    {
        if(cmd_action[i][cmd_state] == c_0)
            continue;
        if(cmd_action[i][cmd_state] == c_2)
            continue;
    //     // if(cmd_action[i][cmd_state] == c_12)
    //     //     continue;
    //     // if(cmd_action[i][cmd_state] == c_13)
    //     //     continue;
    //     // if(cmd_action[i][cmd_state] == c_1)
    //     //     continue;
    //     // if(cmd_action[i][cmd_state] == c_32)
    //     //     continue;
        printf(" %s",keyword[i]);        
    }
    printf("\n\n");

    c_0(gptr,tt,n,s);
    return 0;
}
/* invalid command */
int c_2(GLOBALS *g, int tt, int *n, char *s)
{
	if(s)
     	printf("  %s is not a vlaid command in state %d\n\n> ",s,cmd_state);
     else
     	printf("  %d is not a vlaid command in state %d\n> ",*n,cmd_state);

    while(pop_cmd_q(tbuf));	//dump the token stack
    return 0;
}
/* system reboot */ 
int c_3(GLOBALS *g, int tt, int *n, char *s) 
{
    printf("\n\nrebooting system ................\n\n");
	sleep(1);
    reboot();
    // c_0(tt,n,s);  
    return 0;
}
/***********  command parser state machine tables end  *************/

void cmd_fsm(GLOBALS *g,uint8_t *state)
{
    static int         tt, num,*n_ptr;         
    static char        *s_ptr,*tbuf;

    tbuf = g->tbuf;

   tt = cmd_type(tbuf);
   // printf("cmd_fsm called: tbuf <%s>, \ttbuf type <%d>, \tstate <%d>\n",tbuf,tt, *state);
    if((tt==1)||(tt==2))
    {
        n_ptr = NULL;
        s_ptr = tbuf;
    }
    else if(tt==0) 
    {
        sscanf(tbuf,"%u",&num);
        n_ptr = &num;
        s_ptr = NULL;
    }
    else
    {
        num = tt;
        n_ptr = &num;
        s_ptr = NULL;
    }
    // printf("call cmd_action[%d][%d]([tbuf type][state]), (<%d>,<%d>,<%s>)\n",tt,*state,tt,*n_ptr,s_ptr);    
	if(cmd_action[tt][*state](g,tt,n_ptr,s_ptr)==0)   //fire off an fsm action routine
	    *state = cmd_new_state[tt][*state];         //transition to next state
	else
	{
	    printf("*** error returned from action routine\n");
	}
	return;
}




