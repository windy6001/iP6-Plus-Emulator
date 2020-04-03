/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         WinThread.c                     **/
/**                                                         **/
/** by windy 2002-2004                                      **/
/*************************************************************/
#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include "../SThread.h"

#define USE_CRITICALSECTION


// **************************************************
//		OSD_CreateThread
// **************************************************
// ThreadProc　 新しいスレッドの開始アドレス
// arg			新しいスレッドに渡すパラメータ
// Id			新しいスレッドを識別するID
//SThread* OSD_CreateThread( DWORD WINAPI (*ThreadProc)( void*) ,int *arg)
SThread* OSD_CreateThread( int (SCALL *ThreadProc)( void*) ,int *arg)
	{
	SThread *thread;
	int     Id;

	thread = malloc( sizeof( SThread));
	if( !thread ) return( (SThread*)NULL);

	thread->handle = CreateThread(
		NULL,                    // SD
		0,                       // initial stack size
		(LPTHREAD_START_ROUTINE)ThreadProc,    // thread function
		(LPVOID)arg,                             // thread argument
		0,                                       // creation option
		(LPDWORD)&Id                              // thread identifier
	);
	thread->ThreadProc = ThreadProc;
	thread->Id = Id;
	return( thread );
	}

// **************************************************
//		OSD_DestroyThread
// **************************************************
void OSD_DestroyThread( SThread *thread)
	{
	 if( thread) 
	 	{
		 CloseHandle( thread->handle);
		 free( thread);
	 	 thread=NULL;
		}
	}

// **************************************************
//		OSD_StopThread
// **************************************************
int OSD_StopThread( SThread * thread)
	{
	 int status;
	 status = SuspendThread( thread->handle);
	 if( status !=0xFFFFFFFF)
	 	return(1);
	 else
	 	return(0);
	}

// **************************************************
//		OSD_ResumeThread
// **************************************************
int OSD_ResumeThread( SThread * thread)
	{
	 int status;
	 status = ResumeThread(  thread->handle);
	 if( status !=0xFFFFFFFF)
	 	return(1);
	 else
	 	return(0);
	}






SMutex* OSD_CreateMutex( void)
	{
	SMutex *mutex;

	mutex = malloc( sizeof( SMutex));
	if( mutex ==NULL) {printf("CreateMutex failed \n"); return (SMutex*)NULL;}

#ifdef USE_CRITICALSECTION
	 InitializeCriticalSection( &mutex->section );

#else
	{
	 HANDLE handle;
	 handle = CreateMutex( NULL,   // lpMutexAttributes
								 FALSE,   // BOOL bInitialOwner
								 NULL ); // LPCTSTR lpName
	 mutex->handle = handle;
	 printf("CreateMutex = %d \n",handle);
	}
#endif


	return( mutex );
	}


int OSD_MutexLock( SMutex *mutex )
	{
	DWORD ret=0;
#ifdef USE_CRITICALSECTION

	if( mutex !=NULL)
		{
		EnterCriticalSection( &mutex->section );
		ret =1;
		}

#else

	if( mutex !=NULL)
		{
		//printf("[WinThread][OSD_MutexLock]  ... ");
		ret =WaitForSingleObject( mutex->handle , 		// handle
	 						50); 			// wait time (dwMilliseconds)
		if( ret != WAIT_FAILED)
			{
			 switch( ret)
				{
				 case WAIT_ABANDONED: printf("abandoned NG \n"); ret =0; break;
				 case WAIT_OBJECT_0: /* printf("Lock OK\n"); */ ret =1; break;
				 case WAIT_TIMEOUT:  printf("timeout NG\n"); ret =0; break;
				}
			}
		else
			{
				printf(" WaitForSingleObject() failed \n");
			}
		
		}
#endif
	return(ret);
	}


int OSD_MutexUnlock( SMutex *mutex )
	{
	DWORD ret=0;
#ifdef USE_CRITICALSECTION

	if( mutex !=NULL)
		{
		LeaveCriticalSection( &mutex->section );
		ret=1;
		}

#else
	if( mutex !=NULL)
		{
		//printf("[WinThread][OSD_MutexUnlock] ...");
		ret = ReleaseMutex( mutex->handle);				// handle
		//if( ret ) 
		//	printf("Unlock OK\n");
		//else
		//	printf("Unlock NG\n");
		}
#endif
	return(ret);
	}


void OSD_CloseMutex( SMutex *mutex )
	{
#ifdef USE_CRITICALSECTION

	if( mutex !=NULL)
		{
		DeleteCriticalSection( &mutex->section );
		mutex =NULL;
		}

#else

	if( mutex !=NULL)
		{
		CloseHandle( mutex->handle);
		free(mutex);
		mutex =NULL;
		}
#endif
	}






#if 0
SEvent* OSD_CreateEvent( char *name)
	{
	HANDLE handle;
	SEvent *event;

	event = (SEvent*) malloc( sizeof( SEvent));
	if( event ==NULL) {printf("CreateEvent failed \n"); return (SEvent*)NULL;}

	handle = CreateEvent( NULL,   // lpMutexAttributes
						 FALSE,    // BOOL bManualReset   FALSE: auto TRUE: manual
						 FALSE,	  // BOOL bINitialState
						  name ); // LPCTSTR mutexName
	printf("CreateEvent = %d \n",handle);

	event->handle = handle;
	printf("OpenEvent = %d \n",handle);
	return( event );
	}

int OSD_OpenEvent( SEvent *event , char *name)
	{
	 HANDLE handle;
	 handle = OpenEvent(SYNCHRONIZE ,	// DWORD dwDesiredAccess ,
				   FALSE ,				// BOOL bInheritHandle ,
				   name);				 // LPCTSTR lpName
	 event->handle = handle;
	}

int OSD_SetEvent( SEvent *event )
	{
#if 0
	DWORD ret=0;

	if( event !=NULL)
		{
		printf("[WinThread][OSD_SetEvent]  ... ");
		ret = SetEvent( event->handle);
		}
	 return(ret);
#endif
	}


int OSD_ResetEvent( SEvent *event )
	{
	DWORD ret=0;

	if( event !=NULL)
		{
		printf("[WinThread][OSD_ResetEvent]  ... ");
		ret = ResetEvent( event->handle);
		}
	 return(ret);
	}

int OSD_CloseEvent( SEvent *event )
	{
	 if( event !=NULL)
		{
		CloseHandle( event->handle);
		free(event);
		}
	}


SCritical* OSD_CreateCritical( void )
	{
	 SCritical *critical= malloc( sizeof( SCritical));
	 if( critical ==NULL) return 0;

	 InitializeCriticalSection( &critical->section );
	 return( critical);
	}


void OSD_CriticalLock( SCritical* critical )
	{
		EnterCriticalSection( &critical->section );
	}


void OSD_CriticalUnlock( SCritical* critical )
	{
		LeaveCriticalSection( &critical->section );
	}

void OSD_TrashCritical( SCritical* critical )
	{
		if( critical !=NULL)
		{
		 DeleteCriticalSection( &critical->section );
		 free(critical);
		 critical= NULL;
		}
		 
	}
#endif

#endif

