#!/bin/sh -e

# Workaround for http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=565663
[ ! -e m4 ] && mkdir m4

gnome-doc-prepare --force
autoreconf --force --install --verbose -Wall -I m4
