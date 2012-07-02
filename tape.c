#include <stdio.h>
#include <malloc.h>

#include "ula.h"
#include "devices.h"
#include "tape.h"

static unsigned long last_tstate;
static int rpf, cst;
static int tape_running;

static char *tape_cnt;
static int tape_size;

static int tape_signal;
static int tape_pos;
static int next_change;
static int tape_state;
static int block_size;
static int state_pos;

void tape_process( unsigned long cycles )
{
    if ( !tape_running )
        return;

    while ( cycles >= next_change )
    {
        cycles -= next_change;
        next_change = 0;

        switch ( tape_state )
        {
            case 0:
                // start block
                if ( tape_pos > tape_size )
                    return;

                block_size = *(unsigned short *)(tape_cnt + tape_pos);
                tape_pos += 2;
                state_pos = tape_cnt[tape_pos] == 0x00 ? 8063 : 3223;
                tape_state ++;
                break;
            case 1:
                next_change = 2168;
                tape_signal = !tape_signal;
                if ( --state_pos == 0 )
                    tape_state ++;
                break;
            case 2:
                next_change = 667;
                tape_signal = !tape_signal;
                tape_state ++;
                break;
            case 3:
                next_change = 735;
                tape_signal = !tape_signal;
                tape_state ++;
                break;
            case 4:
                if ( block_size -- )
                {
                    state_pos = 7;
                    tape_state ++;
                }
                else
                    tape_state = 0;
                break;
            case 5:
            case 6:
                next_change = (tape_cnt[tape_pos] & (1 << state_pos)) ? 1710 : 855;
                tape_signal = !tape_signal;
                tape_state ++;
                break;
            case 7:
                if ( state_pos -- )
                    tape_state = 5;
                else
                {
                    tape_pos ++;
                    tape_state = 4;
                }
                break;
        }
    }
    next_change -= cycles;
}

int tape_get()
{
    tape_process( zxcpu_tstates - last_tstate );
    last_tstate = zxcpu_tstates;
    rpf ++;

    return( tape_signal );
}

void tape_frame()
{
    tape_process( zxcpu_tstates - last_tstate );
    last_tstate = 0;

    //printf("tape frame\n");
    //printf("reads per frame: %d\n", rpf );
    if ( rpf > 500 && rpf < 2000 )
    {
        if ( !tape_running )
        {
            printf("tape start! %d\n", rpf );
            tape_running = 1;
        }
        cst = 0;
    }
    else
    {
        if ( tape_running )
        {
            //printf("tape stop! %d\n", rpf );
            if ( cst >= 50 )
            {
                printf("tape stop! %d\n", rpf );
                tape_running = 0;
            }
            else
                cst ++;
        }
    }

    rpf = 0;
}

static int load_tap( const char *flname )
{
    FILE *fp;

    if ( (fp = fopen( flname, "rb" )) != NULL )
    {
        fseek( fp, 0, SEEK_END );
        tape_size = ftell( fp );

        fseek( fp, 0, SEEK_SET );
        if ( tape_cnt )
            free( tape_cnt );
        tape_cnt = malloc( tape_size );
        fread( tape_cnt, tape_size, 1, fp );

        fclose( fp );

        tape_state = 0;
        next_change = 0;
        tape_pos = 0;

        return( 0 );
    }

    return( -1 );
}


static file_type_t ft_tape[] =
    {
        { "tap", load_tap },
        { 0 }
    };

static file_fiter_t files_open[] =
	{
		{ "Tape", ft_tape },
		{ 0 }
	};


SDevice zxdevice_tape =
	{
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL,
		tape_frame,
		NULL, NULL,
		files_open
	};
