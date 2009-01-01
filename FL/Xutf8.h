/*
 * "$Id:  $"
 *
 * X11 UTF-8 text drawing functions.
 *
 *      Copyright (c) 2000-2009, OksiD Software, Jean-Marc Lienher
 *                      All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *      Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *      Neither the name of OksiD Software nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER 
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  Author: Jean-Marc Lienher ( http://oksid.ch )
 */


#ifndef _Xutf8_h
#define _Xutf8_h

#  ifdef __cplusplus
extern "C" {
#  endif

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include <X11/Xutil.h>

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

int
XGetUtf8FontAndGlyph(
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

int
XUtf8LookupString(
    XIC                 ic,
    XKeyPressedEvent*   event,
    char*               buffer_return,
    int                 bytes_buffer,
    KeySym*             keysym,
    Status*             status_return);

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
 *  End of "$Id: $".
 */ 

