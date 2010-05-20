#include "main.h"

typedef struct
{
    unsigned char r,g,b;
} video_pixel;
video_pixel *vid_buffer_out = NULL;

void video_set_out(void *buf, unsigned width, unsigned height)
{
    vid_buffer_out = (video_pixel *)buf;
}

unsigned long video_last_tstate = 0;

unsigned char *video_memory;
unsigned char video_border;
video_pixel video_palette[16];

unsigned char std_ink[256], std_paper[256];
unsigned short std_adr_ptrn[192], std_adr_attr[192];
unsigned std_flash_counter;

void video_render_std(unsigned long tstate)
{
    while (video_last_tstate < tstate)
    {
        unsigned line, line_x;
        unsigned out_x, out_y;

        if ( video_last_tstate == 0 )
            std_flash_counter = (std_flash_counter + 1) % 32;

        // top nonviewable area
        if (video_last_tstate < ((56-24)*224))
            video_last_tstate = 224 * (56-24);
        // bottom nonviewable area
        else if (video_last_tstate >= ((56+192+24)*224))
            video_last_tstate = 224 * 312;
        // viewable area
        else
        {
            line = video_last_tstate / 224;
            line_x = video_last_tstate % 224;

            out_y = line - (56-24);

            // left nonviewable area
            if (line_x < (0+32-16))
                video_last_tstate = line * 224 + (0+32-16);
            // right nonviewable area
            else if (line_x >= (32+128+16))
                video_last_tstate = (line + 1) * 224;
            // viewable area
            else
            {
                out_x = line_x*2 - (64-32);

                // screen
                if (line >= 56 && line < (56+192) && line_x >= 32 &&  line_x < (32+128))
                {
                    unsigned line_char, scr_line;

                    line_char = (line_x-32)/4;
                    video_last_tstate = line*224+line_char*4+32;  // align tstates
                    for (; (line_char < 32) && (video_last_tstate < tstate); line_char++)
                    {
                        scr_line = line - 56;
                        unsigned char pattern, attribute;
                        pattern = video_memory[line_char | std_adr_ptrn[scr_line]];
                        attribute = video_memory[line_char | std_adr_attr[scr_line]];
                        if ( (attribute & 0x80) && std_flash_counter >= 16 )
                            pattern ^= 0xFF;
                        unsigned i;
                        for (i = 0; i < 8; i++)
                            vid_buffer_out[out_x++ + out_y*320] = video_palette[(pattern&(1<<(7-i)))? std_ink[attribute]:std_paper[attribute]];
                        video_last_tstate += 4;
                    }
                }
                // border
                else
                {
                    unsigned long border_end;
                    border_end = (line >= 56 && line < (56+192) && line_x < (32+128))? (line*224+32) : ((line+1)*224);
                    border_end = (tstate < border_end) ? tstate : border_end;
                    for(;video_last_tstate < border_end; video_last_tstate++)
                    {
                        vid_buffer_out[(out_x++) + (out_y*320)] = video_palette[video_border];
                        vid_buffer_out[(out_x++) + (out_y*320)] = video_palette[video_border];
                    }
                }
            }
        }
    }
}

void video_render_std_fast()
{
    unsigned x, y0, y1, y2, outx, outy;
    outy = 24;
    for(y2 = 0; y2 < 3; y2++)
    for(y1 = 0; y1 < 8; y1++)
    {
    outx = 32;
    for(x = 0; x < 32; x++)
    {
        static unsigned char pattern, attribute, ink, paper;
        attribute = video_memory[x | std_adr_attr[(y2*8+y1)*8]];
        ink = (attribute&7) | ((attribute&0x40)>>3);
        paper = (attribute&0x78)>>3;
        for(y0 = 0; y0 < 8; y0++)
        {
            pattern = video_memory[x | std_adr_ptrn[(y2*8+y1)*8+y0]];
            static unsigned i;
            for (i = 0; i < 8; i++)
                vid_buffer_out[(outx+i) + (outy+y0)*320] =
                    video_palette[(pattern&(1<<(7-i)))? std_ink[attribute]:std_paper[attribute]];
        }
        outx += 8;
    }
    outy += 8;
    }
}

void video_prepare_std()
{
    unsigned i;
    // palette
    for (i = 0; i < 16; i++)
    {
        //video_palette[i].a = 0xFF;
        video_palette[i].r = (i&2)?((i&8)?0xFF:0xC0):0x00;
        video_palette[i].g = (i&4)?((i&8)?0xFF:0xC0):0x00;
        video_palette[i].b = (i&1)?((i&8)?0xFF:0xC0):0x00;
    }
    // attributes
    for (i = 0; i < 0xFF; i++)
    {
        std_ink[i] = (i & 7) | ((i & 0x40) >> 3);
        std_paper[i] = (i & 0x78) >> 3;
    }
    // video addresses
    for (i = 0; i < 192; i++)
    {
        std_adr_ptrn[i] = ((i & 7) << 8) + ((i & 0x38) << 2) + ((i & 0xC0) << 5);
        std_adr_attr[i] = 0x1800 | ((i & 0xF8)<<2);
    }
}

void video_init()
{
    video_prepare_std();
}

void video_uninit()
{

}
