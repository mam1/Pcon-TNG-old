 /**
 * This is the main pc program file.
 */
#include <stdio.h>
#include <propeller.h>
#include <unistd.h>

#include "Pcon.h"
#include "typedefs.h"

/***************************** drivers ********************************/
extern _Driver _FullDuplexSerialDriver;
extern _Driver _FileDriver;
_Driver *_driverlist[] = {&_FullDuplexSerialDriver,&_FileDriver,NULL};

/***************************** support routines ********************************/
void disp_sys(void)	// write system info to serial terminal
{
	printf("\n*** Pcon  %d.%d ***\n\n",_major_version,_minor_version);
	printf("  schedule record size: %d bytes or %d uint32_t(s)\n",_BYTES_PER_SCHEDULE,_INTS_PER_SCHEDULE);
	printf("  max schedule records per channel per day: %d\n",_MAX_SCHEDULE_RECS);
	printf("  working set buffer: %d bytes\n", _BYTES_PER_WORKING_SET);
	printf("  rtc - control block: %d, stack: %d\n",sizeof(RTC_CB),_STACK_SIZE_RTC);
	printf("  dio - control block: %d, stack: %d\n",sizeof(DIO_CB),_STACK_SIZE_DIO);
	#if _DRIVEN == _DIOB
        printf("  system is configured to drive a Parallax Digital IO Board\n");
    #else
        printf("  system is configured to drive 5 IO pins\n");
    #endif


	return;
}


/********************************************************************/
/************************** start main  *****************************/
/********************************************************************/
 int main(void)
{
	char 					c;
/************************ initializations ****************************/
 	sleep(1);   //wait for the serial terminal to start
	disp_sys();	// display system info on serial terminal */


/* set up unbuffered nonblocking io */
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    stdin->_flag &= ~_IOCOOKED;
    stdin->_flag |= _IONONBLOCK;

/************************************************************/
/**************** start main processing loop ****************/
/************************************************************/
    while(1)
    {
        /* check the cog controll blocks for messages */
        // if(ckeck_abort())
        //     return 1;
        /* see if the channel data needs to be saved */
        // if(dio_cb.dio.save_cca)
        // {
        //     dio_cb.dio.save_cca = 0;
        //     if(save_channel_data())
        //         return 1;
        // }

        /* check the token stack */
        // while(pop_cmd_q(tbuf))
        //     cmd_fsm(tbuf,&cmd_state);   //cycle cmd fsm until queue is empty

        /* grab a character from the keyboard if one is present  */
        c = fgetc(stdin);     					//nonblocking read

        /*  process input char */
        if(c==_NO_CHAR)
        	continue; 							//start main loop over  
        fputc(c, stdout);       				//echo char 
        if(c==_CR) fputc(_CR, stdout);   		//second CR after uer input
        // char_fsm(char_type(c),&char_state,&c);  //cycle character processor fsm
    };

    printf("\nnormal termination\n\n");
    return 0;
}
