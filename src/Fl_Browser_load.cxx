// Fl_Browser_load.C
// this should be moved to another source file, since it links stdio?

#include <FL/Fl.H>
#include <FL/Fl_Browser.H>
#include <stdio.h>

int Fl_Browser::load(const char *filename) {
#define MAXFL_BLINE 1024
    char newtext[MAXFL_BLINE];
    int c;
    int i;
    clear();
    if (!filename || !(filename[0])) return 1;
    FILE *fl = fopen(filename,"r");
    if (!fl) return 0;
    i = 0;
    do {
	c = getc(fl);
	if (c == '\n' || c <= 0 || i>=MAXFL_BLINE-1) {
	    newtext[i] = 0;
	    add(newtext);
	    i = 0;
	} else
	    newtext[i++] = c;
    } while (c >= 0);
    fclose(fl);
    return 1;
}
