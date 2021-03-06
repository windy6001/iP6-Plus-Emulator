/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           WinTimer.c                    **/
/** by Windy 2003                                           **/
/*************************************************************/
#ifdef WIN32
#include <stdio.h>
#include <windows.h>
#include "../types.h"
#include "../Timer.h"

static TIMECAPS	tc;

void OSD_InitTimer(void)
	{
	timeGetDevCaps( &tc , sizeof(TIMECAPS) );
	timeBeginPeriod( tc.wPeriodMin  );
	}

void OSD_TrashTimer(void)
	{
	timeEndPeriod(tc.wPeriodMin );
	}

void OSD_Delay(int s)
	{
	Sleep(s);
	}

dword OSD_GetTicks(void)
	{
	return( timeGetTime());
	}


	// ****************************************************************************
//        init_delay: delay関数の初期化
// ****************************************************************************
int lasttime;
int interval;

void init_delay(int fps)
{
	lasttime= timeGetTime();
    interval=(int)(1000/60);
    printf("interval=%d\n", interval);
}

// ****************************************************************************
//        delay: 一定時間 Sleepする
//  Out: 1:遅延が発生しているので重い処理をskipすべき  0:遅延してないので、Sleepしました
// ****************************************************************************
int delay(void)
{
	int delayed;
	int sleeptime,nowtime;
	nowtime= timeGetTime();
	if( lasttime+interval > nowtime)
	 	{
		sleeptime= lasttime+ interval-nowtime;
		Sleep( sleeptime);
	 	printf("sleeptime=%d\n",sleeptime);
//      for( ; timeGetTime()< lasttime+interval; Sleep(1));
	 	delayed=0;
		}
 	else
 		delayed=1;
 	lasttime+= interval;
 	return delayed;
}

#endif
