moduledir = $(libdir)/aim/modules
module_LTLIBRARIES = aim-libhangul.la

gsettings_SCHEMAS = org.aim.engines.libhangul.gschema.xml
@GSETTINGS_RULES@

aim_libhangul_la_SOURCES = aim-libhangul.c
aim_libhangul_la_CFLAGS  = \
	-Wall -Werror \
	-I$(top_srcdir)/libaim \
	-DG_LOG_DOMAIN=\"aim\" \
	$(LIBAIM_DEPS_CFLAGS) \
	$(LIBHANGUL_CFLAGS)

aim_libhangul_la_LDFLAGS = -avoid-version -module \
	$(LIBAIM_DEPS_LIBS) $(LIBHANGUL_LIBS)
aim_libhangul_la_LIBADD  = $(top_builddir)/libaim/libaim.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/aim-libhangul.so
	rm    -f $(DESTDIR)$(moduledir)/aim-libhangul.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/aim-libhangul.so
	-rmdir -p $(DESTDIR)$(moduledir)
