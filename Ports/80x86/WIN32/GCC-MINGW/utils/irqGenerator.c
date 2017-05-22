/*
*********************************************************************************************************
*                                          Generate interrupt events
*                                Used to simulate interrupts for uC/OS-II WIN32 PORT
*
*                          (c) Copyright 2004-2005, Werner.Zimmermann@fht-esslingen.de
*                                           All Rights Reserved
*
* File : irqGenerator.C
* By   : Werner Zimmermann
*********************************************************************************************************
*/


#include <windows.h>

HANDLE irqHandle=NULL;

int main(int argc, char *argv[])
{   char irqName[256];

    if (argc<2)
    {   printf("Usage: irqGenerator <number of irq 0 ... 7>\n");
        return -1;
    }

    sprintf(irqName, "OSirq%s", argv[1]);                               //Supported "interrupt" events OSirq0 ... OSirq7

    if ((irqHandle=OpenEvent(EVENT_ALL_ACCESS, FALSE, irqName))==NULL)  //Open the interrupt event (fails, if uCOS-II is not running)
    {   printf("Could not open %s - exiting\n", irqName);
        return -2;
    }

    SetEvent(irqHandle);                                                //Trigger the "interrupt" event
    return 0;
}
