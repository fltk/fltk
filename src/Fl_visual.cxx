// Fl_visual.C
//
// Set the default visual according to passed switches:

#include <config.h>
#include <FL/Fl.H>
#include <FL/x.H>

#ifdef WIN32
int Fl::visual(int flags) {
  fl_GetDC(0);
  if (flags & FL_DOUBLE) return 0;
  if (!(flags & FL_INDEX) &&
    GetDeviceCaps(fl_gc,BITSPIXEL) <= 8) return 0;
  if ((flags & FL_RGB8) && GetDeviceCaps(fl_gc,BITSPIXEL)<24) return 0;
  return 1;
}
#else

#if USE_XDBE
#include <X11/extensions/Xdbe.h>
#endif

static int test_visual(XVisualInfo& v, int flags) {
  if (v.screen != fl_screen) return 0;
  if (!(flags & FL_INDEX)) {
    if (!v.red_mask) return 0; // detects static, true, and direct color
    if (v.depth <= 8) return 0; // fltk will work better in colormap mode
  }
  if (flags & FL_RGB8) {
    if (v.depth < 24) return 0;
  }
  // for now, fltk does not like colormaps of more than 8 bits:
  if (!v.red_mask && v.depth > 8) return 0;
#if USE_XDBE
  if (flags & FL_DOUBLE) {
    static XdbeScreenVisualInfo *xdbejunk;
    if (!xdbejunk) {
      int event_base, error_base;
      if (!XdbeQueryExtension(fl_display, &event_base, &error_base)) return 0;
      Drawable root = RootWindow(fl_display,fl_screen);
      int numscreens = 1;
      xdbejunk = XdbeGetVisualInfo(fl_display,&root,&numscreens);
      if (!xdbejunk) return 0;
    }
    for (int j = 0; ; j++) {
      if (j >= xdbejunk->count) return 0;
      if (xdbejunk->visinfo[j].visual == v.visualid) break;
    }
  }
#endif
  return 1;
}

int Fl::visual(int flags) {
#if USE_XDBE == 0
  if (flags & FL_DOUBLE) return 0;
#endif
  fl_open_display();
  // always use default if possible:
  if (test_visual(*fl_visual, flags)) return 1;
  // get all the visuals:
  XVisualInfo vTemplate;
  int num;
  XVisualInfo *visualList = XGetVisualInfo(fl_display, 0, &vTemplate, &num);
  // find all matches, use the one with greatest depth:
  XVisualInfo *found = 0;
  for (int i=0; i<num; i++) if (test_visual(visualList[i], flags)) {
    if (!found || found->depth < visualList[i].depth)
      found = &visualList[i];
  }
  if (!found) {XFree((void*)visualList); return 0;}
  fl_visual = found;
  fl_colormap = XCreateColormap(fl_display, RootWindow(fl_display,fl_screen),
				fl_visual->visual, AllocNone);
  return 1;
}

#endif
