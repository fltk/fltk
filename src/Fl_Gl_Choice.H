//
// "$Id$"
//
// OpenGL definitions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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
// fl_create_gl_context takes a window (necessary only on Win32) and an
// Fl_Gl_Choice and returns a new OpenGL context. All contexts share
// display lists with each other.
//
// On X another fl_create_gl_context is provided to create it for any
// X visual.
//
// fl_set_gl_context makes the given OpenGL context current and makes
// it draw into the passed window. It tracks the current one context
// to avoid calling the context switching code when the same context
// is used, though it is a mystery to me why the GLX/WGL libraries
// don't do this themselves...
//
// fl_no_gl_context clears that cache so the next fl_set_gl_context is
// guaranteed to work.
//
// fl_delete_gl_context destroys the context.
//
// This code is used by Fl_Gl_Window, gl_start(), and gl_visual()

#ifndef Fl_Gl_Choice_H
#define Fl_Gl_Choice_H

// Warning: whatever GLContext is defined to must take exactly the same
// space in a structure as a void*!!!
#ifdef WIN32
#  include <FL/gl.h>
#  define GLContext HGLRC
#elif defined(__APPLE_QUARTZ__)
#  include <OpenGL/gl.h>
#ifdef __OBJC__
@class NSOpenGLPixelFormat;
@class NSOpenGLContext;
#else
class NSOpenGLPixelFormat;
class NSOpenGLContext;
#endif // __OBJC__
typedef NSOpenGLContext* FLOpenGLContextPtr;
#  define GLContext FLOpenGLContextPtr
#else
#  include <GL/glx.h>
#  define GLContext GLXContext
#  if ! defined(GLX_VERSION_1_3)
#    typedef void *GLXFBConfig;
#  endif
#endif

// Describes crap needed to create a GLContext.
class Fl_Gl_Choice {
  int mode;
  const int *alist;
  Fl_Gl_Choice *next;
public:
#ifdef WIN32
  int pixelformat;	// the visual to use
  PIXELFORMATDESCRIPTOR pfd; // some wgl calls need this thing
#elif defined(__APPLE_QUARTZ__)
  NSOpenGLPixelFormat* pixelformat;
#else
  XVisualInfo *vis;	// the visual to use
  Colormap colormap;	// a colormap for that visual
  GLXFBConfig best_fb;
#endif
  // Return one of these structures for a given gl mode.
  // The second argument is a glX attribute list, and is used if mode is
  // zero.  This is not supported on Win32:
  static Fl_Gl_Choice *find(int mode, const int *);
};

class Fl_Window;

#ifdef WIN32

GLContext fl_create_gl_context(Fl_Window*, const Fl_Gl_Choice*, int layer=0);

#elif defined(__APPLE_QUARTZ__)

GLContext fl_create_gl_context(Fl_Window*, const Fl_Gl_Choice*, int layer=0);

#else

GLContext fl_create_gl_context(XVisualInfo* vis);

//static inline
 GLContext fl_create_gl_context(Fl_Window*, const Fl_Gl_Choice* g);/* {
  return fl_create_gl_context(g->vis);
}*/

#endif

void fl_set_gl_context(Fl_Window*, GLContext);
void fl_no_gl_context();
void fl_delete_gl_context(GLContext);

#endif

//
// End of "$Id$".
//
