const char emul_version[] = "N/A (sekrit development version)";

#include "main.h"

#include "z80ex.h"
#include "emul.h"
#include "ula.h"
#include "video.h"
#include "input.h"

#include "sync_timer.h"
#include "sound.h"
void (*sync_wait)(void);
void (*sync_start)(void);
void (*sync_stop)(void);
void (*sync_init)(void);
void (*sync_uninit)(void);

void sync_dumb()
{
}

int emul_running;
#ifdef  WINDOWS
HANDLE emul_thread;
DWORD emul_thread_id;
DWORD WINAPI emul_loop( LPVOID )
#else
pthread_t emul_thread;
void *emul_loop(void *arg)
#endif
{
    sync_start();

    zx_life( &emul_running );

    sync_stop();

	return 0;
}

void emul_start()
{
    emul_running = 1;
#ifdef  WINDOWS
	emul_thread = CreateThread(NULL, 0, emul_loop, NULL,
		0, &emul_thread_id);
#else
    pthread_create(&emul_thread, NULL, emul_loop, NULL);
#endif
}

void emul_stop()
{
    emul_running = 0;
#ifdef  WINDOWS
	WaitForSingleObject(emul_thread, INFINITE);
#else
    pthread_join(emul_thread, NULL);
#endif
}

void emul_init()
{
	printf( "PerfectZX - ZX Spectrum emulator\n" );
	printf( "Version %s (revision %d) by Molodtsov Alex\n", emul_version, HGVERSION );

	Z80EX_VERSION *ver_z80ex;
	ver_z80ex = z80ex_get_version();
	printf("Using Z80ex version %s\n\n", ver_z80ex->as_string);

	sync_wait = timer_sync;
	sync_start = timer_start;
	sync_stop = timer_stop;
	sync_init = sync_dumb;
	sync_uninit = sync_dumb;

	sync_wait   = sound_oss_flush;
	sync_start  = sync_dumb;
	sync_stop   = sync_dumb;
	sync_init   = sound_oss_init;
	sync_uninit = sound_oss_uninit;

	sync_wait   = sound_alsa_flush;
	sync_start  = sync_dumb;
	sync_stop   = sync_dumb;
	sync_init   = sound_alsa_init;
	sync_uninit = sound_alsa_uninit;

	/*sync_wait   = sound_pulse_flush;
	sync_start  = sync_dumb;
	sync_stop   = sync_dumb;
	sync_init   = sound_pulse_init;
	sync_uninit = sound_pulse_uninit;*/

    printf("Initializing video...\n");
    video_init();
    printf("Initializing input...\n");
    input_init();
    printf("Initializing emulator...\n");
    zx_init();
    printf("Initializing synchronizator...\n");
    sync_init();

    printf("\nStart!\n");
    emul_start();
}

void emul_uninit()
{
    emul_stop();
    printf("Stop!\n");

    sync_uninit();
    zx_uninit();
    input_uninit();
    video_uninit();
}
