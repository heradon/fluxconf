/* 
 * $Id: fluxconf.c,v 1.7 2004/12/08 16:58:37 babar Exp $
 * =====================| FluxConf | FluxKeys | FluxMenu |=======================
 * 
 *   A tool for Configuring the fluxbox window manager 
 * 
 *   by Fabien Devaux <fab@gcu.info>
 *   splitted and modified by Emmanuel le Chevoir <manu42@free.fr>
 * 
 *   Of course it's GPL
 *   Don't blame me for the pure Gruik style please
 * 
 *  ***************************************************************************
 *  *                                                                         *
 *  *   This program is free software; you can redistribute it and/or modify  *
 *  *   it under the terms of the GNU General Public License as published by  *
 *  *   the Free Software Foundation; either version 2 of the License, or     *
 *  *   (at your option) any later version.                                   *
 *  *                                                                         *
 *  ***************************************************************************
 */


/* 
 * Standard includes -- These should be declared before any other
 * project-specific files 
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

#include "globals.h"            /* define global variables as extern      */
#include "defines.h"            /* contains various defines           */
#include "structures.h"         /* where program structures are defined   */
#include "functions.h"          /* where function prototypes can be found */


/* 
 * These are global variables. Eeeek!... 
 * There's plenty of those crappy things!
 */
struct s_widgets s[NB];
int nblignes;
GList *actlist = NULL;
GtkWidget *mainvbox = NULL;
char **msgs = NULL;

#ifdef GTK2
GtkSizeGroup *size_group = NULL;
#endif

/* spinboxes maker */
GtkWidget *newparam(char *lbl, int val, int max, char *tipmsg, GtkTooltips * tips)
{
    GtkWidget *box, *label, *spin;
    GtkAdjustment *adj;

    box = gtk_hbox_new(FALSE, 5);
    label = gtk_label_new(lbl);
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    adj = (GtkAdjustment *) gtk_adjustment_new(val, /*value */
                                               0.0, /*lower */
                                               max, /*upper */
                                               1.0, /*step inc */
                                               5.0, /*page inc */
                                               5.0);    /*page size */
    spin = gtk_spin_button_new(adj, 0, 0);
    gtk_tooltips_set_tip(tips, spin, tipmsg, NULL);
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(box), spin, FALSE, FALSE, 1);
    return box;
}

/* combobox maker */
GtkWidget *newchoice(char *lbl, char *val, GList * liste, char *tipmsg, GtkTooltips * tips)
{
    GtkWidget *box, *label, *combo;

    box = gtk_hbox_new(FALSE, 5);
    label = gtk_label_new(lbl);
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    combo = gtk_combo_new();
    gtk_combo_set_popdown_strings(GTK_COMBO(combo), liste);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo)->entry), val);
    gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(combo)->entry), FALSE);

    gtk_tooltips_set_tip(tips, GTK_COMBO(combo)->entry, tipmsg, NULL);

#ifdef GTK2
    gtk_size_group_add_widget(size_group, combo);
#endif
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 1);
    return box;
}

GtkWidget *newlitteral(char *lbl, char *val, char *tipmsg, GtkTooltips * tips)
{
    GtkWidget *box, *label, *entry;

    label = gtk_label_new(lbl);
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    box = gtk_hbox_new(FALSE, 5);
    entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), val);
    gtk_tooltips_set_tip(tips, entry, tipmsg, NULL);
#ifdef GTK2
    gtk_size_group_add_widget(size_group, entry);
#endif
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 1);
    return box;
}

/* 
 * superchmod() : changes permissions of the .fluxbox/init file (o+rw) 
 */
void superchmod(void)
{
    char *fullpath;

    fullpath = malloc(MAXPATHLEN);
    snprintf(fullpath, MAXPATHLEN, "%s/.fluxbox/init", getenv("HOME"));
    if (-1 == chmod(fullpath, S_IRUSR | S_IWUSR)) {
	perror("chmod");
	free(fullpath);
	exit(1);
    }
    free(fullpath);
}

/* save the state */
/* void sauver(char **laliste) */
void sauver(GtkButton * button, char **laliste) /* FIXME without first parameter function hangs ... */
{
    int n, x;
    char *fullpath = NULL;
    FILE *file = NULL;
    GList *widlist = NULL;
    GtkWidget *tmpwid, *smallwin;

#ifndef GTK2
    GtkWidget *smalllbl;
#endif
    GtkBoxChild *mome;

    button = NULL; /* FIXME if not present gcc hangs ... */
    fullpath = malloc(MAXPATHLEN);
    snprintf(fullpath, MAXPATHLEN, "%s/.fluxbox/init", getenv("HOME"));
    if (-1 == chmod(fullpath, S_IRUSR | S_IWUSR)) {
        perror ("chmod");
	exit (1);
    }
    /*
    * try to make a backup 
    */
    char tmp_buff[BUFSIZ + 1], * ptr = NULL;
    int source_fd, dest_fd, rdlen;

    memset(tmp_buff, 0, BUFSIZ + 1);

    ptr = strrchr(fullpath, '/');
    if (ptr && ptr - fullpath < BUFSIZ) {
        strncpy(tmp_buff, fullpath, ptr - fullpath);
        snprintf(tmp_buff, BUFSIZ, "%s/init-FCONFBKP.old", tmp_buff);
        dest_fd = open(tmp_buff, O_WRONLY | O_TRUNC | O_CREAT, 0666);
        source_fd = open(fullpath, O_RDONLY);
        if (dest_fd == -1 || source_fd == -1) {
            if (dest_fd < 0)
                close(dest_fd);
            if (source_fd < 0)
                close(source_fd);
        } else {
            while ((rdlen = read(source_fd, tmp_buff, BUFSIZ)) > 0)
                write(dest_fd, tmp_buff, rdlen);
            close(source_fd);
            close(dest_fd);
        }
    }
    file = fopen(fullpath, "r+");

    for (n = 0; n < nblignes + 1; n++) {    /* scan all lines */
        for (x = 0; x < NB; x++) {  /* if some of them correspond to a widget... */
            if (s[x].l == n) {
                if (x < QCM_START) {    /* if spinbut */
                    widlist = GTK_BOX(s[x].wid)->children;
                    mome = widlist->next->data;
                    tmpwid = mome->widget;
                    sprintf(laliste[n], "%s:\t%d\n", msgs[x],
                            gtk_spin_button_get_value_as_int((GtkSpinButton *) tmpwid));
                } else if (x > QCM_START - 1 && x < QCM_STOP) { /* if TRUE/FALSE */
                    sprintf(laliste[n], "%s:\t%s\n", msgs[x],
                            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s[x].wid)) ? "true" :
                            "false");
                } else if (x < LIT_START) { /* if QCM */
                    widlist = GTK_BOX(s[x].wid)->children;
                    mome = widlist->next->data;
                    tmpwid = mome->widget;
                    sprintf(laliste[n], "%s:\t%s\n", msgs[x],
                            gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(tmpwid)->entry)));
                } else {        /* if Litteral entry */
                    widlist = GTK_BOX(s[x].wid)->children;
                    mome = widlist->next->data;
                    tmpwid = mome->widget;
                    sprintf(laliste[n], "%s:\t%s\n", msgs[x],
                            gtk_entry_get_text(GTK_ENTRY(tmpwid)));
                }
            }
        }
#ifdef DEBUG
        g_print("%s", g_locale_from_utf8(laliste[n],-1,0,0,0));
#else
        fputs(g_locale_from_utf8(laliste[n],-1,0,0,0), file);
#endif
    }

    fclose(file);
    chmod(fullpath, S_IRUSR);
    free(fullpath);
#ifndef GTK2
    smallwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    smalllbl =
        gtk_label_new(
            _("Now you must restart fluxbox\n(using the root menu, not killing X).\nDon't forget to press \"Let fluxbox change the conf\" before exiting."));
    gtk_container_add(GTK_CONTAINER(smallwin), smalllbl);
    gtk_widget_set_usize(smallwin, 400, 100);
    gtk_label_set_line_wrap(GTK_LABEL(smalllbl), TRUE);
    gtk_widget_show_all(smallwin);
#else
    smallwin =
        gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
            _("Now you must restart fluxbox (using the root menu, not killing X). Don't forget to press \"Let fluxbox change the conf\" before exiting."));
    gtk_dialog_run(GTK_DIALOG(smallwin));
    gtk_widget_destroy(smallwin);
#endif
}

/* 
 * FIXME: lot of work to do here :
 * - move the [0-14] values to an enum set. This should be much more readable
 * - break the whole 255 lines function into subsets
 * - check allocation boundaries
 * - avoid multiple calls of the same function by storing its result 
 * - move hardcoded values to #defines
 * - add comments
 */
int fluxconf(int argc, char **argv, char standalone)
{
    GtkWidget *win, *vbox1, *vbox2, *mainhbox, *mainvbox, *title, *butbox, *but;
    GtkWidget *vsplit, *hsplit;
    GdkFont *head;
    GtkTooltips *tip;
    GList *items = NULL;
    char *initpath, *buf, **config;
    FILE *file;

#ifndef GTK2
    GtkStyle *monstyle;
#endif
    int i = 1, y, msgsLen;

    config = malloc(sizeof(char **));
    checkptr(config);
    gtk_init(&argc, &argv);

    /*
     * A few changes to try to fix debian bug #148542, 
     * * that is, anyway, obsoleted by the use of GTK2 in 0.8.x
     */
    head = gdk_font_load("-*-lucida-bold-i-*-*-*-180-*-*-*-*-*");
    if (!head)
        head = gdk_font_load("-*-helvetica-bold-i-*-*-*-180-*-*-*-*-*-*");
    /*
     * end fix 
     */

    buf = getenv("HOME");
    initpath = malloc(MAXPATHLEN);
    snprintf(initpath, MAXPATHLEN, "%s/.fluxbox/init", buf);
    chmod(initpath, S_IRUSR | S_IWUSR);
    file = fopen(initpath, "r");
    if (file == NULL) {
        g_print("Can't open %s\n", initpath);
        exit(1);
    }
    buf = malloc(MAXLINELEN);
    checkptr(buf);
    /*
     * get all lines 
     */
    while (fgets(buf, MAXLINELEN, file)) {
        config = realloc(config, 1 + i * sizeof(char *));
        checkptr(config);
        config[i - 1] = (char *) malloc(30 + strlen(buf));  /* I get some unused space for bigger numbers, etc... */
        checkptr(config[i - 1]);
        strcpy(config[i - 1], g_locale_to_utf8(buf,-1,0,0,0));
        for (y = 0; y < NB; y++) {
            msgsLen = strlen(msgs[y]);
            if (!strncmp(config[i - 1], msgs[y], msgsLen)) {
                /*
                 * g_print("msg(%d)=%s",y,config[i-1]); 
                 */
                s[y].l = i - 1;
                if (y < QCM_START)
                    s[y].val = atoi(config[i - 1] + msgsLen + 2);   /* < 7 are spinboxes */
                else if (y > QCM_START - 1 && y < QCM_STOP) {   /* TRUE or FALSE */
                    if (toupper(config[i - 1][msgsLen + 2]) == 'F')
                        s[y].val = 0;
                    else
                        s[y].val = 1;
                } else if (y < LIT_START) { /* multiple choice boxes */
                    s[y].value = malloc(30);    /* FIXME: 30 ? */
                    checkptr(s[y].value);
                    strncpy(s[y].value, config[i - 1] + msgsLen + 2, 30);
                    s[y].value[strlen(s[y].value) - 1] = '\0';  /* remove the "\n" */
                } else {        /* Literals */
                    s[y].value = malloc((EXELEN + 1));
                    checkptr(s[y].value);
                    strncpy(s[y].value, config[i - 1] + msgsLen + 2, EXELEN + 1);
                    s[y].value[strlen(s[y].value) - 1] = '\0';  /* remove the "\n" */
                }
            }
        }
        i++;
    }
    fclose(file);
    free(initpath);
    nblignes = i - 2;

    tip = gtk_tooltips_new();   /* create the tooltip */
    /*
     * main win 
     */
    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), _("Fluxbox Configuration Tool"));

#ifndef GTK2
    title = gtk_label_new(_("Fluxbox Conf. Tool"));
    if (head) {
        monstyle = gtk_style_copy(gtk_widget_get_style(GTK_WIDGET(title)));
        monstyle->font = head;
        gtk_widget_set_style(GTK_WIDGET(title), monstyle);
    }
#else
    title =
        gtk_label_new(_("<span size=\"18000\" weight=\"bold\">Fluxbox Configuration Tool</span>"));
    gtk_label_set_use_markup(GTK_LABEL(title), TRUE);
#endif
    hsplit = gtk_hseparator_new();
    vsplit = gtk_vseparator_new();
    vbox1 = gtk_vbox_new(TRUE, 4);
    vbox2 = gtk_vbox_new(TRUE, 4);
    mainvbox = gtk_vbox_new(FALSE, 2);
    mainhbox = gtk_hbox_new(FALSE, 2);
    butbox = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(win), mainvbox);
    gtk_box_pack_start(GTK_BOX(mainvbox), title, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(mainvbox), mainhbox, FALSE, FALSE, 4);
    title = gtk_label_new(_("(*) is screen independant."));
    /*
     * not working 
     */
    gtk_label_set_justify(GTK_LABEL(title), GTK_JUSTIFY_RIGHT);
    gtk_box_pack_start(GTK_BOX(mainvbox), title, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(mainvbox), hsplit, FALSE, FALSE, 0); /* new */
    gtk_box_pack_start(GTK_BOX(mainvbox), butbox, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(mainhbox), vbox1, TRUE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(mainhbox), vsplit, TRUE, FALSE, 0);  /* new */
    gtk_box_pack_start(GTK_BOX(mainhbox), vbox2, TRUE, FALSE, 4);
    gtk_window_set_policy(GTK_WINDOW(win), 0, 0, 1);    /* user can't resize */
    gtk_signal_connect(GTK_OBJECT(win), "destroy", GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(win), 5);
#ifdef GTK2
    size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
#endif
    /*
     * spinboxes 
     */
    s[0].wid = newparam(_("Tab height"), s[0].val, 100, _("Set the tab's height"), tip);
    gtk_box_pack_start(GTK_BOX(vbox2), s[0].wid, FALSE, FALSE, 0);

    s[1].wid = newparam(_("Tab width"), s[1].val, 300, _("Set the tab's width"), tip);
    gtk_box_pack_start(GTK_BOX(vbox2), s[1].wid, FALSE, FALSE, 0);

    s[2].wid =
        newparam(_("Edges snap threshold"), s[2].val, 100, _("How many pixels will be magnetic"),
                 tip);
    gtk_box_pack_start(GTK_BOX(vbox2), s[2].wid, FALSE, FALSE, 0);

    s[3].wid = newparam(_("Toolbar width (%)"), s[3].val, 100, _("Set the toolbar width"), tip);
    gtk_box_pack_start(GTK_BOX(vbox2), s[3].wid, FALSE, FALSE, 0);

    s[4].wid = newparam(_("Workspaces"), s[4].val, 1024, _("How many workspaces do you want"), tip);
    gtk_box_pack_start(GTK_BOX(vbox2), s[4].wid, FALSE, FALSE, 0);

    s[5].wid =
        newparam(_("Autoraise delay (*)"), s[5].val, 4000, _("Set the delay for windows to raise"),
                 tip);
    gtk_box_pack_start(GTK_BOX(vbox2), s[5].wid, FALSE, FALSE, 0);

    s[6].wid =
        newparam(_("Double click interval (*)"), s[6].val, 2000, _("Hum... so explicit ;)"), tip);
    gtk_box_pack_start(GTK_BOX(vbox2), s[6].wid, FALSE, FALSE, 0);
    /*
     * TRUE FALSE stuffs 
     */
    s[7].wid = gtk_toggle_button_new_with_label(_("Toolbar on top"));
    gtk_tooltips_set_tip(tip, s[7].wid, _("Toolbar will always stay on top"), NULL);
    if (s[7].val)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s[7].wid), 1);
    gtk_box_pack_start(GTK_BOX(vbox2), s[7].wid, FALSE, FALSE, 0);

    s[8].wid = gtk_toggle_button_new_with_label(_("Slit on top"));
    gtk_tooltips_set_tip(tip, s[8].wid, _("The dockbar will stay on top"), NULL);
    if (s[8].val)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s[8].wid), 1);
    gtk_box_pack_start(GTK_BOX(vbox2), s[8].wid, FALSE, FALSE, 0);

    s[9].wid = gtk_toggle_button_new_with_label(_("Slit auto hide"));
    gtk_tooltips_set_tip(tip, s[9].wid, _("The dockbar will autohide"), NULL);
    if (s[9].val)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s[9].wid), 1);
    gtk_box_pack_start(GTK_BOX(vbox2), s[9].wid, FALSE, FALSE, 0);
    /*
     * Combobox 
     */
    items = g_list_append(items, "TopLeft");
    items = g_list_append(items, "CenterLeft");
    items = g_list_append(items, "BottomLeft");
    items = g_list_append(items, "TopCenter");
    items = g_list_append(items, "BottomCenter");
    items = g_list_append(items, "TopRight");
    items = g_list_append(items, "BottomRight");
    items = g_list_append(items, "CenterRight");
    s[10].wid =
        newchoice(_("Slit placement"), s[10].value, items, _("Change the dockbar placement"), tip);
    gtk_box_pack_start(GTK_BOX(vbox1), s[10].wid, FALSE, FALSE, 0);
    items = NULL;

    items = g_list_append(items, "Horizontal");
    items = g_list_append(items, "Vertical");
    s[11].wid =
        newchoice(_("Slit direction"), s[11].value, items, _("Change the dockbar orientation"),
                  tip);
    gtk_box_pack_start(GTK_BOX(vbox1), s[11].wid, FALSE, FALSE, 0);
    items = NULL;

    items = g_list_append(items, "LeftToRight");
    items = g_list_append(items, "RightToLeft");
    s[12].wid =
        newchoice(_("Row direction"), s[12].value, items, _("Set the window placement direction"),
                  tip);
    gtk_box_pack_start(GTK_BOX(vbox1), s[12].wid, FALSE, FALSE, 0);
    items = NULL;

    items = g_list_append(items, "TopToBottom");
    items = g_list_append(items, "BottomToTop");
    s[13].wid =
        newchoice(_("Col direction"), s[13].value, items, _("Set the window placement direction"),
                  tip);
    gtk_box_pack_start(GTK_BOX(vbox1), s[13].wid, FALSE, FALSE, 0);
    items = NULL;

    items = g_list_append(items, "ColSmartPlacement");
    items = g_list_append(items, "RowSmartPlacement");
    items = g_list_append(items, "CascadePlacement");
    items = g_list_append(items, "UnderMousePlacement");
    s[14].wid =
        newchoice(_("Window placement"), s[14].value, items, _("Change the way windows are placed"),
                  tip);
    gtk_box_pack_start(GTK_BOX(vbox1), s[14].wid, FALSE, FALSE, 0);
    items = NULL;

    items = g_list_append(items, "MouseFocus");
    items = g_list_append(items, "ClickToFocus");
    s[15].wid = newchoice(_("Focus model"), s[15].value, items, _("Change the focus method"), tip);
    gtk_box_pack_start(GTK_BOX(vbox1), s[15].wid, FALSE, FALSE, 0);


    s[16].wid =
        newlitteral(_("Key file location (*)"), s[16].value,
                    _("Change the location of the keybinds file"), tip);
    gtk_box_pack_start(GTK_BOX(vbox1), s[16].wid, FALSE, FALSE, 0);
    s[17].wid =
        newlitteral(_("Menu file location (*)"), s[17].value,
                    _("Change the location of the menu file"), tip);
    gtk_box_pack_start(GTK_BOX(vbox1), s[17].wid, FALSE, FALSE, 0);
    s[18].wid =
        newlitteral(_("Theme file location (*)"), s[18].value,
                    _("Change the location of the theme file"), tip);
    gtk_box_pack_start(GTK_BOX(vbox1), s[18].wid, FALSE, FALSE, 0);
    s[19].wid =
        newlitteral(_("Time format"), s[19].value,
                    _
                    ("%y:year(YY)\n%Y:year(YYYY)\n%m:month\n%d:day\n%H:hour(24)\n%I:hour(12)\n%M:minutes\n%p:meridian"),
                    tip);
    gtk_box_pack_start(GTK_BOX(vbox1), s[19].wid, FALSE, FALSE, 0);

    items = NULL;
    /*
     * some buttons 
     */
    but = gtk_button_new_with_label(_("Save"));
    gtk_box_pack_start(GTK_BOX(butbox), but, TRUE, TRUE, 2);
    gtk_tooltips_set_tip(tip, but,
                         _("Save the state, You'll need to RESTART fluxbox using the menu"), NULL);
    gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(sauver), config);

    but = gtk_button_new_with_label(_("Let fluxbox change the conf"));
    gtk_box_pack_start(GTK_BOX(butbox), but, TRUE, TRUE, 2);
    gtk_tooltips_set_tip(tip, but, _("Once changes take effect, press this before quitting"), NULL);
    gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(superchmod), NULL);
    /*
     * but = gtk_button_new_with_label("Start fluxkeys");
     * gtk_box_pack_start(GTK_BOX(butbox), but, TRUE, TRUE, 2);
     * gtk_tooltips_set_tip(tip, but, "Start fluxkeys", NULL);
     * gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(start_it), (gpointer) 1);
     * #ifdef GTK2
     * but = gtk_button_new_with_label("Start fluxmenu");
     * gtk_box_pack_start(GTK_BOX(butbox), but, TRUE, TRUE, 2);
     * gtk_tooltips_set_tip(tip, but, "Start fluxmenu", NULL);
     * gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(start_it), (gpointer)2);
     * #endif
     */

    if (standalone) {
        but = gtk_button_new_with_label(_("Quit"));
        gtk_box_pack_start(GTK_BOX(butbox), but, FALSE, FALSE, 2);
        gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
        gtk_tooltips_set_tip(tip, but, _("This one is useless...   ;)"), NULL);
        gtk_button_set_relief(GTK_BUTTON(but), GTK_RELIEF_NONE);
        makesplash();
    }
    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}

