#include "includes.h"

#define TASK_STK_SIZE	256			//	stack size in bytes
#define POS_Y			10
#define LENGTH			20
#define ROWS			15

OS_STK  TaskStartStack[TASK_STK_SIZE];		//	stack van TaskStart 
OS_STK  TaskArrowStack[TASK_STK_SIZE];		//	stack van TaskStart 

void TaskStart(void *pdata);			//	prototype van TaskStart 
void TaskArrow(void *pdata);			//	prototype van TaskStart 

int main(void)
{
	OSInit();				// Initialize uCOS-II. 

	OSTaskCreate(TaskStart, (void *) 0, &TaskStartStack[TASK_STK_SIZE - 1], 5);		//	create at least one task

	OSStart();				// Start multitasking.  

	return 0;				// NEVER EXECUTED
}

void TaskStart(void *pdata)			//	TaskOne
{
	int row;
	int prio = 10;
	for(row = 5; row < ROWS + 5; row++)
	{
		OSTaskCreate(TaskArrow, (void *) row, &TaskArrowStack[TASK_STK_SIZE - 1], prio);
		OSTimeDly(20);
		prio++;
	}
	//OSTaskCreate(TaskArrow, (void *) 0, &TaskArrowStack[TASK_STK_SIZE - 1], 10);
	
	OSTaskDel( OS_PRIO_SELF );
}

void TaskArrow(void *pdata)			//	TaskTwo
{
	int *r =  (int*)pdata;
	int i, n;
	time_t t;
	INT16S key;
	
	system("@cls||clear");

	char tekst[80];
	strcpy(tekst, "Made By Jan Willem");	
	
	while (TRUE)
	{		
		srand((unsigned) time(&t));
		int delay = rand() % 20 + 3;

		BOOLEAN avail = PC_GetKey(&key);

		if(avail == TRUE)
		{
			if(key == 27)
			{
				system("@cls||clear");
				exit(0);
			}
		}
	
		PC_DispStr(0,0, tekst, DISP_FGND_WHITE);
		
		int i;
		for(i = 0; i <= LENGTH; i++)
		{
			PC_DispChar(i, r, '-', DISP_FGND_WHITE);
			PC_DispChar(i + 1, r, '>', DISP_FGND_WHITE);
			OSTimeDly(delay); 
		}
		
		if(i == LENGTH + 1)
		{
			for(i = 0; i <= LENGTH + 1; i++)
			{
				PC_DispChar(i, r, ' ', DISP_FGND_WHITE);
				//OSTimeDly(30); 
			}
			i = 0;
		}
	}
}

