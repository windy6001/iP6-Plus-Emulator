#ifdef UNIX
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "stdlib.h"
#include "../SThread.h"



// **************************************************
//		OSD_CreateThread
// **************************************************
// ThreadProc　 新しいスレッドの開始アドレス
// arg			新しいスレッドに渡すパラメータ
// Id			新しいスレッドを識別するID
SThread* OSD_CreateThread( int (*ThreadProc)( void*) ,int *arg)
	{
	SThread *thread;
//	int     Id;

	thread = (SThread*) malloc( sizeof( SThread));
	if( !thread ) return( (SThread*)NULL);

	pthread_create( &thread->Id,	// ID
					NULL,			// attr
					ThreadProc,  // thread function
					arg				// thread argument
					);

	thread->ThreadProc = ThreadProc;
	return( thread );
	}

// **************************************************
//		OSD_DestroyThread
// **************************************************
void OSD_DestroyThread( SThread *thread)
	{
	 if( thread) 
	 	{
		 //CloseHandle( thread->handle);
		 free( thread);
	 	 thread=NULL;
		}
	}

// **************************************************
//		OSD_StopThread
// **************************************************
int OSD_StopThread( SThread * thread)
	{
	 /*
	 int status;
	 status = SuspendThread( thread->handle);
	 if( status !=0xFFFFFFFF)
	 	return(1);
	 else
	 	return(0);
	*/
	}

// **************************************************
//		OSD_ResumeThread
// **************************************************
int OSD_ResumeThread( SThread * thread)
	{
	/*
	 int status;
	 status = ResumeThread(  thread->handle);
	 if( status !=0xFFFFFFFF)
	 	return(1);
	 else
	 	return(0);
	*/
	}


/*
SCritical* OSD_CreateCritical( void )
	{
	 return( NULL);
	}


void OSD_CriticalLock( SCritical* critical )
	{
	}


void OSD_CriticalUnlock( SCritical* critical )
	{
	}

void OSD_TrashCritical( SCritical* critical )
	{
	}
*/

  

SMutex* OSD_CreateMutex( void)
	{
#ifdef UNIX
	int    ret;
	SMutex *smutex;
//	pthread_mutex_t mutex;
	smutex = (SMutex*) malloc( sizeof( SMutex));
	if( smutex == NULL) return(NULL);

	ret = pthread_mutex_init( &smutex->handle, NULL);	
	if( ret!=0)
		{
		free(smutex);
		smutex=0;
		}
	
	return( smutex);
#endif

#if 0
	HANDLE handle = CreateMutex( NULL,   // lpMutexAttributes
								 TRUE,   // BOOL bInitialOwner
								 name ); // LPCTSTR lpName
	
	printf("CreateMutex = %d \n",handle);
	return( handle);
#endif
	}


int OSD_MutexLock( SMutex *smutex)
	{
#ifdef UNIX
	if( smutex)
		pthread_mutex_lock( &smutex->handle);
#endif

#if 0
	 WaitForSingleObject(handle , 		// handle
	 					2); 			// dwMilliseconds
#endif
	}


int OSD_MutexUnlock( SMutex *smutex)
	{
#ifdef UNIX
	if( smutex)
		pthread_mutex_unlock( &smutex->handle);
#endif

#if 0
	ReleaseMutex(handle);				// handle
#endif
	}


void  OSD_CloseMutex( SMutex *smutex)
	{
	 if( smutex)
		{
		pthread_mutex_destroy( &smutex->handle); 
		free(smutex);
		smutex=NULL;
		}
#if 0
	 CloseHandle( handle);
#endif
	}

#endif
