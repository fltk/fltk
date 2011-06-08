/* fromdos.c : strip the stupid ^M characters without mistakes! */

/* this can do in-place conversion or be used as a pipe... */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv) {
  int f,c;
  if (argc <= 1) {
    if (isatty(0)) {
      fprintf(stderr,"usage : %s <files>\nStrips ^M characters.\nCan do in-place conversion of many files or can be used in a pipe\n",argv[0]);
      return 1;
    }
    for (;;) {
      c = getchar();
      while (c == '\r') {
	c = getchar();
	if (c != '\n') putchar(c);
      }
      if (c < 0) break;
      putchar(c);
    }
    return 0;
  }
  for (f = 1; f < argc; f++) {
    char* fname = argv[f];
    char tempname[1024];
    FILE* in = fopen(fname,"rb");
    FILE* out;
    int mod = 0;
    if (!in) {
      fprintf(stderr,"%s : %s\n", fname, strerror(errno));
      return 1;
    }
    strcpy(tempname, fname);
    strcat(tempname, ".temp");
    out = fopen(tempname, "wb");
    if (!out) {
      fprintf(stderr,"%s : %s\n", fname, strerror(errno));
      return 1;
    }
    for (;;) {
      c = getc(in);
      while (c == '\r') {
	c = getc(in);
	if (c == '\n') mod=1; else putc(c,out);
      }
      if (c < 0) break;
      putc(c,out);
    }
    fclose(in);
    fclose(out);
    if (!mod) {
      fprintf(stderr,"%s : no change\n", fname);
      unlink(tempname);
    } else if (rename(tempname, fname)) {
      fprintf(stderr,"Can't mv %s %s : %s\n",tempname,fname,strerror(errno));
      return 1;
    }
  }
  return 0;
}
