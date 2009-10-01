#!/bin/sh
autoreconf -f -i

if [ $NOCONFIGURE ]; then
	./configure $@
fi
