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
	-rm -f ${ALL} jpeg_image *~
