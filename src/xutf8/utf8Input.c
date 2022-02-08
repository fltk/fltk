/*
 * Author: Jean-Marc Lienher ( http://oksid.ch )
 * Copyright 2000-2003 by O'ksi'D.
 * Copyright 2004-2021 by Bill Spitzak and others.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     https://www.fltk.org/COPYING.php
 *
 * Please see the following page on how to report bugs and issues:
 *
 *     https://www.fltk.org/bugs.php
 */

#include <config.h>

#if defined(FLTK_USE_X11)

#include "../Xutf8.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_LIBC_ICONV
#include <iconv.h>
#endif
/*
  I haven't found much doc on the web about EUC encodings, so I've used
  GNU libiconv source code as a reference.
  http://clisp.cons.org/~haible/packages-libiconv.html
*/

#define RET_ILSEQ -1
#define RET_TOOFEW(x) (-10 - x)
#define RET_TOOSMALL -2
#define conv_t void*
#define ucs4_t unsigned int
typedef struct {
  unsigned short indx;
  unsigned short used;
} Summary16;

#ifndef X_HAVE_UTF8_STRING
#define NEED_TOWC /* indicates what part of these include files is needed here (avoid compilation warnings) */
#include "lcUniConv/big5.h"
#include "lcUniConv/gb2312.h"
#include "lcUniConv/cp936ext.h"
#include "lcUniConv/jisx0201.h"
#include "lcUniConv/jisx0208.h"
#include "lcUniConv/jisx0212.h"
#include "lcUniConv/ksc5601.h"

static int
XConvertEucTwToUtf8(char* buffer_return, int len) {
  /* FIXME */
#ifdef HAVE_LIBC_ICONV
  iconv_t cd;
  int cdl;
#else
  int i = 0;
#endif
  int l = 0;
  char *buf; /* , *b; */

  if (len < 1) return 0;
  /*b = */ buf = (char*) malloc((unsigned)len);
  memcpy(buf, buffer_return, (unsigned) len);

#ifdef HAVE_LIBC_ICONV
  l = cdl = len;
  cd = iconv_open("EUC-TW", "UTF-8");
  iconv(cd, &b, &len, &buffer_return, &cdl);
  iconv_close(cd);
  l -= cdl;
#else
  while (i < len) {
    unsigned int ucs;
    unsigned char c;
    c = (unsigned char) buf[i];
    if (c < 0x80) {
      ucs = c;
      i++;
    } else if (c >= 0xa1 && c < 0xff && len - i > 1 ) {

#if 0
      unsigned char b[2];
      b[0] = (unsigned char) c - 0x80;
      b[1] = (unsigned char) buf[i + 1] - 0x80;
#endif
      ucs = ' '; i += 2;
    } else if (c == 0x8e &&  len - i > 3) {
      unsigned char c1 =  buf[i + 1];
      unsigned char c2 =  buf[i + 2];
      unsigned char c3 =  buf[i + 3];
#if 0
      unsigned char b[2];
      b[0] = (unsigned char)  buf[i + 2] - 0x80;
      b[1] = (unsigned char)  buf[i + 3] - 0x80;
#endif
      if (c1 >= 0xa1 && c1 <= 0xb0) {
        if (c2 >= 0xa1 && c2 < 0xff && c3 >= 0xa1 && c3 < 0xff) {
          ucs = ' '; i += 4;
        } else {
          ucs = '?'; i++;
        }
      } else {
        ucs = '?'; i++;
      }
    } else {
      ucs = '?';
      i++;
    }
    l += XConvertUcsToUtf8(ucs, buffer_return + l);
  }
#endif
  free(buf);
  return l;
}

static int
XConvertEucKrToUtf8(char* buffer_return, int len) {
  int i = 0, l = 0;
  char *buf;

  if (len < 1) return 0;

  buf = (char*) malloc((unsigned)len);
  memcpy(buf, buffer_return, (unsigned)len);

  while (i < len) {
    unsigned int ucs;
    unsigned char c, c1;
    c = (unsigned char) buf[i];
    if (c < 0x80) {
      ucs = c;
      i++;
    } else if (c >= 0xA1 && c < 0xFF && len - i > 1) {
      c1 = (unsigned char) buf[i + 1];
      if (c1 >= 0xa1 && c1 < 0xff) {
        unsigned char b[2];
        b[0] = c - 0x80;
        b[1] = c1 - 0x80;
        if (ksc5601_mbtowc(NULL, &ucs, b, 2) < 1) {
          ucs = '?';
        }
      } else {
        ucs = '?';
      }
      i += 2;
    } else {
      ucs = '?';
      i++;
    }
    l += XConvertUcsToUtf8(ucs, buffer_return + l);
  }
  free(buf);
  return l;
}

static int
XConvertBig5ToUtf8(char* buffer_return, int len) {
  int i = 0, l = 0;
  char *buf;

  if (len < 1) return 0;
  buf = (char*) malloc((unsigned)len);
  memcpy(buf, buffer_return, (unsigned)len);

  if (len == 1) {
    l += XConvertUcsToUtf8((unsigned int)buf[i], buffer_return + l);
  }
  while (i + 1 < len) {
    unsigned int ucs;
    unsigned char b[2];
    b[0] = (unsigned char) buf[i];
    b[1] = (unsigned char) buf[i + 1];
    if (big5_mbtowc(NULL, &ucs, b, 2) == 2) {
      i += 2;
    } else {
      ucs = '?';
      i++;
    }
    l += XConvertUcsToUtf8(ucs, buffer_return + l);
  }
  free(buf);
  return l;
}

static int
XConvertCp936extToUtf8(char* buffer_return, int len)
{
  int i = 0, l = 0;
  char *buf;

  if (len < 1) return 0;
  buf = (char*) malloc((unsigned)len);
  memcpy(buf, buffer_return, (unsigned)len);

  if (len == 1) {
          l += XConvertUcsToUtf8((unsigned int)buf[i], buffer_return + l);
  }
  while (i + 1 < len) {
          unsigned int ucs;
          unsigned char b[2];
          b[0] = (unsigned char) buf[i];
          b[1] = (unsigned char) buf[i + 1];
          if (cp936ext_mbtowc(NULL, &ucs, b, 2) == 2) {
                  i += 2;
          } else {
              if ( b[0] < 0x80) {
                    ucs = b[0];
                }else{
                              ucs = '?';
                  }
                          i++;
                  }
          l += XConvertUcsToUtf8(ucs, buffer_return + l);
  }
  if(i + 1 == len) {
      l += XConvertUcsToUtf8((unsigned int)buf[i], buffer_return + l);
  }
  free(buf);
  return l;
}

static int
XConvertGb2312ToUtf8(char* buffer_return, int len) {
  int i = 0, l = 0;
  char *buf;

  if (len < 1) return 0;
  buf = (char*) malloc((unsigned)len);
  memcpy(buf, buffer_return, (unsigned)len);

  if (len == 1) {
    l += XConvertUcsToUtf8((unsigned int)buf[i], buffer_return + l);
  }
  while (i + 1 < len) {
    unsigned int ucs;
    unsigned char b[2];
    b[0] = (unsigned char) buf[i];
    b[1] = (unsigned char) buf[i + 1];
    if ( b[0] < 0x80 ) {
      ucs = b[0];
      i++;
    } else if (gb2312_mbtowc(NULL, &ucs, b, 2) == 2) {
      i += 2;
    } else {
      ucs = '?';
      i++;
    }
    l += XConvertUcsToUtf8(ucs, buffer_return + l);
  }
  if (i + 1 == len) {
    l += XConvertUcsToUtf8((unsigned int)buf[i], buffer_return + l);
  }
  free(buf);
  return l;
}

static int
XConvertEucCnToUtf8(char* buffer_return, int len) {
  int i = 0, l = 0;
  char *buf;

  if (len < 1) return 0;
  buf = (char*) malloc((unsigned)len);
  memcpy(buf, buffer_return, (unsigned)len);

  while (i < len) {
    unsigned int ucs;
    unsigned char c, c1;
    c = (unsigned char) buf[i];
    if (c < 0x80) {
      ucs = c;
      i++;
    } else if (c >= 0xA1 && c < 0xFF && len - i > 1) {
      c1 = (unsigned char) buf[i + 1];
      if (c1 >= 0xa1 && c1 < 0xff) {
        unsigned char b[2];
        b[0] = (unsigned char) c;
        b[1] = (unsigned char) c1;
        if (gb2312_mbtowc(NULL, &ucs, b, 2) < 1) {
          ucs = '?';
        }
      } else {
        ucs = '?';
      }
      i += 2;
    } else {
      ucs = '?';
      i++;
    }
    l += XConvertUcsToUtf8(ucs, buffer_return + l);
  }
  free(buf);
  return l;
}

static int
XConvertEucJpToUtf8(char* buffer_return, int len) {
  int i = 0, l = 0;
  char *buf;

  if (len < 1) return 0;
  buf = (char*) malloc((unsigned)len);
  memcpy(buf, buffer_return, (unsigned)len);

  while (i < len) {
    unsigned int ucs;
    unsigned char c, c1;
    c = (unsigned char) buf[i];
    if (c < 0x80) {
      ucs = c;
      i++;
    } else if (c >= 0xA1 && c < 0xFF && len - i > 1) {
      c1 = (unsigned char) buf[i + 1];
      if (c < 0xF5 && c1 >= 0xa1) {
        unsigned char b[2];
        b[0] = c - 0x80;
        b[1] = c1 - 0x80;
        if (jisx0208_mbtowc(NULL, &ucs, b, 2) < 1) {
          ucs = '?';
        }
      } else if (c1 >= 0xA1 && c1 < 0xFF) {
        ucs = 0xE000 + 94 * (c - 0xF5) + (c1 - 0xA1);
      } else {
        ucs = '?';
      }
      i += 2;
    } else if (c == 0x8E && len - i > 1) {
      c1 = (unsigned char) buf[i + 1];
      if (c1 >= 0xa1 && c1 <= 0xe0) {
        if (jisx0201_mbtowc(NULL, &ucs, &c1, 1) != 1) {
          ucs = '?';
        }
      } else {
        ucs = '?';
      }
      i += 2;
    } else if (c == 0x8F && len - i > 2) {
      c = (unsigned char) buf[i + 1];
      c1 = (unsigned char) buf[i + 2];
      if (c >= 0xa1 && c < 0xff) {
        if (c < 0xf5 && c1 >= 0xa1 && c1 < 0xff) {
          unsigned char b[2];
          b[0] = c - 0x80;
          b[1] = c1 - 0x80;
          if (jisx0212_mbtowc(NULL, &ucs, b, 2) < 1) {
            ucs = '?';
          }
        } else {
          ucs = '?';
        }
      } else {
        if (c1 >= 0xa1 && c1 < 0xff) {
          ucs = 0xe3ac + 94 * (c - 0xF5) + (c1 - 0xA1);
        } else {
          ucs = '?';
        }
      }
      i += 3;
    } else {
      ucs = '?';
      i++;
    }
    l += XConvertUcsToUtf8(ucs, buffer_return + l);
  }
  free(buf);
  return l;
}

static int
XConvertEucToUtf8(const char*   locale,
                  char*         buffer_return,
                  int           len,
                  int           bytes_buffer) {

  /* if (!locale) { */
  /* if (!locale || strstr(locale, "UTF") || strstr(locale, "utf")) { */
  if (!locale || strstr(locale, "UTF") || strstr(locale, "utf")) {
    return len;
  }

  if (strstr(locale, "ja")) {
    return XConvertEucJpToUtf8(buffer_return, len);
  } else if (strstr(locale, "Big5") || strstr(locale, "big5")) { /* BIG5 */
    return XConvertBig5ToUtf8(buffer_return, len);
  } else if (strstr(locale, "GBK") || strstr(locale, "gbk")) {
    return XConvertCp936extToUtf8(buffer_return, len);
  } else if (strstr(locale, "zh") || strstr(locale, "chinese-")) {
    if (strstr(locale, "TW") || strstr(locale, "chinese-t")) {
      if (strstr(locale, "EUC") || strstr(locale, "euc") || strstr(locale, "chinese-t")) {
        return XConvertEucTwToUtf8(buffer_return, len);
      }
      return XConvertBig5ToUtf8(buffer_return, len);
    }
    if (strstr(locale, "EUC") || strstr(locale, "euc")) {
      return XConvertEucCnToUtf8(buffer_return, len);
    }
    return XConvertGb2312ToUtf8(buffer_return, len);
  } else if (strstr(locale, "ko")) {
    return XConvertEucKrToUtf8(buffer_return, len);
  }
  return len;
}

int
XUtf8LookupString(XIC                 ic,
                  XKeyPressedEvent*   event,
                  char*               buffer_return,
                  int                 bytes_buffer,
                  KeySym*             keysym,
                  Status*             status_return) {

  long ucs = -1;
  int len;
  len = XmbLookupString(ic, event, buffer_return, bytes_buffer / 5,
                        keysym, status_return);
  if (*status_return == XBufferOverflow) {
    return len * 5;
  }
  if (*keysym > 0 && *keysym < 0x100 && len == 1) {
    if (*keysym < 0x80) {
      ucs = (unsigned char)buffer_return[0];
    } else {
      ucs = (long)*keysym;
    }
  } else  if (((*keysym >= 0x100 && *keysym <= 0xf000) ||
              (*keysym & 0xff000000U) == 0x01000000))
  {
    ucs = XKeysymToUcs(*keysym);
  } else {
    ucs = -2;
  }

  if (ucs > 0) {
    len = XConvertUcsToUtf8((unsigned)ucs, (char *)buffer_return);
  } else if (len > 0) {
    XIM im;
    if (!ic) return 0;
    im = XIMOfIC(ic);
    if (!im) return 0;
    len = XConvertEucToUtf8(XLocaleOfIM(im), buffer_return, len, bytes_buffer);
  }
  return len;
}
#endif /* X11 has UTF-8 */

#endif /* X11 only */
