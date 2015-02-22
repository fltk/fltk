/* "$Id$"
 *
 * Author: Jean-Marc Lienher ( http://oksid.ch )
 * Copyright 2000-2010 by O'ksi'D.
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

#if ! ( defined(_Xutf8_h) || defined(FL_DOXYGEN) )
#define _Xutf8_h

#  ifdef __cplusplus
extern "C" {
#  endif

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include <X11/Xutil.h>
#include <FL/Fl_Export.H>

typedef struct {
	int nb_font;
	char **font_name_list;
	int *encodings;
	XFontStruct **fonts;
	Font fid;
	int ascent;
	int descent;
	int *ranges;
} XUtf8FontStruct;

XUtf8FontStruct *
XCreateUtf8FontStruct (
	Display         *dpy,
	const char      *base_font_name_list);

void
XUtf8DrawString(
        Display         	*display,
        Drawable        	d,
        XUtf8FontStruct  *font_set,
        GC              	gc,
        int             	x,
        int             	y,
        const char      	*string,
        int             	num_bytes);

void
XUtf8_measure_extents(
        Display         	*display,
        Drawable        	d,
        XUtf8FontStruct  *font_set,
        GC              	gc,
        int             	*xx,
        int             	*yy,
        int             	*ww,
        int             	*hh,
        const char      	*string,
        int             	num_bytes);

void
XUtf8DrawRtlString(
        Display         	*display,
        Drawable        	d,
        XUtf8FontStruct  *font_set,
        GC              	gc,
        int             	x,
        int             	y,
        const char      	*string,
        int             	num_bytes);

void
XUtf8DrawImageString(
        Display         *display,
        Drawable        d,
        XUtf8FontStruct         *font_set,
        GC              gc,
        int             x,
        int             y,
        const char      *string,
        int             num_bytes);

int
XUtf8TextWidth(
        XUtf8FontStruct  *font_set,
        const char      	*string,
        int             	num_bytes);
int
XUtf8UcsWidth(
	XUtf8FontStruct  *font_set,
	unsigned int            ucs);

FL_EXPORT int
fl_XGetUtf8FontAndGlyph(
        XUtf8FontStruct  *font_set,
        unsigned int            ucs,
        XFontStruct     **fnt,
        unsigned short  *id);

void
XFreeUtf8FontStruct(
        Display         	*dpy,
        XUtf8FontStruct 	*font_set);


int
XConvertUtf8ToUcs(
	const unsigned char 	*buf,
	int 			len,
	unsigned int 		*ucs);

int
XConvertUcsToUtf8(
	unsigned int 		ucs,
	char 			*buf);

int
XUtf8CharByteLen(
	const unsigned char 	*buf,
	int 			len);

int
XCountUtf8Char(
	const unsigned char *buf,
	int len);

int
XFastConvertUtf8ToUcs(
	const unsigned char 	*buf,
	int 			len,
	unsigned int 		*ucs);

long
XKeysymToUcs(
	KeySym 	keysym);

#ifdef X_HAVE_UTF8_STRING
#define XUtf8LookupString Xutf8LookupString
#else
int
XUtf8LookupString(
    XIC                 ic,
    XKeyPressedEvent*   event,
    char*               buffer_return,
    int                 bytes_buffer,
    KeySym*             keysym,
    Status*             status_return);
#endif

unsigned short
XUtf8IsNonSpacing(
	unsigned int ucs);

unsigned short
XUtf8IsRightToLeft(
        unsigned int ucs);


int
XUtf8Tolower(
        int ucs);

int
XUtf8Toupper(
        int ucs);


#  ifdef __cplusplus
}
#  endif

#endif

/*
 *  End of "$Id$".
 */
