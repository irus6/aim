moduledir = $(libdir)/aim/modules
module_LTLIBRARIES = aim-sunpinyin.la

gsettings_SCHEMAS = org.aim.engines.sunpinyin.gschema.xml
@GSETTINGS_RULES@

aim_sunpinyin_la_SOURCES = aim-sunpinyin.cpp
aim_sunpinyin_la_CXXFLAGS  = \
	-Wall -Werror \
	-I$(top_srcdir)/libaim \
	-DG_LOG_DOMAIN=\"aim\" \
	$(AIM_SUNPINYIN_DEPS_CFLAGS)

aim_sunpinyin_la_LDFLAGS = -avoid-version -module $(AIM_SUNPINYIN_DEPS_LIBS)
aim_sunpinyin_la_LIBADD  = $(top_builddir)/libaim/libaim.la

install-data-hook:
	chmod -x $(DESTDIR)$(moduledir)/aim-sunpinyin.so
	rm    -f $(DESTDIR)$(moduledir)/aim-sunpinyin.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(moduledir)/aim-sunpinyin.so
	-rmdir -p $(DESTDIR)$(moduledir)

DISTCLEANFILES = Makefile.in
