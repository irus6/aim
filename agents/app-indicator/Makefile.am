bin_PROGRAMS = aim-indicator

aim_indicator_SOURCES = aim-indicator.c

aim_indicator_CFLAGS = \
	-Wall \
	-Werror \
	-I$(top_srcdir)/libaim \
	-DAIM_COMPILATION \
	-DG_LOG_DOMAIN=\"aim\" \
	$(GLIB_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(X11_CFLAGS) \
	$(GTK3_CFLAGS) \
	$(APPINDICATOR3_CFLAGS)

aim_indicator_LDADD = $(top_builddir)/libaim/libaim.la

aim_indicator_LDFLAGS = \
	$(GLIB_LIBS) \
	$(GIO_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(X11_LIBS) \
	$(GTK3_LIBS) \
	$(APPINDICATOR3_LIBS)

DISTCLEANFILES = Makefile.in
