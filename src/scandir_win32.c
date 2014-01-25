/*
 * "$Id$"
 *
 * WIN32 scandir function for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2010 by Bill Spitzak and others.
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

#ifndef __CYGWIN__
/* Emulation of posix scandir() call */
#include <FL/fl_utf8.h>
#include <FL/filename.H>
#include "flstring.h"
#include <windows.h>
#include <stdlib.h>

int fl_scandir(const char *dirname, struct dirent ***namelist,
	       int (*select)(struct dirent *),
	       int (*compar)(struct dirent **, struct dirent **)) {
  int len;
  char *findIn, *d, is_dir = 0;
  WIN32_FIND_DATAW findw;
  HANDLE h;
  int nDir = 0, NDir = 0;
  struct dirent **dir = 0, *selectDir;
  unsigned long ret;

  len    = (int) strlen(dirname);
  findIn = (char *)malloc((size_t)(len+10));
  if (!findIn) return -1;
  strcpy(findIn, dirname);

  /* #if defined(__GNUC__) */
  /* #warning FIXME This probably needs to be MORE UTF8 aware now */
  /* #endif */
  for (d = findIn; *d; d++) if (*d=='/') *d='\\';
  if (len==0) { strcpy(findIn, ".\\*"); }
  if ((len==2)&&findIn[1]==':'&&isalpha(findIn[0])) { *d++ = '\\'; *d = 0; }
  if ((len==1)&& (d[-1]=='.')) { strcpy(findIn, ".\\*"); is_dir = 1; }
  if ((len>0) && (d[-1]=='\\')) { *d++ = '*'; *d = 0; is_dir = 1; }
  if ((len>1) && (d[-1]=='.') && (d[-2]=='\\')) { d[-1] = '*'; is_dir = 1; }
  if (!is_dir) { /* this file may still be a directory that we need to list */
    DWORD attr = GetFileAttributes(findIn);
    if (attr&FILE_ATTRIBUTE_DIRECTORY)
      strcpy(d, "\\*");
  }
  { /* Create a block to limit the scope while we find the initial "wide" filename */
     /* unsigned short * wbuf = (unsigned short*)malloc(sizeof(short) *(len + 10)); */
     /* wbuf[fl_utf2unicode(findIn, strlen(findIn), wbuf)] = 0; */
	unsigned short *wbuf = NULL;
	unsigned wlen = fl_utf8toUtf16(findIn, (unsigned) strlen(findIn), NULL, 0); /* Pass NULL to query length */
	wlen++; /* add a little extra for termination etc. */
	wbuf = (unsigned short*)malloc(sizeof(unsigned short)*wlen);
	wlen = fl_utf8toUtf16(findIn, (unsigned) strlen(findIn), wbuf, wlen); /* actually convert the filename */
	wbuf[wlen] = 0; /* NULL terminate the resultant string */
	h = FindFirstFileW(wbuf, &findw); /* get a handle to the first filename in the search */
	free(wbuf); /* release the "wide" buffer before the pointer goes out of scope */
  }
  if (h==INVALID_HANDLE_VALUE) {
    free(findIn);
    ret = GetLastError();
    if (ret != ERROR_NO_MORE_FILES) {
      nDir = -1;
    }
    *namelist = dir;
    return nDir;
  }
  do {
	int l = (int) wcslen(findw.cFileName);
	int dstlen = l * 5 + 1;
	selectDir=(struct dirent*)malloc(sizeof(struct dirent)+dstlen);

     /* l = fl_unicode2utf(findw.cFileName, l, selectDir->d_name); */
	l = fl_utf8fromwc(selectDir->d_name, dstlen, findw.cFileName, l);

	selectDir->d_name[l] = 0;
	if (findw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		/* Append a trailing slash to directory names... */
		strcat(selectDir->d_name, "/");
	}
	if (!select || (*select)(selectDir)) {
		if (nDir==NDir) {
	struct dirent **tempDir = (struct dirent **)calloc(sizeof(struct dirent*), (size_t)(NDir+33));
	if (NDir) memcpy(tempDir, dir, sizeof(struct dirent*)*NDir);
	if (dir) free(dir);
	dir = tempDir;
	NDir += 32;
		}
		dir[nDir] = selectDir;
		nDir++;
		dir[nDir] = 0;
	} else {
		free(selectDir);
	}
   } while (FindNextFileW(h, &findw));
  ret = GetLastError();
  if (ret != ERROR_NO_MORE_FILES) {
    /* don't return an error code, because the dir list may still be valid
       up to this point */
  }
  FindClose(h);

  free (findIn);

  if (compar) qsort(dir, (size_t)nDir, sizeof(*dir),
		    (int(*)(const void*, const void*))compar);

  *namelist = dir;
  return nDir;
}

#endif

/*
 * End of "$Id$".
 */
