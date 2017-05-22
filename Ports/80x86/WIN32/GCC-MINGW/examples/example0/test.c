/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*					WIN32 PORT & LINUX PORT
*                          (c) Copyright 2004, Werner.Zimmermann@fht-esslingen.de
*                                           All Rights Reserved
*
*                                               EXAMPLE #0
*             Used to study the scheduling behaviour of the WIN32 and LINUX port during development
*********************************************************************************************************
*/
#define _CRT_SECURE_NO_WARNINGS   //Required by Visual C++ .NET 2005, ignored by others
#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define TASK_STK_SIZE   1024                                    // Stack size, in bytes

//#define SUSPEND_RESUME                                        // Task 1 and 2 use suspend and resume for scheduling
                                                                // (if not defined: use time delays)

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/
OS_STK  TestTaskStk1[TASK_STK_SIZE];                            // Task stacks
OS_STK  TestTaskStk2[TASK_STK_SIZE];
OS_STK  TestTaskStk3[TASK_STK_SIZE];
OS_STK  TestTaskStk4[TASK_STK_SIZE];

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void TestTask1(void *pdata);                                    // The 3 tasks
void TestTask2(void *pdata);
void TestTask3(void *pdata);
void TestTask4(void *pdata);
void OurIsr1(void);                                             // The interrupt service routine for IRQ 1

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
    OSTaskCreate(TestTask1, (void *) 11, &TestTaskStk1[TASK_STK_SIZE], 11); //Calling sequence -->OSTaskStkInit-->OSTCBInitHook-->OSTaskCreateHook

    PC_IntVectSet(1, OurIsr1);                                              //Install an interrupt service routine for IRQ 1

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
void TestTask1(void *pdata)
{   printf("%4u: ***** Test Task 1 First call *****\n", OSTime);

#if OS_TASK_STAT_EN > 0
    OSStatInit();                                                               //Initialize the statistics task
#endif

    OSTaskCreate(TestTask2, (void *) 22, &TestTaskStk2[TASK_STK_SIZE], 22);     //Create 3 other tasks
    OSTaskCreate(TestTask3, (void *) 33, &TestTaskStk3[TASK_STK_SIZE], 33);
    OSTaskCreate(TestTask4, (void *) 10, &TestTaskStk4[TASK_STK_SIZE], 10);

    while (1)
    {   printf("%4u: ***** Test Task 11 *****\n", OSTime);

#ifdef SUSPEND_RESUME
        OSTaskSuspend(OS_PRIO_SELF);                                            //Calling sequence -->OSTaskSwHook-->OSCtxSw
#else
        OSTimeDly(1);                                                           //Calling sequence -->OSTaskSwHook-->OSCtxSw
#endif
    }
}

/*
*********************************************************************************************************
*                                                Second Task
*********************************************************************************************************
*/
void TestTask2(void *pdata)
{   while (1)
    {   printf("%4u: ***** Test Task 22 *****\n", OSTime);

#ifdef SUSPEND_RESUME
        OSTaskSuspend(OS_PRIO_SELF);
#else
        OSTimeDly(1);
#endif
    }
}

/*
*********************************************************************************************************
*                                                Third Task
*********************************************************************************************************
*/
void TestTask3(void *pdata)
{   while (1)
    {
#ifdef SUSPEND_RESUME
        printf("%4u: ***** Test Task 33 *****\n", OSTime);
        OSTaskResume(11);

        printf("%4u: ***** Test Task 33 *****\n", OSTime);
        OSTaskResume(22);
#endif
        printf("%4u: ***** Test Task 33 *****\n", OSTime);
        OSTimeDly(1);

        if (kbhit())
	{   OSTaskDel(10);
	    OSTaskDel(11);
	    OSTaskDel(22);
	    exit(0);
	}
    }
}

/*
*********************************************************************************************************
*                                                Forth Task
*********************************************************************************************************
*/
void TestTask4(void *pdata)
{   while (1)
    {
        printf("%4u: +++++ Test Task 40 +++++\n", OSTime);
        OSTaskSuspend(10);                                      //Suspend yourself
    }
}


/*
*********************************************************************************************************
*                                     Interrupt service routine for IRQ 1
*                                (use "irqGenerator 1" to trigger this interrupt)
*********************************************************************************************************
*/
void OurIsr1(void)
{       OSIntEnter();
        printf("##### Interrupt service routine for IRQ 1 #####\n");
        OSTaskResume(10);                                       //Trigger task 4
        OSIntExit();
}
