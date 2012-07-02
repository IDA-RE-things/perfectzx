#include "main.h"
#include <stdio.h>
#include "wd1793.h"

#define CLOCKS_PER_FRAME    (1000000/50)

#define STAT_NOT_READY  0x80
#define STAT_WR_PROTECT 0x40
#define STAT_HEAD_LOAD  0x20
#define STAT_REC_TYPE   0x20
#define STAT_WR_FAULT   0x20
#define STAT_SEEK_ERR   0x10
#define STAT_RNF        0x10
#define STAT_CRC_ERR    0x08
#define STAT_TRACK_0    0x04
#define STAT_LOST_DATA  0x04
#define STAT_INDEX      0x02
#define STAT_DRQ        0x02
#define STAT_BUSY       0x01

#define FLAG_H(c)   ((c) & 0x08)
#define FLAG_V(c)   ((c) & 0x04)
#define FLAG_U(c)   ((c) & 0x10)
#define FLAG_R(c)   ((c) & 0x03)

const unsigned pos_delays[4] = { 6000, 12000, 20000, 30000 };

enum wd1793_state
{
    WD1793_IDLE,
    WD1793_TYPE1,
};

static void drive_step( wd1793_t *device )
{
    if ( device->out_status & WD1793_DIR )
    {
        // +1 track
        if ( device->disk.track < 96 )  // don't know real limit
            device->disk.track ++;
    }
    else
    {
        // -1 track
        if ( device->disk.track > 0 )
            device->disk.track --;
    }
}

static void process_wd1793( wd1793_t *device, unsigned clock_cycle )
{
    device->disk.clock += clock_cycle - device->last_clock;
    device->disk.clock %= CLOCKS_PER_FRAME * 10;

    while ( device->last_clock < clock_cycle )
    {
        if ( device->state == WD1793_IDLE )
            device->last_clock = clock_cycle;
    }

    if ( device->state == WD1793_TYPE1 )
    {
        if ( device->disk.clock < 2500 )
            device->status |= STAT_INDEX;
        else
            device->status &= ~STAT_INDEX;
    }
}

static void process_cmd( wd1793_t *device )
{
    unsigned char cmd = device->command;

    if ( (cmd & 0x80) == 0 )
    {
        // type I command
        device->status = STAT_BUSY;
        device->out_status &= ~(WD1793_DRQ | WD1793_INTRQ);

        if ( FLAG_H(cmd) )
            device->out_status |= WD1793_HLD;
        else
            device->out_status &= ~WD1793_HLD;

        if ( (cmd & 0xE0) == 0 )
        {
            // seek and restore commands
            if ( (cmd & 0x10) == 0 )
            {
                // restore command
                device->track = 0xFF;
                device->data = 0;
            }

            device->dsr = device->data;

            while ( device->dsr != device->track )
            {
                if ( device->dsr > device->track )
                {
                    device->out_status |= WD1793_DIR;
                    device->track ++;
                }
                else
                {
                    device->out_status &= ~WD1793_DIR;
                    device->track --;
                }

                drive_step( device );
            }
        }
        else
        {
            // step commands
            if ( (cmd & 0x60) == 4 )        // step in
                device->out_status |= WD1793_DIR;
            else if ( (cmd & 0x60) == 6 )   // step out
                device->out_status &= ~WD1793_DIR;

            if ( FLAG_U(cmd) )
            {
                if ( device->out_status & WD1793_DIR )
                    device->track ++;
                else
                    device->track --;
            }

            drive_step( device );
        }


        /*if ( !(device->out_status & WD1793_DIR) && (device->disk.track == 0) )
        {
            device->track = 0;
            device->count = 1;  // no delay
        }
        else
        {
            drive_step( device );
            device->count = pos_delays[FLAG_R(cmd)];
        }*/

        device->state = WD1793_TYPE1;

        // HACK
        device->status &= ~(STAT_BUSY | STAT_TRACK_0);
        if ( device->disk.track == 0 )
        {
            device->status |= STAT_TRACK_0;
            device->track = 0;
        }
        device->out_status |= WD1793_INTRQ;
    }
    else if ( (cmd & 0x40) == 0 )
    {
        // type II command
        device->status |= STAT_BUSY;
    }
    else if ( (cmd & 0x30) == 0x01 )
    {
        // type IV command
        device->status &= ~STAT_BUSY;
    }
    else
    {
        // type III command
        device->status |= STAT_BUSY;
    }
}

void wd1793_reset( wd1793_t *device )
{
    device->sector = 0x01;
    device->command = 0x03;
    device->status = STAT_NOT_READY;
    device->status |= STAT_HEAD_LOAD | STAT_TRACK_0;
}

void wd1793_write( wd1793_t *device, unsigned char reg, unsigned char data )
{
    reg &= 3;
    process_wd1793( device, zxcpu_tstates * CLOCKS_PER_FRAME / zxcpu_tstates_frame );

    switch ( reg )
    {
        case 0:
            device->command = data;
            process_cmd( device );
            break;
        case 1:
            device->track = data;
            break;
        case 2:
            device->sector = data;
            break;
    }
}

unsigned char wd1793_read( wd1793_t *device, unsigned char reg )
{
    reg &= 3;
    process_wd1793( device, zxcpu_tstates * CLOCKS_PER_FRAME / zxcpu_tstates_frame );

    switch ( reg )
    {
        case 0:
            return device->status;
        case 1:
            return device->track;
        case 2:
            return device->sector;
    }

    return 0xFF;
}

unsigned wd1793_status( wd1793_t *device )
{
    return device->out_status;
}

void wd1793_frame( wd1793_t *device )
{
    process_wd1793( device, CLOCKS_PER_FRAME );
}
