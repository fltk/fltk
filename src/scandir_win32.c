/*
 * Windows scandir function for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2023 by Bill Spitzak and others.
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

#ifndef __CYGWIN__
/* Emulation of POSIX scandir() call with error messages */
#include <FL/platform_types.h>
#include <FL/fl_utf8.h>
#include "flstring.h"
#include <windows.h>
#include <stdlib.h>

/* Get error message string for last failed WIN32 operation
 * in 'errmsg' (if non-NULL), string size limited to errmsg_sz.
 *
 * NOTE: Copied from: fluid/ExternalCodeEditor_WIN32.cxx
 *
 * TODO: Verify works in different languages, with utf8 strings.
 * TODO: This should be made available globally to the FLTK internals, in case
 *       other parts of FLTK need OS error messages..
 */
static void get_ms_errmsg(char *errmsg, int errmsg_sz) {
  DWORD lastErr = GetLastError();
  DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_IGNORE_INSERTS  |
                FORMAT_MESSAGE_FROM_SYSTEM;
  DWORD langid = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
  LPWSTR mbuf = 0;
  DWORD msize = 0;

  /* Early exit if parent doesn't want an errmsg */
  if (!errmsg || errmsg_sz <= 0 ) return;
  /* Get error message from Windows */
  msize = FormatMessageW(flags, 0, lastErr, langid, (LPWSTR)&mbuf, 0, NULL);
  if ( msize == 0 ) {
    fl_snprintf(errmsg, errmsg_sz, "Error #%lu", (unsigned long)lastErr);
  } else {
    char *src;
    char *dst;
    /* convert message to UTF-8 */
    fl_utf8fromwc(errmsg, errmsg_sz, mbuf, msize);
    /* Remove '\r's -- they screw up fl_alert()) */
    src = dst = errmsg;
    for ( ; 1; src++ ) {
      if ( *src == '\0' ) { *dst = '\0'; break; }
      if ( *src != '\r' ) { *dst++ = *src; }
    }
    LocalFree(mbuf);    /* Free the buffer allocated by the system */
  }
}

/*
 * This could use some docs.
 *
 * Returns -1 on error, errmsg returns error string (if non-NULL)
 */
int fl_scandir(const char *dirname, struct dirent ***namelist,
               int (*select)(struct dirent *),
               int (*compar)(struct dirent **, struct dirent **),
               char *errmsg, int errmsg_sz) {
  int len;
  char *findIn, *d, is_dir = 0;
  WIN32_FIND_DATAW findw;
  HANDLE h;
  int nDir = 0, NDir = 0;
  struct dirent **dir = 0, *selectDir;
  unsigned long ret;

  if (errmsg && errmsg_sz>0) errmsg[0] = '\0';
  len    = (int) strlen(dirname);
  findIn = (char *)malloc((size_t)(len+10));
  if (!findIn) {
    /* win32 malloc() docs: "malloc sets errno to ENOMEM if allocation fails" */
    if (errmsg) fl_snprintf(errmsg, errmsg_sz, "%s", strerror(errno));
    return -1;
  }
  strcpy(findIn, dirname);

  for (d = findIn; *d; d++) if (*d == '/') *d = '\\';
  if (len == 0) { strcpy(findIn, ".\\*"); }
  if ((len == 2) && (findIn[1] == ':') && isalpha(findIn[0])) { *d++ = '\\'; *d = 0; }
  if ((len == 1) && (d[-1] == '.')) { strcpy(findIn, ".\\*"); is_dir = 1; }
  if ((len > 0) && (d[-1] == '\\')) { *d++ = '*'; *d = 0; is_dir = 1; }
  if ((len > 1) && (d[-1] == '.') && (d[-2] == '\\')) { d[-1] = '*'; is_dir = 1; }
  { /* Create a block to limit the scope while we find the initial "wide" filename */
    unsigned short *wbuf = NULL;
    unsigned wlen = fl_utf8toUtf16(findIn, (unsigned) strlen(findIn), NULL, 0); /* Pass NULL to query length */
    wlen++; /* add a little extra for termination etc. */
    wbuf = (unsigned short*)malloc(sizeof(unsigned short)*(wlen+2));
    wlen = fl_utf8toUtf16(findIn, (unsigned) strlen(findIn), wbuf, wlen); /* actually convert the filename */
    wbuf[wlen] = 0; /* NULL terminate the resultant string */
    if (!is_dir) { /* this file may still be a directory that we need to list */
      DWORD attr = GetFileAttributesW(wbuf);
      if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) ) {
        wbuf[wlen] = '\\'; wbuf[wlen+1] = '*'; wbuf[wlen+2] = 0;
      }
    }
    h = FindFirstFileW(wbuf, &findw); /* get a handle to the first filename in the search */
    free(wbuf); /* release the "wide" buffer before the pointer goes out of scope */
  }

  if (h == INVALID_HANDLE_VALUE) {
    free(findIn);
    ret = GetLastError();
    if (ret != ERROR_NO_MORE_FILES) {
      nDir = -1;
      get_ms_errmsg(errmsg, errmsg_sz);         /* return OS error msg */
    }
    *namelist = dir;
    return nDir;
  }
  do {
    int l = (int) wcslen(findw.cFileName);
    int dstlen = l * 5 + 1;
    selectDir=(struct dirent*)malloc(sizeof(struct dirent)+dstlen);

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
