QT4_LIB_DIR = `pkg-config --variable=libdir       QtCore`
QT4_MOC     = `pkg-config --variable=moc_location QtCore`

qt4_im_moduledir = $(QT4_LIB_DIR)/qt4/plugins/inputmethods
qt4_im_module_LTLIBRARIES = libqt4im-aim.la

BUILT_SOURCES = im-aim-qt4.moc
libqt4im_aim_la_SOURCES = im-aim-qt4.cpp $(BUILT_SOURCES)

libqt4im_aim_la_CXXFLAGS = \
	-Wall \
	-Werror \
	$(LIBAIM_DEPS_CFLAGS) \
	-I$(top_srcdir)/libaim \
	-DG_LOG_DOMAIN=\"aim\" \
	$(QT4_GUI_CFLAGS) \
	-DQT_NO_KEYWORDS

libqt4im_aim_la_LDFLAGS = -avoid-version -module $(QT4_GUI_LIBS) \
	                        $(LIBAIM_DEPS_LIBS)
libqt4im_aim_la_LIBADD  = $(top_builddir)/libaim/libaim.la

im-aim-qt4.moc: im-aim-qt4.cpp
	$(AM_V_GEN) $(QT4_MOC) im-aim-qt4.cpp -o im-aim-qt4.moc

install-data-hook:
	chmod -x $(DESTDIR)$(qt4_im_moduledir)/libqt4im-aim.so
	rm -f    $(DESTDIR)$(qt4_im_moduledir)/libqt4im-aim.la

uninstall-hook:
	rm -f    $(DESTDIR)$(qt4_im_moduledir)/libqt4im-aim.so

DISTCLEANFILES = Makefile.in $(BUILT_SOURCES)
