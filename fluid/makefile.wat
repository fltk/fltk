#
# "$Id$"
#
# FLUID makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2004 by Bill Spitzak and others.
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
# Please report all bugs and problems on the following page:
#
#     http://www.fltk.org/str.php
#

OBJECTS = &
    CodeEditor.obj &
    Fl_Function_Type.obj &
    Fl_Group_Type.obj &
    Fl_Menu_Type.obj &
    Fl_Type.obj &
    Fl_Widget_Type.obj &
    Fl_Window_Type.obj &
    Fluid_Image.obj &
    about_panel.obj &
    align_widget.obj &
    alignment_panel.obj &
    code.obj &
    factory.obj &
    file.obj &
    function_panel.obj &
    template_panel.obj &
    undo.obj &
    widget_panel.obj

#   fluid.obj should not be in the list, because the watcom.mif file
#   assumes an object file with the same name as the target executable

################################################################

!include ../watcom.mif

all:  $(ODIR)/fluid$(EXEEXT)

$(ODIR)\fluid$(EXEEXT): $(OBJECTS) $(LIBS)

#
# Clean all directories
#
clean : .SYMBOLIC
    @echo Cleaning up.
CLEANEXTS = exe map sym obj lk1
    @for %a in ($(CLEANEXTS)) do -rm -f $(ODIR)\*.%a
    -rm -f *.err

#
# Note: The rebuild target can only be used if you have the original .fl
#       files.  This is normally only used by the FLTK maintainers...
#

rebuild:
    ./fluid -c about_panel.fl
    ./fluid -c alignment_panel.fl
    ./fluid -c function_panel.fl
    ./fluid -c widget_panel.fl

#
# End of "$Id$".
#
