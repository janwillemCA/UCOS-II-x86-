/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                          (c) Copyright 2004-... Werner.Zimmermann@hs-esslingen.de
*                 (Similar to Example 1 of the 80x86 Real Mode port by Jean J. Labrosse)
*                                           All Rights Reserved
*
*                                           Application Template
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 512       /* Size of start task's stacks                        */
#define  TASK_START_PRIO	       0	 /* Priority of your startup task		       */
/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStartStk[TASK_STK_SIZE];	/* Start task's stack				       */

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void  TaskStart(void *data);                  /* Function prototypes of Startup task                   */

/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/
int main (void)
{
    OSInit();                                              /* Initialize uC/OS-II                      */

    /* ToDo: Create semaphores, mailboxes etc. here						       */

    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], TASK_START_PRIO);	/* Create the startup task*/

    OSStart();                                             /* Start multitasking                       */

    return 0;
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void  TaskStart (void *pdata)
{   INT16S key;
    pdata = pdata;                                         /* Prevent compiler warning                 */


#if OS_TASK_STAT_EN > 0
    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */
#endif

    /* Create all other application tasks here        						       */
#ifdef __WIN32__
    printf("##### uCOS-II V%4.2f Port V%4.2f for WIN32 #####\n", ((FP32)OSVersion())/100, ((FP32)OSPortVersion())/100);
    printf("##### (C) by Werner.Zimmermann@hs-esslingen - Startup task\n");
#endif
#ifdef __LINUX__
    printf("##### uCOS-II V%4.2f Port V%4.2f for LINUX #####\n", ((FP32)OSVersion())/100, ((FP32)OSPortVersion())/100);
    printf("##### (C) by Werner.Zimmermann@hs-esslingen - Startup task\n");
#endif

    while (1)						   /* Startup task's infinite loop	       */
    {
	/* ToDo: Place additional code for your startup task, which you want to run once or periodically here */

        printf("Now running for %6u ticks - press ESC to stop\n", OSTime);

        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                exit(0);  	                           /* End program                              */
            }
        }

        /* ToDo: Don't forget to call the uCOS scheduler with OSTimeDly etc., to give other tasks a chance to run */
        OSTimeDly(100);                        		   /* Wait 100 ticks                           */
    }
}
