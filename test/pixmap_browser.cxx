//
// "$Id: pixmap_browser.cxx,v 1.5.2.4 2001/01/22 15:13:41 easysw Exp $"
//
// Another pixmap test program for the Fast Light Tool Kit (FLTK).
//
// On purpose, I do NOT provide a fltk method to turn a file
// into a pixmap.  This program uses a rather simplistic one.
//
// Copyright 1998-2001 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pixmap.H>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <FL/fl_file_chooser.H>
#include <FL/fl_message.H>

Fl_Box *b;
Fl_Window *w;

char **data;
int sizeofdata;
int numlines;

static int hexdigit(int x) {
  if (isdigit(x)) return x-'0';
  if (isupper(x)) return x-'A'+10;
  if (islower(x)) return x-'a'+10;
  return 20;
}

int load_file(const char *name) {
  FILE *f = fopen(name,"r");
  if (!f) {
    fl_message("Can't open %s, %s",name,strerror(errno));
    return 0;
  }
  if (data) {
    for (int i=numlines; i--;) delete[] data[i];
  }
#define BUFSIZE 2048
  char buffer[BUFSIZE];
  int i = 0;
  while (fgets(buffer, BUFSIZE, f)) {
    if (buffer[0] != '\"') continue;
    char *p = buffer;
    char *q = buffer+1;
    while (*q != '\"') {
      if (*q == '\\') switch (*++q) {
      case '\n':
	fgets(q,(buffer+BUFSIZE)-q,f); break;
      case 0:
	break;
      case 'x': {
	q++;
	int n = 0;
	for (int x = 0; x < 3; x++) {
	  int d = hexdigit(*q);
	  if (d > 15) break;
	  n = (n<<4)+d;
	  q++;
	}
	*p++ = n;
      } break;
      default: {
	int c = *q++;
	if (c>='0' && c<='7') {
	  c -= '0';
	  for (int x=0; x<2; x++) {
	    int d = hexdigit(*q);
	    if (d>7) break;
	    c = (c<<3)+d;
	    q++;
	  }
	}
	*p++ = c;
      } break;
      } else {
	*p++ = *q++;
      }
    }
    *p++ = 0;
    if (i >= sizeofdata) {
      sizeofdata = 2*sizeofdata+100;
      char **newdata = new char *[sizeofdata];
      for (int j=0; j<i; j++) newdata[j] = data[j];
      delete[] data;
      data = newdata;
    }
    data[i] = new char[p-buffer];
    memcpy(data[i],buffer,p-buffer);
    i++;
  }
  numlines = i;
  fclose(f);
  return i;
}

Fl_Pixmap *pixmap;
void newpixmap() {
  delete pixmap;
  pixmap = new Fl_Pixmap(data);
  pixmap->label(b);
  w->redraw();
}

static char name[1024];

void file_cb(const char *n) {
  if (!strcmp(name,n)) return;
  if (!load_file(n)) return;
  strcpy(name,n);
  w->label(name);
  newpixmap();
}

void button_cb(Fl_Widget *,void *) {
  fl_file_chooser_callback(file_cb);
  fl_file_chooser("XPM file","*.xpm",name);
  fl_file_chooser_callback(0);
}

int dvisual = 0;
int arg(int, char **argv, int &i) {
  if (argv[i][1] == '8') {dvisual = 1; i++; return 1;}
  return 0;
}

int main(int argc, char **argv) {
  int i = 1;
  if (Fl::args(argc,argv,i,arg) < argc)
    Fl::fatal(" -8 # : use default visual\n%s\n",Fl::help);

  Fl_Window window(400,400); ::w = &window;
  Fl_Box b(0,0,window.w(),window.h()); ::b = &b;
  Fl_Button button(5,5,100,35,"load");
  button.callback(button_cb);
  if (!dvisual) Fl::visual(FL_RGB);
  window.resizable(window);
  window.show(argc,argv);
  return Fl::run();
}

//
// End of "$Id: pixmap_browser.cxx,v 1.5.2.4 2001/01/22 15:13:41 easysw Exp $".
//
