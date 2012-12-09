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

#include <wchar.h>
#include <stdio.h>
char buffer[1000000];

/*** you can try to modifie this value to have better performences **/
#define MAX_DELTA 0x80

int main(int argc, char **argv) {
  char buf[80];
  int len;
  unsigned int i = 0;
  unsigned char *ptr;
  size_t nb;
  int nbb = 0;
  len = fread(buffer, 1, 1000000, stdin);

  buffer[len] = '\0';
  ptr = (unsigned char *)buffer;
  while (*ptr != '\n') ptr++;
  ptr++;
  while (*ptr != '\n') {
    if (*ptr == ',') nbb++;
    ptr++;
  }
  ptr = (unsigned char *)buffer;
  printf("/* %s */\n", argv[1]);
  while (len > 0) {
    unsigned int ucs = 0;
    char *p = ptr;
    char pp[20];
    nb = 0;
    pp[0] = '\0';
    while (*p != 'U') p++;
    strncat(pp, p, 6);
    *pp = '0';
    *(pp+1) = 'x';
    ucs = (unsigned int)strtoul(pp, NULL, 16);;
    if (ucs < 1) {
      printf("ERROR %d %d\n", len, ucs);
      abort();
    }
    if (i != ucs - 1 || !i) {
      if ((ucs - i) > MAX_DELTA || !i) {
	if (i) {
	  printf("};\n");
	  fprintf(stderr, "\t/* end: U+%04X */\n", i);
	}
	if (strcmp(argv[1], "spacing")) {
	  printf("\nstatic const char unicode_to_%s_%db_%04X[] = {\n",
	         argv[1], nbb, ucs);
	  fprintf(stderr, "unicode_to_%s_%db_%04X[]; ", 
		  argv[1], nbb, ucs);
	} else {
	  printf("\nstatic const unsigned short"
		 " ucs_table_%04X[]"
		 " = {\n", ucs);
	  fprintf(stderr, "ucs_table_%04X[]; ", ucs);
	}
      } else {
	while (i < ucs - 1) {
	  i++;
	  if (nbb == 1) {
	    printf("0x00,\n");
	  } else {
	    printf("0x00, 0x00,\n");
	  }
	};
      }
    }
    i = ucs;
    while (*ptr != '\n') {
      printf("%c", *ptr);
      ptr++;
      len--;
    }
    printf("\n");
    ptr++;
    len--;
  }
  printf("};\n");
  fprintf(stderr, "\t/* end: U+%04X */\n", i);
  return 0;
}

/*
 * End of "$Id$".
 */
