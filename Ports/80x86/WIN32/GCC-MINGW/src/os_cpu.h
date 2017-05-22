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
* File : OS_CPU.H
* By   : Werner Zimmermann
*********************************************************************************************************
*/

#ifndef OS_CPU_H
#define OS_CPU_H

#define _CRT_SECURE_NO_WARNINGS   //Required by Visual C++ .NET 2005, ignored by others
#include <windows.h>

/*
*********************************************************************************************************
   uCOS-II standard definitions and declarations.
*********************************************************************************************************
*/

typedef unsigned char           BOOLEAN;
typedef unsigned char           INT8U;
typedef char                    INT8S;
typedef unsigned short          INT16U;
typedef short                   INT16S;
typedef unsigned long           INT32U;
typedef long                    INT32S;
typedef unsigned __int64        INT64U;
typedef __int64                 INT64S;
typedef float                   FP32;
typedef double                  FP64;

typedef INT32U                  OS_STK;
typedef unsigned int   		OS_CPU_SR;

#define  OS_CRITICAL_METHOD     3

#if OS_CRITICAL_METHOD == 3
    #define  OS_ENTER_CRITICAL()    OSDisableInterruptFlag(&cpu_sr)
    #define  OS_EXIT_CRITICAL()     OSEnableInterruptFlag(&cpu_sr)
#else
    #define  OS_ENTER_CRITICAL()    OSDisableInterruptFlag()
    #define  OS_EXIT_CRITICAL()     OSEnableInterruptFlag()
#endif

#define  OS_STK_GROWTH          1
#define  OS_TASK_SW()           OSCtxSw()


void OSCtxSw(void);
void OSIntCtxSw(void);
void OSStartHighRdy(void);

/*
*********************************************************************************************************
   Port-specific definitions and declarations
*********************************************************************************************************
*/

INT16U OSPortVersion(void);

#if OS_CRITICAL_METHOD == 3
    void OSEnableInterruptFlag(OS_CPU_SR* pCpu_Sr);
    void OSDisableInterruptFlag(OS_CPU_SR* pCpu_Sr);
#else
    void OSEnableInterruptFlag(void);
    void OSDisableInterruptFlag(void);
#endif

void OSTaskChangePrioHook(int oldPrio, int newPrio);

/* DEBUGLEVEL	These values can be logically ored to set the debug level for uCOS-II WIN32 port debugging
                Please note, that debugging will create a lot of screen messages and thus may affect
                the real-time performance of your application
   0x00000001   Scheduler
   0x00000002   Task switch
   0x00000004   Task creation/deletion
   0x00000008   Timer
   0x00000010   Initialization
   0x00000020   Idle and stat task
   0x00000040   Scheduler and Time Tick Interrupt Timeouts
   0x00000080   Interrupt-Enable/Disable
 */
#ifndef DEBUGLEVEL
#define DEBUGLEVEL 0	//0x7F
#endif

/* Timeout value in milliseconds for the scheduler - used to detect deadlocks. Set to INFINITE for "slow" applications*/
#define OS_SCHEDULER_TIMEOUT    INFINITE		//10000

/* Timeout value in milliseconds for the time tick interrupt */
#define OS_INTERRUPT_TIMEOUT    INFINITE		//10000

/* If this define is set to TRUE, uCOS-II runs with elevated WIN32 priority to ensure better (soft)-real time behaviour.
   This may decrease the performace of other Windows applications and reduce the responsiveness to user inputs,
   if your uCOS-II generates a high CPU load.
*/
#define OS_BOOST_WIN32_PRIORITY TRUE

/* Call in OSTaskIdleHook() to give non-uCOS threads a change. Otherwise the CPU load may go up to 100% even when uCOS is idling.
*/
#define  OS_SLEEP()		Sleep(10)

/* Debugging function to dump all currently active tasks
*/
void DumpTaskList(void);

#endif

