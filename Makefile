#
# "$Id: Makefile,v 1.12.2.6 2001/03/14 17:20:01 spitzak Exp $"
#
# Top-level makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2001 by Bill Spitzak and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.
#
# Please report all bugs and problems to "fltk-bugs@fltk.org".
#

SHELL=/bin/sh

DIRS	=	src fluid test

all: makeinclude
	@for dir in $(DIRS); do\
		echo "=== making $$dir ===";\
		if test ! -f $$dir/makedepend; then\
			touch $$dir/makedepend;\
		fi;\
		(cd $$dir; $(MAKE) $(MFLAGS)) || break;\
	done

install: makeinclude
	@for dir in $(DIRS); do\
		echo "=== installing $$dir ===";\
		if test ! -f $$dir/makedepend; then\
			touch $$dir/makedepend;\
		fi;\
		(cd $$dir; $(MAKE) $(MFLAGS) install) || break;\
	done
	(cd documentation; $(MAKE) $(MFLAGS) install)

depend: makeinclude
	@for dir in $(DIRS); do\
		echo "=== making dependencies in $$dir ===";\
		if test ! -f $$dir/makedepend; then\
			touch $$dir/makedepend;\
		fi;\
		(cd $$dir; $(MAKE) $(MFLAGS) depend) || break;\
	done

clean:
	-@ rm -f core config.cache *.o *.bck
	@for dir in $(DIRS); do\
		echo "=== cleaning $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS) clean) || break;\
	done

distclean: clean
	rm -f config.log config.h config.status makeinclude

makeinclude: configure configh.in makeinclude.in
	./configure

configure: configure.in
	autoconf

#
# End of "$Id: Makefile,v 1.12.2.6 2001/03/14 17:20:01 spitzak Exp $".
#
