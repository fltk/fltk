//
// "$Id$"
//
// Input header file for the Fast Light Tool Kit (FLTK).
//
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

/* \file
   Fl_Input widget . */

#ifndef Fl_Input_H
#define Fl_Input_H

#include "Fl_Input_.H"

/**
  This is the FLTK text input widget. It displays a single line
  of text and lets the user edit it. Normally it is drawn with an
  inset box and a white background. The text may contain any
  characters (even 0), and will correctly display any UTF text, using
  ^X notation for unprintable control characters. It assumes the
  font can draw any characters of the used scripts, which is true
  for standard fonts under MSWindows and Mac OS X.
  Characters can be input using the keyboard or the character palette/map.
  Character composition is done using using dead keys and/or a compose
  key as defined by the operating system.
</P>
  
  <CENTER><TABLE border=1 WIDTH=90% summary="Fl_Input keyboard and mouse bindings.">
  
  <TR><TD WIDTH=200><B>Mouse button 1</B></TD><TD>Moves the cursor to
  this point. Drag selects characters.  Double click selects words.
  Triple click selects all text.  Shift+click extends the selection.
  When you select text it is automatically copied to the clipboard.
  </TD></TR>
  
  <TR><TD><B>Mouse button 2</B></TD><TD>Insert the clipboard at
  the point clicked.  You can also select a region and replace it with the
  clipboard by selecting the region with mouse button 2.
  </TD></TR>
  
  <TR><TD><B>Mouse button 3</B></TD><TD>Currently acts like button 1.</TD></TR>
  
  <TR><TD><B>Backspace</B></TD><TD>Deletes one character to the left, or
  deletes the selected region.</TD></TR>
  <TR><TD><B>Enter</B></TD><TD>May cause the callback, see when().</TD></TR>
  <TR><TD><B>^A or Home</B></TD><TD>Go to start of line.</TD></TR>
  <TR><TD><B>^B or Left</B></TD><TD>Move left</TD></TR>
  <TR><TD><B>^C</B></TD><TD>Copy the selection to the clipboard</TD></TR>
  <TR><TD><B>^D or Delete</B></TD><TD>Deletes one character to the right
  or deletes the selected region.</TD></TR>
  <TR><TD><B>^E or End</B></TD><TD>Go to the end of line.</TD></TR>
  <TR><TD><B>^F or Right</B></TD><TD>Move right</TD></TR>
  <TR><TD><B>^K</B></TD><TD>Delete to the end of line (next \\n character)
  or deletes a single \\n character.  These deletions are all concatenated
  into the clipboard.</TD></TR>
  <TR><TD><B>^N or Down</B></TD><TD>Move down (for Fl_Multiline_Input
  only, otherwise it moves to the next input field).</TD></TR>
  <TR><TD><B>^P or Up</B></TD><TD>Move up (for Fl_Multiline_Input only,
  otherwise it moves to the previous input field).</TD></TR>
  <TR><TD><B>^U</B></TD><TD>Delete everything.</TD></TR>
  <TR><TD><B>^V or ^Y</B></TD><TD>Paste the clipboard</TD></TR>
  <TR><TD><B>^X or ^W</B></TD><TD>Copy the region to the clipboard and
  delete it.</TD></TR>
  <TR><TD><B>^Z or ^_</B></TD><TD>Undo.  This is a single-level undo
  mechanism, but all adjacent deletions and insertions are concatenated
  into a single "undo".  Often this will undo a lot more than you
  expected.</TD></TR>
  <TR><TD><B>Shift+move</B></TD><TD>Move the cursor but also extend the
  selection.</TD></TR>
  </TABLE></CENTER>
  */
class FL_EXPORT Fl_Input : public Fl_Input_ {
  int handle_key();
  int shift_position(int p);
  int shift_up_down_position(int p);
  void handle_mouse(int keepmark=0);
protected:
  void draw();
public:
  int handle(int);
  Fl_Input(int,int,int,int,const char * = 0);
};

#endif 

//
// End of "$Id$".
//
