// Fl_display.C

// Startup method to set what display to use.
// Using setenv makes programs that are exec'd use the same display.

#include <FL/Fl.H>
#include <stdlib.h>
#include <string.h>

void Fl::display(const char *d) {
  char *e = new char[strlen(d)+13];
  strcpy(e,"DISPLAY=");
  strcpy(e+8,d);
  for (char *c = e+8; *c!=':'; c++) if (!*c) {strcpy(c,":0.0"); break;}
  putenv(e);
}
