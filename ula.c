#include "main.h"
#include "ula.h"
#include "devices.h"

#include "tape.h"

Z80EX_CONTEXT *zxcpu;
unsigned long zxcpu_tstates_main;
unsigned long zxcpu_tstates;
unsigned long zxcpu_tstates_frame;
unsigned long zxcpu_int_start;
unsigned long zxcpu_int_end;
int zx_frame_new;

SDevice **zx_device = NULL;
unsigned zx_device_count = 0;

#define device_scan_asc(func)   for (dev = 0; dev < zx_device_count; dev++)   \
                                    if (zx_device[dev]->func)
#define device_scan_desc(func)  for (dev = zx_device_count ; dev-- ; )  \
                                    if (zx_device[dev]->func)

void zx_create_machine(SDevice **config, unsigned dev_count)
{
    unsigned i, j;
    SDevice **new_machine;
    new_machine = (SDevice**)malloc(dev_count * sizeof(SDevice*));
    memcpy(new_machine, config, dev_count * sizeof(SDevice*));

    for (i = 0; i < dev_count; i++)
    {
        int found = 0;
        for ( j = 0; j < zx_device_count; j++)
            if (new_machine[i] == zx_device[j])
            {
                found = 1;
                zx_device[j] = NULL;
                break;
            }
        if ( !found && new_machine[i]->init )
            new_machine[i]->init();
    }

    // delete unused devices
    for ( i = 0; i < zx_device_count; i++ )
        if ( zx_device[i] == NULL && zx_device[i]->uninit )
            zx_device[i]->uninit();

    free(zx_device);
    zx_device = new_machine;
    zx_device_count = dev_count;

    init_fopen( zx_device );
}

void zx_destroy_machine()
{
    while (zx_device_count--)
        if ( zx_device[zx_device_count]->uninit )
            zx_device[zx_device_count]->uninit();
    free(zx_device);
}

// cpu callback procedures
/*void zxcpu_tstate( Z80EX_CONTEXT *cpu, void *user_data )
{

}*/

#define CALCULATE_TSTATE() \
{ \
    zxcpu_tstates = zxcpu_tstates_main + z80ex_op_tstate(cpu);\
    if ( zxcpu_tstates >= zxcpu_tstates_frame ) \
    { \
        if ( !zx_frame_new ) \
        { \
            zx_frame_end(); \
            zx_frame_new = 1; \
        } \
        /*zxcpu_tstates %= zxcpu_tstates_frame;*/ \
        while ( zxcpu_tstates >= zxcpu_tstates_frame ) \
            zxcpu_tstates -= zxcpu_tstates_frame; \
    } \
}

void zx_frame_end()
{
    unsigned dev;
    // frame done
    device_scan_asc(frame)
        zx_device[dev]->frame();

    video_update();
    //tape_frame();
    sync_wait();
}

Z80EX_BYTE zxcpu_memread(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, int m1_state, void *user_data)
{
    static Z80EX_BYTE value;
    unsigned dev;

    CALCULATE_TSTATE();

    value = 0xFF;	// data bits pulled up to +5V
    device_scan_desc(mem_read)
        if (zx_device[dev]->mem_read(cpu, addr, &value, m1_state )) break;
    return value;
}

void zxcpu_memwrite(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE value, void *user_data)
{
    unsigned dev;

    CALCULATE_TSTATE();

    device_scan_desc(mem_write)
        if (zx_device[dev]->mem_write(cpu, addr, value)) break;
}

Z80EX_BYTE zxcpu_portread(Z80EX_CONTEXT *cpu, Z80EX_WORD port, void *user_data)
{
    static Z80EX_BYTE value;
    unsigned dev;

    CALCULATE_TSTATE();

    value = 0xFF;	// data bits pulled up to +5V
    // process ports using IORQGE block
    device_scan_asc(preport_in)
        if (zx_device[dev]->preport_in(cpu, port, &value)) break;
    // process ports using IORQ filtration
    device_scan_asc(port_in)
        if (zx_device[dev]->port_in(cpu, port, &value)) break;
    return value;
}

void zxcpu_portwrite(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value, void *user_data)
{
    unsigned dev;

    CALCULATE_TSTATE();

    // process ports using IORQGE block
    device_scan_asc(preport_out)
        if (zx_device[dev]->preport_out(cpu, port, value)) break;
    // process ports using IORQ filtration
    device_scan_asc(port_out)
        if (zx_device[dev]->port_out(cpu, port, value)) break;
}

Z80EX_BYTE zxcpu_intread(Z80EX_CONTEXT *cpu, void *user_data)
{
    return 0xFF;
}

// main cycle
void zx_life( int *running )
{
    while ( *running )
    {
        while ( zxcpu_tstates_main < zxcpu_tstates_frame )
        {
            if ((zxcpu_tstates_main >= zxcpu_int_start) && (zxcpu_tstates_main < zxcpu_int_end))
                zxcpu_tstates_main += z80ex_int(zxcpu);
            zxcpu_tstates_main += z80ex_step(zxcpu);
        }
        if ( !zx_frame_new )
            zx_frame_end();
        zx_frame_new = 0;

        //zxcpu_tstates_main %= zxcpu_tstates_frame;
        while ( zxcpu_tstates_main >= zxcpu_tstates_frame ) \
            zxcpu_tstates_main -= zxcpu_tstates_frame;
    }
}

// reset machine
void zx_reset()
{
    unsigned dev;
    z80ex_reset( zxcpu );
    device_scan_asc(reset)
        zx_device[dev]->reset();
}

// create processor, organize devices
void zx_init()
{
    zxcpu = z80ex_create(zxcpu_memread, NULL,
        zxcpu_memwrite, NULL,
        zxcpu_portread, NULL,
        zxcpu_portwrite, NULL,
        zxcpu_intread, NULL);
    //z80ex_set_tstate_callback(zxcpu, zxcpu_tstate, NULL);

    // init device mechanism
    zx_device_count = 0;
    zx_device = NULL;

    // construct machine
    SDevice *zxtest[] = {
            &zxdevice_pentagon128,
            //&zxdevice_gmx,
            //&zxdevice_sid,
            &zxdevice_ay,
            //&zxdevice_megaay,
            &zxdevice_kempston_mouse,
            &zxdevice_saa,
            &zxdevice_tape,
    };
    zx_create_machine(zxtest, sizeof(zxtest) / sizeof(SDevice*));
}

// destroy everything
void zx_uninit()
{
    zx_destroy_machine();
    z80ex_destroy(zxcpu);
}
