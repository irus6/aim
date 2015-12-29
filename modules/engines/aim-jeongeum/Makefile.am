moduledir = $(libdir)/aim/modules
module_LTLIBRARIES = aim-jeongeum.la

gsettings_SCHEMAS = org.aim.engines.jeongeum.gschema.xml
@GSETTINGS_RULES@

aim_jeongeum_la_SOURCES = aim-jeongeum.c
aim_jeongeum_la_CFLAGS  = \
	-Wall -Werror \
	-I$(top_srcdir)/libaim \
	-DG_LOG_DOMAIN=\"aim\" \
	$(LIBAIM_DEPS_CFLAGS) \
	$(LIBHANGUL_CFLAGS)

aim_jeongeum_la_LDFLAGS = -avoid-version -module \
	$(LIBAIM_DEPS_LIBS) $(LIBHANGUL_LIBS)
aim_jeongeum_la_LIBADD  = $(top_builddir)/libaim/libaim.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/aim-jeongeum.so
	rm    -f $(DESTDIR)$(moduledir)/aim-jeongeum.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/aim-jeongeum.so
	-rmdir -p $(DESTDIR)$(moduledir)
