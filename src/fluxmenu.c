/*
 * $Id: fluxmenu.c,v 1.4 2004/12/10 14:47:14 babar Exp $
 * fluxmenu.c for fluxconf
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
#include <stdlib.h>              /* gentenv */
#include <stdio.h>
#include <fcntl.h>
#include <string.h>              /* strcat, strdup */
#include <strings.h>		 /* strcasecmp */
#include <errno.h>
#include <libintl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <gtk/gtk.h>

#include "globals.h"            /* actions for the Keys file */
#include "defines.h"

#include "structures.h"		/* where program structures are defined   */
#include "functions.h"		/* where function prototypes can be found */

#ifdef GTK2

GtkTreeStore *store;
GtkTreeIter dad[MAXMENUDEPTH];

void parseline(char *in, char **one, char **two, char **three, char **four)
{
    int n, d, alloc;
    char *s1, *s2, *s3, *s4;

    s1 = malloc(sizeof(char));
    s2 = malloc(sizeof(char));
    s3 = malloc(sizeof(char));
    s4 = malloc(sizeof(char));
    *s1 = 0;
    *s2 = 0;
    *s3 = 0;
    *s4 = 0;
    
    for (n = 0; in[n]; n++) {
        if (in[n] == '[') {
            n++;
            d = 0;
            alloc = 10;
            s1 = realloc(s1, alloc * sizeof(char));
            while (in[n] != ']' && in[n]) {
                s1[d] = in[n];
                d++;
                n++;
                if (d >= alloc) {
                    alloc *= 2;
                    s1 = realloc(s1, alloc * sizeof(char));
                }
            }
            s1[d] = 0;
        }
        if (in[n] == '(') {
            n++;
            d = 0;
            alloc = 10;
            s2 = realloc(s2, alloc * sizeof(char));
            while (in[n] != ')' && in[n]) {
                s2[d] = in[n];
                d++;
                n++;
                if (d >= alloc) {
                    alloc *= 2;
                    s2 = realloc(s2, alloc * sizeof(char));
                }
            }
            s2[d] = 0;
        }
        if (in[n] == '{') {
            n++;
            d = 0;
            alloc = 10;
            s3 = realloc(s3, alloc * sizeof(char));
            while (in[n] != '}' && in[n]) {
                s3[d] = in[n];
                d++;
                n++;
                if (d >= alloc) {
                    alloc *= 2;
                    s3 = realloc(s3, alloc * sizeof(char));
                }
            }
            s3[d] = 0;
        }
        if (in[n] == '<') {
            n++;
            d = 0;
            alloc = 10;
            s4 = realloc(s4, alloc * sizeof(char));
            while (in[n] != '>' && in[n]) {
                s4[d] = in[n];
                d++;
                n++;
                if (d >= alloc) {
                    alloc *= 2;
                    s4 = realloc(s4, alloc * sizeof(char));
                }
            }
            s4[d] = 0;
        }
    }
    *one = s1;
    *two = s2;
    *three = s3;
    *four = s4;
}

/* void supprow(GtkWidget * widget, gpointer data) */
void supprow(GtkButton * button, GtkWidget *tree)
{
    GtkTreeIter iter,tmpiter;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));

    button = NULL; /* FIXME without this gcc hangs */
    if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
        if(gtk_tree_model_iter_parent(GTK_TREE_MODEL(store),&tmpiter,&iter)) /* if it isn't the root ... */
        { gtk_tree_store_remove(store, &iter);
        } else {
            g_print("Hey dude ! Don't try to delete the root node !\n");
        }
    }
}

void addrow(GtkButton * button, GtkWidget *tree)
{
    GtkTreeIter iter, iter2, nouvo, zesub;
    char *qqchose;

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));

    button = NULL; /* FIXME without this gcc hangs */
    if (gtk_tree_selection_get_selected(selection, NULL, &iter2)) {
        if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(store), &iter, &iter2)) {
            gtk_tree_store_insert_after(store, &nouvo, &iter, &iter2);
            gtk_tree_model_get(GTK_TREE_MODEL(store), &iter2, TYPE, &qqchose, -1);  /* Do something with the data */
            gtk_tree_store_set(store, &nouvo, TYPE, qqchose, TITRE, "", VALEUR, "", EDITABLE, TRUE,
                               -1);
            if (!strcmp(qqchose, "submenu")) {
                gtk_tree_store_append(store, &zesub, &nouvo);
                gtk_tree_store_set(store, &zesub, TYPE, "exec", TITRE, "", VALEUR, "", EDITABLE,
                                   TRUE, -1);
            }
        }
        else {
            gtk_tree_model_get(GTK_TREE_MODEL(store), &iter2, TYPE, &qqchose, -1);  /* Do something with the data */
            gtk_tree_store_prepend(store, &zesub, &iter2);
            gtk_tree_store_set(store, &zesub, TYPE, "submenu", TITRE, "", VALEUR, "", EDITABLE, TRUE, -1);
            gtk_tree_store_append(store,&nouvo,&zesub);
            gtk_tree_store_set(store, &nouvo, TYPE, "exec", TITRE, "", VALEUR, "", EDITABLE, TRUE, -1);
        }
    } else {
        gtk_tree_store_append(store, &iter, &dad[0]);
        gtk_tree_store_set(store, &iter, TYPE, "exec", TITRE, "", VALEUR, "", EDITABLE, TRUE, -1);
    }
}

void addexec(GtkButton * button, GtkWidget *tree)
{
    GtkTreeIter iter, iter2, nouvo;
    char *qqchose;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    if (selection == NULL)
        g_print("ça sux\n");

    button = NULL; /* FIXME without this gcc hangs */
    if (gtk_tree_selection_get_selected(selection, NULL, &iter2)) {
        if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(store), &iter, &iter2)) {
            gtk_tree_model_get(GTK_TREE_MODEL(store), &iter2, TYPE, &qqchose, -1);  /* Do something with the data */
            if (!strcmp(qqchose, "submenu"))
                gtk_tree_store_prepend(store, &nouvo, &iter2);
            else
                gtk_tree_store_insert_after(store, &nouvo, &iter, &iter2);
        }
        else {
                gtk_tree_store_prepend(store, &nouvo, &iter2);
        }
    } else
        gtk_tree_store_append(store, &nouvo, &dad[0]);
    gtk_tree_store_set(store, &nouvo, TYPE, "exec", TITRE, "", VALEUR, "", EDITABLE, TRUE, -1);

}

void addsub(GtkButton * button, GtkWidget *tree)
{
    GtkTreeIter iter, iter2, nouvfold, nouvfils;
    char *qqchose;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));

    button = NULL; /* FIXME without this gcc hangs */
    if (gtk_tree_selection_get_selected(selection, NULL, &iter2)) {
        if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(store), &iter, &iter2)) {
            gtk_tree_model_get(GTK_TREE_MODEL(store), &iter2, TYPE, &qqchose, -1);  /* Do something with the data */
            if (!strcmp(qqchose, "submenu"))
                gtk_tree_store_append(store, &nouvfold, &iter2);
            else
                gtk_tree_store_insert_after(store, &nouvfold, &iter, &iter2);
        }
        else { /* if root is selectioned */
            gtk_tree_model_get(GTK_TREE_MODEL(store), &iter2, TYPE, &qqchose, -1);  /* Do something with the data */
            gtk_tree_store_prepend(store, &nouvfold, &iter2);
        }
            gtk_tree_store_append(store, &nouvfils, &nouvfold);
    } else {
        gtk_tree_store_append(store, &nouvfold, &dad[0]);
        gtk_tree_store_append(store, &nouvfils, &nouvfold);
    }
    gtk_tree_store_set(store, &nouvfold, TYPE, "submenu", TITRE, "", VALEUR, "", EDITABLE, TRUE, -1);
    gtk_tree_store_set(store, &nouvfils, TYPE, "exec", TITRE, "", VALEUR, "", EDITABLE, TRUE, -1);
}

/* void cell_edited(GtkCellRendererText * cell, gchar * path_string, gchar * new_text, gpointer data) */
void cell_edited(GtkCellRendererText * cell, gchar * path_string, gchar * new_text)
{
    GtkTreeIter iter;
    gint *column;
    GtkTreePath *path = gtk_tree_path_new_from_string(path_string);

    column = g_object_get_data(G_OBJECT(cell), "column");
    gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path);
    gtk_tree_store_set(store, &iter, column, g_strdup(new_text), -1);
}

GtkWidget *fillmenu(void)
{
    char *buf, *initpath, *letitre, *letype, *lecmd, *icone;
    FILE *file;
    int level = 1;
    unsigned int n;
    char ok,*homepath;
    GtkCellRenderer *renderer;
    GtkTreePath *path;
    GtkTooltips *tips;
    GtkWidget *tree;
    GtkTreeViewColumn *treeColumn;

    tips = gtk_tooltips_new();

    initpath = malloc(MAXPATHLEN);
    buf = malloc(MAXPATHLEN);

    homepath=getenv("HOME");
    if(!homepath) {
        g_print("Huh ?? you *NEED* the $HOME environment !\n");
        exit(-1);
    }
    snprintf(initpath, MAXPATHLEN, "%s/.fluxbox/init",homepath);
    file = fopen(initpath, "r");
    if(file==NULL) {
        g_print(_("Can't open %s\n"), initpath);
        g_print(_("Make sure you installed fluxbox with this user.\n"));
        exit(1);
    }
    while (fgets(buf, 200, file)) {
        if (!strncasecmp(buf, "session.menuFile:", strlen("session.menuFile:"))) {
            for (n = strlen("session.menuFile:") + 1; buf[n] < 33; n++);
            buf[strlen(buf) - 1] = 0;
            if( (buf+n)[0] == '~' )
                snprintf(initpath, MAXPATHLEN, "%s/%s", homepath,buf+n+2);
            else
                snprintf(initpath, MAXPATHLEN, "%s", buf+n);
        }
    }
    fclose(file);
    initpath = expandFilename(initpath);
    file = fopen(initpath, "r");
    if (file == NULL) {
        g_print("Can't open %s : %s\n", initpath,strerror(errno));
        g_print(ERRFLUXMENU);
        exit(-1);
    }

    store = gtk_tree_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);

    while (fgets(buf, 200, file)) {
        ok = 0;
        for (n = 0; buf[n] < 33 && n < strlen(buf); n++);   /* go to the first real char */

        if (*buf && buf[n] == '[') {
            /*
             * Regarder le type de la ligne, ajouter en fonction, dad contient le niveau du fils avec l'historique... 
             */
            parseline(buf, &letype, &letitre, &lecmd, &icone);
            if (!strcmp(letype, "begin")) {
                gtk_tree_store_append(store, &dad[0], NULL);    /* Acquire an iterator */
                gtk_tree_store_set(store, &dad[0], TYPE, g_locale_to_utf8(letype,-1,0,0,0), TITRE, g_locale_to_utf8(letitre,-1,0,0,0), VALEUR, "", ICONE, g_locale_to_utf8(icone,-1,0,0,0), EDITABLE, TRUE, -1);
            } else if (!strcmp(letype, "end")) {
                level--;
            } else if (!strcmp(letype, "submenu")) {
                gtk_tree_store_append(store, &dad[level], &dad[level - 1]);
                gtk_tree_store_set(store, &dad[level], TYPE, g_locale_to_utf8(letype,-1,0,0,0), TITRE, g_locale_to_utf8(letitre,-1,0,0,0), VALEUR, g_locale_to_utf8(lecmd,-1,0,0,0), ICONE, g_locale_to_utf8(icone,-1,0,0,0), EDITABLE, TRUE, -1);
                level++;
            } else {
                gtk_tree_store_append(store, &dad[level], &dad[level - 1]);
                gtk_tree_store_set(store, &dad[level], TYPE, g_locale_to_utf8(letype,-1,0,0,0), TITRE, g_locale_to_utf8(letitre,-1,0,0,0), VALEUR, g_locale_to_utf8(lecmd,-1,0,0,0), ICONE, g_locale_to_utf8(icone,-1,0,0,0), EDITABLE, TRUE, -1);
            }
        }
    }

    tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_view_set_reorderable(GTK_TREE_VIEW(tree), TRUE);
    path = gtk_tree_path_new_first();
    gtk_tree_view_expand_row(GTK_TREE_VIEW(tree), path, FALSE);
    gtk_tree_path_free(path);

/* type column */
    renderer = gtk_cell_renderer_text_new();
    gtk_signal_connect(GTK_OBJECT(renderer), "edited", GTK_SIGNAL_FUNC(cell_edited), NULL);
    g_object_set_data(G_OBJECT(renderer), "column", (gint *) TYPE);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree),
                                                -1, "Type", renderer,
                                                "text", TYPE, "editable", EDITABLE, NULL);
    treeColumn = gtk_tree_view_get_column(GTK_TREE_VIEW(tree), TYPE);
    gtk_tree_view_column_set_resizable(treeColumn, TRUE);
/* Caption column */
    renderer = gtk_cell_renderer_text_new();
    gtk_signal_connect(GTK_OBJECT(renderer), "edited", GTK_SIGNAL_FUNC(cell_edited), NULL);
    g_object_set_data(G_OBJECT(renderer), "column", (gint *) TITRE);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree),
                                                -1, "Title", renderer,
                                                "text", TITRE, "editable", EDITABLE, NULL);
    treeColumn = gtk_tree_view_get_column(GTK_TREE_VIEW(tree), TITRE);
    gtk_tree_view_column_set_resizable(treeColumn, TRUE);
/* Value column */
    renderer = gtk_cell_renderer_text_new();
    gtk_signal_connect(GTK_OBJECT(renderer), "edited", GTK_SIGNAL_FUNC(cell_edited), NULL);
    g_object_set_data(G_OBJECT(renderer), "column", (gint *) VALEUR);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree),
                                                -1, "Command/Comment", renderer,
                                                "text", VALEUR, "editable", EDITABLE, NULL);
    treeColumn = gtk_tree_view_get_column(GTK_TREE_VIEW(tree), VALEUR);
    gtk_tree_view_column_set_resizable(treeColumn, TRUE);
/* Icon column */
    renderer = gtk_cell_renderer_text_new();
    gtk_signal_connect(GTK_OBJECT(renderer), "edited", GTK_SIGNAL_FUNC(cell_edited), NULL);
    g_object_set_data(G_OBJECT(renderer), "column", (gint *) ICONE);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree),
                                                -1, "Icon path", renderer,
                                                "text", ICONE, "editable", EDITABLE, NULL);
    treeColumn = gtk_tree_view_get_column(GTK_TREE_VIEW(tree), ICONE);
    gtk_tree_view_column_set_resizable(treeColumn, TRUE);

    gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(tree)), GTK_SELECTION_SINGLE);
    
    gtk_tooltips_set_tip(tips, GTK_WIDGET(tree),
                         _("Choices for Type are :\nnop\nexec\nexit\nstyle\nconfig\nsubmenu\nrestart\nreconfig\nstylesdir / stylesmenu\nworkspaces"),
                         NULL);
    if (tree == NULL)
        g_print("ça sux déjà\n");
    return tree;
}

void analyseit(GtkTreeIter iter, int indent, FILE * fd)
{
    char *pre, *deuz, *troiz, *quatr;
    int i;

    gboolean valid = TRUE;
    GtkTreeIter child;

    while (valid) {
        gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, TYPE, &pre, TITRE, &deuz, VALEUR, &troiz, ICONE, &quatr, -1); /* Do something with the data */
        for (i = 0; i < indent; i++)
            putc(' ', fd);
        pre = g_locale_from_utf8(pre,-1,0,0,0);
        deuz = g_locale_from_utf8(deuz,-1,0,0,0);
        troiz = g_locale_from_utf8(troiz,-1,0,0,0);
        quatr = g_locale_from_utf8(quatr,-1,0,0,0);
        fprintf(fd, "%s%s%s %s%s%s %s%s%s %s%s%s\n", 
                pre ? "[" : "", pre ? pre : "", pre ? "]" : "",
                deuz ? "(" : "", deuz ? deuz : "", deuz ? ")" : "", 
                troiz ? "{" : "", troiz ? troiz : "", troiz ? "}" : "",
                quatr ? "<" : "", quatr ? quatr : "", quatr ? ">" : "");
        g_free(pre);
        g_free(deuz);
        g_free(troiz);
        g_free(quatr);
        if (gtk_tree_model_iter_children(GTK_TREE_MODEL(store), &child, &iter)) {
            analyseit(child, indent + 4, fd);
            for (i = 0; i < indent; i++)
                putc(' ', fd);
            fprintf(fd, "[end]\n");
        }
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
    }
}

void savemenu(void)
{
    char *initpath, *buf;
    FILE *file;
    int n;
    initpath = malloc(sizeof(char) * 200);
    buf = malloc(sizeof(char)*200);

    sprintf(initpath, "%s/.fluxbox/init", getenv("HOME"));
    file = fopen(initpath, "r");
    if(file==NULL) {
        g_print(_("Can't open %s\n"), initpath);
        g_print(_("Make sure you installed fluxbox with this user.\n"));
        exit(1);
    }
    while (fgets(buf, 200, file)) {
        if (!strncasecmp(buf, "session.menuFile:", strlen("session.menuFile:"))) {
            for (n = strlen("session.menuFile:") + 1; buf[n] < 33; n++);
            buf[strlen(buf) - 1] = 0;
            strncpy(initpath, buf + n, 200);
        }
    }
    fclose(file);
    initpath = expandFilename(initpath);
    /*
    * try to make a backup
    */
    char tmp_buff[BUFSIZ+1],*ptr;
    int source_fd, dest_fd, rdlen;

    memset(tmp_buff,0,BUFSIZ+1);

    ptr=strrchr(initpath,'/');
    if(ptr && ptr-initpath<BUFSIZ) {
            strncpy(tmp_buff,initpath,ptr-initpath);
            snprintf(tmp_buff,BUFSIZ,"%s/menu-FCONFBKP.old",tmp_buff);
            dest_fd = open(tmp_buff,O_WRONLY|O_TRUNC|O_CREAT,0666);
            source_fd = open(initpath,O_RDONLY);
            if( dest_fd == -1 || source_fd == -1 ) {
                    if(dest_fd<0) close(dest_fd);
                    if(source_fd<0) close(source_fd);
            } else {
                    while((rdlen=read(source_fd,tmp_buff,BUFSIZ))>0)
                            write(dest_fd,tmp_buff,rdlen);
                    close(source_fd);
                    close(dest_fd);
            }
    }
    file = fopen(initpath, "w");
    if (file == NULL) {
        g_print(_("Can't open %s for writing\n"), initpath);
        g_print(ERRFLUXMENU);
        exit(1);
    }

/*  sprintf(initpath, "%s/.fluxbox/menu", getenv("HOME"));
    file = fopen(initpath, "w");
    if (file == NULL) {
        g_print("Can't open %s\n", initpath);
        exit(1);
    }
    */
    analyseit(dad[0], 0, file);
    fclose(file);
}

int fluxmenu(int argc, char **argv, char standalone)
{
    GtkWidget *win, *but, *title, *sw, *mainvbox, *butbox, *hsplit, *tree;

    struct iterstuff {
        GtkTreeIter iter;
        struct iterstuff *suiv;
    } *laliste;
    laliste = malloc(sizeof(struct iterstuff *));
    gtk_init(&argc, &argv);
    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(win), "Fluxbox Configuration Tool");
    gtk_window_set_default_size(GTK_WINDOW(win), 320, 360);
    gtk_window_set_resizable(GTK_WINDOW(win), TRUE);

    title = gtk_label_new("<span size=\"18000\" weight=\"bold\">Fluxbox Configuration Tool</span>");
    gtk_label_set_use_markup(GTK_LABEL(title), TRUE);

    mainvbox = gtk_vbox_new(FALSE, 2);
    butbox = gtk_hbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(win), mainvbox);
    gtk_box_pack_start(GTK_BOX(mainvbox), title, FALSE, FALSE, 4);
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(mainvbox), sw, TRUE, TRUE, 4);
    tree = fillmenu();
    gtk_container_add(GTK_CONTAINER(sw), tree);

/**********************************************************************/

    hsplit = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(mainvbox), hsplit, FALSE, FALSE, 0); /* make it at end */
    gtk_box_pack_start(GTK_BOX(mainvbox), butbox, FALSE, FALSE, 4); /* make it at end */

    but = gtk_button_new_with_mnemonic(_("_Save"));
    gtk_box_pack_start(GTK_BOX(butbox), but, TRUE, TRUE, 2);
    gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(savemenu), NULL);

    but = gtk_button_new_with_mnemonic(_("_Add clever"));
    gtk_box_pack_start(GTK_BOX(butbox), but, TRUE, TRUE, 2);
    gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(addrow), tree);

    but = gtk_button_new_with_mnemonic(_("Add s_ub"));
    gtk_box_pack_start(GTK_BOX(butbox), but, TRUE, TRUE, 2);
    gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(addsub), tree);

    but = gtk_button_new_with_mnemonic(_("Add _exec"));
    gtk_box_pack_start(GTK_BOX(butbox), but, TRUE, TRUE, 2);
    gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(addexec), tree);

    but = gtk_button_new_with_mnemonic(_("_Del"));
    gtk_box_pack_start(GTK_BOX(butbox), but, TRUE, TRUE, 2);
    gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(supprow), tree);
if(standalone) {
    but = gtk_button_new_with_mnemonic(_("_Quit"));
    gtk_box_pack_start(GTK_BOX(butbox), but, FALSE, FALSE, 2);
    gtk_signal_connect(GTK_OBJECT(but), "clicked", GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
    gtk_button_set_relief(GTK_BUTTON(but), GTK_RELIEF_NONE);
}

/**********************************************************************/
    gtk_signal_connect(GTK_OBJECT(win), "destroy", GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
    gtk_widget_show_all(win);
    if(standalone)
        makesplash();
    gtk_main();
    return 0;
}

#endif /* GTK2 */
