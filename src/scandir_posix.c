/*
 * This implementation of 'scandir()' is intended to be POSIX.1-2008 compliant.
 * A POSIX.1-1990 compliant system is required as minimum base.
 * Note:
 * The 'const' declarations were removed to match FLTK 1.3 wrapper (STR #2931)
 *
 * Copyright (c) 2013 by Michael Baeuerle
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file. If this
 * file is missing or damaged, see the license at:
 *
 *     https://www.fltk.org/COPYING.php
 *
 * Please see the following page on how to report bugs and issues:
 *
 *     https://www.fltk.org/bugs.php
 *
 * It is required that 'SIZE_MAX' is at least 'INT_MAX'.
 * Don't use a C++ compiler to build this module.
 *
 * The build system must define 'HAVE_PTHREAD' and link against a potentially
 * required library to switch this implementation into thread-safe mode.
 * The POSIX.1c-1995 extension is required if 'HAVE_PTHREAD' is defined.
 *
 * Note:
 * In theory, a system that provides threads should also provide 'readdir_r()',
 * a thread-safe version of 'readdir()'. In reality this is not always the case.
 * In addition there may be a race condition that can lead to a buffer overflow:
 * http://womble.decadent.org.uk/readdir_r-advisory.html
 */

#include <config.h>
#if defined(FLTK_USE_X11) && !defined(HAVE_SCANDIR)

#ifndef HAVE_PTHREAD
   /* Switch system headers into POSIX.1-1990 mode */
#  define _POSIX_SOURCE
#else  /* HAVE_PTHREAD */
   /* Switch system headers into POSIX.1c-1995 mode */
#  define _POSIX_C_SOURCE  199506L
#endif  /* HAVE_PTHREAD */

#include <sys/types.h>        /* XPG2 require this for '*dir()' functions */
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>           /* For 'malloc()', 'realloc()' and 'qsort()' */
#include <stddef.h>           /* For 'offsetof()', 'NULL' and 'size_t' */
#include <limits.h>           /* For 'INT_MAX' */
#include <string.h>           /* For 'memcpy()' */
#include "flstring.h"         /* For 'fl_snprintf()' */
#if defined(HAVE_PTHREAD) && defined(HAVE_PTHREAD_H)
#  include <pthread.h>
#endif  /* HAVE_PTHREAD */


/* ========================================================================== */
/* At startup allocate memory for this number of result array elements */
#define ENTRIES_MIN  (size_t) 32


/* ========================================================================== */
#ifdef HAVE_PTHREAD
static pthread_mutex_t scandir_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif  /* HAVE_PTHREAD */


/* ========================================================================== */
/*
 * This function reads the next entry from the directory referenced by 'dirp',
 * allocates a buffer for the entry and copies it into this buffer.
 * A pointer to this buffer is written to 'entryp' and the size of the buffer
 * is written to 'len'.
 * Success and a NULL pointer is returned for 'entryp' if there are no more
 * entries in the directory.
 * On success zero is returned and the caller is responsible for 'free()'ing
 * the buffer after use.
 * On error the return value is nonzero, 'entryp' and 'len' are invalid.
 *
 * Should be declared as 'static inline' if the compiler supports that.
 */
static int
readentry(DIR *dirp, struct dirent **entryp, size_t *len)
{
  int result = -1;
  struct dirent *e;

#ifdef HAVE_PTHREAD
  if (!pthread_mutex_lock(&scandir_mutex))
  {
    /* Ensure that there is no code path that bypasses the '_unlock()' call! */
#endif  /* HAVE_PTHREAD */
    errno = 0;
    e = readdir(dirp);
    if (NULL == e)
    {
      if (!errno)
      {
        /* No more entries in directory */
        *entryp = NULL;
        *len = 0;
        result = 0;
      }
    }
    else
    {
      /* Entry found, allocate local buffer */
      *len = offsetof(struct dirent, d_name) + strlen(e->d_name) + (size_t) 1;
      *entryp = (struct dirent *) malloc(*len);
      if (NULL != *entryp)
      {
        memcpy((void *) *entryp, (void *) e, *len);
        /* Force NUL termination at end of buffer */
        ((char *) *entryp)[*len - (size_t) 1] = 0;
        result = 0;
      }
    }
#ifdef HAVE_PTHREAD
    /*
     * In a multithreading environment the system's dirent buffer may be shared
     * between all threads. Therefore the mutex must stay locked until we have
     * copied the data to our thread local buffer.
     */
    pthread_mutex_unlock(&scandir_mutex);
  }
#endif  /* HAVE_PTHREAD */

  return result;
}


/*
 * This could use some docs.
 *
 * Returns -1 on error, errmsg returns error string (if non-NULL)
 */
int
fl_scandir(const char *dir, struct dirent ***namelist,
           int (*sel)(struct dirent *),
           int (*compar)(struct dirent **, struct dirent **),
           char *errmsg, int errmsg_sz)
{
  int result = -1;
  DIR *dirp;
  size_t len, num = 0, max = ENTRIES_MIN;
  struct dirent *entryp, **entries, **p;

  if (errmsg && errmsg_sz>0) errmsg[0] = '\0';
  entries = (struct dirent **) malloc(sizeof(*entries) * max);
  if (NULL == entries) {
    if (errmsg) fl_snprintf(errmsg, errmsg_sz, "out of memory");
    return -1;
  }

  /* Open directory 'dir' (and verify that it really is a directory) */
  dirp = opendir(dir);
  if (NULL == dirp) {
    if (errmsg) fl_snprintf(errmsg, errmsg_sz, "%s", strerror(errno));

    // XXX: This would be a thread safe alternative to the above, but commented
    //      out because we can get either GNU or POSIX versions on linux,
    //      which AFAICT are incompatible: GNU doesn't guarantee errmsg is used
    //      at all, whereas POSIX /only/ fills buffer. The two calls are not really
    //      compatible but have the same name and different return values.. wtf?
    //
    // if (errmsg && errmsg_sz > 0) {
    //   strerror_r(errno, errmsg, errmsg_sz); // thread safe. Might be GNU, might be POSIX
    //   errmsg[errmsg_sz-1] = '\0';           // force null term b/c XSI does not specify
    // }
    return -1;
  }

  /* Read next directory entry */
  while (!readentry(dirp, &entryp, &len))
  {
    if (NULL == entryp)
    {
      /* EOD => Return number of directory entries */
      result = (int) num;
      break;
    }
    /* Apply select function if there is one provided */
    if (NULL != sel)  { if (!sel(entryp))  continue; }
    entries[num++] = entryp;
    if (num >= max) {
      /* Allocate exponentially increasing sized memory chunks */
      if (INT_MAX / 2 >= (int) max)  { max *= (size_t) 2; }
      else {
        errno = ENOMEM;
        break;
      }
      p = (struct dirent **) realloc((void *)entries, sizeof(*entries)*max);
      if (NULL != p) { entries = p; }
      else  break;
    }
  }
  closedir(dirp);
  /*
   * A standard compliant 'closedir()' is allowed to fail with 'EINTR',
   * but the state of the directory structure is undefined in this case.
   * Therefore we ignore the return value because we can't call 'closedir()'
   * again and must hope that the system has released all resources.
   */

  /* Sort entries in array if there is a compare function provided */
  if (NULL != compar) {
    qsort((void *) entries, num, sizeof(*entries),
          (int (*)(const void *, const void *)) compar);
  }
  *namelist = entries;
  /* Check for error */
  if (-1 == result) {
    /* Free all memory we have allocated */
    while (num--)  { free(entries[num]); }
    free(entries);
  }
  return result;
}

#else /* defined(FLTK_USE_X11) && !defined(HAVE_SCANDIR) */

/* avoid (gcc) compiler warning [-Wpedantic]
   "ISO C forbids an empty translation unit" */

typedef int dummy;

#endif /* defined(FLTK_USE_X11) && !defined(HAVE_SCANDIR) */
