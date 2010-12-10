//
// "$Id$"
//
//	A simple example of Fl_Text_Display with colors.
//	For a color text editor, see the 'editor' example in the test directory.
//
// Copyright 2010 Greg Ercolano.
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Display.H>
int main() {
   // Style table
   Fl_Text_Display::Style_Table_Entry stable[] = {
       // FONT COLOR      FONT FACE   FONT SIZE
       // --------------- ----------- --------------
       {  FL_RED,         FL_COURIER, 18 }, // A - Red
       {  FL_DARK_YELLOW, FL_COURIER, 18 }, // B - Yellow
       {  FL_DARK_GREEN,  FL_COURIER, 18 }, // C - Green
       {  FL_BLUE,        FL_COURIER, 18 }, // D - Blue
   };
   Fl_Window *win = new Fl_Window(640, 480, "Simple Text Display With Colors");
   Fl_Text_Display *disp = new Fl_Text_Display(20, 20, 640-40, 480-40);
   Fl_Text_Buffer *tbuff = new Fl_Text_Buffer();	// text buffer
   Fl_Text_Buffer *sbuff = new Fl_Text_Buffer();	// style buffer
   disp->buffer(tbuff);
   int stable_size = sizeof(stable)/sizeof(stable[0]);	// # entries in style table (4)
   disp->highlight_data(sbuff, stable, stable_size, 'A', 0, 0);
   // Text
   tbuff->text("Red Line 1\nYel Line 2\nGrn Line 3\nBlu Line 4\n"
	       "Red Line 5\nYel Line 6\nGrn Line 7\nBlu Line 8\n");
   // Style for text
   sbuff->text("AAAAAAAAAA\nBBBBBBBBBB\nCCCCCCCCCC\nDDDDDDDDDD\n"
	       "AAAAAAAAAA\nBBBBBBBBBB\nCCCCCCCCCC\nDDDDDDDDDD\n");
   win->resizable(*disp);
   win->show();
   return(Fl::run());
}

//
// End of "$Id$".
//
