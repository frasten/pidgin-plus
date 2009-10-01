#!/bin/sh
autoreconf -f -i

if [ ! $? -a ! $NOCONFIGURE ]; then
	./configure $@
fi
