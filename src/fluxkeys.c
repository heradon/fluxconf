/*
 * $Id: fluxkeys.c,v 1.6 2004/12/10 14:47:14 babar Exp $
 * fluxkeys.c for fluxconf
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
#include <stdio.h>              /* gentenv */
#include <string.h>             /* strcat, strdup */
#include <strings.h>            /* strcasecmp */
#include <libintl.h>
#include <sys/param.h>          /* MAXPATHLEN */
#include <gtk/gtk.h>
#include <fcntl.h>

#include "globals.h"            /* actions for the Keys file */
#include "defines.h"

#include "structures.h"         /* where program structures are defined   */
#include "functions.h"          /* where function prototypes can be found */

int fluxkeys(int argc, char **argv, char standalone)
{
    GtkWidget *scrw, *pbox, *butbox, *win, *hsplit, *title, *but, **boxes;
    GdkFont *head;
    GtkTooltips *tip;
    char *initpath, *buf, **config, *touche, *tmp;
    FILE *file;

    /*
     * void killme(GtkButton * button, GtkWidget *what) { gtk_widget_destroy(what); }
     */
#ifndef GTK2
    GtkStyle *monstyle;
#endif
    int i = 1, n, modifiers, x = 0;

    config = (char **) malloc(sizeof(char *));

    checkptr(config);

    gtk_init(&argc, &argv);
    tip = gtk_tooltips_new();   /* create the tooltip */

    initpath = malloc(MAXPATHLEN);

    /*
     * A few changes to try to fix #148542, 
     * * that is, anyway, obsoleted by the use of GTK2 in 0.8.x
     */
    head = gdk_font_load("-*-lucida-bold-i-*-*-*-180-*-*-*-*-*");
    if (!head)
        head = gdk_font_load("-*-helvetica-bold-i-*-*-*-180-*-*-*-*-*-*");
    /*
     * end fix 
     */

    buf = malloc((KEYLEN + ACTLEN + EXELEN) * sizeof(char));
    checkptr(buf);

    /*
     * find the right file to open 
     */
    snprintf(initpath, MAXPATHLEN, "%s/.fluxbox/init", getenv("HOME"));
    file = fopen(initpath, "r");
    if (file == NULL) {
        g_print(_("Can't open %s\n"), initpath);
        g_print(_("Make sure you installed fluxbox with this user.\n"));
        exit(1);
    }
    while (fgets(buf, 200, file)) {
        if (!strncasecmp(buf, "session.keyFile:", strlen("session.keyFile:"))) {
            for (n = strlen("session.keyFile:") + 1; buf[n] < 33; n++);
            buf[strlen(buf) - 1] = 0;
            strncpy(initpath, buf + n, 200);
        }
    }
    fclose(file);
    initpath = expandFilename(initpath);
    file = fopen(initpath, "r");
    if (file == NULL) {
        g_print(_("Can't open %s\n"), initpath);
        g_print(ERRFLUXKEYS);
        exit(1);
    }

    /*
     * read the config 
     */
    while (fgets(buf, KEYLEN + ACTLEN + EXELEN, file)) {
        config = realloc(config, (1 + i) * sizeof(char *));
        checkptr(config);
        config[i - 1] = (char *) malloc(10 + strlen(buf) * sizeof(char));
        checkptr(config[i - 1]);
        if (strlen(buf) > 10 && buf[0] != '!') {
            strcpy(config[i - 1], buf);
            i++;
        }
        /*
         * just stop operations if more than max lines 
         */
        if (i > MAXKEYCOUNT)
            break;
    }
    fclose(file);
    free(initpath);
    nblignes = i - 2;

    boxes = (GtkWidget **) malloc(MAXKEYCOUNT * sizeof(GtkWidget *));
	{ int i;
		for(i=0;actions[i][0];i++)
			actlist = g_list_append(actlist, actions[i]);
	}

    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Fluxbox Configuration Tool");
    gtk_window_set_default_size(GTK_WINDOW(win), 700, 500);
#ifndef GTK2
    title = gtk_label_new("Fluxbox Conf. Tool");
    if (head) {
        monstyle = gtk_style_copy(gtk_widget_get_style(GTK_WIDGET(title)));
        monstyle->font = head;
        gtk_widget_set_style(GTK_WIDGET(title), monstyle);
    }
#else
    title = gtk_label_new("<span size=\"18000\" weight=\"bold\">Fluxbox Configuration Tool</span>");
    gtk_label_set_use_markup(GTK_LABEL(title), TRUE);
#endif
    hsplit = gtk_hseparator_new();
    pbox = gtk_vbox_new(FALSE, 2);
    scrw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrw),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    mainvbox = gtk_vbox_new(FALSE, 2);
    butbox = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(win), pbox);
    gtk_box_pack_start(GTK_BOX(pbox), title, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(pbox), scrw, TRUE, TRUE, 4);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrw), mainvbox);
    /**********************************************************************/
    gtk_box_pack_end(GTK_BOX(pbox), butbox, FALSE, FALSE, 4);   /* make it at end */

    but = gtk_button_new_with_label(_("Save"));
    gtk_box_pack_start(GTK_BOX(butbox), but, TRUE, TRUE, 2);
    gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(savekeys), boxes);

    but = gtk_button_new_with_label(_("Add"));
    gtk_box_pack_start(GTK_BOX(butbox), but, TRUE, TRUE, 2);
    gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(addkey), boxes);

    if (standalone) {
        but = gtk_button_new_with_label(_("Quit"));
        gtk_box_pack_start(GTK_BOX(butbox), but, FALSE, FALSE, 2);
        gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(gtk_main_quit), boxes);
        gtk_button_set_relief(GTK_BUTTON(but), GTK_RELIEF_NONE);
    }

    /********************************************************************************************************/
    touche = (char *) malloc(KEYLEN * sizeof(char));    /* init the ptr */
    tmp = (char *) malloc(ACTLEN * sizeof(char));   /* init the ptr */
    checkptr(touche);
    for (i = 0; i < nblignes + 1; i++) {
        buf = getthings(config[i], &modifiers, touche);
        tmp = firstword(buf, &x);
        buf[strlen(buf) - 1] = '\0';    /* remove the \n */
        if (!strcasecmp(tmp, "ExecCommand")) {
            for (x = strlen(tmp); buf[x] < 33; x++);
            boxes[i] =
                newkey(modifiers, actlist, tmp,
                       touche, buf + x);
        } else
            boxes[i] =
                newkey(modifiers, actlist, buf,
                       touche, "");
        gtk_box_pack_start(GTK_BOX(mainvbox), boxes[i], FALSE, FALSE, 0);
    }

    /********************************************************************************************************/


    gtk_signal_connect(GTK_OBJECT(win), "destroy", GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

    gtk_widget_show_all(win);

    if (standalone) {
        makesplash();
    }
    gtk_main();
    return 0;
}

/* void delkey(GtkButton *button, gpointer data) */
void delkey(GtkButton *button, gpointer data)
{
    button = NULL; /* FIXME without this gcc hangs */
    /* manu@20080811:
     *   Don't decrement line count, we'll skip NULL'ed items. */
    /* nblignes--; */
    gtk_widget_destroy(data);
}

GtkWidget *newkey(int modifiers, GList * liste, char *value, char *touche, char *execcmd)
{
    GtkWidget *hbox, *m1, *m2, *m3, *m4, *m5, *combo, *entry, *key, *del;

    hbox = gtk_hbox_new(FALSE, 1);
    m1 = gtk_toggle_button_new_with_label("Control");
    m2 = gtk_toggle_button_new_with_label("Mod1");
    m3 = gtk_toggle_button_new_with_label("Shift");
    m4 = gtk_toggle_button_new_with_label("Mod4");
    m5 = gtk_toggle_button_new_with_label("OnDesktop");

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m1), modifiers & MOD_CTRL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m2), modifiers & MOD_ALT);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m3), modifiers & MOD_SHIFT);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m4), modifiers & MOD_WIN);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m5), modifiers & MOD_ONDESKTOP);

    combo = gtk_combo_new();
    gtk_combo_set_popdown_strings(GTK_COMBO(combo), liste);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo)->entry), value);
    gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(combo)->entry), FALSE);

    entry = gtk_entry_new_with_max_length(EXELEN);
    gtk_entry_set_text(GTK_ENTRY(entry), g_locale_to_utf8(execcmd,-1,0,0,0));
    key = gtk_entry_new_with_max_length(KEYLEN);
    gtk_entry_set_text(GTK_ENTRY(key), g_locale_to_utf8(touche,-1,0,0,0));

    del = gtk_button_new_with_label("Del");
    /*
     * g_signal_connect (G_OBJECT(del), "clicked", G_CALLBACK(delkey), hbox);
     */
    gtk_signal_connect(GTK_OBJECT(del), "clicked", GTK_SIGNAL_FUNC(delkey), hbox);

    gtk_box_pack_start(GTK_BOX(hbox), m1, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hbox), m2, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hbox), m3, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hbox), m4, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hbox), m5, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hbox), key, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hbox), combo, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(hbox), del, FALSE, FALSE, 1);

    return hbox;
}

/* 
 * FIXME: many things again:
 * - split the huge function into subset
 * - reuse previous subsets, some portion of the code are highly redundant
 * - use more constants and less hardcoded strings
 * - check boundaries of allocated/freed variables
 * - replace all those #ifdef/#endif with a single macro.
 */
void savekeys(GtkButton *button, GtkBox ** boites) /* FIXME GtkButton *button is for compatibility reason */
{
    GtkBoxChild *child;
    GtkWidget *wid;
    int mod, n;
    char *initpath, *buf;

#ifdef GTK2
    G_CONST_RETURN gchar *key, *action, *execact;
#else
    gchar *key, *action, *execact;
#endif
    int x = 0;
    FILE *file;

    initpath = malloc(MAXPATHLEN);
    buf = malloc(MAXPATHLEN);

    button = NULL; /* FIXME without this gcc hangs */
    /*
     * find the right file to open 
     */
    snprintf(initpath, MAXPATHLEN, "%s/.fluxbox/init", getenv("HOME"));
    file = fopen(initpath, "r");
    if (file == NULL) {
        g_print(_("Can't open %s\n"), initpath);
        g_print(_("Make sure you installed fluxbox with this user.\n"));
        exit(1);
    }
    while (fgets(buf, MAXPATHLEN, file)) {
        if (!strncasecmp(buf, "session.keyFile:", strlen("session.keyFile:"))) {
            for (n = strlen("session.keyFile:") + 1; buf[n] < 33; n++);
            buf[strlen(buf) - 1] = 0;
            strncpy(initpath, buf + n, MAXPATHLEN);
	    break;
        }
    }
    fclose(file);
    free(buf);
    initpath = expandFilename(initpath);
    /*
    * try to make a backup 
    */
    /*
    * FIXME: this is highly redundant 
    */
    char tmp_buff[BUFSIZ + 1], *ptr;
    int source_fd, dest_fd, rdlen;

    memset(tmp_buff, 0, BUFSIZ + 1);

    ptr = strrchr(initpath, '/');
    if (ptr && ptr - initpath < BUFSIZ) {
        strncpy(tmp_buff, initpath, ptr - initpath);
        snprintf(tmp_buff, BUFSIZ, "%s/keys-FCONFBKP.old", tmp_buff);
        dest_fd = open(tmp_buff, O_WRONLY | O_TRUNC | O_CREAT, 0666);
        source_fd = open(initpath, O_RDONLY);
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
    file = fopen(initpath, "w");
    if (file == NULL) {
        g_print(_("Can't open %s for writing\n"), initpath);
        g_print(ERRFLUXKEYS);
        exit(1);
    }

    /*
     * key= (char*)malloc(KEYLEN*sizeof(char));
     * action=  (char*)malloc(ACTLEN*sizeof(char));
     * execact=(char*)malloc(EXELEN*sizeof(char));
     */
#ifndef DEBUG
    fprintf(file, _("!Generated by fluxkeys\n"));
#else
    g_print(_("!Generated by fluxkeys\n"));
#endif
    for (x = 0; x < nblignes + 1; x++) {
        mod = MOD_NONE;
	if (NULL == boites[x]->children) {
	    /* manu@20080811: 
	     *   An element in the middle of the list has been removed. 
	     *   Ignore it and process the next one. */
	    continue;
	}
        child = boites[x]->children->data;
        wid = child->widget;
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid)))
            mod |= MOD_CTRL;
        child = boites[x]->children->next->data;
        wid = child->widget;
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid)))
            mod |= MOD_ALT;
        child = boites[x]->children->next->next->data;
        wid = child->widget;
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid)))
            mod |= MOD_SHIFT;
        child = boites[x]->children->next->next->next->data;
        wid = child->widget;
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid)))
            mod |= MOD_WIN;
        child = boites[x]->children->next->next->next->next->data;
        wid = child->widget;
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid)))
            mod |= MOD_ONDESKTOP;
        /*
         * all modifiers are ok 
         */
        child = boites[x]->children->next->next->next->next->next->data;
        wid = child->widget;
        key = gtk_entry_get_text(GTK_ENTRY(wid));   /* get the key entry */
        child = boites[x]->children->next->next->next->next->next->next->data;
        wid = child->widget;
        action = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(wid)->entry));  /* get the action entry */
        child = boites[x]->children->next->next->next->next->next->next->next->data;
        wid = child->widget;
        execact = gtk_entry_get_text(GTK_ENTRY(wid));   /* get the exec action entry */
        if (strlen(key)) {
            if (!mod)
#ifndef DEBUG
                fprintf(file, "None ");
#else
                g_print("None ");
#endif
            else {
                if (mod & MOD_CTRL)
#ifndef DEBUG
                    fprintf(file, "Control ");
#else
                    g_print("Control ");
#endif
                if (mod & MOD_ALT)
#ifndef DEBUG
                    fprintf(file, "Mod1 ");
#else
                    g_print("Mod1 ");
#endif
                if (mod & MOD_SHIFT)
#ifndef DEBUG
                    fprintf(file, "Shift ");
#else
                    g_print("Shift ");
#endif

                if (mod & MOD_WIN)
#ifndef DEBUG
                    fprintf(file, "Mod4 ");
#else
                    g_print("Mod4 ");
#endif
                if (mod & MOD_ONDESKTOP)
#ifndef DEBUG
                    fprintf(file, "OnDesktop ");
#else
                    g_print("OnDesktop ");
#endif
            }
#ifndef DEBUG
            fprintf(file, "%s ", g_locale_from_utf8(key,-1,0,0,0));
            fprintf(file, ":%s", g_locale_from_utf8(action,-1,0,0,0));
            if (!strcasecmp("ExecCommand", action))
                fprintf(file, " %s", g_locale_from_utf8(execact,-1,0,0,0));
            fprintf(file, "\n");
#else
            g_print("%s ", g_locale_from_utf8(key,-1,0,0,0));
            g_print(":%s", g_locale_from_utf8(action,-1,0,0,0));
            if (!strcasecmp("ExecCommand", action))
                g_print(" %s", g_locale_from_utf8(execact,-1,0,0,0));
            g_print("\n");
#endif
        }
    }
    fclose(file);
    free(initpath);
}


/* void addkey(GtkButton * but, GtkWidget ** boites) */
void addkey(GtkButton * button, GtkWidget ** boites)
{
    button = button;
    if (nblignes > MAXKEYCOUNT - 2)
        return;
    nblignes++;
    boites[nblignes] = newkey(MOD_NONE, actlist, "", "", "");
    gtk_box_pack_start(GTK_BOX(mainvbox), boites[nblignes], FALSE, FALSE, 0);
    gtk_widget_show_all(boites[nblignes]);
}


/* 
 * firstword() : returns the first word of a string, skipping non-printable
 * chars at the beginning.
 * returns an allocated pointer.
 */
char *firstword(char *word, int *offset)
{
    int start, end;
    char *final;

    for (start = 0; word[start] < 33; start++); /* while we don't find a printable char, look next char */
    *offset = start;            /* update the offset */
    for (end = start + 1; word[end] > 32; end++);   /* while current char is > 32, get next char */
    final = malloc(end - start + 1);    /* allocate memory for the string plus the ending 0 */
    memset(final, 0, end - start + 1);
    strncpy(final, word + start, end - start);  /* copy the word into final */
    return final;
}

char *getthings(char *word, int *mod, char *thekey)
{
    char *rest;
    int x, max = 1, seek = 0;

    *mod = 0;
    rest = malloc(KEYLEN);
    memset(thekey, 0, 10);

    for (max = 0; word[max] != ':'; max++); /* max is the last "before action" parameter */
    while (seek < max) {
        rest = firstword(word + seek, &x);
        /*
         * g_print("(%s)[%d]\n",rest,seek); 
         */
        seek += x;
        seek += strlen(rest);   /* FIXME?: isn't there a way to avoid repeating this strlen() call n times ? */
        if (!strcasecmp(rest, "Control"))
            *mod |= MOD_CTRL;
        else if (!strcasecmp(rest, "Mod1"))
            *mod |= MOD_ALT;
        else if (!strcasecmp(rest, "Shift"))
            *mod |= MOD_SHIFT;
        else if (!strcasecmp(rest, "Mod4"))
            *mod |= MOD_WIN;
	else if (!strcasecmp(rest, "OnDesktop"))
	    *mod |= MOD_ONDESKTOP;
        else if (!strcasecmp(rest, "None"));
        else if (seek < max) {
            strncpy(thekey, rest, strlen(rest));    /* FIXME: check boundaries. thekey can contain at most 10 bytes */
        }
    }
    free(rest);
    return word + max + 1;      /* FIXME: wow, we are returning a previously
                                 * allocated pointer plus some random value.
                                 * Freeing the resulting pointer might hurt badly.
                                 */
}
