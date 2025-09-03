#
# Makefile used to build example apps by 'make'
#
# Copyright 2020-2022 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#     https://www.fltk.org/COPYING.php
#
# Please see the following page on how to report bugs and issues:
#
#     https://www.fltk.org/bugs.php
#
################################################################################

include Makefile.FLTK

RM    = rm -f
SHELL = /bin/sh
.SILENT:

# Executables
ALL = animgifimage$(EXEEXT) \
      animgifimage-play$(EXEEXT) \
      animgifimage-simple$(EXEEXT) \
      animgifimage-resize$(EXEEXT) \
      browser-simple$(EXEEXT) \
      cairo-draw-x$(EXEEXT) \
      callbacks$(EXEEXT) \
      chart-simple$(EXEEXT) \
      draggable-group$(EXEEXT) \
      grid-simple$(EXEEXT) \
      howto-add_fd-and-popen$(EXEEXT) \
      howto-browser-with-icons$(EXEEXT) \
      howto-drag-and-drop$(EXEEXT) \
      howto-draw-an-x$(EXEEXT) \
      howto-flex-simple$(EXEEXT) \
      howto-menu-with-images$(EXEEXT) \
      howto-parse-args$(EXEEXT) \
      howto-remap-numpad-keyboard-keys$(EXEEXT) \
      howto-simple-svg$(EXEEXT) \
      howto-text-over-image-button$(EXEEXT) \
      menubar-add$(EXEEXT) \
      nativefilechooser-simple-app$(EXEEXT) \
      nativefilechooser-simple$(EXEEXT) \
      progress-simple$(EXEEXT) \
      shapedwindow$(EXEEXT) \
      simple-terminal$(EXEEXT) \
      table-as-container$(EXEEXT) \
      table-simple$(EXEEXT) \
      table-sort$(EXEEXT) \
      table-spreadsheet$(EXEEXT) \
      table-spreadsheet-with-keyboard-nav$(EXEEXT) \
      table-with-keynav$(EXEEXT) \
      table-with-right-column-stretch-fit$(EXEEXT) \
      table-with-right-click-menu$(EXEEXT) \
      tabs-simple$(EXEEXT) \
      textdisplay-with-colors$(EXEEXT) \
      texteditor-simple$(EXEEXT) \
      texteditor-with-dynamic-colors$(EXEEXT) \
      tree-as-container$(EXEEXT) \
      tree-custom-draw-items$(EXEEXT) \
      tree-custom-sort$(EXEEXT) \
      tree-of-tables$(EXEEXT) \
      tree-simple$(EXEEXT) \
      wizard-simple$(EXEEXT)

# default target -- build everything
default all: $(ALL)

# Special rules for building cairo app
cairo-draw-x.o: cairo-draw-x.cxx
	@echo "*** Compile $<..."
	$(CXX) -I.. $(CXXFLAGS_CAIRO) -c $< -o $@
cairo-draw-x$(EXEEXT): cairo-draw-x.o
	@echo "*** Link $<..."
	$(CXX) $< $(LINKFLTK) $(LINKFLTK_CAIRO) -o $@

# clean everything
clean:
	$(RM) $(ALL)
	$(RM) *.o
	$(RM) core
