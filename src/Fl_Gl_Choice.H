//
// "$Id: Fl_Gl_Choice.H,v 1.4 1999/01/07 19:17:20 mike Exp $"
//
// OpenGL definitions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

// Internal interface to set up OpenGL.
//
// A "Fl_Gl_Choice" is used to cache results of calling the
// OpenGL code for system-specific information needed to
// implement a "mode".
// For X this is a visual, and this must be called *before*
// the X window is created.
// For win32 this can be delayed to a more convienent time,
// as it only returns information for modifying a device
// context.
// This is used by Fl_Gl_Window, gl_start(), and gl_visual()

#ifndef Fl_Gl_Choice_H
#define Fl_Gl_Choice_H

#ifdef WIN32
# include <windows.h>
# include <FL/gl.h>
# define GLXContext HGLRC
# define GLX_BUFFER_SIZE      1
# define GLX_RGBA	      2
# define GLX_GREEN_SIZE	      3
# define GLX_ALPHA_SIZE	      4
# define GLX_ACCUM_GREEN_SIZE 5
# define GLX_ACCUM_ALPHA_SIZE 6
# define GLX_DOUBLEBUFFER     7
# define GLX_DEPTH_SIZE	      8
# define GLX_STENCIL_SIZE     9
#else
# include <GL/glx.h>
#endif

// one of these structures is returned:
class Fl_Gl_Choice {
  int mode;
  const int *alist;
  Fl_Gl_Choice *next;
public:
#ifdef WIN32
  PIXELFORMATDESCRIPTOR pfd;
#else
  XVisualInfo *vis;	// the visual to use
  Colormap colormap;	// a colormap to use
#endif
  uchar r,d,o;		// rgb mode, double buffered, overlay flags
  // either use mode flags from gl_draw.H or a literal glX int list.
  // one of the two arguments must be zero!
  static Fl_Gl_Choice *find(int mode, const int *);
};

#ifdef WIN32
// function to create and init the dc needed to draw OpenGL:
class Fl_Window;
HDC fl_private_dc(Fl_Window*, int, Fl_Gl_Choice **gp);
#endif

#endif

//
// End of "$Id: Fl_Gl_Choice.H,v 1.4 1999/01/07 19:17:20 mike Exp $".
//
