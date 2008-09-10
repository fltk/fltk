//
// "$Id$"
//
// OpenGL visual selection code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2005 by Bill Spitzak and others.
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

#include <config.h>
#if HAVE_GL

#  include <FL/Fl.H>
#  include <FL/x.H>
#  include <stdlib.h>
#  include "Fl_Gl_Choice.H"
#  include <FL/gl_draw.H>
#  include "flstring.h"
#  include <FL/fl_utf8.H>

#  ifdef __APPLE__
#    include <FL/Fl_Window.H>
#  endif

#  ifdef WIN32
void fl_save_dc(HWND, HDC);
#  endif

static Fl_Gl_Choice *first;

// this assummes one of the two arguments is zero:
// We keep the list system in Win32 to stay compatible and interpret
// the list later...
Fl_Gl_Choice *Fl_Gl_Choice::find(int m, const int *alistp) {
  Fl_Gl_Choice *g;
  
  for (g = first; g; g = g->next)
    if (g->mode == m && g->alist == alistp) 
      return g;

#  ifdef __APPLE_QD__
  const int *blist;
  int list[32];
    
  if (alistp)
    blist = alistp;
  else {
    int n = 0;
    if (m & FL_INDEX) {
      list[n++] = AGL_BUFFER_SIZE;
      list[n++] = 8; // glut tries many sizes, but this should work...
    } else {
      list[n++] = AGL_RGBA;
      list[n++] = AGL_GREEN_SIZE;
      list[n++] = (m & FL_RGB8) ? 8 : 1;
      if (m & FL_ALPHA) {
	list[n++] = AGL_ALPHA_SIZE;
	list[n++] = (m & FL_RGB8) ? 8 : 1;
      }
      if (m & FL_ACCUM) {
	list[n++] = AGL_ACCUM_GREEN_SIZE;
	list[n++] = 1;
	if (m & FL_ALPHA) {
	  list[n++] = AGL_ACCUM_ALPHA_SIZE;
	  list[n++] = 1;
	}
      }
    }
    if (m & FL_DOUBLE) {
      list[n++] = AGL_DOUBLEBUFFER;
    }
    if (m & FL_DEPTH) {
      list[n++] = AGL_DEPTH_SIZE; list[n++] = 24;
    }
    if (m & FL_STENCIL) {
      list[n++] = AGL_STENCIL_SIZE; list[n++] = 1;
    }
#    ifdef AGL_STEREO
    if (m & FL_STEREO) {
      list[n++] = AGL_STEREO;
    }
#    endif
    list[n] = AGL_NONE;
    blist = list;
  }
  fl_open_display();
  AGLPixelFormat fmt = aglChoosePixelFormat(NULL, 0, (GLint*)blist);
  if (!fmt) return 0;

#elif defined(__APPLE_QUARTZ__)
  // warning: the Quartz version should probably use Core GL (CGL) instead of AGL
  const int *blist;
  int list[32];
   
  if (alistp)
    blist = alistp;
  else {
    int n = 0;
    if (m & FL_INDEX) {
      list[n++] = AGL_BUFFER_SIZE;
      list[n++] = 8; // glut tries many sizes, but this should work...
    } else {
      list[n++] = AGL_RGBA;
      list[n++] = AGL_GREEN_SIZE;
      list[n++] = (m & FL_RGB8) ? 8 : 1;
      if (m & FL_ALPHA) {
        list[n++] = AGL_ALPHA_SIZE;
        list[n++] = (m & FL_RGB8) ? 8 : 1;
      }
      if (m & FL_ACCUM) {
        list[n++] = AGL_ACCUM_GREEN_SIZE;
        list[n++] = 1;
        if (m & FL_ALPHA) {
          list[n++] = AGL_ACCUM_ALPHA_SIZE;
          list[n++] = 1;
        }
      }
    }
    if (m & FL_DOUBLE) {
      list[n++] = AGL_DOUBLEBUFFER;
    }
    if (m & FL_DEPTH) {
      list[n++] = AGL_DEPTH_SIZE; list[n++] = 24;
    }
    if (m & FL_STENCIL) {
      list[n++] = AGL_STENCIL_SIZE; list[n++] = 1;
    }
#    ifdef AGL_STEREO
    if (m & FL_STEREO) {
      list[n++] = AGL_STEREO;
    }
#    endif
    list[n] = AGL_NONE;
    blist = list;
  }
  fl_open_display();
  AGLPixelFormat fmt = aglChoosePixelFormat(NULL, 0, (GLint*)blist);
  if (!fmt) return 0;
  
#  elif !defined(WIN32)    

  const int *blist;
  int list[32];
    
  if (alistp)
    blist = alistp;
  else {
    int n = 0;
    if (m & FL_INDEX) {
      list[n++] = GLX_BUFFER_SIZE;
      list[n++] = 8; // glut tries many sizes, but this should work...
    } else {
      list[n++] = GLX_RGBA;
      list[n++] = GLX_GREEN_SIZE;
      list[n++] = (m & FL_RGB8) ? 8 : 1;
      if (m & FL_ALPHA) {
	list[n++] = GLX_ALPHA_SIZE;
	list[n++] = (m & FL_RGB8) ? 8 : 1;
      }
      if (m & FL_ACCUM) {
	list[n++] = GLX_ACCUM_GREEN_SIZE;
	list[n++] = 1;
	if (m & FL_ALPHA) {
	  list[n++] = GLX_ACCUM_ALPHA_SIZE;
	  list[n++] = 1;
	}
      }
    }
    if (m & FL_DOUBLE) {
      list[n++] = GLX_DOUBLEBUFFER;
    }
    if (m & FL_DEPTH) {
      list[n++] = GLX_DEPTH_SIZE; list[n++] = 1;
    }
    if (m & FL_STENCIL) {
      list[n++] = GLX_STENCIL_SIZE; list[n++] = 1;
    }
    if (m & FL_STEREO) {
      list[n++] = GLX_STEREO;
    }
#    if defined(GLX_VERSION_1_1) && defined(GLX_SGIS_multisample)
    if (m & FL_MULTISAMPLE) {
      list[n++] = GLX_SAMPLES_SGIS;
      list[n++] = 4; // value Glut uses
    }
#    endif
    list[n] = 0;
    blist = list;
  }
    
  fl_open_display();
  XVisualInfo *visp = glXChooseVisual(fl_display, fl_screen, (int *)blist);
  if (!visp) {
#    if defined(GLX_VERSION_1_1) && defined(GLX_SGIS_multisample)
    if (m&FL_MULTISAMPLE) return find(m&~FL_MULTISAMPLE,0);
#    endif
    return 0;
  }

#  else

  // Replacement for ChoosePixelFormat() that finds one with an overlay
  // if possible:
  if (!fl_gc) fl_GetDC(0);
  int pixelformat = 0;
  PIXELFORMATDESCRIPTOR chosen_pfd;
  for (int i = 1; ; i++) {
    PIXELFORMATDESCRIPTOR pfd;
    if (!DescribePixelFormat(fl_gc, i, sizeof(pfd), &pfd)) break;
    // continue if it does not satisfy our requirements:
    if (~pfd.dwFlags & (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL)) continue;
    if (pfd.iPixelType != ((m&FL_INDEX)?PFD_TYPE_COLORINDEX:PFD_TYPE_RGBA)) continue;
    if ((m & FL_ALPHA) && !pfd.cAlphaBits) continue;
    if ((m & FL_ACCUM) && !pfd.cAccumBits) continue;
    if ((!(m & FL_DOUBLE)) != (!(pfd.dwFlags & PFD_DOUBLEBUFFER))) continue;
    if ((!(m & FL_STEREO)) != (!(pfd.dwFlags & PFD_STEREO))) continue;
    if ((m & FL_DEPTH) && !pfd.cDepthBits) continue;
    if ((m & FL_STENCIL) && !pfd.cStencilBits) continue;
    // see if better than the one we have already:
    if (pixelformat) {
      // offering non-generic rendering is better (read: hardware accelleration)
      if (!(chosen_pfd.dwFlags & PFD_GENERIC_FORMAT) &&
          (pfd.dwFlags & PFD_GENERIC_FORMAT)) continue;
      // offering overlay is better:
      else if (!(chosen_pfd.bReserved & 15) && (pfd.bReserved & 15)) {}
      // otherwise more bit planes is better:
      else if (chosen_pfd.cColorBits > pfd.cColorBits) continue;
      else if (chosen_pfd.cDepthBits > pfd.cDepthBits) continue;
    }
    pixelformat = i;
    chosen_pfd = pfd;
  }
  //printf("Chosen pixel format is %d\n", pixelformat);
  if (!pixelformat) return 0;

#  endif

  g = new Fl_Gl_Choice;
  g->mode = m;
  g->alist = alistp;
  g->next = first;
  first = g;

#  ifdef WIN32
  g->pixelformat = pixelformat;
  g->pfd = chosen_pfd;
#  elif defined(__APPLE_QD__)
  g->pixelformat = fmt;
#  elif defined(__APPLE_QUARTZ__)
  // warning: the Quartz version should probably use Core GL (CGL) instead of AGL
  g->pixelformat = fmt;
#  else
  g->vis = visp;

  if (/*MaxCmapsOfScreen(ScreenOfDisplay(fl_display,fl_screen))==1 && */
      visp->visualid == fl_visual->visualid &&
      !fl_getenv("MESA_PRIVATE_CMAP"))
    g->colormap = fl_colormap;
  else
    g->colormap = XCreateColormap(fl_display, RootWindow(fl_display,fl_screen),
				  visp->visual, AllocNone);
#  endif

  return g;
}

static GLContext *context_list = 0;
static int nContext = 0, NContext = 0;

static void add_context(GLContext ctx) {
  if (!ctx) return;
  if (nContext==NContext) {
    if (!NContext) NContext = 8;
    NContext *= 2;
    context_list = (GLContext*)realloc(
      context_list, NContext*sizeof(GLContext));
  }
  context_list[nContext++] = ctx;
}

static void del_context(GLContext ctx) {
  int i; 
  for (i=0; i<nContext; i++) {
    if (context_list[i]==ctx) {
      memmove(context_list+i, context_list+i+1,
        (nContext-i-1) * sizeof(GLContext));
      context_list[--nContext] = 0;
      break;
    }
  }
  if (!nContext) gl_remove_displaylist_fonts();
}

#  ifdef WIN32

GLContext fl_create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g, int layer) {
  Fl_X* i = Fl_X::i(window);
  HDC hdc = i->private_dc;
  if (!hdc) {
    hdc = i->private_dc = GetDCEx(i->xid, 0, DCX_CACHE);
    fl_save_dc(i->xid, hdc);
    SetPixelFormat(hdc, g->pixelformat, (PIXELFORMATDESCRIPTOR*)(&g->pfd));
#    if USE_COLORMAP
    if (fl_palette) SelectPalette(hdc, fl_palette, FALSE);
#    endif
  }
  GLContext context =
    layer ? wglCreateLayerContext(hdc, layer) : wglCreateContext(hdc);
  if (context) {
    if (context_list && context_list[0]) 
      wglShareLists(context_list[0], context);
    add_context(context);
  }
  return context;
}

#  elif defined(__APPLE_QD__)
GLContext fl_create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g, int layer) {
    GLContext context, shared_ctx = context_list ? context_list[0] : 0;
    context = aglCreateContext( g->pixelformat, shared_ctx);
    if (!context) return 0;
    add_context((GLContext)context);
    if ( window->parent() ) {
      Rect wrect; GetWindowPortBounds( fl_xid(window), &wrect );
      GLint rect[] = { window->x(), wrect.bottom-window->h()-window->y(), window->w(), window->h() }; 
      aglSetInteger( (GLContext)context, AGL_BUFFER_RECT, rect );
      aglEnable( (GLContext)context, AGL_BUFFER_RECT );
    }
    aglSetDrawable( context, GetWindowPort( fl_xid(window) ) );
    return (context);
}
#  elif defined(__APPLE_QUARTZ__)
  // warning: the Quartz version should probably use Core GL (CGL) instead of AGL
  GLContext fl_create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g, int layer) {
    GLContext context, shared_ctx = context_list ? context_list[0] : 0;
    context = aglCreateContext( g->pixelformat, shared_ctx);
    if (!context) return 0;
    add_context((GLContext)context);
    if ( window->parent() ) {
      Rect wrect; GetWindowPortBounds( fl_xid(window), &wrect );
      GLint rect[] = { window->x(), wrect.bottom-window->h()-window->y(), window->w(), window->h() };
      aglSetInteger( (GLContext)context, AGL_BUFFER_RECT, rect );
      aglEnable( (GLContext)context, AGL_BUFFER_RECT );
    }
    aglSetDrawable( context, GetWindowPort( fl_xid(window) ) );
    return (context);
}
#  else

GLContext fl_create_gl_context(XVisualInfo* vis) {
  GLContext shared_ctx = context_list ? context_list[0] : 0;
  GLContext context = glXCreateContext(fl_display, vis, shared_ctx, 1);
  if (context)
    add_context(context);
  return context;
}

#  endif

static GLContext cached_context;
static Fl_Window* cached_window;

void fl_set_gl_context(Fl_Window* w, GLContext context) {
  if (context != cached_context || w != cached_window) {
    cached_context = context;
    cached_window = w;
#  ifdef WIN32
    wglMakeCurrent(Fl_X::i(w)->private_dc, context);
#  elif defined(__APPLE_QD__)
    if ( w->parent() ) { //: resize our GL buffer rectangle
      Rect wrect; GetWindowPortBounds( fl_xid(w), &wrect );
      GLint rect[] = { w->x(), wrect.bottom-w->h()-w->y(), w->w(), w->h() };
      aglSetInteger( context, AGL_BUFFER_RECT, rect );
      aglEnable( context, AGL_BUFFER_RECT );
    }
    aglSetDrawable(context, GetWindowPort( fl_xid(w) ) ); 
    aglSetCurrentContext(context);
#  elif defined(__APPLE_QUARTZ__)
    // warning: the Quartz version should probably use Core GL (CGL) instead of AGL
    if ( w->parent() ) { //: resize our GL buffer rectangle
      Rect wrect; GetWindowPortBounds( fl_xid(w), &wrect );
      GLint rect[] = { w->x(), wrect.bottom-w->h()-w->y(), w->w(), w->h() };
      aglSetInteger( context, AGL_BUFFER_RECT, rect );
      aglEnable( context, AGL_BUFFER_RECT );
    }
    aglSetDrawable(context, GetWindowPort( fl_xid(w) ) );
    aglSetCurrentContext(context);
#  else
    glXMakeCurrent(fl_display, fl_xid(w), context);
#  endif
  }
}

void fl_no_gl_context() {
  cached_context = 0;
  cached_window = 0;
#  ifdef WIN32
  wglMakeCurrent(0, 0);
#  elif defined(__APPLE_QD__)
  aglSetCurrentContext(0);
#  elif defined(__APPLE_QUARTZ__)
  // warning: the Quartz version should probably use Core GL (CGL) instead of AGL
  aglSetCurrentContext(0);
#  else
  glXMakeCurrent(fl_display, 0, 0);
#  endif
}

void fl_delete_gl_context(GLContext context) {
  if (cached_context == context) fl_no_gl_context();
#  ifdef WIN32
  wglDeleteContext(context);
#  elif defined(__APPLE_QD__)
  aglSetCurrentContext( NULL );
  aglSetDrawable( context, NULL );    
  aglDestroyContext( context );
#  elif defined(__APPLE_QUARTZ__)
  // warning: the Quartz version should probably use Core GL (CGL) instead of AGL
  aglSetCurrentContext( NULL );
  aglSetDrawable( context, NULL );
  aglDestroyContext( context );
#  else
  glXDestroyContext(fl_display, context);
#  endif
  del_context(context);
}

#endif // HAVE_GL


//
// End of "$Id$".
//
