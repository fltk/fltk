#
# "$Id$"
#
# Test/example program makefile for the Fast Light Tool Kit (FLTK).
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

!include ../watcom.mif

ALL =   &
    $(ODIR)/unittests$(EXEEXT) &
    $(ODIR)/adjuster$(EXEEXT) &
    $(ODIR)/arc$(EXEEXT) &
    $(ODIR)/ask$(EXEEXT) &
    $(ODIR)/bitmap$(EXEEXT) &
    $(ODIR)/boxtype$(EXEEXT) &
    $(ODIR)/browser$(EXEEXT) &
    $(ODIR)/button$(EXEEXT) &
    $(ODIR)/buttons$(EXEEXT) &
    $(ODIR)/checkers$(EXEEXT) &
    $(ODIR)/clock$(EXEEXT) &
    $(ODIR)/colbrowser$(EXEEXT) &
    $(ODIR)/color_chooser$(EXEEXT) &
    $(ODIR)/cursor$(EXEEXT) &
    $(ODIR)/curve$(EXEEXT) &
    $(ODIR)/demo$(EXEEXT) &
    $(ODIR)/doublebuffer$(EXEEXT) &
    $(ODIR)/editor$(EXEEXT) &
    $(ODIR)/fast_slow$(EXEEXT) &
    $(ODIR)/file_chooser$(EXEEXT) &
    $(ODIR)/fonts$(EXEEXT) &
    $(ODIR)/forms$(EXEEXT) &
    $(ODIR)/hello$(EXEEXT) &
    $(ODIR)/help$(EXEEXT) &
    $(ODIR)/iconize$(EXEEXT) &
    $(ODIR)/image$(EXEEXT) &
    $(ODIR)/inactive$(EXEEXT) &
    $(ODIR)/input$(EXEEXT) &
    $(ODIR)/keyboard$(EXEEXT) &
    $(ODIR)/label$(EXEEXT) &
    $(ODIR)/line_style$(EXEEXT) &
    $(ODIR)/list_visuals$(EXEEXT) &
    $(ODIR)/mandelbrot$(EXEEXT) &
    $(ODIR)/menubar$(EXEEXT) &
    $(ODIR)/message$(EXEEXT) &
    $(ODIR)/minimum$(EXEEXT) &
    $(ODIR)/native-filechooser$(EXEEXT) &
    $(ODIR)/navigation$(EXEEXT) &
    $(ODIR)/output$(EXEEXT) &
    $(ODIR)/overlay$(EXEEXT) &
    $(ODIR)/pack$(EXEEXT) &
    $(ODIR)/pixmap$(EXEEXT) &
    $(ODIR)/pixmap_browser$(EXEEXT) &
    $(ODIR)/preferences$(EXEEXT) &
    $(ODIR)/radio$(EXEEXT) &
    $(ODIR)/resize$(EXEEXT) &
    $(ODIR)/resizebox$(EXEEXT) &
    $(ODIR)/scroll$(EXEEXT) &
    $(ODIR)/subwindow$(EXEEXT) &
    $(ODIR)/symbols$(EXEEXT) &
    $(ODIR)/tabs$(EXEEXT) &
    $(ODIR)/threads$(EXEEXT) &
    $(ODIR)/tile$(EXEEXT) &
    $(ODIR)/tiled_image$(EXEEXT) &
    $(ODIR)/valuators$(EXEEXT)

GLALL = &
    $(ODIR)/cube$(EXEEXT) &
    $(ODIR)/CubeView$(EXEEXT) &
    $(ODIR)/fractals$(EXEEXT) &
    $(ODIR)/fullscreen$(EXEEXT) &
    $(ODIR)/gl_overlay$(EXEEXT) &
    $(ODIR)/glpuzzle$(EXEEXT) &
    $(ODIR)/shape$(EXEEXT)

all:    $(ALL) $(GLALL)

gldemos:    $(GLALL)


# FLUID file rules. We could put them in ../watcom.mif really, but that needs testing.
.fl.cxx:
    echo Generating $<...
    -..\fluid\$(ODIR)\fluid$(EXEEXT) -c $[@

.fl.h:
    echo Generating $<...
    -..\fluid\$(ODIR)\fluid$(EXEEXT) -c $[@

# All demos depend on the FLTK library...
$(ALL): $(LIBNAME)

# General demos..... Normally a executable depending on an object file of the same name
# shouldn't need a target line. But if different output directories are used, changes
# in sources files are not picked up, so we do need a line per target.

$(ODIR)/unittests$(EXEEXT) :      $(ODIR)/unittests.obj

$(ODIR)/adjuster$(EXEEXT) :       $(ODIR)/adjuster.obj

$(ODIR)/arc$(EXEEXT) :            $(ODIR)/arc.obj

$(ODIR)/ask$(EXEEXT) :            $(ODIR)/ask.obj

$(ODIR)/bitmap$(EXEEXT) :         $(ODIR)/bitmap.obj

$(ODIR)/boxtype$(EXEEXT) :        $(ODIR)/boxtype.obj

$(ODIR)/browser$(EXEEXT) :        $(ODIR)/browser.obj

$(ODIR)/button$(EXEEXT) :         $(ODIR)/button.obj

$(ODIR)/buttons$(EXEEXT) :        $(ODIR)/buttons.obj

$(ODIR)/checkers$(EXEEXT) :       $(ODIR)/checkers.obj

$(ODIR)/clock$(EXEEXT) :          $(ODIR)/clock.obj

$(ODIR)/colbrowser$(EXEEXT) :     $(ODIR)/colbrowser.obj

$(ODIR)/color_chooser$(EXEEXT) :  $(ODIR)/color_chooser.obj

$(ODIR)/cursor$(EXEEXT) :         $(ODIR)/cursor.obj

$(ODIR)/curve$(EXEEXT) :          $(ODIR)/curve.obj

$(ODIR)/demo$(EXEEXT) :           $(ODIR)/demo.obj

$(ODIR)/doublebuffer$(EXEEXT) :   $(ODIR)/doublebuffer.obj

$(ODIR)/editor$(EXEEXT) :         $(ODIR)/editor.obj

$(ODIR)/fast_slow$(EXEEXT) :      $(ODIR)/fast_slow.obj

$(ODIR)/file_chooser$(EXEEXT) :   $(ODIR)/file_chooser.obj

$(ODIR)/fonts$(EXEEXT) :          $(ODIR)/fonts.obj

$(ODIR)/forms$(EXEEXT) :          $(ODIR)/forms.obj

$(ODIR)/hello$(EXEEXT) :          $(ODIR)/hello.obj

$(ODIR)/help$(EXEEXT) :           $(ODIR)/help.obj

$(ODIR)/iconize$(EXEEXT) :        $(ODIR)/iconize.obj

$(ODIR)/image$(EXEEXT) :          $(ODIR)/image.obj

$(ODIR)/inactive$(EXEEXT) :       $(ODIR)/inactive.obj

$(ODIR)/input$(EXEEXT) :          $(ODIR)/input.obj

$(ODIR)/label$(EXEEXT) :          $(ODIR)/label.obj

$(ODIR)/line_style$(EXEEXT) :     $(ODIR)/line_style.obj

$(ODIR)/list_visuals$(EXEEXT) :   $(ODIR)/list_visuals.obj

$(ODIR)/menubar$(EXEEXT) :        $(ODIR)/menubar.obj

$(ODIR)/message$(EXEEXT) :        $(ODIR)/message.obj

$(ODIR)/minimum$(EXEEXT) :        $(ODIR)/minimum.obj

$(ODIR)/native-filechooser$(EXEEXT) :     $(ODIR)/native-filechooser.obj

$(ODIR)/navigation$(EXEEXT) :     $(ODIR)/navigation.obj

$(ODIR)/output$(EXEEXT) :         $(ODIR)/output.obj

$(ODIR)/overlay$(EXEEXT) :        $(ODIR)/overlay.obj

$(ODIR)/pack$(EXEEXT) :           $(ODIR)/pack.obj

$(ODIR)/pixmap$(EXEEXT) :         $(ODIR)/pixmap.obj

$(ODIR)/pixmap_browser$(EXEEXT) : $(ODIR)/pixmap_browser.obj

$(ODIR)/preferences$(EXEEXT) :    $(ODIR)/preferences.obj

$(ODIR)/radio$(EXEEXT) :          $(ODIR)/radio.obj

$(ODIR)/resize$(EXEEXT) :         $(ODIR)/resize.obj

$(ODIR)/resizebox$(EXEEXT) :      $(ODIR)/resizebox.obj

$(ODIR)/scroll$(EXEEXT) :         $(ODIR)/scroll.obj

$(ODIR)/subwindow$(EXEEXT) :      $(ODIR)/subwindow.obj

$(ODIR)/symbols$(EXEEXT) :        $(ODIR)/symbols.obj

$(ODIR)/tabs$(EXEEXT) :           $(ODIR)/tabs.obj

$(ODIR)/threads$(EXEEXT) :        $(ODIR)/threads.obj

$(ODIR)/tile$(EXEEXT) :           $(ODIR)/tile.obj

$(ODIR)/tiled_image$(EXEEXT) :    $(ODIR)/tiled_image.obj

$(ODIR)/valuators$(EXEEXT) :      $(ODIR)/valuators.obj

# Because keyboard_ui.obj is listed first, fluid will be used to generate the .cxx and .h file
# so that when keyboard.obj is built, keyboard_ui.h is there.
KBDOBJECTS=keyboard_ui.obj keyboard.obj
$(ODIR)/keyboard$(EXEEXT): $(KBDOBJECTS) keyboard_ui.h
    @%create $^*.lk1
    @for %i in ($(KBDOBJECTS)) do @%append $^*.lk1 F $(ODIR)/%i
    @for %i in ($(LIBS)) do @%append $^*.lk1 L %i
    @for %i in ($(EXTRA_LIBS)) do @%append $^*.lk1 L %i
    @for %i in ($(SYSLIBS)) do @%append $^*.lk1 L %i
    $(LN) $(LNOPTS) name $^@ op map=$^* @$^*.lk1
    @del $^*.lk1
    @set KBDOBJECTS=

MDLOBJECTS=mandelbrot_ui.obj mandelbrot.obj
$(ODIR)/mandelbrot$(EXEEXT): $(MDLOBJECTS)
    @%create $^*.lk1
    @for %i in ($(MDLOBJECTS)) do @%append $^*.lk1 F $(ODIR)/%i
    @for %i in ($(LIBS)) do @%append $^*.lk1 L %i
    @for %i in ($(EXTRA_LIBS)) do @%append $^*.lk1 L %i
    @for %i in ($(SYSLIBS)) do @%append $^*.lk1 L %i
    $(LN) $(LNOPTS) name $^@ op map=$^* @$^*.lk1
    @del $^*.lk1

# All OpenGL demos depend on the FLTK and FLTK_GL libraries...
$(GLALL): $(LIBNAME) $(LIBNAMEGL)

# OpenGL demos...

$(ODIR)/cube$(EXEEXT) :            $(ODIR)/cube.obj

$(ODIR)/fullscreen$(EXEEXT) :      $(ODIR)/fullscreen.obj

$(ODIR)/gl_overlay$(EXEEXT) :      $(ODIR)/gl_overlay.obj

$(ODIR)/glpuzzle$(EXEEXT) :        $(ODIR)/glpuzzle.obj

$(ODIR)/shape$(EXEEXT) :           $(ODIR)/shape.obj

CBVOBJECTS = CubeView.obj CubeViewUI.obj CubeMain.obj
$(ODIR)/CubeView$(EXEEXT): $(CBVOBJECTS)
    @%create $^*.lk1
    @for %i in ($(CBVOBJECTS)) do @%append $^*.lk1 F $(ODIR)/%i
    @for %i in ($(LIBS)) do @%append $^*.lk1 L %i
    @for %i in ($(EXTRA_LIBS)) do @%append $^*.lk1 L %i
    @for %i in ($(SYSLIBS)) do @%append $^*.lk1 L %i
    $(LN) $(LNOPTS) name $^@ op map=$^* @$^*.lk1
    @del $^*.lk1

FRTOBJECTS = fractals.obj fracviewer.obj
$(ODIR)/fractals$(EXEEXT): $(FRTOBJECTS)
    @%create $^*.lk1
    @for %i in ($(FRTOBJECTS)) do @%append $^*.lk1 F $(ODIR)/%i
    @for %i in ($(LIBS)) do @%append $^*.lk1 L %i
    @for %i in ($(EXTRA_LIBS)) do @%append $^*.lk1 L %i
    @for %i in ($(SYSLIBS)) do @%append $^*.lk1 L %i
    $(LN) $(LNOPTS) name $^@ op map=$^* @$^*.lk1
    @del $^*.lk1

#
# Clean all directories
#
clean : .SYMBOLIC
    @echo Cleaning up.
CLEANEXTS = exe map sym obj lk1
    @for %a in ($(CLEANEXTS)) do -rm -f $(ODIR)\*.%a
    -rm -f *.err
FLUIDMADE = fastslow inactive keyboard_ui preferences radio resize tabs valuators
    @for %a in ($(FLUIDMADE)) do -rm -f %a.cxx %a.h

#
# End of "$Id$".
#
