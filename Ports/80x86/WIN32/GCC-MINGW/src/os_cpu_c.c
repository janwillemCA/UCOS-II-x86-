/*
*********************************************************************************************************
*                                                uC/OS-II
*                                The Real-Time Kernel (by Jean J. Labrosse)
*
*                                             WIN32 PORT
*
*                          (c) Copyright 2004-... Werner.Zimmermann@hs-esslingen.de
*                                           All Rights Reserved
*
* File : OS_CPU_C.C
* By   : Werner Zimmermann
*
* Changes:  1.40 Fixed a bug in OS_log(), thanks to Hu JW. 13 Jul 2004
*	    1.50 Substituted DWORD by INT32U, BOOL by BOOLEAN
*	    1.60 Changes in the PC_Elapsed... functions in the Cygwin and Linux port. Use direct reads
*		 for the Pentium time stamp counter. No change in the Visual C++ port.
*           1.70 Changes to OSScheduleThread for detecting tasks deleted by OSTaskDel in the WIN32 port.
*                Suggested by Hu JW.
*	    2.00 Modifications in the Linux port, no changes in the WIN32 ports, 18 Mar 2005
*	    2.10 Include paths modified to work with uCOS-II 2.70
*           3.00 Modifications to work with uCOS-II 2.80, 30 May 2006, OSTimeTickCallback removed,
*		 made OSTaskIdleHook() and OSTaskCreateHook() instead of OSTCBInitHook() user callable.
*           3.10 Modification in os_cfg.h to ensure compatibility wiht uCOS-II 2.84, 06 April 2007
*		 Major changes in OSScheduleThread() and OSTcbInitHook() to make the code compatible
*		 with Validated Software's verification suite for uCOS-II (www.validatedsoftware.com
*	    3.20 Updated mail address and copyright messages, 03 March 2008
*           3.30 Modified OSTaskDelHook() to avoid memory leakage when deleting tasks. 26 March 2008
	    3.40 Make sources compatible with Visual Studio .NET 2003 and 2005
*
*********************************************************************************************************
*/
#define OS_PORT_VERSION 340					//Version number of the uCOS-II WIN32 port

/*
*********************************************************************************************************
   Includes
*********************************************************************************************************
*/
#define _CRT_SECURE_NO_WARNINGS

#define _WIN32_WINNT  0x0400
#include    <windows.h>
#include    <winbase.h>
#include    <mmsystem.h>

#include    <stdio.h>
#include    <stdlib.h>
#include    <assert.h>

#if (OS_VERSION <= 270)
#include    "os_cpu.h"
#include    "os_cfg.h"
#include    "ucos_ii.h"
#else
#include    "ucos_ii.h"
#endif
/*
*********************************************************************************************************
   Global variables
*********************************************************************************************************
*/
HANDLE  hScheduleEvent, hScheduleThread;			//Scheduler thread variables
volatile HANDLE  hTaskThread[OS_LOWEST_PRIO + 2];		//Map uCOS tasks to WIN32 threads
volatile INT8S   taskSuspended[OS_LOWEST_PRIO + 2];		//Suspend status of mapped tasks/threads
volatile OS_TCB  *pTaskTcb[OS_LOWEST_PRIO + 2];			//Pointer to task TCBs

#define NINTERRUPTS 8						//Number of interrupt events (DO NOT CHANGE)
HANDLE  hInterruptEvent[NINTERRUPTS], hInterruptThread;		//Interrupt handling thread variables
void (*interruptTable[NINTERRUPTS])();
BOOLEAN virtualInterruptFlag=TRUE;
INT32U   interruptThreadId = 0;

CRITICAL_SECTION criticalSection;				//Used to protect critical sections

BOOLEAN idleTrigger = TRUE;					//Trigger a message, when the idle task is
								//invoked (used for debugging purposes)

#define NLOG	16						//Log last scheduled tasks (used for
INT16U taskLog[NLOG];						//... debugging purposes)

int GetThreadIndexForTask(OS_TCB *pTcb);			//Get index of thread in hTaskThread
void ExecuteDeleteTask(int i);

/*
*********************************************************************************************************
   Port-specific functions
*********************************************************************************************************
*/

// DBGPRINT ******************************************************************
// Debug output
void DBGPRINT(INT32U debugLevel, const char *fmt,...)
{   va_list argptr;
    FILE *fd;

    if ((debugLevel & DEBUGLEVEL) == 0)				//Debug output selection
        return;
    if (DEBUGLEVEL < 0x10000000UL)                              //Screen output (does influence real-time performance!)
    {   va_start(argptr, fmt);
        vprintf(fmt, argptr);
        va_end(argptr);
    } else                                                      //File output (does influence real-time performance!)
    {   va_start(argptr, fmt);
        if ((fd = fopen("ucos.log","a+"))!=NULL)
        {   vfprintf(fd, fmt, argptr);
            fclose(fd);
        }
        va_end(argptr);
    }
}

// DumpTaskList ****************************************************************
// Dump a list of all OS tasks by parsing the TCB list
void DumpTaskList(void)
{   int i;
    OS_TCB *pTcb = OSTCBList;

    if (OSTCBCur)
	printf("Cur  pTcb=%X  Prio=%d  Stat=%d\n", OSTCBCur, OSTCBCur->OSTCBPrio, OSTCBCur->OSTCBStat);

    while (pTcb)
    {	printf("Task pTcb=%X  Prio=%d  Stat=%d ", pTcb, pTcb->OSTCBPrio, pTcb->OSTCBStat);
	for (i=0; i <= OS_LOWEST_PRIO; i++) 			//Search for task in thread array
	{   if (pTcb == pTaskTcb[i])
	    {   break;
	    }
	}
	if (i > OS_LOWEST_PRIO)
	{   printf("Thread: not found\n");
	    Beep(400, 500);
	} else
	{   printf("Thread Index: %d  Suspend: %d\n", i, taskSuspended[i]);
	}
    	pTcb = pTcb->OSTCBNext;
    }
}

// OSLog *********************************************************************
// Log the last NLOG scheduled tasks in taskLog (with taskLog[0] = last task)
void OSLog(INT16U prio)
{   int i;

    for (i=NLOG-1; i > 0; i--)					//Shift the previous logged values by one
        taskLog[i]=taskLog[i-1];
    taskLog[0]=prio;						//Log the last one into taskLog[0]
}

// OSPortVersion *************************************************************
// Return the version number of the uCOS-II WIN32 port
INT16U OSPortVersion(void)
{    return OS_PORT_VERSION;
}

// Handle Control - Break and Control - C
BOOLEAN CtrlBreakHandler(INT32U ctrl)
{   if (ctrl==CTRL_C_EVENT)					//Handler if CTRL-C is pressed
    {	printf("---Exiting OSPrioCur=%u-------------\n", OSPrioCur);	//---Display current task priority
    } else if (ctrl==CTRL_BREAK_EVENT)				//Handler if CTRL-BREAK is pressed
    {	printf("---Exiting OSPrioCur=%u-------------\n", OSPrioCur);	//---Display current task priority and exit
 	exit(0);
    }
    return TRUE;
}

// OSEnableInterruptFlag ****************************************************
// Enable the interrupt flag
#if OS_CRITICAL_METHOD == 3
    void OSEnableInterruptFlag(OS_CPU_SR* pCpu_Sr)
#else
    void OSEnableInterruptFlag(void)
#endif
{
    if (virtualInterruptFlag==FALSE)				//If the timer interrupt previously was disabled,
    {
#if OS_CRITICAL_METHOD == 3
	virtualInterruptFlag=*pCpu_Sr;
#else
    	virtualInterruptFlag=TRUE;
#endif
        if ((virtualInterruptFlag==TRUE) && (GetCurrentThreadId()!=interruptThreadId))
            ResumeThread(hInterruptThread);			//... resume the interrupt thread
    }
    DBGPRINT(0x00000080, ">>> ODEnableInterruptFlag %2d\n", virtualInterruptFlag);
}

// OSDisableInterruptFlag ****************************************************
// Disable the Interrupt Flag
#if OS_CRITICAL_METHOD == 3
     void OSDisableInterruptFlag(OS_CPU_SR* pCpu_Sr)
#else
     void OSDisableInterruptFlag(void)
#endif
{
    if (virtualInterruptFlag==TRUE)				//If the timer interrupt previously was enabled,
    {   if (GetCurrentThreadId()!=interruptThreadId)		//... suspend the interrupt thread ...
            SuspendThread(hInterruptThread);
#if OS_CRITICAL_METHOD == 3
        *pCpu_Sr = (OS_CPU_SR) virtualInterruptFlag;
        virtualInterruptFlag=FALSE;
#else
	virtualInterruptFlag=FALSE;
#endif
    }
    DBGPRINT(0x00000080, ">>> OSDisableInterrupts   %2d\n", virtualInterruptFlag);
}

// OSDummyISR *****************************************************************
// Dummy interrupt service routine, if an interrupt is called for which an ISR has
// has not been installed
void OSDummyISR(void)
{   MessageBox(NULL, "Got unsupported interrupt", "uCOS-II WIN32", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
}

// GetThreadIndexForTask ******************************************************
// Search, if the specified Tcb is already known. If yes, return thread index,
// otherwise return -1
int GetThreadIndexForTask(OS_TCB *pTcb)
{   int i, j;
    for (i=0; i <= OS_LOWEST_PRIO+2; i++) 			//Search thread array for known priority
    {   if (pTcb == pTaskTcb[i])				//If found in thread array, ...
	{   for (j=0; j <= OS_LOWEST_PRIO+2; j++)		//... search in task array
	    {	if (pTcb == OSTCBPrioTbl[j])			//If found in task array, ...
		{   if (i==j)					//... check, if priorities are the same
			return j;
		    else
		    {	if (hTaskThread[j]!=NULL)
			{   printf("ERROR: Thread j=%d already exists\n", j);
			    exit(-1);
			}
			//printf("INFO: Changed task priority from %d to %d\n", i, j);
			//DumpTaskList();
			hTaskThread[j]	=hTaskThread[i];
			taskSuspended[j]=taskSuspended[i];
			pTaskTcb[j]	=pTaskTcb[i];
			hTaskThread[i]	=NULL;
			taskSuspended[i]=0;
			pTaskTcb[i]	=NULL;
			//DumpTaskList();
			return j;
		    }
		}
	    }
	}
    }
    return -1;
}

// OSScheduleThread ***********************************************************
// Start tasks, triggered by hScheduleEvent
void OSScheduleThread(INT32U param)
{   char temp[256];
    INT16S oldIndex, nextIndex;
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR cpu_sr = TRUE;
#endif
    DBGPRINT(0x00000001, "*** OSScheduleThread First Call\n");

    while (1)
    {   if (WaitForSingleObject(hScheduleEvent, OS_SCHEDULER_TIMEOUT) == WAIT_TIMEOUT)	//Wait for a scheduler event (with timeout)
        {   sprintf(temp, "ERROR: Scheduler timed out in OSScheduleThread  %u --> %u   IF=%u  <-%u<%u<%u<%u<%u<%u<%u<%u<-\n",
                                                OSPrioCur, OSPrioHighRdy, virtualInterruptFlag,
                                                taskLog[0], taskLog[1], taskLog[2], taskLog[3],
                                                taskLog[4], taskLog[5], taskLog[6], taskLog[7]
                                                );
            DBGPRINT(0x00000040, temp);
            MessageBox(NULL, temp, "UCOS-II WIN32", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);	//In case of timeout, display an error message ...
            OSRunning=0;

            SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlBreakHandler, FALSE);
            return;										//... and exit (will return to OSStartHighRdy())
        }
        OSTaskSwHook();									//Call the task switch hook function
        EnterCriticalSection(&criticalSection);

	if (taskSuspended[OSPrioCur]==-1)
	{   DBGPRINT(0x00000001, "Old Task %d marked for deletion - %X - %X\n", OSPrioCur, hTaskThread[OSPrioCur], OSTCBCur);
	    SuspendThread(hTaskThread[OSPrioCur]);
	    ExecuteDeleteTask(OSPrioCur);
	    oldIndex = OSPrioCur;
        } else
        {   oldIndex = GetThreadIndexForTask(OSTCBCur);
        }
	nextIndex= GetThreadIndexForTask(OSTCBHighRdy);
	if (oldIndex==-1 || nextIndex==-1)
	{   printf("ERROR: Internal scheduling error: oldIndex=%d - %d   nextIndex=%d - %d\n",oldIndex, OSTCBCur->OSTCBPrio, nextIndex, OSTCBHighRdy->OSTCBPrio);
	    exit(-1);
	}

	OSTCBCur  = OSTCBHighRdy;							//Now logically switch to the new task
        OSPrioCur = (INT8U) nextIndex;

        idleTrigger = TRUE;
        DBGPRINT(0x00000001, "*** OSScheduleThread from %2u to %2u\n", oldIndex, nextIndex);

        if (OSTCBPrioTbl[oldIndex] == NULL)						//If a task has been deleted
        {   if (hTaskThread[oldIndex])							//... remove it, normally this should never happen
            {   if (TerminateThread(hTaskThread[oldIndex], 0)==FALSE)
            	{   printf("ERROR: Terminate thread for task %d (in OSScheduleThread) WIN32 error code: %Xh\n", oldIndex, GetLastError());
        	}
		CloseHandle(hTaskThread[oldIndex]);
            }
            hTaskThread[oldIndex] = NULL;
            taskSuspended[oldIndex]=0;
	} else
	if (oldIndex != nextIndex && taskSuspended[oldIndex]==0)			//If switching context to a new task ...
        {   if (SuspendThread(hTaskThread[oldIndex])!=0)				//... suspend the thread associated with the current task
                printf("ERROR: SuspendThread() (in OSSCheduleThread) failed with error %Xh - task %d - %d\n", GetLastError(), oldIndex, taskSuspended[oldIndex]);
            taskSuspended[oldIndex]++;							//(update suspend counter to avoid multiple suspends of the same task)
        }

        if (taskSuspended[nextIndex]>0)
        {   taskSuspended[nextIndex]--;							//(updates suspend counter to avoid multiple resumes of the same task)
            if (taskSuspended[nextIndex] < 0)
                taskSuspended[nextIndex]=0;
            if (virtualInterruptFlag==FALSE)
#if OS_CRITICAL_METHOD == 3
                OSEnableInterruptFlag(&cpu_sr);
#else
                OSEnableInterruptFlag();
#endif
            OSLog(nextIndex);
            if (ResumeThread(hTaskThread[nextIndex])!=1)					//... and resume the thread associated with the new task
                printf("ERROR: ResumeThread() (in OSSCheduleThread) failed with error %Xh\n", GetLastError());
        } else
        {   if (virtualInterruptFlag==FALSE)
#if OS_CRITICAL_METHOD == 3
                OSEnableInterruptFlag(&cpu_sr);
#else
                OSEnableInterruptFlag();
#endif
        }
        LeaveCriticalSection(&criticalSection);
    }
}

// OSInterruptThread **********************************************************
// Time tick interrupt processing
void OSInterruptThread(INT32U param)
{   char temp[256];
    INT32U eventType;

    DBGPRINT(0x00000001, "*** OSInterruptThread First Call\n");

    while (1)
    {   //if (WaitForSingleObject(hInterruptEvent, OS_INTERRUPT_TIMEOUT) == WAIT_TIMEOUT)	//Wait for a timer interrupt event
    	eventType=WaitForMultipleObjects(NINTERRUPTS, hInterruptEvent, FALSE, OS_INTERRUPT_TIMEOUT);
    	if (eventType == WAIT_TIMEOUT)	//Wait for a timer interrupt event
        {   sprintf(temp, "ERROR: Interrupt timed out in OSInterruptThread   IF=%u\n", virtualInterruptFlag);
            DBGPRINT(0x00000040, temp);
            MessageBox(NULL, temp, "UCOS-II WIN32", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);		//In case of timeout, display an error message ...
            OSRunning=0;
            SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlBreakHandler, FALSE);
            exit(-1);									//... and exit
        }
        EnterCriticalSection(&criticalSection);
        DBGPRINT(0x00000001, "*** OSInterruptThread\n");
        if (virtualInterruptFlag==FALSE)
            DBGPRINT(0x00000001, "*** virtualInteruptFlag disabled when calling OSInterruptThread XXX %d\n", virtualInterruptFlag);
        DBGPRINT(0x00000001, "--- OSIntEnter\n");
        OSIntEnter();

        if (eventType==WAIT_OBJECT_0)							//Call uCOS' interrupt processing
        {   ResetEvent(hInterruptEvent[0]);
            DBGPRINT(0x00000001, "--- OSTimeTick\n");
            interruptTable[0]();							//IRQ0 reserved for time tick: OSTimeTick();
        } else if (eventType==WAIT_OBJECT_0+1)
        {   ResetEvent(hInterruptEvent[1]);
            interruptTable[1]();
        } else if (eventType==WAIT_OBJECT_0+2)
        {   ResetEvent(hInterruptEvent[2]);
            interruptTable[2]();
        } else if (eventType==WAIT_OBJECT_0+3)
        {   ResetEvent(hInterruptEvent[3]);
            interruptTable[3]();
        } else if (eventType==WAIT_OBJECT_0+4)
        {   ResetEvent(hInterruptEvent[4]);
            interruptTable[4]();
        } else if (eventType==WAIT_OBJECT_0+5)
        {   ResetEvent(hInterruptEvent[5]);
            interruptTable[5]();
        } else if (eventType==WAIT_OBJECT_0+6)
        {   ResetEvent(hInterruptEvent[6]);
            interruptTable[6]();
        } else if (eventType==WAIT_OBJECT_0+7)
        {   ResetEvent(hInterruptEvent[7]);
            interruptTable[7]();
        } else
        {   MessageBox(NULL, temp, "UCOS-II WIN32", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);		//In case of timeout, display an error message ...
        }
        DBGPRINT(0x00000001, "--- OSIntExit\n");
        OSIntExit();
        LeaveCriticalSection(&criticalSection);
    }
}


/*
*********************************************************************************************************
   uCOS-II Functions
*********************************************************************************************************
*/

// OSTimeTickInit ************************************************************
// Initialize the WIN32 multimedia timer to simulate time tick interrupts
void OSTimeTickInit()
{   TIMECAPS timecaps;

    DBGPRINT(0x00000008, "*** OSTimeTickInit\n");

    if (timeGetDevCaps((LPTIMECAPS) & timecaps, sizeof(timecaps)) != TIMERR_NOERROR)
    {   printf("uCOS-II ERROR: Timer could not be installed 1\n");
        exit(-1);
    }
    if (timeBeginPeriod(timecaps.wPeriodMin) != TIMERR_NOERROR)
    {   printf("uCOS-II ERROR: Timer could not be installed 2\n");
        exit(-1);
    }
    if (timeSetEvent(1000 / OS_TICKS_PER_SEC, 0, (LPTIMECALLBACK) hInterruptEvent[0], 0, TIME_PERIODIC | TIME_CALLBACK_EVENT_SET) == 0)
    {   printf("uCOS-II ERROR: Timer could not be installed 3\n");
        exit(-1);
    }
}

// OSTaskStkInit *************************************************************
// This function does initialize the stack of a task (this is only a dummy function
// for compatibility reasons, because this stack is not really used (except to refer
// to the task parameter *pdata)
OS_STK *OSTaskStkInit(void (*task) (void *pd), void *pdata, OS_STK *ptos, INT16U opt)
{   OS_STK *stk;

    DBGPRINT(0x00000010, "*** OSTaskStkInit\n");

    stk = (OS_STK *) ptos;                                      // Load stack pointer
    *--stk = (INT32U) pdata;					// Push pdata on the stack
    *--stk = (INT32U) task;					// Push the task start address on the stack

    return stk;
}


/*
*******************************************************************************
   Internal Hook functions (used by the WIN32 port, not user hookable)
*******************************************************************************
*/

// OSInitHookBegin ***********************************************************
// This hook is invoked at the beginning of the OS initialization. MUST NOT BE DEFINED BY THE APPLICATION.
void OSInitHookBegin()
{   int i;
    char temp[256];
    static BOOLEAN isInitialized = FALSE;

    DBGPRINT(0x00000010, "*** OSInitHookBegin\n");

    if (isInitialized)
    {	for (i=0; i < OS_LOWEST_PRIO + 2; i++)
    	{   if (hTaskThread[i])
    	    {	if (TerminateThread(hTaskThread[i], 0)==FALSE)
        	{   printf("ERROR: Terminate thread for task %d (in OSInitHookBegin) %Xh\n", i, GetLastError());
        	}
    	        CloseHandle(hTaskThread[i]);
    	    }
	    hTaskThread[i]=NULL;
	    taskSuspended[i]=0;
	    pTaskTcb[i]=NULL;
    	}
    	return;
    } else
    {	isInitialized=TRUE;
    }

    SetProcessAffinityMask(GetCurrentProcess(), 0x1); // dwProcessAffinityMask -> 1 = run only on CPU 0
    hScheduleEvent  = CreateEvent(NULL, FALSE, FALSE, NULL);	//Create the scheduler and interrupt event

    for (i=0; i< NINTERRUPTS; i++)
    {	sprintf(temp,"OSirq%u",i);
    	hInterruptEvent[i] = CreateEvent(NULL, TRUE, FALSE, temp);
        interruptTable[i]  = OSDummyISR;
    }

    InitializeCriticalSection(&criticalSection);
    hScheduleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) OSScheduleThread,  NULL, 0, (INT32U*) &i);	//Create the scheduler and interrupt thread
    hInterruptThread= CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) OSInterruptThread, NULL, 0, &interruptThreadId);
    interruptTable[0] = OSTimeTick;

    SetPriorityClass( hScheduleThread,  THREAD_PRIORITY_HIGHEST);//Set the scheduler and interrupt threads to maximum WIN32 priority
    SetThreadPriority(hScheduleThread,  THREAD_PRIORITY_TIME_CRITICAL);
    SetPriorityClass( hInterruptThread, THREAD_PRIORITY_HIGHEST);
    SetThreadPriority(hInterruptThread, THREAD_PRIORITY_HIGHEST);

    SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlBreakHandler, TRUE);
    Sleep(0);							//Give Windows a chance to start the scheduler and interrupt thread

    //ADD YOUR OWN HOOK CODE HERE IF NECESSARY
}

// OSTCBInitHook **********************************************************
// This hook is invoked during task creation. MUST NOT BE DEFINED BY THE APPLICATION.
void OSTCBInitHook(OS_TCB * pTcb)
{   int i;

    DBGPRINT(0x00000004, "*** OSTCBInitHook %u\n", pTcb->OSTCBPrio);

    if ((i=GetThreadIndexForTask(pTcb))!=-1)			//Test if the TCB is already known
    {	if (TerminateThread(hTaskThread[i], 0)==FALSE)
        {   printf("ERROR: Terminate thread (in OSTCBInitHook) %Xh\n", GetLastError());
        }
        CloseHandle(hTaskThread[i]);
        hTaskThread[i]=NULL;
        taskSuspended[i]=0;
        pTaskTcb[i]=NULL;
        Beep(400, 1000);
        printf("INFO: A thread for a task with priority %d (original prio %d) already existed and was terminated\n", i, pTcb->OSTCBPrio);
    }

    if ((hTaskThread[pTcb->OSTCBPrio] = CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE) *(pTcb->OSTCBStkPtr),
                                                (void *) *(pTcb->OSTCBStkPtr + 1), CREATE_SUSPENDED, (INT32U*) &i))==NULL)	//Map uCOS-II task to WIN32 thread
    {   printf("ERROR: Create thread error %Xh\n", pTcb->OSTCBPrio, GetLastError());
    	assert(hTaskThread[pTcb->OSTCBPrio]);
    }

    taskSuspended[pTcb->OSTCBPrio]=1;									//Create thread in WIN32 suspended state
    pTaskTcb[pTcb->OSTCBPrio]=pTcb;

    if (OS_BOOST_WIN32_PRIORITY && (pTcb->OSTCBPrio!=OS_LOWEST_PRIO))           //Boost WIN32 thread priorites (except idle thread)
        SetThreadPriority(hTaskThread[pTcb->OSTCBPrio], THREAD_PRIORITY_ABOVE_NORMAL);

    //ADD YOUR OWN HOOK CODE HERE IF NECESSARY
}

void RemoteExitThread(void)					// This functions exits the WIN32 thread associated with a deleted uCOS task
{   //SetEvent(hScheduleEvent);					// Call the scheduler, just in case
    DBGPRINT(0x00000004,"Finally terminating a delete task's WIN32 thread\n");
    ExitThread(0);
}

void ExecuteDeleteTask(int i)
{    CONTEXT ctx;
     assert(taskSuspended[i]==-1);

     DBGPRINT(0x00000004, "Delete task %d - go\n", i);
     SetThreadPriority(hTaskThread[i], THREAD_PRIORITY_TIME_CRITICAL);
     ctx.ContextFlags=CONTEXT_FULL;				// Modify the deleted task's thread context in such a way, that it calls RemoteExitThread
     GetThreadContext(hTaskThread[i], &ctx);			// when it is resumed
     ctx.Eip=(DWORD) RemoteExitThread;
     SetThreadContext(hTaskThread[i], &ctx);
     ResumeThread(hTaskThread[i]);				// Resume the thread, so that it can terminate itself
     WaitForSingleObject(hTaskThread[i], INFINITE);
     CloseHandle(hTaskThread[i]);
     hTaskThread[i] = NULL;
     pTaskTcb[i]=NULL;
     taskSuspended[i] = 0;
     DBGPRINT(0x00000004, "Delete task %d - done\n", i);
}

// OSTaskDelHook *************************************************************
// This hook is invoked during task deletion.
void OSTaskDelHook(OS_TCB * pTcb)
{   int i;
    DBGPRINT(0x00000004, "********** Delete Task %d from Task %d********* \n", pTcb->OSTCBPrio, OSPrioCur);

    //DumpTaskList();
    if ((i=GetThreadIndexForTask(pTcb))!=-1)
    {	taskSuspended[i] = -1;					// Mark as deleted
    	if (i!=OSPrioCur)
    	{   ExecuteDeleteTask(i);
    	}else
    	{   DBGPRINT(0x00000004, "********** Delete Task %d - %d - marked to be deleted ********* %X - %X\n", i, OSPrioCur, hTaskThread[i], OSTCBCur);
    	}
    }
    //ADD YOUR OWN HOOK CODE HERE IF NECESSARY
}

// OSTaskChangePrioHook ******************************************************
/* Hook to be notified, when task priority is changed be OSChangeTaskPriority().
   This hook is not an offical uCOS-II hook. But if your programs use OSTaskChangePrio()
   and do not run correctly, try to hook this routine, by inserting a call to this hook
   in function OSTaskChangePrio (...) in file OS_Task.c. At the end of this function
   insert:

    OSTaskChangePrioHook(oldprio, newprio); //<<<--- New, insert this
    OS_EXIT_CRITICAL();			    //<<<--- Existing code

    if (OSRunning == OS_TRUE)		    //<<<--- Existing code
    {   OS_Sched();
    }

*/
void OSTaskChangePrioHook(int oldPrio, int newPrio)
{
    printf("*** OSTaskChangePrio Task %d changes priority of task %d to %d\n", OSPrioCur, oldPrio, newPrio);
    hTaskThread[newPrio]   = hTaskThread[oldPrio];
    taskSuspended[newPrio] = taskSuspended[oldPrio];
    pTaskTcb[newPrio] = pTaskTcb[oldPrio];

    DBGPRINT(0x00000004, "*** OSTaskChangePrio Task %d changes priority of task %d to %d\n", OSPrioCur, oldPrio, newPrio);

    hTaskThread[oldPrio]   = NULL;
    taskSuspended[oldPrio] = 0;
    pTaskTcb[oldPrio] = NULL;

    if (oldPrio == OSPrioCur)
	OSPrioCur = newPrio;
}

/*
*******************************************************************************
   Hook functions (user hookable)
*******************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0

/*
void OSInitHookBegin()                  MUST NOT BE DEFINED BY THE APPLICATION, see above.
*/

// OSInitHookEnd *************************************************************
// This hook is invoked at the end of the OS initialization.
void OSInitHookEnd()
{   DBGPRINT(0x00000010, "*** OSInitHookEnd\n");
}

void OSTaskCreateHook(OS_TCB * pTcb)
{   DBGPRINT(0x00000004, "*** OSTaskCreateHook %u\n", pTcb->OSTCBPrio);
}

/*
void OSTaskDelHook(OS_TCB * pTcb)	MUST NOT BE DEFINED BY THE APPLICATION, see above
*/


/*
void OSTCBInitHook(OS_TCB *pTcb)	MUST NOT BE DEFINED BY THE APPLICATION, see above
*/

// OSTaskIdleHook ************************************************************
// This hook is invoked from the idle task.
void OSTaskIdleHook()
{   if (idleTrigger)						//Issue a debug message, each time the idle task is reinvoked
    {   DBGPRINT(0x00000020, "*** OSTaskIdleHook\n");
        idleTrigger = FALSE;
    }

    OS_SLEEP();							//Give Windows a chance to run other applications to when uCOS-II idles
}

// OSTaskStatHook ************************************************************
// This hook is invoked by the statistical task every second.
void OSTaskStatHook()
{
}

// OSTimeTickHook ************************************************************
// This hook is invoked during a time tick.
void OSTimeTickHook()
{
}

// OSTaskSwHook **************************************************************
// This hook is invoked during a task switch.
// OSTCBCur points to the current task (being switched out).
// OSTCBHighRdy points on the new task (being switched in).
void OSTaskSwHook()
{
}

#else
#pragma message("INFO: Hook functions must be defined in the application")
#endif

/*
*******************************************************************************
   Internal Task switch functions
*******************************************************************************
*/

// OSStartHighRdy *************************************************************
// Start first task
void OSStartHighRdy(void)
{   //OSTaskSwHook();						//Call the task switch hook function

    OSTimeTickInit();                                           //Initialize time ticks

    // Increment OSRunning by 1
    OSRunning++;

    DBGPRINT(0x00000002, "*** OSStartHighRdy   from %2u to %2u\n", OSPrioCur, OSPrioHighRdy);

    SetEvent(hScheduleEvent);                                   //Trigger scheduling thread

    WaitForSingleObject(hScheduleThread, INFINITE);		//Wait, until the scheduling thread quits.
    								//This should never happen during normal operation
    printf("INFO: Primary thread killed - exiting\n");
}

// OSCtxSw ********************************************************************
// Task context switch
void OSCtxSw(void)
{   DBGPRINT(0x00000002, "*** OSCtxSw          from %2u to %2u\n", OSPrioCur, OSPrioHighRdy);
    SetEvent(hScheduleEvent);					//Trigger scheduling thread
}

// OSIntCtxSw *****************************************************************
// Interrupt context switch
void OSIntCtxSw(void)
{   DBGPRINT(0x00000002, "*** OSCIntCtxSw      from %2u to %2u\n", OSPrioCur, OSPrioHighRdy);
    SetEvent(hScheduleEvent);                                   //Trigger scheduling thread
}
