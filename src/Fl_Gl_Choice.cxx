//
// "$Id: Fl_Gl_Choice.cxx,v 1.5.2.1 1999/09/16 05:34:25 bill Exp $"
//
// OpenGL visual selection code for the Fast Light Tool Kit (FLTK).
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

#include <config.h>
#if HAVE_GL

#include <FL/Fl.H>
#include <FL/x.H>
#include <stdlib.h>

#include "Fl_Gl_Choice.H"

static Fl_Gl_Choice *first;
GLXContext fl_first_context;

// this assummes one of the two arguments is zero:
// We keep the list system in Win32 to stay compatible and interpret
// the list later...
Fl_Gl_Choice *Fl_Gl_Choice::find(int mode, const int *alist) {
  Fl_Gl_Choice *g;
  
  for (g = first; g; g = g->next)
    if (g->mode == mode && g->alist == alist) 
      return g;

#ifndef WIN32    
  const int *blist;
  int list[32];
    
  if (alist)
    blist = alist;
  else {
    int n = 0;
    if (mode & FL_INDEX) {
      list[n++] = GLX_BUFFER_SIZE;
      list[n++] = 8; // glut tries many sizes, but this should work...
    } else {
      list[n++] = GLX_RGBA;
      list[n++] = GLX_GREEN_SIZE;
      list[n++] = (mode & FL_RGB8) ? 8 : 1;
      if (mode & FL_ALPHA) {
	list[n++] = GLX_ALPHA_SIZE;
	list[n++] = 1;
      }
      if (mode & FL_ACCUM) {
	list[n++] = GLX_ACCUM_GREEN_SIZE;
	list[n++] = 1;
	if (mode & FL_ALPHA) {
	  list[n++] = GLX_ACCUM_ALPHA_SIZE;
	  list[n++] = 1;
	}
      }
    }
    if (mode & FL_DOUBLE) {
      list[n++] = GLX_DOUBLEBUFFER;
    }
    if (mode & FL_DEPTH) {
      list[n++] = GLX_DEPTH_SIZE; list[n++] = 1;
    }
    if (mode & FL_STENCIL) {
      list[n++] = GLX_STENCIL_SIZE; list[n++] = 1;
    }
#if defined(GLX_VERSION_1_1) && defined(GLX_SGIS_multisample)
    if (mode & FL_MULTISAMPLE) {
      list[n++] = GLX_SAMPLES_SGIS;
      list[n++] = 4; // value Glut uses
    }
#endif
    list[n] = 0;
    blist = list;
  }
    
  fl_open_display();
  XVisualInfo *vis = glXChooseVisual(fl_display, fl_screen, (int *)blist);
  if (!vis) {
# if defined(GLX_VERSION_1_1) && defined(GLX_SGIS_multisample)
    if (mode&FL_MULTISAMPLE) return find(mode&~FL_MULTISAMPLE,0);
# endif
    return 0;
  }

#else

  PIXELFORMATDESCRIPTOR pfd = { 
    sizeof(PIXELFORMATDESCRIPTOR), 1, 
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
    PFD_TYPE_RGBA, 8 };

  if (mode & FL_INDEX) {
    pfd.iPixelType = PFD_TYPE_COLORINDEX;
    pfd.cColorBits = 8;
  } else {
    if (mode & FL_ALPHA) pfd.cAlphaBits = 8;
    if (mode & FL_ACCUM) {
      pfd.cAccumBits = 6;	// Wonko: I didn't find any documentation on those bits
      pfd.cAccumGreenBits = 1;	// Wonko: They don't seem to get any support yet (4/98)
      if (mode & FL_ALPHA) pfd.cAccumAlphaBits = 1;
    }
  }
  if (mode & FL_DOUBLE) pfd.dwFlags |= PFD_DOUBLEBUFFER;
  if (mode & FL_DEPTH) pfd.cDepthBits = 16;
  if (mode & FL_STENCIL) pfd.cStencilBits = 1;
  pfd.bReserved = 1; // always ask for overlay

#endif

  g = new Fl_Gl_Choice;
  g->mode = mode;
  g->alist = alist;
  g->next = first;
  first = g;

#ifdef WIN32
  memcpy(&g->pfd, &pfd, sizeof(PIXELFORMATDESCRIPTOR));
  g->d = ((mode&FL_DOUBLE) != 0);
  g->r = (mode & FL_INDEX);
  g->o = 0; // not an overlay
#else
  g->vis = vis;
  g->colormap = 0;
  int i;
  glXGetConfig(fl_display, vis, GLX_DOUBLEBUFFER, &i); g->d = i;
  glXGetConfig(fl_display, vis, GLX_RGBA, &i); g->r = i;
  glXGetConfig(fl_display, vis, GLX_LEVEL, &i); g->o = i;

  if (/*MaxCmapsOfScreen(ScreenOfDisplay(fl_display,fl_screen))==1 && */
      vis->visualid == fl_visual->visualid &&
      !getenv("MESA_PRIVATE_CMAP"))
    g->colormap = fl_colormap;
  else
    g->colormap = XCreateColormap(fl_display, RootWindow(fl_display,fl_screen),
				  vis->visual, AllocNone);
#endif

  return g;
}

#ifdef WIN32

HDC fl_private_dc(Fl_Window* w, int mode, Fl_Gl_Choice **gp) {
  Fl_X* i = Fl_X::i(w);
  if (!i->private_dc) {
    i->private_dc = GetDCEx(i->xid, 0, DCX_CACHE);
    Fl_Gl_Choice *g = Fl_Gl_Choice::find(mode, 0);
	if (gp) *gp = g;
    int pixelFormat = ChoosePixelFormat(i->private_dc, &g->pfd);
    if (!pixelFormat) {Fl::error("Insufficient GL support"); return NULL;}
    SetPixelFormat(i->private_dc, pixelFormat, &g->pfd);
#if USE_COLORMAP
    if (fl_palette) SelectPalette(i->private_dc, fl_palette, FALSE);
#endif
  }
  return i->private_dc;
}

#endif

static GLXContext cached_context;

static Fl_Window* cached_window;

void fl_set_gl_context(Fl_Window* w, GLXContext c) {
  if (c != cached_context || w != cached_window) {
    cached_context = c;
    cached_window = w;
#ifdef WIN32
    wglMakeCurrent(Fl_X::i(w)->private_dc, c);
#else
    glXMakeCurrent(fl_display, fl_xid(w), c);
#endif
  }
}

void fl_no_gl_context() {
  cached_context = 0;
  cached_window = 0;
#ifdef WIN32
  wglMakeCurrent(0, 0);
#else
  glXMakeCurrent(fl_display, 0, 0);
#endif
}

#endif

//
// End of "$Id: Fl_Gl_Choice.cxx,v 1.5.2.1 1999/09/16 05:34:25 bill Exp $".
//
