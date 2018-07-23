/* $Id: defines.h,v 1.3 2004/12/02 17:40:57 babar Exp $ */
/* macro definitions */
#ifdef GTK2
#define _(String) g_locale_to_utf8(gettext (String),strlen(gettext(String)),0,0,0)
#else
#define _(String) gettext (String)
#endif

#define die(msg) {fprintf(stderr,"Error in '%s' (l. %d) : %s\n",__FILE__,__LINE__,msg);exit(-1);}


/* misc messages definitions */
#define ERRFLUXMENU _("\
Please, change the menu location with fluxconf to a writable one\n\
 (e.g ~/.fluxbox/fluxbox-menu).\n\
Fluxmenu will then be able to save your fluxbox menu configuration.\n")

#define ERRFLUXKEYS _("\
Unable to write the key file\n\
Make a copy of the original file to a place you can write\n\
Please, run 'fluxconf' to change the location of this file to one you can write.\n")


/* constant values */
#define NB 20                   /* number of items */
#define KEYLEN 30
#define ACTLEN 30
#define EXELEN 200

#define MAXKEYCOUNT 100
#define MAXLINELEN 1024
#define MAXMENUDEPTH 10

#define QCM_START 7             /* first TRUE/FALSE position */
#define QCM_STOP 10             /* first NON TRUE/FALSE position */
#define LIT_START 16

#define STANDALONE 1


#define MOD_NONE	0
#define MOD_CTRL	1
#define MOD_ALT		2
#define MOD_SHIFT	4
#define MOD_WIN		8
#define MOD_ONDESKTOP	16

/* enums */
#ifdef GTK2
enum {
    TYPE,
    TITRE,
    VALEUR,
    ICONE,
    EDITABLE,
    N_COLUMNS,
};
#endif
