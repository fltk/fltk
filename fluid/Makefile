PROGRAM = fluid

CPPFILES = \
	Fl_Function_Type.C \
	Fl_Menu_Type.C \
	Fl_Group_Type.C \
	Fl_Widget_Type.C \
	Fl_Type.C \
	Fl_Window_Type.C \
	Fluid_Image.C \
	code.C \
	factory.C \
	file.C \
	fluid.C \
	about_panel.C \
	widget_panel.C \
	alignment_panel.C \
	function_panel.C \
	gif.C

################################################################

OBJECTS = $(CPPFILES:.C=.o)

include ../makeinclude

.SUFFIXES : .C .c .o .do .fl .H

.C.o :
	@echo $<:
	@$(CXX) -I.. $(CXXFLAGS) -c $<
.c.o :
	@echo $<:
	@$(CC) -I.. $(CFLAGS) -c $<
.C :
	@echo $@:
	@$(CXX) -I.. $(CXXFLAGS) -o $@ $< -L../lib -lfltk $(LDLIBS)

.fl.C .fl.H :
	-fluid -c $<

# better way to do this for gnu make:
# %.H %.C: %.fl
#	fluid -c $<

$(PROGRAM) : $(OBJECTS) ../lib/$(LIBNAME)
	@echo linking $@:
	@$(CXX) $(CXXFLAGS) -o $(PROGRAM) $(OBJECTS) -L../lib -lfltk $(LDLIBS)

clean :
	-@ rm -f *.o $(PROGRAM) $(CLEAN) core *~ makedepend
	@touch makedepend

depend:
	$(MAKEDEPEND) -I.. $(CXXFLAGS) $(CPPFILES) $(CFILES) > makedepend
include makedepend

install: $(PROGRAM)
	strip $(PROGRAM)
	cp $(PROGRAM) $(bindir)/$(PROGRAM)
	@chmod a+rx,g-w,o-w $(bindir)/$(PROGRAM)

uninstall:
	-@ rm -f $(bindir)/$(PROGRAM)

################################################################

PROGRAM_D = $(PROGRAM)_d

debug: $(PROGRAM_D)

OBJECTS_D = $(CPPFILES:.C=.do) $(CFILES:.c=.do)

.C.do :
	@echo $<:
	@$(CXX) -I.. $(CXXFLAGS_D) -c -o $@ $<
.c.do :
	@echo $<:
	@$(CC) -I.. $(CFLAGS_D) -c -o $@ $<

$(PROGRAM_D) : $(OBJECTS_D) ../lib/$(LIBNAME_D)
	@echo linking $@:
	@$(CXX) $(CXXFLAGS_D) -o $(PROGRAM_D) $(OBJECTS_D) -L../lib -lfltk_d $(LDLIBS)
