//
// "$Id: Fl_Gl_Window.cxx,v 1.12.2.17 2000/09/15 07:52:51 spitzak Exp $"
//
// OpenGL window code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2000 by Bill Spitzak and others.
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

#include <config.h>
#if HAVE_GL

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Gl_Window.H>
#include "Fl_Gl_Choice.H"
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////

// The symbol SWAP_TYPE defines what is in the back buffer after doing
// a glXSwapBuffers().

// The OpenGl documentation says that the contents of the backbuffer
// are "undefined" after glXSwapBuffers().  However, if we know what
// is in the backbuffers then we can save a good deal of time.  For
// this reason you can define some symbols to describe what is left in
// the back buffer.

// Having not found any way to determine this from glx (or wgl) I have
// resorted to letting the user specify it with an environment variable,
// GL_SWAP_TYPE, it should be equal to one of these symbols:

// contents of back buffer after glXSwapBuffers():
#define UNDEFINED 1 	// anything
#define SWAP 2		// former front buffer (same as unknown)
#define COPY 3		// unchanged
#define NODAMAGE 4	// unchanged even by X expose() events

static char SWAP_TYPE; // 0 = determine it from environment variable

////////////////////////////////////////////////////////////////

int Fl_Gl_Window::can_do(int a, const int *b) {
  return Fl_Gl_Choice::find(a,b) != 0;
}

void Fl_Gl_Window::show() {
#ifndef _WIN32
  if (!shown()) {
    if (!g) {
      g = Fl_Gl_Choice::find(mode_,alist);
      if (!g) {Fl::error("Insufficient GL support"); return;}
    }
    Fl_X::make_xid(this, g->vis, g->colormap);
    if (overlay && overlay != this) ((Fl_Gl_Window*)overlay)->show();
  }
#endif
  Fl_Window::show();
}

void Fl_Gl_Window::invalidate() {
  valid(0);
#ifndef _WIN32
  if (overlay) ((Fl_Gl_Window*)overlay)->valid(0);
#endif
}

int Fl_Gl_Window::mode(int m, const int *a) {
  if (m == mode_ && a == alist) return 0;
  mode_ = m; alist = a;
#ifdef _WIN32
  // destroy context and g:
  if (shown()) {hide(); show();}
#else
  // under X, if the visual changes we must make a new X window (!):
  if (shown()) {
    Fl_Gl_Choice *g1 = g;
    g = Fl_Gl_Choice::find(mode_,alist);
    if (!g || g->vis->visualid != g1->vis->visualid || g->d != g1->d) {
      hide(); show();
    }
  }
#endif
  return 1;
}

void Fl_Gl_Window::make_current() {
  if (!context) {
#ifdef _WIN32
    context = wglCreateContext(fl_private_dc(this, mode_,&g));
    if (fl_first_context) wglShareLists(fl_first_context, (GLXContext)context);
    else fl_first_context = (GLXContext)context;
#else
    context = glXCreateContext(fl_display, g->vis, fl_first_context, 1);
    if (!fl_first_context) fl_first_context = (GLXContext)context;
#endif
    valid(0);
  }
  fl_set_gl_context(this, (GLXContext)context);
#if defined(_WIN32) && USE_COLORMAP
  if (fl_palette) {
    fl_GetDC(fl_xid(this));
    SelectPalette(fl_gc, fl_palette, FALSE);
    RealizePalette(fl_gc);
  }
#endif // USE_COLORMAP
  glDrawBuffer(GL_BACK);
  current_ = this;
}

void Fl_Gl_Window::ortho() {
// Alpha NT seems to have a broken OpenGL that does not like negative coords:
#ifdef _M_ALPHA
  glLoadIdentity();
  glViewport(0, 0, w(), h());
  glOrtho(0, w(), 0, h(), -1, 1);
#else
  GLint v[2];
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, v);
  glLoadIdentity();
  glViewport(w()-v[0], h()-v[1], v[0], v[1]);
  glOrtho(w()-v[0], w(), h()-v[1], h(), -1, 1);
#endif
}

void Fl_Gl_Window::swap_buffers() {
#ifdef _WIN32
#if HAVE_GL_OVERLAY
  // Do not swap the overlay, to match GLX:
  wglSwapLayerBuffers(Fl_X::i(this)->private_dc, WGL_SWAP_MAIN_PLANE);
#else
  SwapBuffers(Fl_X::i(this)->private_dc);
#endif
#else
  glXSwapBuffers(fl_display, fl_xid(this));
#endif
}

#if HAVE_GL_OVERLAY && defined(_WIN32)
uchar fl_overlay; // changes how fl_color() works
int fl_overlay_depth = 0;
#endif

void Fl_Gl_Window::flush() {
  uchar save_valid = valid_;
#ifdef _WIN32
  // SGI 320 messes up overlay with user-defined cursors:
  bool fixcursor =
    Fl_X::i(this)->cursor && Fl_X::i(this)->cursor != fl_default_cursor;
  if (fixcursor) SetCursor(0);
#endif

#if HAVE_GL_OVERLAY && defined(_WIN32)
  // Draw into hardware overlay planes:
  if (overlay && overlay != this
      && (damage()&(FL_DAMAGE_OVERLAY|FL_DAMAGE_EXPOSE) || !save_valid)) {
    fl_set_gl_context(this, (GLXContext)overlay);
    if (fl_overlay_depth)
      wglRealizeLayerPalette(Fl_X::i(this)->private_dc, 1, TRUE);
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    fl_overlay = 1;
    draw_overlay();
    fl_overlay = 0;
    valid(save_valid);
    wglSwapLayerBuffers(Fl_X::i(this)->private_dc, WGL_SWAP_OVERLAY1);
    if (damage() == FL_DAMAGE_OVERLAY) { // main layer is undamaged
      if (fixcursor) SetCursor(Fl_X::i(this)->cursor);
      return;
    }
  }
#endif

  make_current();

  if (g->d) {

    if (!SWAP_TYPE) {
      SWAP_TYPE = UNDEFINED;
      const char* c = getenv("GL_SWAP_TYPE");
      if (c) {
	if (!strcmp(c,"COPY")) SWAP_TYPE = COPY;
	else if (!strcmp(c, "NODAMAGE")) SWAP_TYPE = NODAMAGE;
      }
    }

    if (SWAP_TYPE == NODAMAGE) {

      // don't draw if only overlay damage or expose events:
      if ((damage()&~(FL_DAMAGE_OVERLAY|FL_DAMAGE_EXPOSE)) || !save_valid)
	draw();
      swap_buffers();

    } else if (SWAP_TYPE == COPY) {

      // don't draw if only the overlay is damaged:
      if (damage() != FL_DAMAGE_OVERLAY || !save_valid) draw();
      swap_buffers();

    } else { // SWAP_TYPE == UNDEFINED

      // If we are faking the overlay, use CopyPixels to act like
      // SWAP_TYPE == COPY.  Otherwise overlay redraw is way too slow.
      if (overlay == this) {
	// don't draw if only the overlay is damaged:
	if (damage1_ || damage() != FL_DAMAGE_OVERLAY || !save_valid) draw();
	// we use a seperate context for the copy because rasterpos must be 0
	// and depth test needs to be off:
	static GLXContext ortho_context = 0;
	static Fl_Gl_Window* ortho_window = 0;
	int init = !ortho_context;
	if (init) {
#ifdef _WIN32
	  ortho_context = wglCreateContext(Fl_X::i(this)->private_dc);
#else
	  ortho_context =glXCreateContext(fl_display,g->vis,fl_first_context,1);
#endif
	}
	fl_set_gl_context(this, ortho_context);
	if (init || !save_valid || ortho_window != this) {
	  glDisable(GL_DEPTH_TEST);
	  glReadBuffer(GL_BACK);
	  glDrawBuffer(GL_FRONT);
	  glLoadIdentity();
	  glViewport(0, 0, w(), h());
	  glOrtho(0, w(), 0, h(), -1, 1);
	  glRasterPos2i(0,0);
	  ortho_window = this;
	}
	glCopyPixels(0,0,w(),h(),GL_COLOR);
	make_current(); // set current context back to draw overlay
	damage1_ = 0;

      } else {

	damage1_ = damage();
	clear_damage(~0); draw();
	swap_buffers();

      }

    }

    if (overlay==this) { // fake overlay in front buffer
      glDrawBuffer(GL_FRONT);
      draw_overlay();
      glDrawBuffer(GL_BACK);
      glFlush();
    }

  } else {	// single-buffered context is simpler:

    draw();
    if (overlay == this) draw_overlay();
    glFlush();

  }

#ifdef _WIN32
  if (fixcursor) SetCursor(Fl_X::i(this)->cursor);
#endif
  valid(1);
}

void Fl_Gl_Window::resize(int X,int Y,int W,int H) {
  if (W != w() || H != h()) {
    valid(0);
#ifndef _WIN32
    if (!resizable() && overlay && overlay != this)
      ((Fl_Gl_Window*)overlay)->resize(0,0,W,H);
#endif
  }
  Fl_Window::resize(X,Y,W,H);
}

void Fl_Gl_Window::hide() {
  if (context) {
    fl_no_gl_context();
    if (context != fl_first_context) {
#ifdef _WIN32
      wglDeleteContext((GLXContext)context);
#else
      glXDestroyContext(fl_display, (GLXContext)context);
#endif
    }
// This causes incompatibility with some OpenGL libraries
// I don't think this is not necessary in any case, right?
//#ifdef GLX_MESA_release_buffers
//    glXReleaseBuffersMESA(fl_display, fl_xid(this));
//#endif
    context = 0;
  }
#if HAVE_GL_OVERLAY && defined(_WIN32)
  if (overlay && overlay != this && (GLXContext)overlay != fl_first_context) {
    wglDeleteContext((GLXContext)overlay);
    overlay = 0;
  }
#endif
  Fl_Window::hide();
}

Fl_Gl_Window::~Fl_Gl_Window() {
  hide();
//  delete overlay; this is done by ~Fl_Group
}

void Fl_Gl_Window::init() {
  end(); // we probably don't want any children
  box(FL_NO_BOX);
  mode_ = FL_RGB | FL_DEPTH | FL_DOUBLE;
  alist = 0;
  context = 0;
  g = 0;
  overlay = 0;
}

void Fl_Gl_Window::draw_overlay() {}

#endif

//
// End of "$Id: Fl_Gl_Window.cxx,v 1.12.2.17 2000/09/15 07:52:51 spitzak Exp $".
//
