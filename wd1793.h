#ifndef PZX_WD1793
#define PZX_WD1793

typedef struct
{
    unsigned command;  // not sure if it is needed
    unsigned status;
    unsigned char track, sector;
    unsigned data;
    unsigned out_status;

    unsigned last_clock;
    int count;
    int state;
    unsigned dsr;

    struct
    {
        unsigned track;
        unsigned position;
        unsigned long clock;
    } disk;
} wd1793_t;

void wd1793_reset( wd1793_t *device );
void wd1793_write( wd1793_t *device, unsigned char reg, unsigned char data );
unsigned char wd1793_read( wd1793_t *device, unsigned char reg );
unsigned wd1793_status( wd1793_t *device );
void wd1793_frame( wd1793_t *device );

/* bit mask returned by wd1793_status() */
#define WD1793_DRQ      (1 << 0)
#define WD1793_INTRQ    (1 << 1)
#define WD1793_HLD      (1 << 2)
#define WD1793_DIR      (1 << 3)

#endif  /* PZX_WD1793 */
