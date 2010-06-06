include ../makeinclude

ALL = tabs-simple$(EXEEXT) \
      tree-simple$(EXEEXT)

# default target -- build everything
default: $(ALL)

tabs-simple$(EXEEXT): tabs-simple.cxx
	fltk-config --compile tabs-simple.cxx

tree-simple$(EXEEXT): tree-simple.cxx
	fltk-config --compile tree-simple.cxx

# clean everything
clean:
	$(RM) $(ALL)
	$(RM) *.o
	$(RM) core
