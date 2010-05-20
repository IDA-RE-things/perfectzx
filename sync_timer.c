#include "sync_timer.h"

#ifdef  WINDOWS
// windows timer
UINT timer_id;
HANDLE timer_waiting, timer_done;

void CALLBACK timer_tick(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	WaitForSingleObject( timer_waiting, INFINITE );
	SetEvent( timer_done );
}

void timer_sync()
{
	SetEvent( timer_waiting );
	WaitForSingleObject( timer_done, INFINITE );
}

void timer_start()
{
	timer_waiting = CreateEvent( NULL, false, false, NULL );
	timer_done = CreateEvent( NULL, false, false, NULL );
	timer_id = timeSetEvent( 20, 100, timer_tick, 0, TIME_PERIODIC);
}

void timer_stop()
{
	while( timeKillEvent( timer_id ) != TIMERR_NOERROR )
		SetEvent( timer_waiting );
	CloseHandle( timer_waiting );
	CloseHandle( timer_done );
}

#else

// POSIX timer
#include <time.h>
#include <signal.h>
#include <semaphore.h>

sem_t timer_waiting, timer_done;

void timer_tick (union sigval sig)
{
    sem_wait(&timer_waiting);
    sem_post(&timer_done);
}

void timer_sync()
{
    sem_post(&timer_waiting);
    sem_wait(&timer_done);
}

timer_t timer_id;
struct sigevent timer_signal;
struct itimerspec timer_spec;

void timer_start()
{
    sem_init(&timer_waiting, 0, 0);
    sem_init(&timer_done, 0, 0);

	timer_signal.sigev_notify = SIGEV_THREAD;
    timer_signal.sigev_notify_function = timer_tick;
    timer_signal.sigev_notify_attributes = NULL;
    timer_create(CLOCK_MONOTONIC, &timer_signal, &timer_id);
	timer_spec.it_interval.tv_sec = timer_spec.it_value.tv_sec = 0;
    timer_spec.it_interval.tv_nsec = timer_spec.it_value.tv_nsec = 20000000;
    timer_settime(timer_id, 0, &timer_spec, NULL);
}

void timer_stop()
{
	timer_delete(timer_id);
	sem_destroy(&timer_waiting);
	sem_destroy(&timer_done);
}
#endif
