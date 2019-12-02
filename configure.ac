dnl ***************************
dnl *** Initialize autoconf ***
dnl ***************************
AC_INIT([gooroom-notice-applet], [0.1.0])
AC_PREREQ([2.63])

dnl ***************************
dnl *** Initialize automake ***
dnl ***************************
AM_INIT_AUTOMAKE([1.8 dist-xz no-dist-gzip foreign])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE()
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl *******************************
dnl *** Check for UNIX variants ***
dnl *******************************
AC_AIX()
AC_ISC_POSIX()
AC_MINIX()

dnl ********************************
dnl *** Check for basic programs ***
dnl ********************************
AC_PROG_CC()
AM_PROG_CC_C_O()
AC_PROG_LD()
AC_PROG_INSTALL()
IT_PROG_INTLTOOL([0.35.2])

dnl **************************
dnl *** Initialize libtool ***
dnl **************************
LT_PREREQ([2.4])
LT_INIT([dlopen disable-static])

dnl **********************************
dnl *** Check for standard headers ***
dnl **********************************
AC_HEADER_STDC()
AC_CHECK_HEADERS([stdlib.h string.h errno.h unistd.h])

dnl ******************************
dnl *** Check for i18n support ***
dnl ******************************
GETTEXT_PACKAGE=gooroom-notice-applet
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext Package])
AM_GLIB_GNU_GETTEXT

dnl ***********************************
dnl *** Optional debug message***
dnl ***********************************
dnl AC_DEFINE_UNQUOTED([DEBUG_MSG], [], ["Output debug message"])

dnl ***********************************
dnl *** Check for required packages ***
dnl ***********************************
GTK_REQUIRED=3.20.0
GLIB_REQUIRED=2.44.0
LIBPANEL_REQUIRED=3.24.1
DBUS_REQUIRED=0.32
DBUS_GLIB_REQUIRED=0.84
LIBWEBKIT2GTK_REQUIRED=2.20

PKG_CHECK_MODULES(GTK, gtk+-3.0 >= $GTK_REQUIRED)
PKG_CHECK_MODULES(GLIB, glib-2.0 >= GLIB_REQUIRED)
PKG_CHECK_MODULES(GNOME_APPLETS, libpanel-applet >= $LIBPANEL_REQUIRED)
PKG_CHECK_MODULES(DBUS, dbus-1 > DBUS_REQUIRED)
PKG_CHECK_MODULES(DBUS_GLIB, dbus-glib-1)
PKG_CHECK_MODULES(LIBNOTIFY, libnotify)
PKG_CHECK_MODULES(LIBWEBKIT2GTK, webkit2gtk-4.0 > LIBWEBKIT2GTK_REQUIRED)
PKG_CHECK_MODULES([JSON_C], json-c)

AC_ARG_WITH([libpanel-applet-dir], [], [LIBPANEL_APPLET_DIR=$withval], [LIBPANEL_APPLET_DIR=""])
if test "$LIBPANEL_APPLET_DIR" == ""; then
	LIBPANEL_APPLET_DIR=`$PKG_CONFIG --variable=libpanel_applet_dir libpanel-applet`
fi

AC_MSG_NOTICE([installing applets in $LIBPANEL_APPLET_DIR])
AC_SUBST(LIBPANEL_APPLET_DIR)


AC_OUTPUT([
Makefile
icons/Makefile
icons/22x22/Makefile
icons/22x22/status/Makefile
icons/scalable/Makefile
icons/scalable/apps/Makefile
data/Makefile
src/Makefile
po/Makefile.in
])
