#ifdef WIN32
#define SCALL __cdecl
#else
#define SCALL
#endif


#ifdef UNIX
#include <pthread.h>
#endif
#ifdef WIN32
#include <windows.h>
#endif

typedef struct _thread {
#ifdef WIN32
	HANDLE handle;
	int    Id;
#endif

#ifdef UNIX
	pthread_t Id;
#endif
	void   *ThreadProc;

} SThread;

typedef struct _mutex {
#ifdef UNIX
	pthread_mutex_t handle;
#endif
#ifdef WIN32
	HANDLE handle;
#endif
#ifdef WIN32
	CRITICAL_SECTION section;
#endif

} SMutex;


typedef struct _event {
#ifdef WIN32
	HANDLE handle;
#endif
} SEvent;


SThread* OSD_CreateThread( int (SCALL *ThreadProc)( void*) ,int *arg);
void OSD_DestroyThread( SThread *thread);

int OSD_StopThread( SThread * thread);
int OSD_ResumeThread( SThread * thread);




SMutex* OSD_CreateMutex( void);
int OSD_MutexLock(   SMutex* mutex );
int OSD_MutexUnlock( SMutex* mutex );
void OSD_CloseMutex(  SMutex* mutex );

/*
SEvent* OSD_CreateEvent( char *name);
int OSD_OpenEvent( SEvent *event , char *name);
int OSD_SetEvent( SEvent *event );
int OSD_ResetEvent( SEvent *event );
int OSD_CloseEvent( SEvent *event );

typedef struct _critical {
#ifdef WIN32
	CRITICAL_SECTION section;
#endif
} SCritical;

SCritical* OSD_CreateCritical( void );
void OSD_CriticalLock( SCritical* critical );
void OSD_CriticalUnlock( SCritical* critical );
void OSD_TrashCritical( SCritical* critical );
*/
