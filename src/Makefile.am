# $Id: Makefile.am,v 1.1.1.1 2004/02/02 19:45:27 fab Exp $
ALL_LANG= fr fi
SUBDIRS = 
bin_PROGRAMS=fluxconf
fluxconf_SOURCES=fluxconf.c fluxmenu.c fluxkeys.c fluxbare.c actions.c 
fluxconf_INCLUDES=include/globals.h include/defines.h include/functions.h xpm/dalogo.h xpm/fc_menu.xpm xpm/fc_keys.xpm xpm/fc_conf.xpm
fluxconf_CFLAGS=-Iinclude/ -Ixpm/ -ggdb -D_GNU_SOURCE -Werror

#noinst_PROGRAMS=fluxkeys
#fluxconf_LDFLAGS=`$(GTKCONF) --libs --cflags`
#CFLAGS=`$(GTKCONF) --cflags`
#fluxkeys_SOURCES=
#fluxkeys: fluxconf
#	ln -sf fluxconf fluxkeys
install-exec-hook:
	@ln -sf $(bindir)/fluxconf $(bindir)/fluxkeys
	@ln -sf $(bindir)/fluxconf $(bindir)/fluxbare
	@if ldd fluxconf |grep pango > /dev/null ; then ln -sf $(bindir)/fluxconf $(bindir)/fluxmenu; fi
	@for lan in ${ALL_LANG} ; do \
		${mkinstalldirs} $(datadir)/locale/$$lan/LC_MESSAGES; \
		${INSTALL} -m444 $(top_srcdir)/po/$$lan.mo $(datadir)/locale/$$lan/LC_MESSAGES/fluxconf.mo ;\
	done ;

uninstall-local:
	rm -f $(bindir)/fluxkeys
	rm -f $(bindir)/fluxmenu
	rm -f $(bindir)/fluxbare
	for lan in ${ALL_LANG} ; do \
		rm -f $(datadir)/locale/$$lan/LC_MESSAGES/fluxconf.mo ;\
	done ;

clean:
	rm -f $(fluxconf_SOURCES:%.c=fluxconf-%.o)
	rm -f fluxconf fluxmenu fluxkeys fluxbare
