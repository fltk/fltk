//
// "$Id: glut_font.cxx,v 1.4.2.4 2001/01/22 15:13:41 easysw Exp $"
//
// GLUT bitmap font routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

// (sort of) emulation of Glut's bitmap drawing functions, using FL's
// font stuff.  Not all the fonts match!

#include <config.h>
#if HAVE_GL

#include <FL/glut.H>
#include <FL/gl.h>

Glut_Bitmap_Font glutBitmap9By15 = {FL_SCREEN, 15};
Glut_Bitmap_Font glutBitmap8By13 = {FL_SCREEN, 13};
Glut_Bitmap_Font glutBitmapTimesRoman10 = {FL_TIMES, 10};
Glut_Bitmap_Font glutBitmapTimesRoman24 = {FL_TIMES, 24};
Glut_Bitmap_Font glutBitmapHelvetica10 = {FL_HELVETICA, 10};
Glut_Bitmap_Font glutBitmapHelvetica12 = {FL_HELVETICA, 12};
Glut_Bitmap_Font glutBitmapHelvetica18 = {FL_HELVETICA, 18};

void glutBitmapCharacter(void* font, int character) {
  gl_font(((Glut_Bitmap_Font *)font)->font,((Glut_Bitmap_Font *)font)->size);
  char a[1]; a[0] = character;
  gl_draw(a,1);
}

int glutBitmapWidth(void* font, int character) {
  gl_font(((Glut_Bitmap_Font *)font)->font,((Glut_Bitmap_Font *)font)->size);
  return int(gl_width(character)+.5);
}

#endif

//
// End of "$Id: glut_font.cxx,v 1.4.2.4 2001/01/22 15:13:41 easysw Exp $".
//
