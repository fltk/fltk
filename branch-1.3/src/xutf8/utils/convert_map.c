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
 *    read the http://www.unicode.org/Public/MAPPINGS/ and create something
 *    usable in C.
 */

#include <wchar.h>
#include <stdio.h>

char buffer[1000000];

int JIS0208(unsigned char * ptr) {
  int i = 0;
  unsigned int fmap;
  unsigned int ucs;
  while(*ptr != '\t') { ptr++; i++; }
  ptr++; i++; *(ptr+6) = '\0';
  fmap = (unsigned int)strtoul(ptr, NULL, 16);
  while(*ptr != '\0') { ptr++; i++; }
  i++; ptr++; *(ptr+6) = '\0';
  ucs = (unsigned int)strtoul(ptr, NULL, 16);
  if (ucs) 
    printf("/* U+%04X */ 0x%02X, 0x%02X,\n", ucs, 
	  (fmap & 0xFF00) >> 8, fmap & 0xFF);
  while(*ptr != '\0') { ptr++; i++; }
  i++; ptr++;
  while(*ptr != '\n') { ptr++; i++; }
  i++;
  return i;
}

int JIS0201(unsigned char * ptr) {
  int i = 0;
  unsigned int fmap;
  unsigned int ucs;
  *(ptr+4) = '\0';
  fmap = (unsigned int)strtoul(ptr, NULL, 16);
  while(*ptr != '\0') { ptr++; i++; }
  i++; ptr++; *(ptr+6) = '\0';
  ucs = (unsigned int)strtoul(ptr, NULL, 16);
  if (*(ptr + 1) != 'x') {
    printf("/* EOF */\n");
    abort();
  }
  if (ucs) printf("/* U+%04X */ 0x%02X,\n", ucs, (unsigned char)fmap);
  while(*ptr != '\0') { ptr++; i++; }
  i++; ptr++;
  while(*ptr != '\n') { ptr++; i++; }
  i++;
  return i;
}

int ADOBE(unsigned char * ptr) {
  int i = 0;
  unsigned int fmap;
  unsigned int ucs;
  *(ptr+4) = '\0';
  ucs = (unsigned int)strtoul(ptr, NULL, 16);
  while(*ptr != '\0') { ptr++; i++; }
  i++; ptr++; *(ptr+2) = '\0';
  fmap = (unsigned int)strtoul(ptr, NULL, 16);
  if (fmap < 1) {
    printf("/* EOF */\n");
    abort();
  }
  if (ucs) printf("/* U+%04X */ 0x%02X,\n", ucs, (unsigned char)fmap);
  while(*ptr != '\0') { ptr++; i++; }
  i++; ptr++;
  while(*ptr != '\n') { ptr++; i++; }
  i++;
  return i;
}


int JIS0212(unsigned char * ptr) {
  int i = 0;
  unsigned int fmap;
  unsigned int ucs;
  *(ptr+6) = '\0';
  fmap = (unsigned int)strtoul(ptr, NULL, 16);
  ptr += 7;
  i += 7;
  while(*ptr == ' ') { ptr++; i++; }
  /* i++; ptr++; */
  *(ptr+6) = '\0';
  ucs = (unsigned int)strtoul(ptr, NULL, 16);
  if (*(ptr + 1) != 'x') {
    printf("/* EOF */\n");
    abort();
  }
  if (ucs)
    printf("/* U+%04X */ 0x%02X, 0x%02X,\n", ucs, 
	  (fmap & 0xFF00) >> 8, fmap & 0xFF);
  while(*ptr != '\0') { ptr++; i++; }
  i++; ptr++;
  while(*ptr != '\n') { ptr++; i++; }
  i++;
  return i;
}

int main(int argc, char **argv) {
  char buf[80];
  int len;
  int i;
  unsigned char *ptr;
  size_t nb;
  len = fread(buffer, 1, 1000000, stdin);

  buffer[len] = '\0';
  ptr = (unsigned char *)buffer;
  while (*ptr !='\n') {ptr++; len--;};
  ptr++; len--;
  while (*ptr == '#') {
    while (*ptr !='\n') {
      ptr++;
      len--;
    }
    ptr++;
    len--;
  }

  while (len > 0) {
    nb = 0;
    if (!strcmp("jisx0208.1983-0", argv[1])) {
      nb = JIS0208(ptr);
    } else if (!strcmp("jisx0201.1976-0", argv[1])) {
      nb = JIS0201(ptr);
    } else if (!strcmp("jisx0212.1990-0", argv[1])) {
      nb = JIS0212(ptr);
    } else if (!strcmp("gb2312.1980-0", argv[1])) {
      nb = JIS0212(ptr);
    } else if (!strcmp("ksc5601.1987-0", argv[1])) {
      nb = JIS0212(ptr);
    } else if (!strcmp("big5-0", argv[1])) {
      nb = JIS0212(ptr);
    } else if (!strncmp("iso8859", argv[1], 7)) {
      nb = JIS0201(ptr);
    } else if (!strcmp("koi8-1", argv[1])) {
      nb = JIS0201(ptr);
    } else if (!strcmp("dingbats", argv[1]) ||
	       !strcmp("symbol", argv[1])) 
    {
      nb = ADOBE(ptr);
    } else {
      len = 0;
    }
    ptr += nb;
    len = len - nb;
  }
  return 0;
}

/*
 * End of "$Id$".
 */
