#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

//#include <windows.h>
#include "../types.h"
#include "../Timer.h"

static struct timeval start;

void OSD_InitTimer(void)
	{
	printf("Init timer..... ");
	gettimeofday(&start, NULL);
	printf("Ok\n");
	}

void OSD_TrashTimer(void)
	{
	}

void OSD_Delay(int s)
	{
	 //printf("OSD_Delay %d\n",s);
	 usleep( s* 1000);
	}

dword OSD_GetTicks(void)
	{
	struct timeval now;
	dword ticks;

	gettimeofday(&now, NULL);
	//ticks=(now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000;	
	
	ticks = (now.tv_sec*1000+now.tv_usec/1000) - (start.tv_sec*1000+ start.tv_usec/10000);
	//printf("ticks %d\n",ticks);
	return(ticks);
	}


