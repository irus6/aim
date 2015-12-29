bin_PROGRAMS = aim-daemon

aim_daemon_SOURCES = aim-daemon.c

aim_daemon_CFLAGS = \
	-Wall \
	-Werror \
	-I$(top_srcdir)/libaim \
	-DAIM_COMPILATION \
	-DG_LOG_DOMAIN=\"aim\" \
	-DAIM_MODULE_DIR=\"$(libdir)/aim/modules\" \
	$(LIBAIM_DEPS_CFLAGS)

aim_daemon_LDFLAGS = $(GOBJECT_LIBS)
aim_daemon_LDADD   = $(top_builddir)/libaim/libaim.la

DISTCLEANFILES = Makefile.in
