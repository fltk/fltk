#! /bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.
srcdir=`cd $srcdir && pwd`

ORIGDIR=`pwd`
cd $srcdir

autoconf --force || exit 1
automake --add-missing
if test ! -e config.sub; then echo NOTE: Using frozen copy of config.sub; cp misc/config.sub . ; fi
if test ! -e config.guess; then echo NOTE: Using frozen copy of config.guess; cp misc/config.guess . ; fi
cd $ORIGDIR || exit $?

test -n "$NOCONFIGURE" || $srcdir/configure "$@"
