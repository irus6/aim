dnl Process this file with autoconf to produce a configure script.

AC_INIT(aim, 0.1, https://github.com/cogniti/aim/issues)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror])
AM_MAINTAINER_MODE([disable])
AM_SILENT_RULES([yes])

AM_PROG_AR
AC_PROG_CC
AC_PROG_CXX

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************

IT_PROG_INTLTOOL(0.35.0)
GETTEXT_PACKAGE=aim
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [gettext package name])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR(AIM_LOCALE_DIR)

LT_INIT([disable-static])

LIBAIM_REQUIRES="glib-2.0 gio-2.0 gio-unix-2.0 gmodule-2.0 gobject-introspection-1.0 x11"
LIBAIM_PRIVATE="gtk+-3.0"
AC_SUBST(LIBAIM_REQUIRES)
AC_SUBST(LIBAIM_PRIVATE)

PKG_CHECK_MODULES(LIBAIM_DEPS, [$LIBAIM_REQUIRES $LIBAIM_PRIVATE])
PKG_CHECK_MODULES(GLIB, [glib-2.0])
PKG_CHECK_MODULES(GIO, [gio-2.0])
PKG_CHECK_MODULES(GOBJECT, [gobject-2.0])
PKG_CHECK_MODULES(GTK3, [gtk+-3.0])
PKG_CHECK_MODULES(GTK2, [gtk+-2.0])
PKG_CHECK_MODULES(LIBHANGUL, libhangul >= 0.0.12)
PKG_CHECK_MODULES(X11, [x11])

dnl ***************************************************************************
dnl aim-sunpinyin
dnl ***************************************************************************

PKG_CHECK_MODULES(AIM_SUNPINYIN_DEPS, [sunpinyin-2.0 $LIBAIM_REQUIRES])

dnl ***************************************************************************
dnl  Qt
dnl ***************************************************************************

PKG_CHECK_MODULES(QT4_GUI, [QtGui])
PKG_CHECK_MODULES(QT5_WIDGETS, [Qt5Widgets])
PKG_CHECK_MODULES(QT5_PLATFORM_SUPPORT, [Qt5PlatformSupport])

AC_PATH_PROGS([QT5_MOC], [moc-qt5 moc], [no],
              [`pkg-config --variable=host_bins Qt5Core`])
if test "x$QT5_MOC" = "xno"; then
  AC_MSG_ERROR([moc for qt5 not found])
fi

QT5_LIB_DIR=`pkg-config --variable=libdir Qt5PlatformSupport`

case `pkg-config --variable=host_bins Qt5Core` in
  */qt5/*)
    QT5_IM_MODULE_DIR=$QT5_LIB_DIR/qt5/plugins/platforminputcontexts
    ;;
  */qt/*)
    QT5_IM_MODULE_DIR=$QT5_LIB_DIR/qt/plugins/platforminputcontexts
    ;;
  *)
    AC_MSG_ERROR(Can not determine Qt5 plugins/platforminputcontexts directory)
    ;;
esac

AC_SUBST(QT5_IM_MODULE_DIR)

dnl ***************************************************************************
dnl  GTK+
dnl ***************************************************************************

case `uname -m` in
  x86_64)
    BITS=64
    ;;
  *)
    BITS=32
    ;;
esac

GTK3_LIBDIR=`pkg-config --variable=libdir gtk+-3.0`
GTK3_BINARY_VERSION=`pkg-config --variable=gtk_binary_version gtk+-3.0`
AC_SUBST(GTK3_LIBDIR)
AC_SUBST(GTK3_BINARY_VERSION)
AC_PATH_PROGS([GTK_QUERY_IMMODULES3],
              [gtk-query-immodules-3.0 gtk-query-immodules-3.0-$BITS], [no],
              [/usr/bin:$GTK3_LIBDIR/libgtk-3-0])
if test "x$GTK_QUERY_IMMODULES3" = "xno"; then
  AC_MSG_ERROR([gtk-query-immodules-3.0 or gtk-query-immodules-3.0-$BITS not found])
fi

GTK2_LIBDIR=`pkg-config --variable=libdir gtk+-2.0`
GTK2_BINARY_VERSION=`pkg-config --variable=gtk_binary_version gtk+-2.0`
AC_SUBST(GTK2_LIBDIR)
AC_SUBST(GTK2_BINARY_VERSION)
AC_PATH_PROGS([GTK_QUERY_IMMODULES2],
              [gtk-query-immodules-2.0 gtk-query-immodules-2.0-$BITS], [no],
              [/usr/bin:$GTK2_LIBDIR/libgtk2.0-0])
if test "x$GTK_QUERY_IMMODULES2" = "xno"; then
  AC_MSG_ERROR([gtk-query-immodules-2.0 or gtk-query-immodules-2.0-$BITS not found])
fi

dnl ***************************************************************************
dnl  icon cache
dnl ***************************************************************************

AC_PATH_PROGS([GTK_UPDATE_ICON_CACHE],
              [gtk-update-icon-cache-3.0 gtk-update-icon-cache], [no],
              [/usr/bin:$GTK3_LIBDIR/libgtk-3-0:$GTK2_LIBDIR/libgtk2.0-0])
if test "x$GTK_UPDATE_ICON_CACHE" = "xno"; then
  AC_MSG_ERROR([gtk-update-icon-cache-3.0 or gtk-update-icon-cache not found])
fi

AC_PATH_PROGS([RSVG_CONVERT], [rsvg-convert rsvg], [no])
if test "x$RSVG_CONVERT" = "xno"; then
  AC_MSG_ERROR([rsvg-convert or rsvg is not found, please install librsvg2-bin])
fi

dnl ***************************************************************************
dnl Build aim-indicator by default
dnl ***************************************************************************

AC_ARG_ENABLE([aim-indicator],
  AS_HELP_STRING([--disable-aim-indicator], [Disable aim-indicator]))

AS_IF([test "x$enable_aim_indicator" != "xno"],
  [PKG_CHECK_MODULES(APPINDICATOR3, [appindicator3-0.1])])

AM_CONDITIONAL([ENABLE_AIM_INDICATOR],
               [test "$enable_aim_indicator" != "no"])

GLIB_GSETTINGS
GOBJECT_INTROSPECTION_CHECK([0.10.1])

AC_OUTPUT([
  Makefile
  daemon/Makefile
  data/Makefile
  data/im-config/Makefile
  data/icons/Makefile
  indicator/Makefile
  libaim/Makefile
  libaim/aim.pc
  modules/Makefile
  modules/clients/Makefile
  modules/clients/gtk/Makefile
  modules/clients/qt4/Makefile
  modules/clients/qt5/Makefile
  modules/engines/Makefile
  modules/engines/aim-libhangul/Makefile
  modules/engines/aim-sunpinyin/Makefile
  po/Makefile.in
])
