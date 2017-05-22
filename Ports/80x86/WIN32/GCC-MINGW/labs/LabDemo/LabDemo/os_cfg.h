/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*                                  uC/OS-II Configuration File for V2.52
*                                         with additions for V2.8x
*                          (c) Copyright 1992-2001, Jean J. Labrosse, Weston, FL
*
*                                         WIN32 PORT & LINUX PORT
*                          (c) Copyright 2004-... Werner.Zimmermann@fht-esslingen.de
*
*                                           All Rights Reserved
*
*
* File : OS_CFG.H
* By   : Jean J. Labrosse, modified for the WIN32 port by Werner Zimmermann
*********************************************************************************************************
*/

#ifndef OS_CFG_H
#define OS_CFG_H

/*
*********************************************************************************************************
*                                         uC/OS-II CONFIGURATION
*********************************************************************************************************
*/

#define OS_ARG_CHK_EN             1    /* Enable (1) or Disable (0) argument checking                  */

#define OS_CPU_HOOKS_EN           1    /* uC/OS-II hooks are found in the processor port files         */

#define OS_LOWEST_PRIO           63    /* Defines the lowest priority that can be assigned ...         */
                                       /* ... MUST NEVER be higher than 63 (since V2.8x: 254)!         */

#define OS_MAX_EVENTS           128    /* Max. number of event control blocks in your application ...  */
                                       /* ... MUST be > 0 , should be >= no of mailboxes + queues + mutexes + semaphores */
#define OS_MAX_FLAGS             32    /* Max. number of Event Flag Groups    in your application ...  */
                                       /* ... MUST be > 0                                              */
#define OS_MAX_MEM_PART          32    /* Max. number of memory partitions ...                         */
                                       /* ... MUST be > 0                                              */
#define OS_MAX_QS                32    /* Max. number of queue control blocks in your application ...  */
                                       /* ... MUST be > 0                                              */
#define OS_MAX_TASKS             62    /* Max. number of tasks in your application ...                 */
                                       /* ... MUST be >= 2                                             */

#define OS_TASK_IDLE_STK_SIZE   512    /* Idle task stack size (# of OS_STK wide entries)              */

#define OS_TASK_STAT_EN           0    /* Enable (1) or Disable(0) the statistics task                 */
#define OS_TASK_STAT_STK_SIZE   512    /* Statistics task stack size (# of OS_STK wide entries)        */

#define OS_SCHED_LOCK_EN          1    /*     Include code for OSSchedLock() and OSSchedUnlock()       */

#define OS_TICKS_PER_SEC          60    /* Set the number of ticks in one second                        */

                                       /* ----------------------- EVENT FLAGS ------------------------ */
#define OS_FLAG_EN                1    /* Enable (1) or Disable (0) code generation for EVENT FLAGS    */
#define OS_FLAG_WAIT_CLR_EN       1    /* Include code for Wait on Clear EVENT FLAGS                   */
#define OS_FLAG_ACCEPT_EN         1    /*     Include code for OSFlagAccept()                          */
#define OS_FLAG_DEL_EN            1    /*     Include code for OSFlagDel()                             */
#define OS_FLAG_QUERY_EN          1    /*     Include code for OSFlagQuery()                           */


                                       /* -------------------- MESSAGE MAILBOXES --------------------- */
#define OS_MBOX_EN                1    /* Enable (1) or Disable (0) code generation for MAILBOXES      */
#define OS_MBOX_ACCEPT_EN         1    /*     Include code for OSMboxAccept()                          */
#define OS_MBOX_DEL_EN            1    /*     Include code for OSMboxDel()                             */
#define OS_MBOX_POST_EN           1    /*     Include code for OSMboxPost()                            */
#define OS_MBOX_POST_OPT_EN       1    /*     Include code for OSMboxPostOpt()                         */
#define OS_MBOX_QUERY_EN          1    /*     Include code for OSMboxQuery()                           */


                                       /* --------------------- MEMORY MANAGEMENT -------------------- */
#define OS_MEM_EN                 1    /* Enable (1) or Disable (0) code generation for MEMORY MANAGER */
#define OS_MEM_QUERY_EN           1    /*     Include code for OSMemQuery()                            */


                                       /* ---------------- MUTUAL EXCLUSION SEMAPHORES --------------- */
#define OS_MUTEX_EN               1    /* Enable (1) or Disable (0) code generation for MUTEX          */
#define OS_MUTEX_ACCEPT_EN        1    /*     Include code for OSMutexAccept()                         */
#define OS_MUTEX_DEL_EN           1    /*     Include code for OSMutexDel()                            */
#define OS_MUTEX_QUERY_EN         1    /*     Include code for OSMutexQuery()                          */


                                       /* ---------------------- MESSAGE QUEUES ---------------------- */
#define OS_Q_EN                   1    /* Enable (1) or Disable (0) code generation for QUEUES         */
#define OS_Q_ACCEPT_EN            1    /*     Include code for OSQAccept()                             */
#define OS_Q_DEL_EN               1    /*     Include code for OSQDel()                                */
#define OS_Q_FLUSH_EN             1    /*     Include code for OSQFlush()                              */
#define OS_Q_POST_EN              1    /*     Include code for OSQPost()                               */
#define OS_Q_POST_FRONT_EN        1    /*     Include code for OSQPostFront()                          */
#define OS_Q_POST_OPT_EN          1    /*     Include code for OSQPostOpt()                            */
#define OS_Q_QUERY_EN             1    /*     Include code for OSQQuery()                              */


                                       /* ------------------------ SEMAPHORES ------------------------ */
#define OS_SEM_EN                 1    /* Enable (1) or Disable (0) code generation for SEMAPHORES     */
#define OS_SEM_ACCEPT_EN          1    /*    Include code for OSSemAccept()                            */
#define OS_SEM_DEL_EN             1    /*    Include code for OSSemDel()                               */
#define OS_SEM_QUERY_EN           1    /*    Include code for OSSemQuery()                             */


                                       /* --------------------- TASK MANAGEMENT ---------------------- */
#define OS_TASK_CHANGE_PRIO_EN    0    /*     Include code for OSTaskChangePrio()                      */
#define OS_TASK_CREATE_EN         1    /*     Include code for OSTaskCreate()                          */
#define OS_TASK_CREATE_EXT_EN     1    /*     Include code for OSTaskCreateExt()                       */
#define OS_TASK_DEL_EN            1    /*     Include code for OSTaskDel()                             */
#define OS_TASK_SUSPEND_EN        1    /*     Include code for OSTaskSuspend() and OSTaskResume()      */
#define OS_TASK_QUERY_EN          1    /*     Include code for OSTaskQuery()                           */


                                       /* --------------------- TIME MANAGEMENT ---------------------- */
#define OS_TIME_DLY_HMSM_EN       1    /*     Include code for OSTimeDlyHMSM()                         */
#define OS_TIME_DLY_RESUME_EN     1    /*     Include code for OSTimeDlyResume()                       */
#define OS_TIME_GET_SET_EN        1    /*     Include code for OSTimeGet() and OSTimeSet()             */


                                       /* ---------------------- MISCELLANEOUS ----------------------- */
#if OS_VERSION < 280
  #ifndef NO_TYPEDEF_OS_FLAGS
    typedef INT32U OS_FLAGS;   	       /* Date type for event flag bits (8, 16 or 32 bits)             */
  #endif
#endif

/* New options in V2.8x ********************************************************************************/
                                       /* ----------------------- uC/OS-View -------------------------   */
#define OS_VIEW_MODULE            0    /* When 1, indicate that uC/OS-View is present                    */
                                       /* --------------------- TASK STACK SIZE ----------------------   */
#define OS_TASK_TMR_STK_SIZE    128    /* Timer      task stack size (# of OS_STK wide entries)          */
                                       /* ---------------------- MISCELLANEOUS -----------------------   */
#define OS_DEBUG_EN               0    /* Enable(1) debug variables                                      */
#define OS_EVENT_NAME_SIZE       16    /* Determine the size of the name of a Sem, Mutex, Mbox or Q      */
#define OS_TASK_STAT_STK_CHK_EN   1    /* Check task stacks from statistic task                          */
#define OS_TICK_STEP_EN           1    /* Enable tick stepping feature for uC/OS-View                    */
                                       /* ----------------------- EVENT FLAGS ------------------------   */
#define OS_FLAG_NAME_SIZE        16    /*     Determine the size of the name of an event flag group      */
#define OS_FLAGS_NBITS           32    /* Size in #bits of OS_FLAGS data type (8, 16 or 32)              */
                                       /* -------------------- MESSAGE MAILBOXES --------------------- */
#define OS_MBOX_PEND_ABORT_EN     1    /*     Include code for OSMboxPendAbort()                       */
                                       /* --------------------- MEMORY MANAGEMENT --------------------   */
#define OS_MEM_NAME_SIZE         16    /*     Determine the size of a memory partition name              */
                                       /* ---------------------- MESSAGE QUEUES ---------------------- */
#define OS_Q_PEND_ABORT_EN        1    /*     Include code for OSQPendAbort()                          */
                                       /* ------------------------ SEMAPHORES ------------------------   */
#define OS_SEM_PEND_ABORT_EN      1    /*    Include code for OSSemPendAbort()                         */
#define OS_SEM_SET_EN             1    /*    Include code for OSSemSet()                                 */
                                       /* --------------------- TASK MANAGEMENT ----------------------   */
#define OS_TASK_NAME_SIZE        16    /*     Determine the size of a task name                          */
#define OS_TASK_PROFILE_EN        1    /*     Include variables in OS_TCB for profiling                  */
#define OS_TASK_SW_HOOK_EN        1    /*     Include code for OSTaskSwHook()                            */
                                       /* --------------------- TIME MANAGEMENT ----------------------   */
#define OS_TIME_TICK_HOOK_EN      1    /*     Include code for OSTimeTickHook()                          */
                                       /* --------------------- TIMER MANAGEMENT ---------------------   */
#define OS_TMR_EN                 0    /* Enable (1) or Disable (0) code generation for TIMERS           */
#define OS_TASK_TMR_PRIO (OS_LOWEST_PRIO-4)	 /* Priority of timer management task                    */
#define OS_TMR_CFG_MAX           16    /*     Maximum number of timers                                   */
#define OS_TMR_CFG_NAME_SIZE     16    /*     Determine the size of a timer name                         */
#define OS_TMR_CFG_WHEEL_SIZE     8    /*     Size of timer wheel (#Spokes)                              */
#define OS_TMR_CFG_TICKS_PER_SEC 10    /*     Rate at which timer management task runs (Hz)              */

#define OS_APP_HOOKS_EN 1
#define OS_EVENT_MULTI_EN 1

#endif
