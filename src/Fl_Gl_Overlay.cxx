// Fl_Gl_Overlay.C

// Methods on Fl_Gl_Window that create an overlay window.  Because
// many programs don't need the overlay, this is seperated into this
// source file so it is not linked in if not used.

// Under X this is done by creating another window, of class _Fl_Gl_Overlay
// which is a subclass of Fl_Gl_Window except it uses the overlay planes.
// A pointer to this is stored in the "overlay" pointer of the Fl_Gl_Window.

// Under win32 another GLX context is created to draw into the overlay
// and it is stored in into the "overlay" pointer.

// In both cases if overlay hardware is unavailable, the overlay is
// "faked" by drawing into the main layers.  This is indicated by
// setting overlay == this.

#include <config.h>
#if HAVE_GL

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/x.H>
#include "Fl_Gl_Choice.H"
#include <stdlib.h>

#if HAVE_GL_OVERLAY

#ifndef WIN32

extern XVisualInfo *fl_find_overlay_visual();
extern XVisualInfo *fl_overlay_visual;
extern Colormap fl_overlay_colormap;
extern unsigned long fl_transparent_pixel;
static Fl_Gl_Choice overlay_choice;
extern uchar fl_overlay;

class _Fl_Gl_Overlay : public Fl_Gl_Window {
  void draw();
public:
  void show();
  _Fl_Gl_Overlay(int x, int y, int w, int h) :
    Fl_Gl_Window(x,y,w,h) {
    overlay_choice.vis = fl_overlay_visual;
    overlay_choice.colormap = fl_overlay_colormap;
    overlay_choice.r = 0;
    overlay_choice.d = 0;
    overlay_choice.o = 1;
    g = &overlay_choice;
    deactivate();
  }
};

void _Fl_Gl_Overlay::draw() {
  if (damage() != FL_DAMAGE_EXPOSE) glClear(GL_COLOR_BUFFER_BIT);
  Fl_Gl_Window *w = (Fl_Gl_Window *)parent();
  uchar save_valid = w->valid_;
  w->valid_ = valid_;
  fl_overlay = 1;
  w->draw_overlay();
  fl_overlay = 0;
  valid_ = w->valid_;
  w->valid_ = save_valid;
}

void _Fl_Gl_Overlay::show() {
  if (shown()) {Fl_Gl_Window::show(); return;}
  fl_background_pixel = int(fl_transparent_pixel);
  Fl_Gl_Window::show();
  fl_background_pixel = -1;
  // find the outermost window to tell wm about the colormap:
  Fl_Window *w = window();
  for (;;) {Fl_Window *w1 = w->window(); if (!w1) break; w = w1;}
  XSetWMColormapWindows(fl_display, fl_xid(w), &(Fl_X::i(this)->xid), 1);
}

int Fl_Gl_Window::can_do_overlay() {
  return fl_find_overlay_visual() != 0;
}

#else // WIN32:

static int no_overlay_hardware;
int Fl_Gl_Window::can_do_overlay() {
  if (no_overlay_hardware) return 0;
  // need to write a test here...
  return 1;
}

extern GLXContext fl_first_context;

#endif

#else

int Fl_Gl_Window::can_do_overlay() {return 0;}

#endif

void Fl_Gl_Window::make_overlay() {
  if (!overlay) {
#if HAVE_GL_OVERLAY
#ifdef WIN32
    if (!no_overlay_hardware) {
      HDC hdc = fl_private_dc(this, mode_,&g);
      GLXContext context = wglCreateLayerContext(hdc, 1);
      if (!context) { // no overlay hardware
	no_overlay_hardware = 1;
      } else {
	// copy all colors except #0 into the overlay palette:
	COLORREF pcr[256];
	for (int i = 0; i < 256; i++) {
	  uchar r,g,b; Fl::get_color((Fl_Color)i,r,g,b);
	  pcr[i] = RGB(r,g,b);
	}
	wglSetLayerPaletteEntries(hdc, 1, 1, 255, pcr+1);
	wglRealizeLayerPalette(hdc, 1, TRUE);
	if (fl_first_context) wglShareLists(fl_first_context, context);
	else fl_first_context = context;
	overlay = context;
	valid(0);
	return;
      }
    }
#else
    if (can_do_overlay()) {
      _Fl_Gl_Overlay* o = new _Fl_Gl_Overlay(0,0,w(),h());
      overlay = o;
      add_resizable(*o);
      o->show();
      return;
    }
#endif
#endif
    overlay = this; // fake the overlay
  }
}

void Fl_Gl_Window::redraw_overlay() {
  if (!shown()) return;
  make_overlay();
#ifndef WIN32
  if (overlay != this)
    ((Fl_Gl_Window*)overlay)->redraw();
  else
#endif
    damage(FL_DAMAGE_OVERLAY);
}

void Fl_Gl_Window::make_overlay_current() {
  make_overlay();
#if HAVE_GL_OVERLAY
  if (overlay != this) {
#ifdef WIN32
    wglMakeCurrent(Fl_X::i(this)->private_dc, (GLXContext)overlay);
#else
    Fl_Gl_Window* w = (Fl_Gl_Window*)overlay;
    w->make_current();
#endif
  } else
#endif
    glDrawBuffer(GL_FRONT);
}

void Fl_Gl_Window::hide_overlay() {
#if HAVE_GL_OVERLAY
#ifdef WIN32
  // nothing needs to be done?  Or should it be erased?
#else
  if (overlay && overlay!=this) ((Fl_Gl_Window*)overlay)->hide();
#endif
#endif
}

#endif
