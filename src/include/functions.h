/*
 * $Id: functions.h,v 1.3 2004/12/08 18:07:14 babar Exp $
 * functions.h for (self)
 *
 * Made by manu
 * Login   <manu@insia.org>
 * 
 * Started on  Mon 02 Feb 2004 04:43:04 PM CET manu
 * Last update Mon 02 Feb 2004 06:22:05 PM CET manu
 */

#ifndef _FUNCTIONS_H
# define _FUNCTIONS_H

int fluxconf(int argc, char *argv[], char standalone);
int fluxkeys(int argc, char *argv[], char standalone);
int fluxmenu(int argc, char *argv[], char standalone);
int fluxbare(int argc, char *argv[], char standalone);

char * expandFilename(char * filename) ;
void makesplash(void);
void checkptr(void *);
char * getthings(char * word, int * mod, char * thekey);
char * firstword(char * word, int * offset);

void savekeys(GtkButton *button, GtkBox ** boxes);
void addkey(GtkButton * button, GtkWidget ** boxes);
/* void addkey(GtkWidget ** boxes); */
GtkWidget * newkey(int modifiers, 
                    GList * liste, char *value, char *touche, char *execcmd
                    );

int show(GtkWidget * wid);
int hide(GtkWidget * wid);
void makeitems(char screen);
void program_run(int argc, char *argv[], char standalone);
char screenparse(int argc, char *argv[]);
void usage(char const *progname, int errorcode);
#endif /* !_FUNCTIONS_H */

