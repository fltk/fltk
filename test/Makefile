#
# "$Id"
#
# Test/example program makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998 by Bill Spitzak and others.
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
# Please report all bugs and problems to "fltk-bugs@easysw.com".
#

ALL =	adjuster arc ask bitmap boxtype browser button buttons checkers \
	clock colbrowser color_chooser cube cursor curve demo doublebuffer \
	file_chooser fonts forms fractals fullscreen gl_overlay glpuzzle \
	hello iconize image input keyboard label list_visuals mandelbrot \
	menubar message minimum navigation output overlay pixmap \
	pixmap_browser radio resizebox scroll shape shiny subwindow \
	symbols tabs tile valuators fast_slow resize pack inactive style

all:	${ALL}

include ../makeinclude

${ALL} :  ../lib/$(LIBNAME)

.SUFFIXES : .C .c .o .do

.C:
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) $< -L../lib -lfltk ${LDLIBS} -o $@

# Programs needing the OpenGL libraries:
cube: cube.C
	@echo $@:
	@${CXX} -I.. ${CXXFLAGS} cube.C -L../lib -lfltk ${GLDLIBS} -o $@
fullscreen: fullscreen.C
	@echo $@:
	@${CXX} -I.. ${CXXFLAGS} fullscreen.C -L../lib -lfltk ${GLDLIBS} -o $@
fractals: fractals.C
	@echo $@:
	@${CXX} -I.. ${CXXFLAGS} fractals.C -L../lib -lfltk ${GLDLIBS} -o $@
gl_overlay: gl_overlay.C
	@echo $@:
	@${CXX} -I.. ${CXXFLAGS} gl_overlay.C -L../lib -lfltk ${GLDLIBS} -o $@
glpuzzle: glpuzzle.C
	@echo $@:
	@${CXX} -I.. ${CXXFLAGS} glpuzzle.C -L../lib -lfltk ${GLDLIBS} -o $@
shape: shape.C
	@echo $@:
	@${CXX} -I.. ${CXXFLAGS} shape.C -L../lib -lfltk ${GLDLIBS} -o $@
shiny: shiny.C
	@echo $@:
	@${CXX} -I.. ${CXXFLAGS} shiny.C -L../lib -lfltk ${GLDLIBS} -o $@

# If you have libjpeg installed, you might want to try this test program:

jpeg_image: jpeg_image.C
	@echo $@:
	@${CXX} -I.. ${CXXFLAGS} -I../../../local/jpeg-6b -L../../../local/jpeg-6b jpeg_image.C -L../lib -lfltk ${LDLIBS} -ljpeg -lXext -o $@

clean:
	-@ rm -f ${ALL} jpeg_image *.o core *~

#
# End of "$Id: Makefile,v 1.3 1998/10/20 21:06:19 mike Exp $".
#
