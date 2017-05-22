//	Labdemo	by Jan Verhoeven

#include "includes.h"

#define TASK_STK_SIZE 256			//	stack size in bytes

OS_STK  TaskStartStack[TASK_STK_SIZE];		//	stack van TaskStart 
OS_STK  TaskOneStack[TASK_STK_SIZE];		//	stack van TaskOne 
OS_STK  TaskTwoStack[TASK_STK_SIZE];		//	stack van TaskTwo 

void TaskOne(void *pdata);			//	prototype van TaskOne 
void TaskTwo(void *pdata);			//	prototype van TaskTwo 
void TaskStart(void *pdata);			//	prototype van TaskStart 

int main(void)
{
	OSInit();				// Initialize uCOS-II. 

	OSTaskCreate(TaskStart, (void *) 0, &TaskStartStack[TASK_STK_SIZE - 1], 5);		//	create at least one task

	OSStart();				// Start multitasking.  

	return 0;				// NEVER EXECUTED
}

void TaskStart(void *pdata)			//	TaskOne
{		
	OSTaskCreate(TaskOne, (void *) 0, &TaskOneStack[TASK_STK_SIZE - 1], 10);
	OSTaskCreate(TaskTwo, (void *) 0, &TaskTwoStack[TASK_STK_SIZE - 1], 11);

	while (TRUE)
	{
		OSTimeDly(10);  

		if (kbhit())
			exit(0);
	}
}

void TaskOne(void *pdata)			//	TaskOne
{		
	int count = 0;

	while (TRUE)
	{
		printf("%d\t", ++count);
		OSTimeDly(10);  
	}
}

void TaskTwo(void *pdata)			//	TaskTwo
{		
	while (TRUE)
	{
		OSTimeDly(30); 
		printf("\n"); 
	}
}

