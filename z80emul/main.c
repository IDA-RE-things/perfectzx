#include <stdlib.h>
#include <gtk/gtk.h>

#include "z80.h"

z80_t cpu;

GtkWidget *reg_widget[12] = { 0 };
GtkWidget *op_widget = NULL;

void step_click( GtkButton *button, gpointer   user_data )
{
    z80_dis_t op;
    z80_step( &cpu );

    z80_dis( cpu.pc, &op );
    gtk_label_set_label( GTK_LABEL(op_widget), op.op );
}

void create_interface( GtkWidget *parent )
{
    GtkWidget *registers = NULL;
    GtkWidget *step = NULL;
    int i;

    registers = gtk_table_new( 6, 4, FALSE );

    for ( i = 0; i < 12; i ++ )
        reg_widget[i] = gtk_entry_new();

    gtk_table_attach_defaults( GTK_TABLE(registers), gtk_label_new( "AF:"), 0, 1, 0, 1 );
    gtk_table_attach_defaults( GTK_TABLE(registers), reg_widget[0], 1, 2, 0, 1 );
    gtk_table_attach_defaults( GTK_TABLE(registers), gtk_label_new( "BC:"), 0, 1, 1, 2 );
    gtk_table_attach_defaults( GTK_TABLE(registers), reg_widget[1], 1, 2, 1, 2 );
    gtk_table_attach_defaults( GTK_TABLE(registers), gtk_label_new( "DE:"), 0, 1, 2, 3 );
    gtk_table_attach_defaults( GTK_TABLE(registers), reg_widget[2], 1, 2, 2, 3 );
    gtk_table_attach_defaults( GTK_TABLE(registers), gtk_label_new( "HL:"), 0, 1, 3, 4 );
    gtk_table_attach_defaults( GTK_TABLE(registers), reg_widget[3], 1, 2, 3, 4 );

    gtk_table_attach_defaults( GTK_TABLE(registers), gtk_label_new( "AF':"), 2, 3, 0, 1 );
    gtk_table_attach_defaults( GTK_TABLE(registers), reg_widget[4], 3, 4, 0, 1 );
    gtk_table_attach_defaults( GTK_TABLE(registers), gtk_label_new( "BC':"), 2, 3, 1, 2 );
    gtk_table_attach_defaults( GTK_TABLE(registers), reg_widget[5], 3, 4, 1, 2 );
    gtk_table_attach_defaults( GTK_TABLE(registers), gtk_label_new( "DE':"), 2, 3, 2, 3 );
    gtk_table_attach_defaults( GTK_TABLE(registers), reg_widget[6], 3, 4, 2, 3 );
    gtk_table_attach_defaults( GTK_TABLE(registers), gtk_label_new( "HL':"), 2, 3, 3, 4 );
    gtk_table_attach_defaults( GTK_TABLE(registers), reg_widget[7], 3, 4, 3, 4 );

    gtk_table_attach_defaults( GTK_TABLE(registers), gtk_label_new( "IX:"), 4, 5, 0, 1 );
    gtk_table_attach_defaults( GTK_TABLE(registers), reg_widget[8], 5, 6, 0, 1 );
    gtk_table_attach_defaults( GTK_TABLE(registers), gtk_label_new( "IY:"), 4, 5, 1, 2 );
    gtk_table_attach_defaults( GTK_TABLE(registers), reg_widget[9], 5, 6, 1, 2 );
    gtk_table_attach_defaults( GTK_TABLE(registers), gtk_label_new( "SP:"), 4, 5, 2, 3 );
    gtk_table_attach_defaults( GTK_TABLE(registers), reg_widget[10], 5, 6, 2, 3 );
    gtk_table_attach_defaults( GTK_TABLE(registers), gtk_label_new( "PC:"), 4, 5, 3, 4 );
    gtk_table_attach_defaults( GTK_TABLE(registers), reg_widget[11], 5, 6, 3, 4 );

    z80_dis_t op;
    z80_dis( cpu.pc, &op );
    op_widget = gtk_label_new( op.op );
    gtk_table_attach_defaults( GTK_TABLE(registers), op_widget, 0, 4, 4, 5 );

    step = gtk_button_new_with_label( "step" );
    g_signal_connect( step, "clicked", G_CALLBACK(step_click), NULL );
    gtk_table_attach_defaults( GTK_TABLE(registers), step, 4, 6, 4, 5 );

    gtk_container_add( GTK_CONTAINER(parent), registers );
}

int main( int argc, char *argv[])
{
    GtkWidget *window = NULL;

    FILE *fp;
    fp = fopen( "sos.rom", "rb" );
    fread( memory, 1, 16384, fp );
    fclose( fp );

    gtk_init( &argc, &argv );

    window = gtk_window_new( GTK_WINDOW_TOPLEVEL );

    g_signal_connect( window, "destroy", G_CALLBACK(gtk_main_quit), NULL );

    create_interface( window );

    //gtk_widget_show( window );
    gtk_widget_show_all( window );

    gtk_main();


    return 0;
}
