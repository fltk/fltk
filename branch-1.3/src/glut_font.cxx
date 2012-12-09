//
// "$Id$"
//
// GLUT font routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//
// The stroked text code was copied from FreeGLUT 2.4.0 which carries
// the following notice:
//
//     Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
//

// (sort of) emulation of Glut's bitmap drawing functions, using FL's
// font stuff.  Not all the fonts match!

#include <config.h>
#if HAVE_GL
#  include <FL/glut.H>

Fl_Glut_Bitmap_Font glutBitmap9By15 = {FL_SCREEN, 15};
Fl_Glut_Bitmap_Font glutBitmap8By13 = {FL_SCREEN, 13};
Fl_Glut_Bitmap_Font glutBitmapTimesRoman10 = {FL_TIMES, 10};
Fl_Glut_Bitmap_Font glutBitmapTimesRoman24 = {FL_TIMES, 24};
Fl_Glut_Bitmap_Font glutBitmapHelvetica10 = {FL_HELVETICA, 10};
Fl_Glut_Bitmap_Font glutBitmapHelvetica12 = {FL_HELVETICA, 12};
Fl_Glut_Bitmap_Font glutBitmapHelvetica18 = {FL_HELVETICA, 18};

void glutBitmapCharacter(void* font, int character) {
  gl_font(((Fl_Glut_Bitmap_Font *)font)->font,((Fl_Glut_Bitmap_Font *)font)->size);
  char a[1]; a[0] = character;
  gl_draw(a,1);
}

int glutBitmapHeight(void* font) {
  gl_font(((Fl_Glut_Bitmap_Font *)font)->font,((Fl_Glut_Bitmap_Font *)font)->size);
  return gl_height();
}

int glutBitmapLength(void *font, const unsigned char *string) {
  gl_font(((Fl_Glut_Bitmap_Font *)font)->font,((Fl_Glut_Bitmap_Font *)font)->size);
  const char *s = (const char*)string;
  return int(gl_width(s)+.5);
}

void glutBitmapString(void *font, const unsigned char *string) {
  gl_font(((Fl_Glut_Bitmap_Font *)font)->font,((Fl_Glut_Bitmap_Font *)font)->size);
  const char *s = (const char*)string;
  gl_draw(s);
}

int glutBitmapWidth(void* font, int character) {
  gl_font(((Fl_Glut_Bitmap_Font *)font)->font,((Fl_Glut_Bitmap_Font *)font)->size);
  return int(gl_width(character)+.5);
}


/*
 * Draw a stroke character
 */
void glutStrokeCharacter(void* fontID, int character) {
  const Fl_Glut_StrokeChar *schar;
  const Fl_Glut_StrokeStrip *strip;
  int i, j;
  Fl_Glut_StrokeFont* font = (Fl_Glut_StrokeFont *)fontID;

  if (character < 0 || character >= font->Quantity) return;

  schar = font->Characters[character];
  if (!schar) return;

  strip = schar->Strips;
  
  for (i = 0; i < schar->Number; i++, strip++)
  {
    glBegin(GL_LINE_STRIP);
    for (j = 0; j < strip->Number; j++)
      glVertex2f(strip->Vertices[j].X, strip->Vertices[j].Y);
    glEnd();
  }

  glTranslatef(schar->Right, 0.0, 0.0);
}

void glutStrokeString(void* fontID, const unsigned char *string) {
  unsigned char c;
  int i, j;
  float length = 0.0;
  Fl_Glut_StrokeFont* font = (Fl_Glut_StrokeFont *)fontID;

  if (!string || ! *string) return;
  
 /*
  * Step through the string, drawing each character.
  * A newline will simply translate the next character's insertion
  * point back to the start of the line and down one line.
  */
  while ((c = *string++) != 0) {
    if (c < font->Quantity) {
      if (c == '\n') {
	glTranslatef(-length, -(float)(font->Height), 0.0);
	length = 0.0;
      } else  {
        /* Not an EOL, draw the bitmap character */
	const Fl_Glut_StrokeChar *schar = font->Characters[c];
	if (schar) {
	  const Fl_Glut_StrokeStrip *strip = schar->Strips;

	  for (i = 0; i < schar->Number; i++, strip++) {
	    glBegin(GL_LINE_STRIP);
	    for (j = 0; j < strip->Number; j++)
	      glVertex2f(strip->Vertices[j].X, strip->Vertices[j].Y);
	    glEnd();
	  }

	  length += schar->Right;
	  glTranslatef(schar->Right, 0.0, 0.0);
	}
      }
    }
  }
}

/*
 * Return the width in pixels of a stroke character
 */
int glutStrokeWidth( void* fontID, int character )
{
  const Fl_Glut_StrokeChar *schar;
  Fl_Glut_StrokeFont* font = (Fl_Glut_StrokeFont *)fontID;
  if (character < 0 || character >= font->Quantity) return 0;
  schar = font->Characters[ character ];

  return schar ? (int)(schar->Right + 0.5) : 0;
}

/*
 * Return the width of a string drawn using a stroke font
 */
int glutStrokeLength(void* fontID, const unsigned char* string) {
  unsigned char c;
  float length = 0.0;
  float this_line_length = 0.0;
  Fl_Glut_StrokeFont* font = (Fl_Glut_StrokeFont *)fontID;

  if (!string || ! *string) return 0;

  while ((c = *string++) != 0) {
    if (c < font->Quantity) {
      if (c == '\n') {
        /* EOL; reset the length of this line */
	if (length < this_line_length) length = this_line_length;
	this_line_length = 0.0;
      } else {
        /* Not an EOL, increment the length of this line */
	const Fl_Glut_StrokeChar *schar = font->Characters[c];
	if (schar) this_line_length += schar->Right;
      }
    }
  }

  if (length < this_line_length) length = this_line_length;

  return (int)(length + 0.5);
}

/*
 * Returns the height of a stroke font
 */
GLfloat glutStrokeHeight(void* fontID) {
  Fl_Glut_StrokeFont* font = (Fl_Glut_StrokeFont *)fontID;
  return font->Height;
}

#endif // HAVE_GL

//
// End of "$Id$".
//
