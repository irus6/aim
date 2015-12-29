gsettings_SCHEMAS = org.aim.clients.gtk.gschema.xml
@GSETTINGS_RULES@

AM_CFLAGS = \
	-Wall -Werror \
	-I$(top_srcdir)/libaim \
	-DG_LOG_DOMAIN=\"aim\" \
	$(GLIB_CFLAGS)

# for gtk3

gtk3_im_moduledir = $(GTK3_LIBDIR)/gtk-3.0/$(GTK3_BINARY_VERSION)/immodules
gtk3_im_module_LTLIBRARIES = im-aim-gtk3.la

im_aim_gtk3_la_SOURCES  = im-aim.c
im_aim_gtk3_la_CFLAGS   = $(AM_CFLAGS) $(GTK3_CFLAGS)
im_aim_gtk3_la_LDFLAGS  = $(GTK3_LIBS) -avoid-version -module
im_aim_gtk3_la_LIBADD   = $(top_builddir)/libaim/libaim.la

# for gtk2

gtk2_im_moduledir = $(GTK2_LIBDIR)/gtk-2.0/$(GTK2_BINARY_VERSION)/immodules
gtk2_im_module_LTLIBRARIES = im-aim-gtk2.la

im_aim_gtk2_la_SOURCES  = im-aim.c
im_aim_gtk2_la_CFLAGS   = $(AM_CFLAGS) $(GTK2_CFLAGS)
im_aim_gtk2_la_LDFLAGS  = $(GTK2_LIBS) -avoid-version -module
im_aim_gtk2_la_LIBADD   = $(top_builddir)/libaim/libaim.la

DISTCLEANFILES = Makefile.in

install-data-hook:
	chmod -x $(DESTDIR)$(gtk3_im_moduledir)/im-aim-gtk3.so
	chmod -x $(DESTDIR)$(gtk2_im_moduledir)/im-aim-gtk2.so
	rm -f $(DESTDIR)$(gtk3_im_moduledir)/im-aim-gtk3.la
	rm -f $(DESTDIR)$(gtk2_im_moduledir)/im-aim-gtk2.la

uninstall-hook:
	rm -f $(DESTDIR)$(gtk3_im_moduledir)/im-aim-gtk3.so
	rm -f $(DESTDIR)$(gtk2_im_moduledir)/im-aim-gtk2.so
	$(GTK_QUERY_IMMODULES2) --update-cache
	$(GTK_QUERY_IMMODULES3) --update-cache
