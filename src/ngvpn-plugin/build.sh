#!/bin/sh

#
# Build an OpenVPN plugin module on *nix.  The argument should
# be the base name of the C source file (without the .c).
#

# This directory is where we will look for openvpn-plugin.h
# CPPFLAGS="${CPPFLAGS:--I../../../include}"

CC="${CC:-gcc}"
CFLAGS="${CFLAGS:--O2 -Wall -g}"
LDFLAGS=-lssl

$CC $CFLAGS -fPIC -c ngvpn-plugin.c && \
$CC $CFLAGS -fPIC -shared $LDFLAGS -Wl,-soname,ngvpn-plugin.so -o ngvpn-plugin.so ngvpn-plugin.o -lc
