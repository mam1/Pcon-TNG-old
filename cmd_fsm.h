/**************************************************/
/*  cmd_fsm.h version 0.0                         */
/*  header file for cmd_fsm.c                     */
/*                                                */
/**************************************************/
#ifndef CMD_FSM_H_
#define CMD_FSM_H_

/* fuctions */
void cmd_fsm(GLOBALS *,uint8_t *);

char *build_prompt(char *, int);
void reset_active(void);
void reset_edit(void);


#endif