include ../makeinclude

ALL = howto-add_fd-and-popen$(EXEEXT) \
      howto-parse-args$(EXEEXT) \
      tabs-simple$(EXEEXT) \
      tree-simple$(EXEEXT) \
      wizard-simple$(EXEEXT)

# default target -- build everything
default: $(ALL)

tabs-simple$(EXEEXT): tabs-simple.cxx
	fltk-config --compile tabs-simple.cxx

# clean everything
clean:
	$(RM) $(ALL)
	$(RM) *.o
	$(RM) core
