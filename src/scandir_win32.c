//
// "$Id: scandir_win32.c,v 1.6 1998/10/21 14:21:11 mike Exp $"
//
// WIN32 scandir function for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

// Emulation of posix scandir() call

#include <config.h>
#include <FL/filename.H>
#include <string.h>
#include <windows.h>

#ifdef __cplusplus
extern "C"
#endif 
int scandir(const char *dirname, struct dirent ***namelist,
    int (*select)(struct dirent *),
    int (*compar)(struct dirent **, struct dirent **)) {

  int len = strlen(dirname);
  char *findIn = new char[len+5]; strcpy(findIn, dirname);
  for (char *d = findIn; *d; d++) if (*d=='/') *d='\\';
  if ((len==0)) { strcpy(findIn, ".\\*"); }
  if ((len==1)&& (d[-1]=='.')) { strcpy(findIn, ".\\*"); }
  if ((len>0) && (d[-1]=='\\')) { *d++ = '*'; *d = 0; }
  if ((len>1) && (d[-1]=='.') && (d[-2]=='\\')) { d[-1] = '*'; }
  
  WIN32_FIND_DATA find;
  HANDLE h;
  int nDir = 0, NDir = 0;
  struct dirent **dir = 0, *selectDir;
  /*
  selectDir = (struct dirent*)new char[sizeof(dirent)+1];
  strcpy(selectDir->d_name, ".");
  dir[0] = selectDir;
  selectDir = (struct dirent*)new char[sizeof(dirent)+2];
  strcpy(selectDir->d_name, "..");
  dir[1] = selectDir;
  */
  unsigned long ret;

  if ((h=FindFirstFile(findIn, &find))==INVALID_HANDLE_VALUE) {
    ret = GetLastError();
    if (ret != ERROR_NO_MORE_FILES) {
      // TODO: return some error code
    }
    *namelist = dir;
    return nDir;
  }
  do {
    selectDir=(struct dirent*)new char[sizeof(dirent)+strlen(find.cFileName)];
    strcpy(selectDir->d_name, find.cFileName);
    if (!select || (*select)(selectDir)) {
      if (nDir==NDir) {
	struct dirent **tempDir = new struct dirent*[NDir+33];
	if (NDir) memcpy(tempDir, dir, sizeof(struct dirent*)*NDir);
	if (dir) delete dir;
	dir = tempDir;
	NDir += 32;
      }
      dir[nDir] = selectDir;
      nDir++;
      dir[nDir] = 0;
    } else {
      delete selectDir;
    }
  } while (FindNextFile(h, &find));
  ret = GetLastError();
  if (ret != ERROR_NO_MORE_FILES) {
    // TODO: return some error code
  }
  FindClose(h);

  delete findIn;

  if (compar) qsort (dir, nDir, sizeof(*dir),
		     (int(*)(const void*, const void*))compar);

  *namelist = dir;
  return nDir;
}

int alphasort (struct dirent **a, struct dirent **b) {
  return strcmp ((*a)->d_name, (*b)->d_name);
}

//
// End of "$Id: scandir_win32.c,v 1.6 1998/10/21 14:21:11 mike Exp $".
//
