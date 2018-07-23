/* $Id: structures.h,v 1.1.1.1 2004/02/02 19:45:30 fab Exp $
 * structures.h for fluxconf
 *
 * Started on  Mon 02 Feb 2004 04:16:30 PM CET manu
 * Last update Mon 02 Feb 2004 05:44:00 PM CET manu
 */

#ifndef _STRUCTURES_H
# define _STRUCTURES_H

struct s_programs {
	char * name;
	int (*start)(int argc, char *argv[], char standalone);
};

struct s_widgets {
    int val;
    char * value;
    int l;
    GtkWidget *wid;
};

#endif /* !_STRUCTURES_H */
