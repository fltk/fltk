// Replace .ext with new extension
// If no . in name, append new extension
// If new extension is null, act like it is ""

#include <FL/filename.H>
#include <string.h>

char *filename_setext(char *buf, const char *ext) {
  char *q = (char *)filename_ext(buf);
  if (ext) strcpy(q,ext); else *q = 0;
  return(buf);
}
