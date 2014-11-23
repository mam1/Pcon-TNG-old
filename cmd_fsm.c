/*
 * cmd_fsm.c
 *
 *  Created on: Nov 23, 2014
 *      Author: mam1
 */

#include <stdio.h>
#include <string.h>
#include "Pcon.h"
#include "char_fsm.h"

/*********************** externals **************************/
 extern int          cmd_state,char_state;
 extern char         input_buffer[_INPUT_BUFFER],*input_buffer_ptr;
 extern char         c_name[_CHANNEL_NAME_SIZE][_NUMBER_OF_CHANNELS];
 extern uint32_t     working_schedules[];
/* code to text conversion */
extern char *day_names_long[7];
extern char *day_names_short[7];
extern char *onoff[2];
extern char *con_mode[3];
extern char *sch_mode[2];
/*********************** globals **************************/

/*********************** functions **************************/
int is_valid_int(const char *str)
{
   if (*str == '-')     //negative numbers
      ++str;
   if (!*str)           //empty string or just "-"
      return 0;
   while (*str)         //check for non-digit chars in the rest of the string
   {
      if (!isdigit(*str))
         return 0;
      else
         ++str;
   }
   return -1;
}
int cmd_type(char *c)
{
    int     i;

    /*test for an empty command que */
    if((*c==NULL) || (*c==' '))
        return 3;

    /* test for a keyword */
    for(i=3;i<_CMD_TOKENS;i++)
    {
        if(strlen(c) == strlen(keyword[i]))
            if(strncmp(c,keyword[i],strlen(c))==0)
                return i;
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
/***************************************/
/*****  command  parser fsm start ******/
/***************************************/

/* key word list */
char    *keyword[_CMD_TOKENS] = {
/*  0 */    "INT",
/*  1 */    "STR",
/*  2 */    "OTHER",
/*  3 */    "EMPTY",
/*  4 */    "copy",
/*  5 */    "paste",
/*  6 */    "delete",
/*  7 */    "file",
/*  8 */    "edit",
/*  9 */    "quit",
/* 10 */    "cancel",
/* 11 */    "name",
/* 12 */    "mode",
/* 13 */    "zero",
/* 14 */    "on",
/* 15 */    "off",
/* 16 */    "system",
/* 17 */    "status",
/* 18 */    "time",
/* 19 */    "set",
/* 20 */    "shutdown",
/* 21 */    "startup",
/* 22 */    "reboot",
/* 23 */    "save",
/* 24 */    "schedule",
/* 25 */    "channel",
/* 26 */    "load",
/* 27 */    "help",
/* 28 */    "?"  };

/* cmd processor state transition table */
int cmd_new_state[_CMD_TOKENS][_CMD_STATES] ={
/*                   0  1  2  3  4  5  6  7  8   9  10  11  12  13 */
/*  0  INT      */  {0, 2, 2, 3, 0, 5, 6, 7, 9, 10, 11, 12, 12, 13},
/*  1  STR      */  {0, 1, 2, 0, 4, 5, 7, 7, 8,  9, 10, 11, 12, 13},
/*  2  OTHER    */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/*  3  EMPTY    */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/*  4  copy     */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9,  8, 11, 12, 13},
/*  5  paste    */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9,  8, 11, 12, 13},
/*  6  delete   */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9,  8, 11, 10, 13},
/*  7  file     */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/*  8  edit     */  {0, 1, 2, 3, 4, 8, 6, 7, 8,  9, 10, 11, 12, 13},
/*  9  quit     */  {0, 0, 0, 3, 4, 0, 0, 7, 5,  5,  5, 10, 10, 13},
/* 10  cancel   */  {0, 1, 0, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 11  name     */  {0, 1, 3, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 12  mode     */  {0, 1, 4, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 13  zero     */  {0, 1, 0, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 14  on       */  {0, 1, 0, 3, 4, 5, 6, 7, 8,  9, 10, 11, 10, 13},
/* 15  off      */  {0, 1, 0, 3, 4, 5, 6, 7, 8,  9, 10, 11, 10, 13},
/* 16  system   */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 17  status   */  {0, 0, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 18  time     */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 19  set      */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 20  shutdown */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 21  startup  */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 22  reboot   */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 23  save     */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 24  schedule */  {5, 5, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 25  channel  */  {1, 1, 2, 3, 4, 1, 6, 7, 8,  9, 10, 11, 12, 13},
/* 26  load     */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 27  help     */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13},
/* 28  ?        */  {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13}};

/*cmd processor functions */
int c_0(int,int *,char *); /* prompt if needed else do nothing */
int c_1(int,int *,char *); /* display all valid commands for the current state */
int c_2(int,int *,char *); /* zero on time for channel */
int c_3(int,int *,char *); /* prompt if q empty */
int c_4(int,int *,char *); /* set active channel */
int c_5(int,int *,char *); /* display info for all channels */
int c_6(int,int *,char *); /* save info for all channels */
int c_7(int,int *,char *); /* load info for all channels */
int c_8(int,int *,char *); /* set channel name */
int c_9(int,int *,char *); /* nop */
int c_10(int,int *,char *); /* set channel control mode */
int c_11(int,int *,char *); /* set channel state on */
int c_12(int,int *,char *); /* unknown command */
int c_13(int,int *,char *); /* invalid command */
int c_14(int,int *,char *); /* save schedule buffer */
int c_15(int,int *,char *); /* prompt for channel control mode */
int c_16(int,int *,char *); /* prompt for channel state */
int c_17(int,int *,char *); /* display data for a single channel */
int c_18(int,int *,char *); /* display formatted schedules */
int c_19(int,int *,char *); /* set channel state to off */
int c_20(int,int *,char *); /* set active day */
int c_21(int,int *,char *); /* set active hour */
int c_22(int,int *,char *); /* set active minute */
int c_23(int,int *,char *); /* display time and date */
int c_24(int,int *,char *); /* set schedule record state to on */
int c_25(int,int *,char *); /* set schedule record state to off */
int c_26(int,int *,char *); /* delete schedule record */
int c_27(int,int *,char *); /* load working schedule buffer from SD card */
int c_28(int,int *,char *); /* save working schedule buffer to SD card */
int c_29(int,int *,char *); /* dump schedule buffer */
int c_30(int,int *,char *); /* display edit buffer schedule for active day and channel */
int c_31(int,int *,char *); /* display system configuration information */
int c_32(int,int *,char *); /* invalid integer */
int c_33(int,int *,char *); /* system reboot */
int c_34(int,int *,char *); /* stop rtc & dio cogs */
int c_35(int,int *,char *); /* start rtc & dio cogs */
int c_36(int,int *,char *); /* copy schedule for (channel,day) into hold buffer */
int c_37(int,int *,char *); /* quit channel mode */
int c_38(int,int *,char *); /* quit schedule mode */
int c_39(int,int *,char *); /* quit schedule edit mode */

/* cmd processor action table - initialized with fsm functions */

CMD_ACTION_PTR cmd_action[_CMD_TOKENS][_CMD_STATES] = {
/*                STATE 0     1     2     3     4     5     6     7     8     9    10    11    12    13 */
/*  0  INT      */  {c_32,  c_4,  c_0,  c_0, c_10,  c_0,  c_0,  c_0,  c_4, c_20, c_21, c_22,  c_0,  c_0},
/*  1  STR      */  {c_12, c_12,  c_0,  c_8,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  2  OTHER    */  {c_12, c_12,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  3  EMPTY    */  { c_0,  c_9,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  4  copy     */  {c_13, c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  5  paste    */  {c_13, c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  6  delete   */  {c_13, c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_26,  c_0},
/*  7  file     */  {c_13, c_13,  c_0,  c_0,  c_0,  c_9,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  8  edit     */  {c_13, c_13,  c_0,  c_0,  c_0,  c_9,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  9  quit     */  {c_13,  c_9, c_37, c_37, c_37, c_38, c_38, c_38, c_38, c_38, c_38, c_39, c_39,  c_0},
/* 10  cancel   */  {c_13, c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 11  name     */  {c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 12  mode     */  {c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 13  zero     */  {c_13,  c_0,  c_2,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 14  on       */  {c_13,  c_0, c_11,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_24,  c_0},
/* 15  off      */  {c_13,  c_0, c_19,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_25,  c_0},
/* 16  system   */  {c_31, c_31, c_31, c_31, c_31, c_31, c_31, c_31, c_31, c_31, c_31, c_31, c_31, c_31},
/* 17  status   */  {c_18,  c_5,  c_5,  c_5,  c_5, c_18, c_18, c_18, c_18, c_18, c_18, c_18, c_18, c_18},
/* 18  time     */  {c_23, c_23, c_23, c_23, c_23, c_23, c_23, c_23, c_23, c_23, c_23, c_23, c_23, c_23},
/* 19  set      */  { c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 20  shutdown */  {c_34,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 21  startup  */  {c_35,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 22  reboot   */  {c_33,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 23  save     */  {c_13,  c_6,  c_0,  c_0,  c_0, c_28,  c_28,c_28, c_27, c_27, c_14,  c_0,  c_0,  c_0},
/* 24  schedule */  { c_9,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 25  channel  */  { c_9,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 26  load     */  {c_13,  c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 27  help     */  { c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1},
/* 28  ?        */  { c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1}};

/***************start fsm support functions ********************/




/**************** command fsm action routines ******************/
/* do nothing except prompt if needed */
int c_0(int tt, int *n, char *s)
{
    if(test_cmd_q()== 0)
    {
        *prompt_buffer = '\0';
        printf("%s\n>> ",build_prompt(prompt_buffer,tt,0));
    }
    return 0;
}
/* display all valid commands for the current state */
int c_1(int tt, int *n, char *s)
{

    return 0;
}
/* ping BBB */
int c_2(int tt, int *n, char *s)
{
	return 0;
}
/*  */
int c_3(int tt, int *n, char *s)
{

    return 0;
}

/*****************************************************/
/*********  command parser state machine end  ********/
/*****************************************************/
/* cycle state machine */
void cmd_fsm(char *token,int *state)
{
    static int         tt, num,*n_ptr;
    static char        *s_ptr;

    tt = cmd_type(token);
//    printf("cmd_fsm called: token <%s>, token type <%i>, state <%i>\n",token,tt, *state);
    if((tt==1)||(tt==2))
    {
        n_ptr = NULL;
        s_ptr = token;
    }
    else if(tt==0)
    {
        sscanf(token,"%u",&num);
        n_ptr = &num;
        s_ptr = NULL;
    }
    else
    {
        num = tt;
        n_ptr = &num;
        s_ptr = NULL;
    }
//    printf("call cmd_action[%i][%i](<%i>,<%i>,<%s>)\n",tt,*state,tt,*n_ptr,s_ptr);
    if(cmd_action[tt][*state](tt,n_ptr,s_ptr)==0)   //fire off an fsm action routine
        *state = cmd_new_state[tt][*state];         //transition to next state
    else
    {
        // printf("*** error returned from action routine\n");
    }
    return;
}

