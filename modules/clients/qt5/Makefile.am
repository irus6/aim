QT5_GUI_VERSION      = `pkg-config --modversion          Qt5Gui`
QT5_GUI_INCLUDE_DIR  = `pkg-config --variable=includedir Qt5Gui`
QT5_GUI_INCLUDE_PATH = $(QT5_GUI_INCLUDE_DIR)/QtGui/$(QT5_GUI_VERSION)/QtGui

qt5_im_moduledir = $(QT5_IM_MODULE_DIR)
qt5_im_module_LTLIBRARIES = libqt5im-aim.la

BUILT_SOURCES = im-aim-qt5.moc
libqt5im_aim_la_SOURCES = im-aim-qt5.cpp $(BUILT_SOURCES)

libqt5im_aim_la_CXXFLAGS = \
	-Wall \
	-Werror \
	$(LIBAIM_DEPS_CFLAGS) \
	-I$(top_srcdir)/libaim \
	-DG_LOG_DOMAIN=\"aim\" \
	$(QT5_PLATFORM_SUPPORT_CFLAGS) \
	-I$(QT5_GUI_INCLUDE_PATH) \
	-DQT_NO_KEYWORDS

libqt5im_aim_la_LDFLAGS = -avoid-version -module \
	                        $(QT5_WIDGETS_LIBS) \
	                        $(QT5_PLATFORM_SUPPORT_LIBS) \
	                        $(LIBAIM_DEPS_LIBS)
libqt5im_aim_la_LIBADD  = $(top_builddir)/libaim/libaim.la

im-aim-qt5.moc: im-aim-qt5.cpp
	$(AM_V_GEN) $(QT5_MOC) -I $(QT5_GUI_INCLUDE_PATH) im-aim-qt5.cpp \
	                       -o im-aim-qt5.moc

install-data-hook:
	chmod -x $(DESTDIR)$(qt5_im_moduledir)/libqt5im-aim.so
	rm -f    $(DESTDIR)$(qt5_im_moduledir)/libqt5im-aim.la

uninstall-hook:
	 rm    -f $(DESTDIR)$(qt5_im_moduledir)/libqt5im-aim.so
	-rmdir -p $(DESTDIR)$(qt5_im_moduledir)

DISTCLEANFILES = Makefile.in $(BUILT_SOURCES)
