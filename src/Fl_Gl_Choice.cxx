// Internal interface to select glX visuals
// Called by Fl_Gl_Window.C and by gl_visual() (in gl_start.C)

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
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL /*| PFD_DEPTH_DONTCARE*/,
    PFD_TYPE_RGBA, 24 };

  if (mode & FL_INDEX) {
    pfd.iPixelType = PFD_TYPE_COLORINDEX;
    pfd.cColorBits = 8;
  } else {
    if (mode & FL_ALPHA) pfd.cAlphaBits = 8;
    if (mode & FL_ACCUM) {
      pfd.cAccumBits = 6;	// Wonko: I didn't find any documentation on those bits
      pfd.cAccumGreenBits = 1;// Wonko: They don't seem to get anny support yet (4/98)
      if (mode & FL_ALPHA) pfd.cAccumAlphaBits = 1;
    }
  }
  if (mode & FL_DOUBLE) pfd.dwFlags |= PFD_DOUBLEBUFFER;
  // if (!(mode & FL_DEPTH)) pfd.dwFlags = PFD_DEPTH_DONTCARE;
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

#endif
