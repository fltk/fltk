//
// "$Id$"
//
// OpenGL visual selection code for the Fast Light Tool Kit (FLTK).
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

#include <config.h>
#if HAVE_GL

#  include <FL/Fl.H>
#  include <FL/x.H>
#  include <stdlib.h>
#  include "Fl_Gl_Choice.H"
#  include <FL/gl_draw.H>
#  include "flstring.h"
#  include <FL/fl_utf8.h>

#  ifdef WIN32
void fl_save_dc(HWND, HDC);
#elif defined(__APPLE__)
extern void gl_texture_reset();
#endif

#if defined(USE_X11)
static XVisualInfo *gl3_getvisual(const int *blist, GLXFBConfig *pbestFB)
{
  int glx_major, glx_minor;
  
  // FBConfigs were added in GLX version 1.3.
  if ( !glXQueryVersion( fl_display, &glx_major, &glx_minor ) ||
      ( ( glx_major == 1 ) && ( glx_minor < 3 ) ) || ( glx_major < 1 ) )
  {
    //printf("Invalid GLX version");
    return NULL;
  }
  
  //printf( "Getting matching framebuffer configs\n" );
  int fbcount;
  GLXFBConfig* fbc = glXChooseFBConfig(fl_display, DefaultScreen(fl_display), blist, &fbcount);
  if (!fbc)
  {
    //printf( "Failed to retrieve a framebuffer config\n" );
    return NULL;
  }
  //printf( "Found %d matching FB configs.\n", fbcount );
  
  // Pick the FB config/visual with the most samples per pixel
  int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
  
  int i;
  for (i=0; i<fbcount; ++i)
  {
    XVisualInfo *vi = glXGetVisualFromFBConfig( fl_display, fbc[i] );
    if ( vi )
    {
      int samp_buf, samples;
      glXGetFBConfigAttrib( fl_display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
      glXGetFBConfigAttrib( fl_display, fbc[i], GLX_SAMPLES       , &samples  );
      /*printf( "  Matching fbconfig %d, visual ID 0x%2lx: SAMPLE_BUFFERS = %d, SAMPLES = %d\n",
       i, vi -> visualid, samp_buf, samples );*/
      if ( best_fbc < 0 || (samp_buf && samples > best_num_samp) )
        best_fbc = i, best_num_samp = samples;
      if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
        worst_fbc = i, worst_num_samp = samples;
    }
    XFree( vi );
  }
  
  GLXFBConfig bestFbc = fbc[ best_fbc ];
  
  // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
  XFree( fbc );
  
  // Get a visual
  XVisualInfo *vi = glXGetVisualFromFBConfig( fl_display, bestFbc );
  *pbestFB = bestFbc;
  return vi;
}
#endif

static Fl_Gl_Choice *first;

// this assumes one of the two arguments is zero:
// We keep the list system in Win32 to stay compatible and interpret
// the list later...
Fl_Gl_Choice *Fl_Gl_Choice::find(int m, const int *alistp) {
  Fl_Gl_Choice *g;
  
  for (g = first; g; g = g->next)
    if (g->mode == m && g->alist == alistp) 
      return g;

#if defined(USE_X11)    
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
  XVisualInfo *visp = NULL;
  GLXFBConfig best_fb = NULL;
  if (m & FL_OPENGL3) {
    visp = gl3_getvisual((const int *)blist, &best_fb);
  }
  if (!visp) {
    visp = glXChooseVisual(fl_display, fl_screen, (int *)blist);
    if (!visp) {
#    if defined(GLX_VERSION_1_1) && defined(GLX_SGIS_multisample)
      if (m&FL_MULTISAMPLE) return find(m&~FL_MULTISAMPLE,0);
#    endif
      return 0;
    }
  }

#elif defined(__APPLE_QUARTZ__)
  fl_open_display();
  NSOpenGLPixelFormat* fmt = Fl_X::mode_to_NSOpenGLPixelFormat(m, alistp);
  if (!fmt) return 0;
  
#elif defined(WIN32)

// STR #3119: select pixel format with composition support
// ... and no more than 32 color bits (8 bits/color)
// Ref: PixelFormatDescriptor Object
// https://msdn.microsoft.com/en-us/library/cc231189.aspx
#if !defined(PFD_SUPPORT_COMPOSITION)
# define PFD_SUPPORT_COMPOSITION (0x8000)
#endif

#define DEBUG_PFD (0) // 1 = PFD selection debug output, 0 = no debug output

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

#if DEBUG_PFD
    printf("pfd #%d supports composition: %s\n", i, (pfd.dwFlags & PFD_SUPPORT_COMPOSITION) ? "yes" : "no");
    printf("    ... & PFD_GENERIC_FORMAT: %s\n", (pfd.dwFlags & PFD_GENERIC_FORMAT) ? "generic" : "accelerated");
    printf("    ... Overlay Planes      : %d\n", pfd.bReserved & 15);
    printf("    ... Color & Depth       : %d, %d\n", pfd.cColorBits, pfd.cDepthBits);
    if (pixelformat)
      printf("        current pixelformat : %d\n", pixelformat);
    fflush(stdout);
#endif // DEBUG_PFD

    // see if better than the one we have already:
    if (pixelformat) {
      // offering non-generic rendering is better (read: hardware acceleration)
      if (!(chosen_pfd.dwFlags & PFD_GENERIC_FORMAT) &&
          (pfd.dwFlags & PFD_GENERIC_FORMAT)) continue;
      // offering overlay is better:
      else if (!(chosen_pfd.bReserved & 15) && (pfd.bReserved & 15)) {}
      // otherwise prefer a format that supports composition (STR #3119)
      else if ((chosen_pfd.dwFlags & PFD_SUPPORT_COMPOSITION) &&
	       !(pfd.dwFlags & PFD_SUPPORT_COMPOSITION)) continue;
      // otherwise more bit planes is better, but no more than 32 (8 bits per channel):
      else if (pfd.cColorBits > 32 || chosen_pfd.cColorBits > pfd.cColorBits) continue;
      else if (chosen_pfd.cDepthBits > pfd.cDepthBits) continue;
    }
    pixelformat = i;
    chosen_pfd = pfd;
  }

#if DEBUG_PFD
  static int bb = 0;
  if (!bb) {
    bb = 1;
    printf("PFD_SUPPORT_COMPOSITION = 0x%x\n", PFD_SUPPORT_COMPOSITION);
  }
  printf("Chosen pixel format is %d\n", pixelformat);
  printf("Color bits = %d, Depth bits = %d\n", chosen_pfd.cColorBits, chosen_pfd.cDepthBits);
  printf("Pixel format supports composition: %s\n", (chosen_pfd.dwFlags & PFD_SUPPORT_COMPOSITION) ? "yes" : "no");
  fflush(stdout);
#endif // DEBUG_PFD

  if (!pixelformat) return 0;
#else
# error platform unsupported
#endif

  g = new Fl_Gl_Choice;
  g->mode = m;
  g->alist = alistp;
  g->next = first;
  first = g;

#if defined(USE_X11)
  g->vis = visp;
  g->best_fb = best_fb;

  if (/*MaxCmapsOfScreen(ScreenOfDisplay(fl_display,fl_screen))==1 && */
      visp->visualid == fl_visual->visualid &&
      !fl_getenv("MESA_PRIVATE_CMAP"))
    g->colormap = fl_colormap;
  else
    g->colormap = XCreateColormap(fl_display, RootWindow(fl_display,fl_screen),
				  visp->visual, AllocNone);
#  elif defined(WIN32)
  g->pixelformat = pixelformat;
  g->pfd = chosen_pfd;
#  elif defined(__APPLE_QUARTZ__)
  g->pixelformat = fmt;
#  else
#    error unsupported platform
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

#if defined(USE_X11)

static bool ctxErrorOccurred = false;
static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
  ctxErrorOccurred = true;
  return 0;
}


GLContext fl_create_gl_context(Fl_Window *window, const Fl_Gl_Choice* g) {
  GLContext shared_ctx = 0;
  if (context_list && nContext) shared_ctx = context_list[0];
  
  typedef GLContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLContext, Bool, const int*);
  
  // It is not necessary to create or make current to a context before calling glXGetProcAddressARB
  static glXCreateContextAttribsARBProc glXCreateContextAttribsARB =
  (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");
  
  GLContext ctx = 0;
  
  // Check for the GLX_ARB_create_context extension string and the function.
  // If either is not present, use GLX 1.3 context creation method.
  const char *glxExts = glXQueryExtensionsString(fl_display, fl_screen);
  if (g->best_fb && strstr(glxExts, "GLX_ARB_create_context") && glXCreateContextAttribsARB ) {
    int context_attribs[] =
    {
      GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
      GLX_CONTEXT_MINOR_VERSION_ARB, 2,
      //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
      //GLX_CONTEXT_PROFILE_MASK_ARB ,   GLX_CONTEXT_CORE_PROFILE_BIT_ARB ,
      None
    };
    ctxErrorOccurred = false;
    XErrorHandler oldHandler = XSetErrorHandler(&ctxErrorHandler);
    ctx = glXCreateContextAttribsARB( fl_display, g->best_fb, shared_ctx, True, context_attribs );
    // Sync to ensure any errors generated are processed.
    XSync( fl_display, False );
    if (ctxErrorOccurred) ctx = 0;
    XSetErrorHandler(oldHandler);
  }
  if (!ctx) { // use OpenGL 1-style context creation
    ctx = glXCreateContext(fl_display, g->vis, shared_ctx, true);
  }
  if (ctx)
    add_context(ctx);
//glXMakeCurrent(fl_display, fl_xid(window), ctx);printf("%s\n", glGetString(GL_VERSION));
  return ctx;
}


GLContext fl_create_gl_context(XVisualInfo* vis) {
  GLContext shared_ctx = 0;
  if (context_list && nContext) shared_ctx = context_list[0];
  GLContext context = glXCreateContext(fl_display, vis, shared_ctx, 1);
  if (context)
    add_context(context);
  return context;
}

#elif defined(WIN32)

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
    if (context_list && nContext) 
      wglShareLists(context_list[0], context);
    add_context(context);
  }
  return context;
}

#  elif defined(__APPLE_QUARTZ__)

GLContext fl_create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g, int layer) {
  GLContext context, shared_ctx = 0;
  if (context_list && nContext) shared_ctx = context_list[0];
  // resets the pile of string textures used to draw strings
  // necessary before the first context is created
  if (!shared_ctx) gl_texture_reset();
  context = Fl_X::create_GLcontext_for_window(g->pixelformat, shared_ctx, window);
  if (!context) return 0;
  add_context((GLContext)context);
  return (context);
}
#  else
#    error unsupported platform
#  endif

static GLContext cached_context;
static Fl_Window* cached_window;

void fl_set_gl_context(Fl_Window* w, GLContext context) {
  if (context != cached_context || w != cached_window) {
    cached_context = context;
    cached_window = w;
#  if defined(USE_X11)
    glXMakeCurrent(fl_display, fl_xid(w), context);
#  elif defined(WIN32)
    wglMakeCurrent(Fl_X::i(w)->private_dc, context);
#  elif defined(__APPLE_QUARTZ__)
    Fl_X::GLcontext_makecurrent(context);
#  else
#   error unsupported platform
#  endif
  }
}

void fl_no_gl_context() {
  cached_context = 0;
  cached_window = 0;
#  if defined(USE_X11)
  glXMakeCurrent(fl_display, 0, 0);
#  elif defined(WIN32)
  wglMakeCurrent(0, 0);
#  elif defined(__APPLE_QUARTZ__)
  Fl_X::GL_cleardrawable();
#  else
#    error unsupported platform
#  endif
}

void fl_delete_gl_context(GLContext context) {
  if (cached_context == context) fl_no_gl_context();
#  if defined(USE_X11)
  glXDestroyContext(fl_display, context);
#  elif defined(WIN32)
  wglDeleteContext(context);
#  elif defined(__APPLE_QUARTZ__)
  Fl_X::GLcontext_release(context);
#  else
#    error unsupported platform
#  endif
  del_context(context);
}

#endif // HAVE_GL


//
// End of "$Id$".
//
