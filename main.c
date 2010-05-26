//#include <windows.h>
#include "main.h"
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "emul.h"
#include "video.h"
#include "input.h"

GtkWidget *scr_area = NULL;
GdkPixbuf *scr_pixbuf;
//guchar gbuf[320*240*3];

static gboolean screen_update(GtkWidget *area, GdkEventExpose *event, GPtrArray *parray)
{
    gdk_draw_pixbuf(area->window,
        area->style->base_gc[GTK_WIDGET_STATE (area)],
        scr_pixbuf,
        0, 0,
        0, 0,
        320, 240,
        GDK_RGB_DITHER_NONE, 0, 0);
  return FALSE;//TRUE;
}

static gboolean event_key(GtkWidget *window, GdkEventKey *event, GPtrArray *parray)
{
    //printf( "Key: %d\n", event->keyval );
    input_event_keyboard(event->hardware_keycode, (event->type == GDK_KEY_PRESS));
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

int main( int argc, char *argv[] )
{
    GtkWidget *main_win = NULL;
    GdkPixbuf *main_win_icon;

    g_thread_init(NULL);
    gdk_threads_init();
    // Initialize GTK+
    //g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
    gtk_init(&argc, &argv);
    //g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

    /* Create the main window */
    main_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_container_set_border_width(GTK_CONTAINER(main_win), 8);
    gtk_window_set_title(GTK_WINDOW(main_win), "PerfectZX");
    gtk_window_set_position(GTK_WINDOW(main_win), GTK_WIN_POS_CENTER);
    main_win_icon = gdk_pixbuf_new_from_file("icon.png", NULL);
    gtk_window_set_icon(GTK_WINDOW(main_win), main_win_icon);

    gtk_widget_realize(main_win);
    g_signal_connect(main_win, "destroy", gtk_main_quit, NULL);

    // keyboard handlers
    g_signal_connect(main_win, "key-press-event", G_CALLBACK (event_key), NULL);
    g_signal_connect(main_win, "key-release-event", G_CALLBACK (event_key), NULL);

    scr_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 0, 8, 320, 240);
    //scr_pixbuf = gdk_pixbuf_new_from_data(gbuf, GDK_COLORSPACE_RGB, 0, 8, 320, 240, 320*3, NULL, NULL);
    scr_area = gtk_drawing_area_new();
    g_signal_connect(G_OBJECT(scr_area), "expose_event", G_CALLBACK(screen_update), NULL);
    gtk_widget_set_size_request(scr_area, 320, 240);
    gtk_container_add(GTK_CONTAINER(main_win), scr_area);
    video_set_out(gdk_pixbuf_get_pixels(scr_pixbuf),
        gdk_pixbuf_get_width(scr_pixbuf),
        gdk_pixbuf_get_height(scr_pixbuf));
    //video_set_out(gbuf, 320, 240);

    gtk_widget_show_all(main_win);

	emul_init();

    gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

    emul_uninit();

    g_object_unref(G_OBJECT(scr_pixbuf));

	return 0;
}
