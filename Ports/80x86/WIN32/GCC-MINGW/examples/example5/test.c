/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*					WIN32 PORT & LINUX PORT
*                          (c) Copyright 2004, Werner.Zimmermann@fht-esslingen.de
*                                           All Rights Reserved
*
*                                               EXAMPLE #5
*             Used to study the scheduling behaviour of the WIN32 and LINUX port during development
*
* NOTE: This program will fails when running for some minutes, because OSTaskDel() does lead to memory
*       leaks. This is not a fault of uCOS-II, but a bug in the WIN32 port, which currently cannot be solved
*       due to the nature of the port's implementation. In the WIN32 port a uCOS-II task is mapped to a
*       WIN32 thread. However, WIN32 threads can only be terminated by other threads, using WIN32 API function
*       TerminateThread(), which causes memory leaks.
*
*********************************************************************************************************
*/
#define _CRT_SECURE_NO_WARNINGS  //Required by Visual C++ .NET 2005, ignored by others
#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define TASK_STK_SIZE   1024                                    // Stack size, in bytes
#define BASE_PRIO	32

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/
OS_STK  TestTaskStkX[TASK_STK_SIZE];                            // Task stacks
OS_STK  TestTaskStk[8][TASK_STK_SIZE];

int toggle = 1;

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void TestTaskX(void *pdata);                                    // Tasks
void TestTask(void *pdata);

/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/
int main(void)
{
    // Display a banner.
#ifdef __WIN32__
    printf("##### uCOS-II V%4.2f Port V%4.2f for WIN32 #####\n", ((FP32)OSVersion())/100, ((FP32)OSPortVersion())/100);
#endif
#ifdef __LINUX__
    printf("##### uCOS-II V%4.2f Port V%4.2f for LINUX #####\n", ((FP32)OSVersion())/100, ((FP32)OSPortVersion())/100);
#endif

     // Initialize uCOS-II.
    OSInit();                                                               //Calling sequence -->OSInitHookBegin-->OSTaskStkInit-->OSTCBInitHook-->OSTaskCreateHook-->OSInitHookEnd

    // Create the first task
    OSTaskCreate(TestTaskX, (void *) 0, &TestTaskStkX[TASK_STK_SIZE], 4); //Calling sequence -->OSTaskStkInit-->OSTCBInitHook-->OSTaskCreateHook

    // Start multitasking.
    OSStart();                                                              //Calling sequence -->OSTaskSwHook-->OSStartHighRdy

    /* NEVER EXECUTED */
    printf("main(): We should never execute this line\n");

    return 0;
}

/*
*********************************************************************************************************
*                                                First Task (startup task)
*********************************************************************************************************
*/
void TestTaskX(void *pdata)
{   int i;
    printf("%4u: ***** Test Task X with prio %d First call *****\n", OSTime, OSPrioCur);

#if OS_TASK_STAT_EN > 0
    OSStatInit();                                                               //Initialize the statistics task
#endif


    while (1)
    {   if (toggle==1)
        {   printf("%4u: ***** Test Task X ***** Creating new task\n", OSTime);
            OSTaskCreate(TestTask, (void*)(0), &TestTaskStk[0][TASK_STK_SIZE], BASE_PRIO);     //Create the next task
        } else if (toggle<8)
	{   OSSchedLock();
            printf("%4u: ***** Test Task X ***** Changing task priorities\n", OSTime);
	    if (toggle % 2)
	    {   for (i=0; i<8; i++)
      	     	    OSTaskChangePrio(BASE_PRIO+i,BASE_PRIO-i);
      	    } else
      	    {  for (i=0; i<8; i++)
      	    	    OSTaskChangePrio(BASE_PRIO-i,BASE_PRIO+i);
      	    }
            OSSchedUnlock();
	} else if (toggle==9)
	{   printf("%4u: ***** Test Task X ***** Deleting tasks 0 ... 7\n", OSTime);
	    if (toggle % 2)
	    {   for (i=0; i<8; i++)
      	    	    OSTaskDel(BASE_PRIO-i);
      	    } else
      	    {  for (i=0; i<8; i++)
      	    	    OSTaskDel(BASE_PRIO+i);
       	    }
	} else if (toggle>15)
    	{   toggle=0;
    	    printf("%4u: ***** Test Task X *****\n", OSTime);
        } else
	{   printf("%4u: ***** Test Task X *****\n", OSTime);
	}
        toggle++;
        OSTimeDly(1);                                                           //Calling sequence -->OSTaskSwHook-->OSCtxSw
    }
}

/*
*********************************************************************************************************
*                                                Second Task
*********************************************************************************************************
*/
void TestTask(void *pdata)
{   int i=(int) pdata;
    printf("%4u: ***** Test Task %d with prio %d First call *****\n", OSTime, i, OSPrioCur);

    if ((int) pdata<7)
    {	printf("%4u: ***** Test Task %d ***** Creating new task\n", OSTime, i);
    	OSTaskCreate(TestTask, (void*)(i+1), &TestTaskStk[(int) pdata][TASK_STK_SIZE], BASE_PRIO-(i+1));     //Create the next task
    }

    while (1)
    {   printf("%4u: ***** Test Task %d with prio %d *****\n", OSTime, i, OSPrioCur);
        if (toggle==4 && (i==6 || i==7))
        {
            printf("%4u: ***** Test Task %d with prio %d deletes itself*****\n", OSTime, i, OSPrioCur);
            OSTaskDel(OS_PRIO_SELF);
        }
        OSTimeDly(1);
    }
}
