//#include <windows.h>
#include "main.h"
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <ctype.h>

#include "emul.h"
#include "ula.h"
#include "video.h"
#include "input.h"

#define SCR_SCALE 2

GtkWidget *scr_area = NULL;
GdkPixbuf *scr_pixbuf;
//guchar gbuf[320*240*3];

#define pause_wait()    \
    {   \
        gdk_threads_leave();    \
        emul_stop();   \
        gdk_threads_enter();    \
    }

static gboolean screen_update(GtkWidget *area, GdkEventExpose *event, GPtrArray *parray)
{
    /*gdk_draw_pixbuf(area->window,
        area->style->base_gc[GTK_WIDGET_STATE (area)],
        scr_pixbuf,
        0, 0,
        0, 0,
        320, 240,
        GDK_RGB_DITHER_NONE, 0, 0);*/
    GdkPixbuf *dst_pixbuf;

    dst_pixbuf = gdk_pixbuf_scale_simple( scr_pixbuf, 320*SCR_SCALE, 240*SCR_SCALE, GDK_INTERP_NEAREST );

    cairo_t *cr = gdk_cairo_create( area->window );
    gdk_cairo_set_source_pixbuf( cr, dst_pixbuf, 0, 0 );
    cairo_paint( cr );
    cairo_destroy( cr );

    g_object_unref(G_OBJECT(dst_pixbuf));

    return FALSE;//TRUE;
}

static gboolean event_key(GtkWidget *window, GdkEventKey *event, GPtrArray *parray)
{
    //printf( "Key: %d\n", event->keyval );
    return input_event_keyboard(event->hardware_keycode, (event->type == GDK_KEY_PRESS));
}

GdkCursor *cursor_blank;
gint pointer_x_old, pointer_y_old;
gint mouse_last_x, mouse_last_y;

static gboolean event_button( GtkWidget *widget, GdkEventButton *event, gpointer user_data )
{
    GdkScreen *screen;
    GdkDisplay *display;
    unsigned char butmask;

    if ( event->type == GDK_BUTTON_PRESS )
    {
    screen = gtk_widget_get_screen( widget );
    display = gtk_widget_get_display( widget );

    if ( !gdk_pointer_is_grabbed() )
    {
        if ( event->button != 2 )   /* middle button unlocks pointer */
        {
            gint x_center, y_center;

            //printf( "grabbing pointer\n" );

            x_center = gdk_screen_get_width( screen ) / 2;
            y_center = gdk_screen_get_height( screen ) / 2;

            pointer_x_old = event->x_root;
            pointer_y_old = event->y_root;

            gdk_pointer_grab( widget->window, FALSE, GDK_POINTER_MOTION_MASK |
                              GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK,
                              NULL, cursor_blank, event->time );

            gdk_display_warp_pointer( display, screen,
                                      x_center, y_center );
            mouse_last_x = x_center;
            mouse_last_y = y_center;
        }
    }
    else if ( event->button == 2 )
    {
        //printf( "ungrabbing pointer\n" );
        gdk_pointer_ungrab( event->time );
        gdk_display_warp_pointer( display, screen,
                                  pointer_x_old, pointer_y_old );
        mouse_last_x = pointer_x_old;
        mouse_last_y = pointer_y_old;
    }
    }

    /* zx stuff */
    switch ( event->button )
    {
        case 1:
            butmask = 1; break;
        case 2:
            butmask = 4; break;
        case 3:
            butmask = 2; break;
        default:
            butmask = 0;
    }

    if ( event->type == GDK_BUTTON_PRESS )
        zxmouse_but |= butmask;
    else if ( event->type == GDK_BUTTON_RELEASE )
        zxmouse_but &= ~butmask;

    return FALSE;
}

static gboolean event_motion( GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    gint x_center, y_center;
    GdkScreen *screen;

    //printf( "Mouse is moving! dx=%f dy=%f\n", event->x_root - mouse_last_x, event->y_root - mouse_last_y );
    zxmouse_x += event->x_root - mouse_last_x;
    zxmouse_y += event->y_root - mouse_last_y;

    mouse_last_x = event->x_root;
    mouse_last_y = event->y_root;

    screen = gtk_widget_get_screen( widget );
    x_center = gdk_screen_get_width( screen ) / 2;
    y_center = gdk_screen_get_height( screen ) / 2;

    if ( gdk_pointer_is_grabbed() &&
        ( event->x_root != x_center || event->y_root != y_center ) )
    {
        gdk_display_warp_pointer( gtk_widget_get_display( widget ),
                                  screen,
                                  x_center, y_center );
        mouse_last_x = x_center;
        mouse_last_y = y_center;
    }

    /* zx stuff */
    zxmouse_but = (event->state & GDK_BUTTON1_MASK ? 1 : 0) |
                  (event->state & GDK_BUTTON3_MASK ? 2 : 0) |
                  (event->state & GDK_BUTTON2_MASK ? 4 : 0);

    return FALSE;
}

static gboolean event_crossing( GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    if ( !gdk_pointer_is_grabbed() )
    {
        mouse_last_x = event->x_root;
        mouse_last_y = event->y_root;
    }

    return FALSE;
}

void video_update()
{
    // get GTK thread lock
    gdk_threads_enter();

    if ( GTK_IS_WIDGET( scr_area ) )
    {
        // update screen
        gtk_widget_queue_draw( scr_area );
        // flush commands
        gdk_flush();
    }

    // release GTK thread lock
    gdk_threads_leave();
}

GtkWidget *dialog_open;
GtkFileFilter *filter = NULL;
GtkFileFilter *filter_known = NULL;
typedef struct
{
    unsigned index;
    file_type_t *type;
} filter_t;
filter_t *filters;
unsigned filter_count;

void init_fopen( SDevice **config )
{
    unsigned dev, i, ext;
    unsigned curf;
    char extbuf[256];

    if ( dialog_open )
        gtk_widget_destroy( dialog_open );
        //g_object_unref(G_OBJECT(dialog_open));
    /*if ( filter )
        g_object_unref(G_OBJECT(filter));
    if ( filter_known )
        g_object_unref(G_OBJECT(filter_known));*/

    /* get total count of filters */
    filter_count = 0;
    for (dev = 0; dev < zx_device_count; dev++)
        if (config[dev]->files_open)
        {
            i = 0;
            while ( config[dev]->files_open[ i++ ].type )
                filter_count ++;
        }

    /* allocate memory for filters */
    filters = calloc( filter_count, sizeof(filter_t) );

    dialog_open = gtk_file_chooser_dialog_new ("Open File",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);

    filter_known = gtk_file_filter_new();
    gtk_file_filter_set_name( filter_known, "All formats" );
    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(dialog_open), filter_known );


    /* process filters */
    curf = 0;
    for (dev = 0; dev < zx_device_count; dev++)
        if (config[dev]->files_open)
        {
            i = 0;
            while ( config[dev]->files_open[i].type )
            {
                filters[curf].type = config[dev]->files_open[i].type;
                filter = gtk_file_filter_new();
                gtk_file_filter_set_name( filter, config[dev]->files_open[i].name );
                ext = 0;
                while ( config[dev]->files_open[i].type[ext].process )
                {
                    /* this SHIT is for case insensitiveness */
                    char *extop;
                    char *extp = config[dev]->files_open[i].type[ext].extention;
                    strcpy( extbuf, "*." );
                    extop = extbuf + 2;
                    while ( *extp )
                    {
                        *(extop++) = '[';
                        *(extop++) = tolower( *extp );
                        *(extop++) = toupper( *extp );
                        *(extop++) = ']';
                        extp ++;
                    }
                    *(extop++) = '\0';

                    gtk_file_filter_add_pattern( filter, extbuf );
                    gtk_file_filter_add_pattern( filter_known, extbuf );
                    ext ++;
                }
                gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(dialog_open), filter );
                i ++;
            }
        }
}


void signal_reset( GtkToolButton *toolbutton, gpointer user_data )
{
    pause_wait();
    zx_reset();
    emul_start();
}

void signal_open( GtkToolButton *toolbutton, gpointer user_data )
{
    unsigned curf, i;
    pause_wait();

    if ( gtk_dialog_run (GTK_DIALOG (dialog_open)) == GTK_RESPONSE_ACCEPT )
    {

        char *filename, *ext;
        filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(dialog_open) );
        /*printf("Filter %d\n", g_slist_index( gtk_file_chooser_list_filters( GTK_FILE_CHOOSER(dialog) ),
                                             gtk_file_chooser_get_filter( GTK_FILE_CHOOSER(dialog) ) ) );*/
        ext = strrchr( filename, '.' );
        if ( ext > strrchr( filename, '/' ) )
            ext ++;

        for ( curf = 0; curf < filter_count; curf++ )
        {
            i = 0;
            while ( filters[curf].type[i].process )
            {
                if ( strcasecmp( ext, filters[curf].type[i].extention ) == 0 )
                    if ( filters[curf].type->process( filename ) == 0 )
                    {
                        curf = filter_count;    // me thinks it's not very smart, but i don't see any other choise
                        break;
                    }
                i ++;
            }
        }
        if ( curf == filter_count )
            printf( "Couldn't open file %s!\n", filename );

        g_free (filename);
    }
    gtk_widget_hide( dialog_open );
    //gtk_widget_destroy( dialog_open );

    emul_start();
}

int main( int argc, char *argv[] )
{
    GtkWidget *main_win = NULL;
    GtkWidget *main_cont = NULL;

    GtkWidget *main_toolbar = NULL;
    GtkToolItem *mtbut = NULL;

    g_thread_init(NULL);
    gdk_threads_init();
    // Initialize GTK+
    //g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
    gtk_init(&argc, &argv);
    //g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

    /* Create the main window */
    main_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    g_set_application_name("PerfectZX");
    gtk_window_set_position(GTK_WINDOW(main_win), GTK_WIN_POS_CENTER);
    gtk_window_set_default_icon_from_file( "icon.svg", NULL );

    gtk_widget_realize(main_win);
    g_signal_connect(main_win, "destroy", gtk_main_quit, NULL);

    // toolbar
    main_toolbar = gtk_toolbar_new();

    mtbut = gtk_tool_button_new_from_stock( GTK_STOCK_OPEN );
    g_signal_connect(mtbut, "clicked", G_CALLBACK(signal_open), NULL);
    gtk_toolbar_insert( GTK_TOOLBAR(main_toolbar), mtbut, -1 );
    mtbut = gtk_tool_button_new_from_stock( GTK_STOCK_SAVE_AS );
    gtk_toolbar_insert( GTK_TOOLBAR(main_toolbar), mtbut, -1 );
    mtbut = gtk_tool_button_new_from_stock( GTK_STOCK_REFRESH );
    g_signal_connect(mtbut, "clicked", G_CALLBACK(signal_reset), NULL);
    gtk_toolbar_insert( GTK_TOOLBAR(main_toolbar), mtbut, -1 );
    mtbut = gtk_tool_button_new_from_stock( GTK_STOCK_MEDIA_PAUSE );
    gtk_toolbar_insert( GTK_TOOLBAR(main_toolbar), mtbut, -1 );

    // screen
    scr_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 0, 8, 320, 240);
    //scr_pixbuf = gdk_pixbuf_new_from_data(gbuf, GDK_COLORSPACE_RGB, 0, 8, 320, 240, 320*3, NULL, NULL);
    scr_area = gtk_drawing_area_new();
    g_signal_connect(G_OBJECT(scr_area), "expose_event", G_CALLBACK(screen_update), NULL);
    gtk_widget_set_size_request(scr_area, 320*SCR_SCALE, 240*SCR_SCALE);
    gtk_widget_add_events( scr_area, GDK_ALL_EVENTS_MASK );
    gtk_widget_set_can_focus( scr_area, TRUE );
    gtk_widget_set_can_default( scr_area, TRUE );
    video_set_out( gdk_pixbuf_get_pixels(scr_pixbuf),
                   gdk_pixbuf_get_width(scr_pixbuf),
                   gdk_pixbuf_get_height(scr_pixbuf) );
    //video_set_out(gbuf, 320, 240);

    // keyboard handlers
    g_signal_connect( scr_area, "key-press-event", G_CALLBACK(event_key), NULL );
    g_signal_connect( scr_area, "key-release-event", G_CALLBACK(event_key), NULL );
    g_signal_connect( scr_area, "button-press-event", G_CALLBACK(event_button), NULL );
    g_signal_connect( scr_area, "button-release-event", G_CALLBACK(event_button), NULL );
    g_signal_connect( scr_area, "motion-notify-event", G_CALLBACK(event_motion), NULL );
    g_signal_connect( scr_area, "enter-notify-event", G_CALLBACK(event_crossing), NULL );

    cursor_blank = gdk_cursor_new( GDK_BLANK_CURSOR );
    gdk_display_get_pointer( gtk_widget_get_display( scr_area ), NULL, &mouse_last_x, &mouse_last_y, NULL );

    // compose everything
    main_cont = gtk_vbox_new( 0, 0 );
    gtk_container_add( GTK_CONTAINER(main_cont), main_toolbar );
    gtk_container_add( GTK_CONTAINER(main_cont), scr_area );
    gtk_widget_grab_focus( scr_area );
    //gtk_widget_grab_default( scr_area );
    //gtk_container_set_focus_child( GTK_CONTAINER(main_cont), scr_area );

    gtk_container_add( GTK_CONTAINER(main_win), main_cont );

    gtk_widget_show_all(main_win);

	emul_init();

    gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

    emul_uninit();

    g_object_unref(G_OBJECT(scr_pixbuf));

	return 0;
}
