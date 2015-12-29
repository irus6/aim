lib_LTLIBRARIES = libaim.la

BUILT_SOURCES =      \
	aim-enum-types.h \
	aim-enum-types.c \
	aim-marshalers.h \
	aim-marshalers.c \
	$(NULL)

IMdkit_SOURCES = \
	IMdkit/FrameMgr.h \
	IMdkit/IMdkit.h \
	IMdkit/Xi18n.h \
	IMdkit/Xi18nX.h \
	IMdkit/XimFunc.h \
	IMdkit/XimProto.h \
	IMdkit/FrameMgr.c \
	IMdkit/i18nAttr.c \
	IMdkit/i18nClbk.c \
	IMdkit/i18nIc.c \
	IMdkit/i18nIMProto.c \
	IMdkit/i18nMethod.c \
	IMdkit/i18nOffsetCache.c \
	IMdkit/i18nPtHdr.c \
	IMdkit/i18nUtil.c \
	IMdkit/i18nX.c \
	IMdkit/IMConn.c \
	IMdkit/IMMethod.c \
	IMdkit/IMValues.c \
	$(NULL)

libaim_la_SOURCES = \
	aim.h \
	aim-private.h \
	aim-private.c \
	aim-im.c \
	aim-im.h \
	aim-types.c \
	aim-types.h \
	aim-events.c \
	aim-events.h \
	aim-engine.c \
	aim-engine.h \
	aim-english.c \
	aim-english.h \
	aim-module.h \
	aim-module.c \
	aim-module-manager.h \
	aim-module-manager.c \
	aim-key-syms.h \
	aim-message.h \
	aim-message.c \
	aim-candidate.h \
	aim-candidate.c \
	aim-agent.c \
	aim-agent.h \
	aim-connection.c \
	aim-connection.h \
	aim-server.c \
	aim-server.h \
	$(BUILT_SOURCES) \
	$(IMdkit_SOURCES) \
	$(NULL)

libaim_la_CFLAGS = \
	-Wall \
	-Werror \
	-I$(top_srcdir)/libaim \
	-DAIM_COMPILATION \
	-DG_LOG_DOMAIN=\"aim\" \
	-DAIM_MODULE_DIR=\"$(libdir)/aim/modules\" \
	$(LIBAIM_DEPS_CFLAGS)

libaim_la_LDFLAGS = $(LIBAIM_DEPS_LIBS) -avoid-version

aimincludedir = $(includedir)/aim
aiminclude_HEADERS = \
	aim.h \
	aim-agent.h \
	aim-candidate.h \
	aim-connection.h \
	aim-engine.h \
	aim-english.h \
	aim-enum-types.h \
	aim-events.h \
	aim-im.h \
	aim-key-syms.h \
	aim-message.h \
	aim-module-manager.h \
	aim-private.h \
	aim-server.h \
	aim-types.h

aim-marshalers.h: aim-marshalers.list
	$(AM_V_GEN) glib-genmarshal --prefix=aim_cclosure_marshal \
	                            --header aim-marshalers.list > aim-marshalers.h

aim-marshalers.c: aim-marshalers.list
	$(AM_V_GEN) glib-genmarshal --prefix=aim_cclosure_marshal \
	                            --body aim-marshalers.list > aim-marshalers.c

aim-enum-types.h: aim-message.h aim-enum-types.h.template
	$(AM_V_GEN) glib-mkenums --identifier-prefix Aim \
	                         --template aim-enum-types.h.template \
	                           aim-message.h aim-key-syms.h aim-types.h > aim-enum-types.h

aim-enum-types.c: aim-message.h aim-enum-types.c.template
	$(AM_V_GEN) glib-mkenums --identifier-prefix Aim \
	                         --template aim-enum-types.c.template \
	                           aim-message.h aim-key-syms.h aim-types.h > aim-enum-types.c

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = aim.pc

gsettings_SCHEMAS = org.aim.gschema.xml
@GSETTINGS_RULES@

EXTRA_DIST = aim.pc.in

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS  = --add-include-path=$(srcdir) \
                              --warn-all --warn-error \
                              --identifier-prefix=Aim --symbol-prefix=aim
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

introspection_sources = aim-agent.h

Aim-0.1.gir: libaim.la
Aim_0_1_gir_INCLUDES = GObject-2.0
Aim_0_1_gir_CFLAGS   = $(libaim_la_CFLAGS) -I$(top_srcdir)
Aim_0_1_gir_LIBS     = libaim.la
Aim_0_1_gir_FILES    = $(introspection_sources)

INTROSPECTION_GIRS += Aim-0.1.gir

girdir   = $(INTROSPECTION_GIRDIR)
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir   = $(INTROSPECTION_TYPELIBDIR)
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

uninstall-hook:
	-rmdir -p $(DESTDIR)$(libdir)/aim
	-rmdir -p $(DESTDIR)$(IMdkitincludedir)

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(gir_DATA) $(typelib_DATA)

DISTCLEANFILES = Makefile.in
