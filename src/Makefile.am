gnomeappletdir = $(pkglibdir)
gnomeapplet_LTLIBRARIES = libgooroom-notice-applet.la

libgooroom_notice_applet_la_CPPFLAGS = \
	-I. \
	-I$(srcdir) \
	-I$(top_srcdir) \
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DWNCK_I_KNOW_THIS_IS_UNSTABLE \
	-DGDK_DISABLE_DEPRECATED

BUILT_SOURCES = \
	applet-resources.c \
	applet-resources.h

libgooroom_notice_applet_la_SOURCES = \
	$(BUILT_SOURCES)	\
	gooroom-notice-applet.h	\
	gooroom-notice-applet.c	\
	$(NULL)

libgooroom_notice_applet_la_CFLAGS = \
	$(GLIB_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(DBUS_GLIB_CFLAGS) \
	$(LIBWEBKIT2GTK_CFLAGS) \
	$(AM_CFLAGS)

libgooroom_notice_applet_la_LDFLAGS = \
	-avoid-version \
	-module \
	$(AM_LDFLAGS)

libgooroom_notice_applet_la_LIBADD = \
	$(GLIB_LIBS) \
	$(GTK_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(DBUS_LIBS) \
	$(DBUS_GLIB_LIBS) \
	$(LIBWEBKIT2GTK_LIBS) \
	$(GNOME_APPLETS_LIBS)

resource_files = $(shell glib-compile-resources --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/gresource.xml)
applet-resources.c: gresource.xml $(resource_files)
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-source --c-name applet $<
applet-resources.h: gresource.xml $(resource_files)
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-header --c-name applet $<

CLEANFILES = $(BUILT_SOURCES)
