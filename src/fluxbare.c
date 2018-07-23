/*
 * $Id: fluxbare.c,v 1.3 2004/12/10 14:47:14 babar Exp $
 * fluxbare.c for fluxconf
 *
 * This file was created by Emmanuel le Chevoir <manu42@free.fr>
 * Original code by Fabien Devaux <fab@gcu.info>
 * 
 * ***************************************************************************
 * *                                                                         *
 * *   This program is free software; you can redistribute it and/or modify  *
 * *   it under the terms of the GNU General Public License as published by  *
 * *   the Free Software Foundation; either version 2 of the License, or     *
 * *   (at your option) any later version.                                   *
 * *                                                                         *
 * ***************************************************************************
 */

#include <unistd.h>
#include <stdlib.h>             /* gentenv */
#include <stdio.h>
#include <fcntl.h>
#include <string.h>             /* strcat, strdup */
#include <strings.h>            /* strcasecmp */
#include <ctype.h>              /* toupper */
#include <errno.h>
#include <libintl.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <gtk/gtk.h>
#include <libgen.h>             /* basename(3) */

#include "xpm/fc_menu.xpm"
#include "xpm/fc_conf.xpm"
#include "xpm/fc_keys.xpm"

/* import XPM */
#include "xpm/dalogo.h"         /* application bootsplash         */

#include "globals.h"            /* define global variables as extern      */
#include "defines.h"            /* contains various defines           */
#include "structures.h"         /* where program structures are defined   */
#include "functions.h"

/* 
 * The structure struct s_programs contains the list of possible prognames,
 * and the name of the function to launch when called with each progname.
 * It contains two values per elements: 
 *    1. a string representing the name of the program
 *    2. a pointer to the appropriate function
 */
static struct s_programs proglist[] = {
    {"fluxconf", fluxconf},
    {"fluxkeys", fluxkeys},
#ifdef GTK2
    {"fluxmenu", fluxmenu},
#endif
    {"fluxbare", fluxbare},
    {NULL, NULL}
};


/* void start_it(GtkButton * button, char what) */
static void start_it(GtkButton * button, char what)
{
    button = NULL;  /* FIXME without this gcc hangs ... */
    switch (what) {
	case 0:
	    fluxconf(0,NULL,0);
	    break;
	case 1:
	    fluxkeys(0,NULL,0);
	    break;
#ifdef GTK2
	case 2:
	    fluxmenu(0,NULL,0);
	    break;
#endif
    }
}

static GtkWidget * CreateIco(GdkWindow *win, char *label,gchar **zexpm, char prog_number)
{
    GtkWidget *but,*vbox,*lbl,*wico;
    GdkPixmap *ico;
    gint prgnum = prog_number;

    but = gtk_button_new();
    vbox = gtk_vbox_new(FALSE,2);
    gtk_container_add(GTK_CONTAINER(but),vbox);
    lbl=gtk_label_new(label);
    ico = gdk_pixmap_create_from_xpm_d(win, NULL, NULL, zexpm);
    wico = gtk_pixmap_new(ico, NULL);
    gtk_box_pack_start(GTK_BOX(vbox),wico,TRUE,TRUE,2);
    gtk_box_pack_start(GTK_BOX(vbox),lbl,TRUE,TRUE,2);
    gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(start_it), GINT_TO_POINTER(prgnum));
    return but;
}

int fluxbare(int argc, char **argv, char standalone)
{
    GtkWidget *win,*butbox,*but;

    gtk_init(&argc,&argv);
    standalone = standalone; /* hack: avoid warnings and keep prototypes homgeneous */

    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "FluxConf launcher");
    butbox = gtk_hbox_new(FALSE, 2);

    gtk_container_add(GTK_CONTAINER(win), butbox);

    gtk_widget_realize(win);

    but=CreateIco(win->window,"fluxconf",fc_conf_xpm,0);
    gtk_box_pack_start(GTK_BOX(butbox), but, TRUE, TRUE, 2);
    but=CreateIco(win->window,"fluxkeys",fc_keys_xpm,1);
    gtk_box_pack_start(GTK_BOX(butbox), but, TRUE, TRUE, 2);
    but=CreateIco(win->window,"fluxmenu",fc_menu_xpm,2);
    gtk_box_pack_start(GTK_BOX(butbox), but, TRUE, TRUE, 2);

    gtk_signal_connect(GTK_OBJECT(win), "destroy", GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}




/* *******************************************************************************
                               main functions 
******************************************************************************* */
void makesplash(void)
{
#ifndef DEBUG
    GtkWidget *logowin, *popuplogo;
    GdkPixmap *logo;

    logowin = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_widget_realize(logowin);
    logo = gdk_pixmap_create_from_xpm_d(logowin->window, NULL, NULL, fluxconf_xpm);
    popuplogo = gtk_pixmap_new(logo, NULL);
    gtk_container_add(GTK_CONTAINER(logowin), popuplogo);
    gtk_window_set_position(GTK_WINDOW(logowin), GTK_WIN_POS_CENTER);

    gtk_widget_show_all(logowin);
    gtk_timeout_add(100, (GtkFunction) show, logowin);  /* show the logo */
    gtk_timeout_add(3500, (GtkFunction) hide, logowin); /* hide the logo */
#endif
}

/* FIXME: what's the point of returning an integer value ? */
int show(GtkWidget * wid)
{
    gtk_widget_show_all(wid);
    return 0;
}

/* FIXME: what's the point of returning an integer value ? */
int hide(GtkWidget * wid)
{
    gtk_widget_hide(wid);
    return 0;
}

int main(int argc, char **argv)
{
    char screen;

    bindtextdomain("fluxconf", LOCALEDIR);
    textdomain("fluxconf");
    screen = screenparse(argc, argv);
    makeitems(screen);
    program_run(argc, argv, STANDALONE);
    if (NULL != msgs)
        free(msgs);
    return 0;
}

/* expands the given filename, possibly destroys old pointer */
char *expandFilename(char *filename)
{
    while (*filename <= 32)
        filename++;
    if (strncmp("~/", filename, 2) == 0) {  /* if filename starts with ~/, expand with $HOME */
        int homelen = strlen(getenv("HOME"));
        int len = strlen(filename) + homelen + 1;
        char *newname = malloc(len);

        checkptr(newname);
        strncpy(newname, getenv("HOME"), homelen);
        strcpy(newname + homelen, filename + 1);
        free(filename);
        return newname;
    }
    return filename;
}

/* 
 * checkptr() : Checks pointer validity. FIXME: replace this function with
 * asserts and xmalloc macros 
 */
void checkptr(void *ptr)
{
    if (ptr == NULL) {
        g_print(_("No more memory :(\n"));
        exit(1);
    }
}

void makeitems(char screen)
{
    char *orig[] = {
        "session.screen0.tab.height",   /* first ;) */
        "session.screen0.tab.width",
        "session.screen0.edgeSnapThreshold",
        "session.screen0.toolbar.widthPercent",
        "session.screen0.workspaces",
        "session.autoRaiseDelay",
        "session.doubleClickInterval",
        "session.screen0.toolbar.onTop",    /* first TRUE/FALSE */
        "session.screen0.slit.onTop",
        "session.screen0.slit.autoHide",
        "session.screen0.slit.placement",   /* first QCM */
        "session.screen0.slit.direction",
        "session.screen0.rowPlacementDirection",
        "session.screen0.colPlacementDirection",
        "session.screen0.windowPlacement",
        "session.screen0.focusModel",
        "session.keyFile",      /* first litteral */
        "session.menuFile",
        "session.styleFile",
        "session.screen0.strftimeFormat"
    };
    int n;

    msgs = malloc(sizeof(char *) * NB);
    for (n = 0; n < NB; n++) {
        msgs[n] = strdup(orig[n]);
        if (!strncmp("session.screen0", msgs[n], 15))
            msgs[n][14] = screen;   /* replace default screen location ([0-9]) */
    }
}

/* 
 * function usage()
 *
 * SYNOPSIS: Describe program usage. 
 * RETURNS: 
 * 	exits the program, returning errorcode
 */
void usage(char const *progname, int errorcode)
{
    fprintf(stderr, "usage: %s [-sN]\n\twhere N is the screen number. (default: 0)\n", progname);
    exit(errorcode);
}

/* 
 * function screenparse()
 *
 * SYNOPSIS: parses commandline and returns selected screen
 * RETURNS: 
 * 	parsed screen char
 */
char screenparse(int argc, char *argv[])
{
    char screen = '0';

    if (argc > 2)
        usage(argv[0], 1);
    if (argc == 2) {
        if (strlen(argv[1]) == 3 &&
            (0 == strncmp("-s", argv[1], 2)) && (argv[1][2] <= '9' && argv[1][2] >= '0'))
            screen = argv[1][2];
        else
            usage(argv[0], 1);
    }
    return screen;
}

/* 
 * function program_run()
 *
 * SYNOPSIS: parses argv[0] and launches the appropriate function
 * RETURNS: 
 * 	-
 */
void program_run(int argc, char *argv[], char standalone)
{
    char *name = NULL;
    struct s_programs *program;

    name = basename(argv[0]);
    for (program = proglist; program->name != NULL; ++program) {
        if (0 == strcmp(program->name, name))
            program->start(argc, argv, standalone);
    }
    if (NULL == program)
        g_print(_("Unknown executable name \"%s\" !\n"), argv[0]);
}

