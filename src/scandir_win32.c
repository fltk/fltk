// scandir_win32.C

// Emulation of posix scandir() call

#include <config.h>
#include <FL/filename.H>
#include <string.h>
#include <windows.h>

#ifdef __cplusplus
extern "C"
#endif 
int scandir(const char *dirname, struct dirent ***namelist,
    int (*select)(const struct dirent *),
    int (*compar)(const struct dirent **, const struct dirent **)) {

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

int alphasort (const struct dirent **a, const struct dirent **b) {
  return strcmp ((*a)->d_name, (*b)->d_name);
}
