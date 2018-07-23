SUBDIRS = src docs po
DIST_SUBDIRS = src docs po
EXTRA_DIST = src/include src/xpm

dist-hook:
	-chmod -R a+r $(distdir)
	rm -rf `find $(distdir)/src/include -name CVS`
	rm -rf `find $(distdir)/src/xpm -name CVS`

distclean: clean
	rm -fr config.status
	rm -fr config.log
