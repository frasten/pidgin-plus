AC_INIT([pidgin-plus], [0.1.0], [daniele.athome@gmail.com])
AC_CANONICAL_SYSTEM
AM_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_DEFINE_UNQUOTED(PP_VERSION, "AC_PACKAGE_VERSION", [Plus! Plugin Version])

AC_DISABLE_STATIC

dnl #######################################################################
dnl # Setup libtool
dnl #######################################################################
AM_PROG_LIBTOOL
#LIBTOOL="$LIBTOOL --silent"

dnl #######################################################################
dnl # Check for purple
dnl #######################################################################
PKG_CHECK_MODULES(PURPLE, purple,
[
	AC_DEFINE(HAVE_PURPLE, 1, [Define if we've found libpurple.])
])

HAVE_PURPLE="yes"
AM_CONDITIONAL(HAVE_PURPLE, true)

if test x"$prefix" = x"NONE" ; then
	PURPLE_LIBDIR=`pkg-config --variable=libdir purple`
	PURPLE_DATADIR=`pkg-config --variable=datadir purple`
else
	PURPLE_LIBDIR="$libdir"
	PURPLE_DATADIR="$datadir"
fi

PURPLE_PIXMAPSDIR=""

# this is a hack but should work fine.
# we use the libpurple datadir for PP_LOCALEDIR since we are not breaking up
# the pot's yet, and need to make sure they goto the same place
PP_LOCALEDIR="PURPLE_DATADIR/locale"
AC_DEFINE_UNQUOTED(PP_LOCALEDIR, ["$PP_LOCALEDIR"], [The localedir to use])

if test x"$PURPLE_LIBDIR" != x"" ; then
	PURPLE_LIBDIR="$PURPLE_LIBDIR/purple-2"
fi

if test x"$PURPLE_DATADIR" != x"" ; then
	PURPLE_PIXMAPSDIR="$PURPLE_DATADIR/pixmaps/libpurple"
	AC_DEFINE_UNQUOTED(PURPLE_PIXMAPSDIR, "$PURPLE_PIXMAPSDIR", [Libpurple pixmaps directory])
fi

AC_SUBST(PURPLE_CFLAGS)
AC_SUBST(PURPLE_LIBS)
AC_SUBST(PURPLE_LIBDIR)
AC_SUBST(PURPLE_DATADIR)
AC_SUBST(PURPLE_PIXMAPSDIR)


dnl #######################################################################
dnl # Check for pidgin
dnl #######################################################################
PKG_CHECK_MODULES(PIDGIN, pidgin,
[
	AC_DEFINE(HAVE_PIDGIN, 1, [Define if we've found pidgin.])
	HAVE_PIDGIN="yes"
	AM_CONDITIONAL(HAVE_PIDGIN, true)
], [
	AC_MSG_RESULT([no])
	HAVE_PIDGIN="no"
	AM_CONDITIONAL(HAVE_PIDGIN, false)
])

if test x"$prefix" = x"NONE" ; then
	PIDGIN_LIBDIR=`pkg-config --variable=libdir pidgin`
	PIDGIN_DATADIR=`pkg-config --variable=datadir pidgin`
else
	PIDGIN_LIBDIR="$libdir"
	PIDGIN_DATADIR="$datadir"
fi

PIDGIN_PIXMAPSDIR=""

if test x"$PIDGIN_LIBDIR" != x"" ; then
	PIDGIN_LIBDIR="$PIDGIN_LIBDIR/pidgin"
fi

if test x"$PIDGIN_DATADIR" != x"" ; then
	PIDGIN_PIXMAPSDIR="$PIDGIN_DATADIR/pixmaps/pidgin"
	AC_DEFINE_UNQUOTED(PIDGIN_PIXMAPSDIR, "$PIDGIN_PIXMAPSDIR", [Pidgin pixmaps directory])
fi

AC_SUBST(PIDGIN_CFLAGS)
AC_SUBST(PIDGIN_LIBS)
AC_SUBST(PIDGIN_LIBDIR)
AC_SUBST(PIDGIN_DATADIR)
AC_SUBST(PIDGIN_PIXMAPSDIR)


dnl #######################################################################
dnl # check for gtk and other dependencies
dnl #######################################################################
HAVE_GLIB="no"
PKG_CHECK_MODULES(GLIB, [glib-2.0], HAVE_GLIB="yes", HAVE_GLIB="no")

if test x"$HAVE_GLIB" = x"no" ; then
	echo "glib development headers were not found.  glib development headers"
	echo "are required to build $PACKAGE."
	exit 1
fi
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

HAVE_GTK="no"
GTK_CFLAGS=""
GTK_LIBS=""
PKG_CHECK_MODULES(GTK, [gtk+-2.0], HAVE_GTK="yes", HAVE_GTK="no")
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

dnl #######################################################################
dnl # Finish up
dnl #######################################################################
AC_OUTPUT([
	Makefile
	plus/Makefile
])

dnl #######################################################################
dnl # Ouput!!
dnl #######################################################################
echo;
echo $PACKAGE $VERSION Configuration complete
echo;

PP_PIDGIN_BUILD=plus
AC_SUBST(PP_PIDGIN_BUILD)

echo Build purple plugins.............: $HAVE_PURPLE
if test x"$HAVE_PURPLE" = x"yes" ; then
	echo Installing purple plugins to.....: `eval eval echo $PURPLE_LIBDIR`
	echo Installing purple plugin data to.: `eval eval echo $PURPLE_DATADIR`
	if test x"$PP_PURPLE_BUILD" = x"" ; then
		echo Purple plugins to be built.......: none
	else
		echo Purple plugins to be built.......:
		echo $PP_PURPLE_BUILD | xargs -n 4 echo "	"
	fi
fi
echo;

echo Build pidgin plugins.............: $HAVE_PIDGIN
if test x"$HAVE_PIDGIN" = x"yes" ; then
	echo Installing pidgin plugins to.....: `eval eval echo $PIDGIN_LIBDIR`
	echo Installing pidgin plugin data to.: `eval eval echo $PIDGIN_DATADIR`
	if test x"$PP_PIDGIN_BUILD" = x"" ; then
		echo Pidgin plugins to be built.......: none
	else
		echo Pidgin plugins to be built.......:
		echo $PP_PIDGIN_BUILD | xargs -n 4 echo "	"
	fi
fi
echo;

echo Type make to compile
echo;
