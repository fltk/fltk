//
// Visual list utility for the Fast Light Tool Kit (FLTK).
//
// List all the visuals on the screen, and dumps anything interesting
// about them to stdout.
//
// Does not use FLTK under X11.
//
// This file may be #included in another program to make a function to
// call to list the visuals.  Fl.H must be included first to indicate this.
//
// Copyright 1998-2021 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#ifndef Fl_H
#  define NEED_MAIN 1 // when not included by another FLTK program
#endif

#include <FL/platform.H> // for FLTK_USE_X11

#ifdef FLTK_USE_X11

#include <config.h>

#define HAVE_MULTIBUF 0

#ifdef NEED_MAIN

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>

Display *fl_display;
int fl_screen;
const char *dname;
void fl_open_display() {
  fl_display = XOpenDisplay(dname);
  if (!fl_display) {
    fprintf(stderr,"Can't open display: %s\n",XDisplayName(dname));
    exit(1);
  }
  fl_screen = DefaultScreen(fl_display);
}

#endif // NEED_MAIN

const char *ClassNames[] = {
  "StaticGray ",
  "GrayScale  ",
  "StaticColor",
  "PseudoColor",
  "TrueColor  ",
  "DirectColor"
};

// SERVER_OVERLAY_VISUALS property element:
typedef struct _OverlayInfo {
  long overlay_visual;
  long transparent_type;
  long value;
  long layer;
} OverlayInfo;

#if HAVE_MULTIBUF
#include <X11/extensions/multibuf.h>
#endif

static void print_mask(XVisualInfo* p) {
  int n = 0;
  int what = 0;
  int print_anything = 0;
  char buf[20];
  char *q = buf;
  *q = 0;
  int b; unsigned int m; for (b=32,m=0x80000000; ; b--,m>>=1) {
    int new_what = 0;
    if (p->red_mask&m) new_what = 'r';
    else if (p->green_mask&m) new_what = 'g';
    else if (p->blue_mask&m) new_what = 'b';
    else new_what = '?';
    if (new_what != what) {
      if (what && (what != '?' || print_anything)) {
        q += snprintf(q, sizeof(buf) - (q-buf), "%d%c", n, what);
        print_anything = 1;
      }
      what = new_what;
      n = 1;
    } else {
      n++;
    }
    if (!b) break;
  }
  printf("%7s", buf);
}

void list_visuals() {
  fl_open_display();
  XVisualInfo vTemplate;
  int num;
  XVisualInfo *visualList = XGetVisualInfo(fl_display,0,&vTemplate,&num);

  XPixmapFormatValues *pfvlist;
  static int numpfv;
  pfvlist = XListPixmapFormats(fl_display, &numpfv);

  OverlayInfo *overlayInfo = 0;
  int numoverlayinfo = 0;
  Atom overlayVisualsAtom = XInternAtom(fl_display,"SERVER_OVERLAY_VISUALS",1);
  if (overlayVisualsAtom) {
    unsigned long sizeData, bytesLeft;
    Atom actualType;
    int actualFormat;
    if (!XGetWindowProperty(fl_display, RootWindow(fl_display, fl_screen),
                           overlayVisualsAtom, 0L, 10000L, False,
                           overlayVisualsAtom, &actualType, &actualFormat,
                           &sizeData, &bytesLeft,
                           (unsigned char **) &overlayInfo))
      numoverlayinfo = int(sizeData/4);
  }

#if HAVE_MULTIBUF
  int event_base, error_base;
  XmbufBufferInfo *mbuf, *sbuf;
  int nmbuf = 0, nsbuf = 0;
  if (XmbufQueryExtension(fl_display,&event_base, &error_base)) {
    XmbufGetScreenInfo(fl_display,RootWindow(fl_display,fl_screen),
                       &nmbuf, &mbuf, &nsbuf, &sbuf);
  }
#endif

  for (int i=0; i<num; i++) {
    XVisualInfo *p = visualList+i;

    XPixmapFormatValues *pfv;
    for (pfv = pfvlist; ; pfv++) {
      if (pfv >= pfvlist+numpfv) {pfv = 0; break;} // should not happen!
      if (pfv->depth == p->depth) break;
    }

    int j = pfv ? pfv->bits_per_pixel : 0;
    printf(" %2ld: %s %2d/%d", p->visualid, ClassNames[p->c_class],
           p->depth, j);
    if (j < 10) putchar(' ');

    print_mask(p);

    for (j=0; j<numoverlayinfo; j++) {
      OverlayInfo *o = &overlayInfo[j];
      if (o->overlay_visual == long(p->visualid)) {
        printf(" overlay(");
        if (o->transparent_type==1) printf("transparent pixel %ld, ",o->value);
        else if (o->transparent_type==2) printf("transparent mask %ld, ",o->value);
        else printf("opaque, ");
        printf("layer %ld)", o->layer);
      }
    }

#if HAVE_MULTIBUF
    for (j=0; j<nmbuf; j++) {
      XmbufBufferInfo *m = &mbuf[j];
      if (m->visualid == p->visualid)
        printf(" multibuffer(%d)", m->max_buffers);
    }
    for (j=0; j<nsbuf; j++) {
      XmbufBufferInfo *m = &sbuf[j];
      if (m->visualid == p->visualid)
        printf(" stereo multibuffer(%d)", m->max_buffers);
    }
#endif

    if (p->visualid==XVisualIDFromVisual(DefaultVisual(fl_display,fl_screen)))
      printf(" (default visual)");

    putchar('\n');
  }
  if ( overlayInfo ) { XFree(overlayInfo); overlayInfo = 0; }
}

#endif // FLTK_USE_X11

#ifdef NEED_MAIN

#  ifndef FLTK_USE_X11
#    include <FL/fl_ask.H>
#  endif

int main(int argc, char** argv) {
#  ifdef FLTK_USE_X11
  if (argc == 1);
  else if (argc == 2 && argv[1][0]!='-') dname = argv[1];
  else {fprintf(stderr,"usage: %s <display>\n",argv[0]); exit(1);}
  list_visuals();
  return 0;
#  else
  fl_alert("Currently, this program works only under X.");
  return 1;
#  endif // FLTK_USE_X11
}

#endif // NEED_MAIN
