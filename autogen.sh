#!/bin/sh
if [ "$1" = "clean" ]; then
	if [ -f "Makefile" ]; then
		make distclean
	fi
	rm -rf INSTALL *.in *.la *.lo *~ aclocal.m4 autom4te.cache/ config.guess \
		config.h config.log config.status config.sub configure depcomp \
		install-sh libtool ltmain.sh missing .deps .libs stamp-h1 \
		plus/Makefile.in
	exit
fi

autoreconf -f -i

if [ ! "$NOCONFIGURE" ]; then
	./configure $@
fi
