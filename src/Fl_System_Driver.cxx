//
// A base class for platform specific system calls.
//
// Copyright 1998-2022 by Bill Spitzak and others.
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

/**
 \cond DriverDev
 \defgroup DriverDeveloper Driver Developer Documentation
 \{
 */

#include "Fl_System_Driver.H"
#include <FL/Fl.H>
#include "Fl_Timeout.h"
#include <FL/Fl_File_Icon.H>
#include <FL/fl_utf8.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "flstring.h"
#include <time.h>

const int Fl_System_Driver::fl_NoValue =     0x0000;
const int Fl_System_Driver::fl_WidthValue =  0x0004;
const int Fl_System_Driver::fl_HeightValue = 0x0008;
const int Fl_System_Driver::fl_XValue =      0x0001;
const int Fl_System_Driver::fl_YValue =      0x0002;
const int Fl_System_Driver::fl_XNegative =   0x0010;
const int Fl_System_Driver::fl_YNegative =   0x0020;

// This default key table is used for all system drivers that don't define
// and/or use their own table. It is defined here "static" and assigned
// in the constructor to avoid static initialization race conditions.
//
// As of January 2022 the only platform is Windows. X11 does not
// use a key table at all.
// Platforms that use their own key tables must assign them in their
// constructors (which overwrites the pointer and size).

static Fl_System_Driver::Keyname default_key_table[] = {
  {' ',           "Space"},
  {FL_BackSpace,  "Backspace"},
  {FL_Tab,        "Tab"},
  {0xff0b/*XK_Clear*/, "Clear"},
  {FL_Enter,      "Enter"}, // X says "Enter"
  {FL_Pause,      "Pause"},
  {FL_Scroll_Lock, "Scroll_Lock"},
  {FL_Escape,     "Escape"},
  {FL_Home,       "Home"},
  {FL_Left,       "Left"},
  {FL_Up,         "Up"},
  {FL_Right,      "Right"},
  {FL_Down,       "Down"},
  {FL_Page_Up,    "Page_Up"}, // X says "Prior"
  {FL_Page_Down,  "Page_Down"}, // X says "Next"
  {FL_End,        "End"},
  {FL_Print,      "Print"},
  {FL_Insert,     "Insert"},
  {FL_Menu,       "Menu"},
  {FL_Num_Lock,   "Num_Lock"},
  {FL_KP_Enter,   "KP_Enter"},
  {FL_Shift_L,    "Shift_L"},
  {FL_Shift_R,    "Shift_R"},
  {FL_Control_L,  "Control_L"},
  {FL_Control_R,  "Control_R"},
  {FL_Caps_Lock,  "Caps_Lock"},
  {FL_Meta_L,     "Meta_L"},
  {FL_Meta_R,     "Meta_R"},
  {FL_Alt_L,      "Alt_L"},
  {FL_Alt_R,      "Alt_R"},
  {FL_Delete,     "Delete"}
};


int Fl_System_Driver::command_key = 0;
int Fl_System_Driver::control_key = 0;


int fl_command_modifier() {
  if (!Fl_System_Driver::command_key) Fl::system_driver();
  return Fl_System_Driver::command_key;
}


int fl_control_modifier() {
  if (!Fl_System_Driver::control_key) Fl::system_driver();
  return Fl_System_Driver::control_key;
}


Fl_System_Driver::Fl_System_Driver()
{
  // initialize default key table (used in fl_shortcut.cxx)
  key_table = default_key_table;
  key_table_size = sizeof(default_key_table)/sizeof(*default_key_table);
  command_key = FL_CTRL;
  control_key = FL_META;
}

Fl_System_Driver::~Fl_System_Driver()
{
}

void Fl_System_Driver::warning(const char* format, ...) {
  va_list args;
  va_start(args, format);
  Fl::system_driver()->warning(format, args);
  va_end(args);
}

void Fl_System_Driver::warning(const char* format, va_list args) {
  vfprintf(stderr, format, args);
  fputc('\n', stderr);
  fflush(stderr);
}

void Fl_System_Driver::error(const char* format, ...) {
  va_list args;
  va_start(args, format);
  Fl::system_driver()->error(format, args);
  va_end(args);
}

void Fl_System_Driver::error(const char *format, va_list args) {
  vfprintf(stderr, format, args);
  fputc('\n', stderr);
  fflush(stderr);
}

void Fl_System_Driver::fatal(const char* format, ...) {
  va_list args;
  va_start(args, format);
  Fl::system_driver()->fatal(format, args);
  va_end(args);
}

void Fl_System_Driver::fatal(const char *format, va_list args) {
  vfprintf(stderr, format, args);
  fputc('\n', stderr);
  fflush(stderr);
  exit(1);
}

/* the following function was stolen from the X sources as indicated. */

/* Copyright    Massachusetts Institute of Technology  1985, 1986, 1987 */
/* $XConsortium: XParseGeom.c,v 11.18 91/02/21 17:23:05 rws Exp $ */

/*
 Permission to use, copy, modify, distribute, and sell this software and its
 documentation for any purpose is hereby granted without fee, provided that
 the above copyright notice appear in all copies and that both that
 copyright notice and this permission notice appear in supporting
 documentation, and that the name of M.I.T. not be used in advertising or
 publicity pertaining to distribution of the software without specific,
 written prior permission.  M.I.T. makes no representations about the
 suitability of this software for any purpose.  It is provided "as is"
 without express or implied warranty.
 */

/*
    XParseGeometry parses strings of the form
   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
   width, height, xoffset, and yoffset are unsigned integers.
   Example:  "=80x24+300-49"
   The equal sign is optional.
   It returns a bitmask that indicates which of the four values
   were actually found in the string.  For each value found,
   the corresponding argument is updated;  for each value
   not found, the corresponding argument is left unchanged.
 */

static int ReadInteger(char* string, char** NextString)
{
  int Result = 0;
  int Sign = 1;

  if (*string == '+')
    string++;
  else if (*string == '-') {
    string++;
    Sign = -1;
  }
  for (; (*string >= '0') && (*string <= '9'); string++) {
    Result = (Result * 10) + (*string - '0');
  }
  *NextString = string;
  if (Sign >= 0)
    return (Result);
  else
    return (-Result);
}

int Fl_System_Driver::XParseGeometry(const char* string, int* x, int* y,
                   unsigned int* width, unsigned int* height)
{
  int mask = Fl_System_Driver::fl_NoValue;
  char *strind;
  unsigned int tempWidth = 0, tempHeight = 0;
  int tempX = 0, tempY = 0;
  char *nextCharacter;

  if ( (string == NULL) || (*string == '\0')) return(mask);
  if (*string == '=')
    string++;  /* ignore possible '=' at beg of geometry spec */

  strind = (char *)string;
  if (*strind != '+' && *strind != '-' && *strind != 'x') {
    tempWidth = ReadInteger(strind, &nextCharacter);
    if (strind == nextCharacter)
      return (0);
    strind = nextCharacter;
    mask |= fl_WidthValue;
  }

  if (*strind == 'x' || *strind == 'X') {
    strind++;
    tempHeight = ReadInteger(strind, &nextCharacter);
    if (strind == nextCharacter)
      return (0);
    strind = nextCharacter;
    mask |= fl_HeightValue;
  }

  if ((*strind == '+') || (*strind == '-')) {
    if (*strind == '-') {
      strind++;
      tempX = -ReadInteger(strind, &nextCharacter);
      if (strind == nextCharacter)
        return (0);
      strind = nextCharacter;
      mask |= fl_XNegative;

    } else {
      strind++;
      tempX = ReadInteger(strind, &nextCharacter);
      if (strind == nextCharacter)
        return(0);
      strind = nextCharacter;
    }
    mask |= fl_XValue;
    if ((*strind == '+') || (*strind == '-')) {
      if (*strind == '-') {
        strind++;
        tempY = -ReadInteger(strind, &nextCharacter);
        if (strind == nextCharacter)
          return(0);
        strind = nextCharacter;
        mask |= fl_YNegative;

      } else {
        strind++;
        tempY = ReadInteger(strind, &nextCharacter);
        if (strind == nextCharacter)
          return(0);
        strind = nextCharacter;
      }
      mask |= fl_YValue;
    }
  }

  /* If strind isn't at the end of the string the it's an invalid
   geometry specification. */

  if (*strind != '\0') return (0);

  if (mask & fl_XValue)
    *x = tempX;
  if (mask & fl_YValue)
    *y = tempY;
  if (mask & fl_WidthValue)
    *width = tempWidth;
  if (mask & fl_HeightValue)
    *height = tempHeight;
  return (mask);
}

unsigned Fl_System_Driver::utf8towc(const char* src, unsigned srclen, wchar_t* dst, unsigned dstlen) {
  const char* p = src;
  const char* e = src+srclen;
  unsigned count = 0;
  if (dstlen) for (;;) {
    if (p >= e) {
      dst[count] = 0;
      return count;
    }
    if (!(*p & 0x80)) { /* ascii */
      dst[count] = *p++;
    } else {
      int len; unsigned ucs = fl_utf8decode(p,e,&len);
      p += len;
      dst[count] = (wchar_t)ucs;
    }
    if (++count == dstlen) {dst[count-1] = 0; break;}
  }
  /* we filled dst, measure the rest: */
  while (p < e) {
    if (!(*p & 0x80)) p++;
    else {
      int len; fl_utf8decode(p,e,&len);
      p += len;
    }
    ++count;
  }
  return count;
}

unsigned Fl_System_Driver::utf8fromwc(char* dst, unsigned dstlen, const wchar_t* src, unsigned srclen)
{
  unsigned i = 0;
  unsigned count = 0;
  if (dstlen) for (;;) {
    unsigned ucs;
    if (i >= srclen) {dst[count] = 0; return count;}
    ucs = src[i++];
    if (ucs < 0x80U) {
      dst[count++] = ucs;
      if (count >= dstlen) {dst[count-1] = 0; break;}
    } else if (ucs < 0x800U) { /* 2 bytes */
      if (count+2 >= dstlen) {dst[count] = 0; count += 2; break;}
      dst[count++] = 0xc0 | (ucs >> 6);
      dst[count++] = 0x80 | (ucs & 0x3F);
    } else if (ucs >= 0x10000) {
      if (ucs > 0x10ffff) {
        ucs = 0xfffd;
        goto J1;
      }
      if (count+4 >= dstlen) {dst[count] = 0; count += 4; break;}
      dst[count++] = 0xf0 | (ucs >> 18);
      dst[count++] = 0x80 | ((ucs >> 12) & 0x3F);
      dst[count++] = 0x80 | ((ucs >> 6) & 0x3F);
      dst[count++] = 0x80 | (ucs & 0x3F);
    } else {
J1:
      /* all others are 3 bytes: */
      if (count+3 >= dstlen) {dst[count] = 0; count += 3; break;}
      dst[count++] = 0xe0 | (ucs >> 12);
      dst[count++] = 0x80 | ((ucs >> 6) & 0x3F);
      dst[count++] = 0x80 | (ucs & 0x3F);
    }
  }
  /* we filled dst, measure the rest: */
  while (i < srclen) {
    unsigned ucs = src[i++];
    if (ucs < 0x80U) {
      count++;
    } else if (ucs < 0x800U) { /* 2 bytes */
      count += 2;
    } else if (ucs >= 0x10000 && ucs <= 0x10ffff) {
      count += 4;
    } else {
      count += 3;
    }
  }
  return count;
}

unsigned Fl_System_Driver::utf8to_mb(const char* src, unsigned srclen, char* dst, unsigned dstlen) {
  wchar_t lbuf[1024];
  wchar_t* buf = lbuf;
  unsigned length = fl_utf8towc(src, srclen, buf, 1024);
  int ret; // note: wcstombs() returns unsigned(length) or unsigned(-1)
  if (length >= 1024) {
    buf = (wchar_t*)(malloc((length+1)*sizeof(wchar_t)));
    fl_utf8towc(src, srclen, buf, length+1);
  }
  if (dstlen) {
    ret = (int)wcstombs(dst, buf, dstlen);
    if (ret >= (int)dstlen-1) ret = (int)wcstombs(0,buf,0);
  } else {
    ret = (int)wcstombs(0,buf,0);
  }
  if (buf != lbuf) free(buf);
  if (ret >= 0) return (unsigned)ret;
  // on any errors we return the UTF-8 as raw text...
  if (srclen < dstlen) {
    memcpy(dst, src, srclen);
    dst[srclen] = 0;
  } else {
    // Buffer insufficent or buffer query
  }
  return srclen;
}

unsigned Fl_System_Driver::utf8from_mb(char* dst, unsigned dstlen, const char* src, unsigned srclen) {
  wchar_t lbuf[1024];
  wchar_t* buf = lbuf;
  int length;
  unsigned ret;
  length = (int)mbstowcs(buf, src, 1024);
  if (length >= 1024) {
    length = (int)mbstowcs(0, src, 0)+1;
    buf = (wchar_t*)(malloc(length*sizeof(wchar_t)));
    mbstowcs(buf, src, length);
  }
  if (length >= 0) {
    ret = fl_utf8fromwc(dst, dstlen, buf, length);
    if (buf != lbuf) free(buf);
    return ret;
  }
  // errors in conversion return the UTF-8 unchanged
  if (srclen < dstlen) {
    memcpy(dst, src, srclen);
    dst[srclen] = 0;
  } else {
    // Buffer insufficent or buffer query
  }
  return srclen;
}

int Fl_System_Driver::clocale_vprintf(FILE *output, const char *format, va_list args) {
  return vfprintf(output, format, args);
}

int Fl_System_Driver::clocale_vsnprintf(char *output, size_t output_size, const char *format, va_list args) {
  return 0; // overridden in platform drivers
}

int Fl_System_Driver::clocale_vsscanf(const char *input, const char *format, va_list args) {
  return 0; // overridden in platform drivers
}

int Fl_System_Driver::filename_expand(char *to,int tolen, const char *from) {
  char *temp = new char[tolen];
  strlcpy(temp,from, tolen);
  char *start = temp;
  char *end = temp+strlen(temp);

  int ret = 0;

  for (char *a=temp; a<end; ) { // for each slash component
    char *e; for (e=a; e<end && *e != '/'; e++) {/*empty*/} // find next slash
    const char *value = 0; // this will point at substitute value
    switch (*a) {
      case '~': // a home directory name
        if (e <= a+1) { // current user's directory
          value = getenv("HOME");
        } else {        // another user's directory
          char t = *e; *(char *)e = 0;
          value = getpwnam(a+1);
          *(char *)e = t;
        }
        break;
      case '$':         /* an environment variable */
      {char t = *e; *(char *)e = 0; value = getenv(a+1); *(char *)e = t;}
        break;
    }
    if (value) {
      // substitutions that start with slash delete everything before them:
      if (value[0] == '/') start = a;
      int t = (int) strlen(value); if (value[t-1] == '/') t--;
      if ((end+1-e+t) >= tolen) end += tolen - (end+1-e+t);
      memmove(a+t, e, end+1-e);
      end = a+t+(end-e);
      *end = '\0';
      memcpy(a, value, t);
      ret++;
    } else {
      a = e+1;
    }
  }
  strlcpy(to, start, tolen);
  delete[] temp;
  return ret;
}

int Fl_System_Driver::file_browser_load_directory(const char *directory, char *filename,
                                                  size_t name_size, dirent ***pfiles,
                                                  Fl_File_Sort_F *sort,
                                                  char *errmsg, int errmsg_sz)
{
  return filename_list(directory, pfiles, sort, errmsg, errmsg_sz);
}

int Fl_System_Driver::file_type(const char *filename)
{
  return Fl_File_Icon::ANY;
}

void Fl_System_Driver::add_fd(int fd, int when, Fl_FD_Handler cb, void *d)
{
  // nothing to do, reimplement in driver if needed
}

void Fl_System_Driver::add_fd(int fd, Fl_FD_Handler cb, void *d)
{
  // nothing to do, reimplement in driver if needed
}

void Fl_System_Driver::remove_fd(int fd, int when)
{
  // nothing to do, reimplement in driver if needed
}

void Fl_System_Driver::remove_fd(int fd)
{
  // nothing to do, reimplement in driver if needed
}

FILE *Fl_System_Driver::fopen(const char* f, const char *mode) {
  return ::fopen(f, mode);
}

void Fl_System_Driver::open_callback(void (*)(const char *)) {
}

// Get elapsed time since Jan 1st, 1970.
void Fl_System_Driver::gettime(time_t *sec, int *usec) {
  *sec =  time(NULL);
  *usec = 0;
}

/**
  Execute platform independent parts of Fl::wait(double).

  Platform drivers \b MUST override this virtual method to do
  their own stuff and call this base class method to run
  the platform independent wait functions.

  Overridden methods will typically call this method early and perform
  platform-specific operations after that in order to work with the
 \p time_to_wait value possibly modified by this method.
  However, some platform drivers may need to do extra stuff before
  calling this method, for instance setting up a memory pool on macOS.

  \param[in] time_to_wait max time to wait

  \return   new (max) time to wait after elapsing timeouts
*/
double Fl_System_Driver::wait(double time_to_wait) {

  // delete all widgets that were listed during callbacks
  Fl::do_widget_deletion();

  Fl_Timeout::do_timeouts();
  Fl::run_checks();
  Fl::run_idle();

  // the idle function may turn off idle, we can then wait,
  // or it leaves Fl::idle active and we set time_to_wait to 0
  if (Fl::idle) {
    time_to_wait = 0.0;
  } else {
    // limit time by next timer interval
    time_to_wait = Fl_Timeout::time_to_wait(time_to_wait);
  }
  return time_to_wait;
}

/**
 \}
 \endcond
 */
