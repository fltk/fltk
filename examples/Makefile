include Makefile.FLTK

RM    = rm -f
SHELL = /bin/sh
.SILENT:

# Executables
ALL = clipboard$(EXEEXT) \
      howto-add_fd-and-popen$(EXEEXT) \
      howto-browser-with-icons$(EXEEXT) \
      howto-drag-and-drop$(EXEEXT) \
      howto-parse-args$(EXEEXT) \
      howto-text-over-image-button$(EXEEXT) \
      menubar-add$(EXEEXT) \
      nativefilechooser-simple-app$(EXEEXT) \
      progress-simple$(EXEEXT) \
      shapedwindow$(EXEEXT) \
      table-as-container$(EXEEXT) \
      table-simple$(EXEEXT) \
      table-sort$(EXEEXT) \
      table-spreadsheet$(EXEEXT) \
      table-spreadsheet-with-keyboard-nav$(EXEEXT) \
      table-with-keynav$(EXEEXT) \
      tabs-simple$(EXEEXT) \
      textdisplay-with-colors$(EXEEXT) \
      texteditor-simple$(EXEEXT) \
      tree-as-container$(EXEEXT) \
      tree-custom-draw-items$(EXEEXT) \
      tree-custom-sort$(EXEEXT) \
      tree-of-tables$(EXEEXT) \
      tree-simple$(EXEEXT) \
      wizard-simple$(EXEEXT)

# default target -- build everything
default all: $(ALL)

# clean everything
clean:
	$(RM) $(ALL)
	$(RM) *.o
	$(RM) core
