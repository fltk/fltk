//
// "$Id$"
//
// OpenGL definitions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

// Internal interface to set up OpenGL.
//
// A "Fl_Gl_Choice" is created from an OpenGL mode and holds information
// necessary to create a window (on X) and to create an OpenGL "context"
// (on both X and Win32).
//
// create_gl_context takes a window (necessary only on Win32) and an
// Fl_Gl_Choice and returns a new OpenGL context. All contexts share
// display lists with each other.
//
// On X another create_gl_context is provided to create it for any
// X visual.
//
// set_gl_context makes the given OpenGL context current and makes
// it draw into the passed window. It tracks the current one context
// to avoid calling the context switching code when the same context
// is used, though it is a mystery to me why the GLX/WGL libraries
// don't do this themselves...
//
// delete_gl_context destroys the context.
//
// This code is used by Fl_Gl_Window, gl_start(), and gl_visual()

#ifndef Fl_Gl_Choice_H
#define Fl_Gl_Choice_H

#ifdef FL_CFG_GFX_QUARTZ
#  include <OpenGL/gl.h>
#  define FL_GL_CHOICE_PLATFORM_SPECIFIC_MEMBERS  void* pixelformat;
#endif // FL_CFG_GFX_QUARTZ


#ifdef FL_CFG_GFX_XLIB
#  include <GL/glx.h>
#  define FL_GL_CHOICE_PLATFORM_SPECIFIC_MEMBERS  \
      XVisualInfo *vis; /* the visual to use */ \
      Colormap colormap; // a colormap for that visual
#endif // FL_CFG_GFX_XLIB*/


#ifdef FL_CFG_GFX_GDI
#  include <FL/gl.h>
#  define FL_GL_CHOICE_PLATFORM_SPECIFIC_MEMBERS  \
      int pixelformat;	     /* the visual to use */ \
      PIXELFORMATDESCRIPTOR pfd; // some wgl calls need this thing
#endif // FL_CFG_GFX_GDI


#if defined(FL_PORTING) && !defined(FL_GL_CHOICE_PLATFORM_SPECIFIC_MEMBERS)
#  pragma message "Define platform-specific members of the Fl_Gl_Choice class"
#  define FL_GL_CHOICE_PLATFORM_SPECIFIC_MEMBERS
#endif

// Describes crap needed to create a GLContext.
class Fl_Gl_Choice {
  friend class Fl_Gl_Window_Driver;
  int mode;
  const int *alist;
  Fl_Gl_Choice *next;
public:
  Fl_Gl_Choice(int m, const int *alistp, Fl_Gl_Choice *n) : mode(m), alist(alistp), next(n) {}
  FL_GL_CHOICE_PLATFORM_SPECIFIC_MEMBERS
};

#undef FL_GL_CHOICE_PLATFORM_SPECIFIC_MEMBERS

#endif // Fl_Gl_Choice_H

//
// End of "$Id$".
//
