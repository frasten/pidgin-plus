plusdir = $(PIDGIN_LIBDIR)

plus_la_LDFLAGS = -module -avoid-version

if HAVE_PIDGIN

plus_LTLIBRARIES = plus.la

plus_la_SOURCES = \
	plus.c

plus_la_LIBADD = \
	$(GLIB_LIBS) \
	$(PURPLE_LIBS)

endif

AM_CPPFLAGS = \
	$(PURPLE_CFLAGS) $(PIDGIN_CFLAGS)
