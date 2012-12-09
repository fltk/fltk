/* "$Id: $"
 *
 * Author: Jean-Marc Lienher ( http://oksid.ch )
 * Copyright 2000-2003 by O'ksi'D.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     http://www.fltk.org/COPYING.php
 *
 * Please report all bugs and problems on the following page:
 *
 *     http://www.fltk.org/str.php
 */

/*
 *   UTF-8 X test program (It contains MINIMAL code to support XIM !!!)
 *
 ****************
 * To test it do:
 ****************

kinput2 -canna
XMODIFIERS="@im=kinput2"; export XMODIFIERS
LANG=ja_JP; export LANG
./test

	to open a conversion window press "Shift space"
	type some keys.
	press space.
	select glyph with arrows keys.
	press return.
	press return.
	press "Shift space" to close the window

LANG=ar_AE; export LANG
LANG=he_IL; export LANG
export LANG=ja_JP; export XMODIFIERS="@im=kinput2"
export LANG=ja_JP; export XMODIFIERS="@im=nicolatter"
export LANG=ja_JP; export XMODIFIERS="@im=jmode"
export LANG=ja_JP; export XMODIFIERS="@im=htt"
export LANG=ko_KR; export XMODIFIERS="@im=Ami"
export LANG=zh_TW; export XMODIFIERS="@im=xcin-zh_TW"
export LANG=zh_TW; export XMODIFIERS="@im=xcin-zh_CN"
export LANG=C; export XMODIFIERS="@im=interxim"
**********************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include "Xutf8.h"
#include <X11/Xlocale.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/Xmd.h>

char *jp_txt = "é  UTF-8 e\xCC\x82=\xC3\xAA"
		"  \357\274\270\357\274\254\357\274\246\357\274"
		"\244\345\220\215\343\201\247\346\214\207    \345\256\232"
		"\343\201\231\343\202\213";

char *rtl_txt = "->e\xCC\x82=\xC3\xAA";

XIM xim_im = NULL;
XIC xim_ic = NULL;
static XIMStyles* xim_styles = NULL;
XUtf8FontStruct *fontset;
GC gc;
int x = 2;
int y = 40;

int main(int argc, char**argv) {
  char **missing_charset_list;
  int missing_charset_count;
  XGCValues xgcv;
  unsigned long mask;
  Display* dpy;
  int scr;
  Window w, root;
  XSetWindowAttributes set_attr;
  int i;
  XIMStyle *style;
  static char buf[128];
  KeySym keysym = 0;
  Status status;
  XWMHints wm_hints;
  XClassHint class_hints;
  XIMStyle input_style = 0;
  char **font_name_list;
  char *def_string;
  XFontStruct **font_struct_list;
  char **font_encoding_list;
  int nb_font;
  int len = 0;
  int no_xim = 0;

  printf ("A -> %c \n", XUtf8Tolower('A'));
  if (!setlocale(LC_ALL, ""))
    puts("locale not supported by C library, locale unchanged");

  if (!XSetLocaleModifiers(""))
    puts("X locale modifiers not supported, using default");
  
  dpy = XOpenDisplay(0);
  if (!dpy) { puts("cannot open display.\n"); exit(-1); }
  scr = DefaultScreen(dpy);
  root = RootWindow(dpy, scr);
  set_attr.event_mask = KeyPressMask|FocusChangeMask;
  set_attr.background_pixel = WhitePixel(dpy, DefaultScreen(dpy));
  set_attr.border_pixel = BlackPixel(dpy, DefaultScreen(dpy));
  w = XCreateWindow(dpy, root, 10,10,200,100,0, 
		    DefaultDepth(dpy, DefaultScreen(dpy)),
		    InputOutput, DefaultVisual(dpy, DefaultScreen(dpy)),
		    CWEventMask | CWBackPixel | CWBorderPixel, &set_attr);
  if (!w) {
    puts("cannot creat window.\n");
    exit(-1);
  }

  class_hints.res_name = "test";
  class_hints.res_class = "Test";
  wm_hints.input = True;
  wm_hints.flags = InputHint;

  XmbSetWMProperties(dpy, w, "test", "test", NULL, 0,
		     NULL, &wm_hints, &class_hints);

  XMapWindow(dpy, w);
  xim_im = XOpenIM(dpy, NULL, "test", "Test");
  if (!xim_im) { 
    puts("cannot Open Input Manager: Try default.\n"); 
    XSetLocaleModifiers("@im=");
    xim_im = XOpenIM(dpy, NULL, "test", "Test");
    if (!xim_im) {
      puts("Failed exiting.\n");
      exit(-1);
    }
  }
  XGetIMValues (xim_im, XNQueryInputStyle, &xim_styles, NULL, NULL);
  for (i = 0, style = xim_styles->supported_styles;
       i < xim_styles->count_styles; i++, style++) {
    if (i == 0 && *style == (XIMStatusNone|XIMPreeditNone)) {
      printf("this is not a XIM server !!!\n");
      no_xim = 1;
    }
    printf("input style : 0x%X\n", *style);
  }

  xim_ic = XCreateIC(xim_im,
		     XNInputStyle, 
		     (XIMPreeditNothing | XIMStatusNothing),
		     XNClientWindow, w,
		     XNFocusWindow, w,
		     NULL);
  if (!xim_ic) {
    puts("cannot create Input Context.\n");
    exit(-1);
  }
  XFree(xim_styles);
  XSetICFocus(xim_ic);

  /***************************************************************
   *  I don't recommend to use a font base name list similar 
   *  to the following one in a real application ;-) 
   *  You should use an iso8859-1 font, plus a single font for 
   *  your language.
   ***************************************************************/
  fontset = XCreateUtf8FontStruct(dpy, 
	  "-*-*-*-*-*-*-*-*-*-*-*-*-iso8858-3," /* not valid */
	  "-*-*-medium-r-*-*-*-*-*-*-*-*-iso8859-1,"
	  "-*-*-*-*-*-*-*-*-*-*-*-*-iso8859-6,"
	  "-*-*-*-*-*-*-*-*-*-*-*-*-iso8859-8,"
	  "-*-*-*-*-*-*-*-*-*-*-*-*-ksc5601.1987-0,"
	  "-*-symbol-*-*-*-*-*-*-*-*-*-*-adobe-fontspecific," 
	  "-*-*-*-*-*-*-*-*-*-*-*-*-iso8859-2,"
	  "-*-*-*-*-*-*-*-*-*-*-*-*-koi8-1,"
	  "-*-*-*-*-*-*-*-*-*-*-*-*-jisx0208.1983-0,"
	  "-*-*-*-*-*-*-*-*-*-*-*-*-jisx0212.1990-0,"
	  "-*-*-*-*-*-*-*-*-*-*-*-*-big5-0,"
	  "-*-*-*-*-*-*-*-*-*-*-*-*-jisx0201.1976-0,"
	  "-*-unifont-*-*-*-*-*-*-*-*-*-*-iso10646-1[0x300 0x400_0x500],"
	  "-*-*-*-*-*-*-*-*-*-*-*-*-*-*"); 

  /* THIS PART IS NOT REQUIERED */
  nb_font = fontset->nb_font;

  while (nb_font > 0) {
    nb_font--;
    if (fontset->fonts[nb_font]) {
      printf("encoding=\"\" fid=%d \n  %s\n", 
      /*     fontset->encodings[nb_font], */
	     fontset->fonts[nb_font]->fid,
	     fontset->font_name_list[nb_font]);
    }
  }
  /* END OF NOT REQUIERED PART*/

  mask = (GCForeground | GCBackground);
  xgcv.foreground = BlackPixel(dpy, DefaultScreen(dpy));
  xgcv.background = WhitePixel(dpy, DefaultScreen(dpy));

  gc = XCreateGC(dpy, w, mask, &xgcv);
  if (!gc) {
    puts("cannot create Graphic Context.\n");
    exit(-1);
  }

  /***************************************************************/
  while (1) {
    int filtered;
    static XEvent xevent;
    static XVaNestedList list1 = 0;
    int r;

    XNextEvent(dpy, &xevent);
    if (xevent.type == KeyPress) {
      XKeyEvent *e = (XKeyEvent*) &xevent;
      printf ("0x%X %d\n", e->state, e->keycode);
    }
    if (xevent.type == DestroyNotify) {
      /* XIM server has crashed */
      no_xim = 1;
      XSetLocaleModifiers("@im=");
      xim_im = XOpenIM(dpy, NULL, "test", "Test");
      if (xim_im) {
	xim_ic = XCreateIC(xim_im,
	                   XNInputStyle, (XIMPreeditNothing | XIMStatusNothing),
			   XNClientWindow, w,
			   XNFocusWindow, w,
			   NULL);
      } else {
	xim_ic = NULL;
      }
      if (!xim_ic) {
	puts("Crash recovery failed. exiting.\n");
	exit(-1);
      }
    }
    if (xevent.type != DestroyNotify) {
      filtered = XFilterEvent(&xevent, 0);
    }
    if (xevent.type == FocusOut && xim_ic) XUnsetICFocus(xim_ic);
    if (xevent.type == FocusIn && xim_ic) XSetICFocus(xim_ic);

    if (xevent.type == KeyPress && !filtered) {
      len = XUtf8LookupString(xim_ic, &xevent.xkey, 
			      buf, 127, &keysym, &status);

      if (len == 1 && buf[0] == '\b') {
	x -= XUtf8TextWidth(fontset, buf, len);
	XUtf8DrawImageString(dpy, w, fontset, gc, 
			     x, y, buf, len);
      } else if (len == 1 && buf[0] == '\r') {
	y += fontset->ascent + fontset->descent;
	x = 0;
	XCloseIM(xim_im);
      } else {
	XUtf8DrawImageString(dpy, w, fontset, gc, x, y, buf, len);
	x += XUtf8TextWidth(fontset, buf, len);
      }

      XUtf8DrawString(dpy, w, fontset, gc, 0, 20, jp_txt, strlen(jp_txt));
      XUtf8DrawString(dpy, w, fontset, gc, 50, 90, rtl_txt, strlen(rtl_txt));
      XUtf8DrawRtlString(dpy, w, fontset, gc, 50, 90, rtl_txt, strlen(rtl_txt));
      buf[len] = 0;
      printf("'%s' %d %x\n", buf, keysym, keysym);
      buf[0] = 0;
    }
    if (filtered) {
      printf("Dead key\n");
    }
  }
  XFreeUtf8FontStruct(dpy, fontset);
  return 0;
}

/*
 * End of "$Id$".
 */
