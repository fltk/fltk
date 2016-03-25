//
// "$Id$"
//
// Font demo program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>

Fl_Double_Window *form;
Fl_Tile *tile;
Fl_Window *vector_font_editor = 0;

class FontDisplay : public Fl_Widget {
  void draw();
public:
  int font, size;
  FontDisplay(Fl_Boxtype B, int X, int Y, int W, int H, const char* L = 0) :
    Fl_Widget(X,Y,W,H,L) {box(B); font = 0; size = 14;}
};
void FontDisplay::draw() {
  draw_box();
  fl_font((Fl_Font)font, size);
  fl_color(FL_BLACK);
  fl_draw(label(), x()+3, y()+3, w()-6, h()-6, align());
}

FontDisplay *textobj;

Fl_Hold_Browser *fontobj, *sizeobj;

int **sizes;
int *numsizes;
int pickedsize = 14;

void font_cb(Fl_Widget *, long) {
  int fn = fontobj->value();
  if (!fn) return;
  fn--;
  textobj->font = fn;
  sizeobj->clear();
  int n = numsizes[fn];
  int *s = sizes[fn];
  if (!n) {
    // no sizes
  } else if (s[0] == 0) {
    // many sizes;
    int j = 1;
    for (int i = 1; i<64 || i<s[n-1]; i++) {
      char buf[20];
      if (j < n && i==s[j]) {sprintf(buf,"@b%d",i); j++;}
      else sprintf(buf,"%d",i);
      sizeobj->add(buf);
    }
    sizeobj->value(pickedsize);
  } else {
    // some sizes
    int w = 0;
    for (int i = 0; i < n; i++) {
      if (s[i]<=pickedsize) w = i;
      char buf[20];
      sprintf(buf,"@b%d",s[i]);
      sizeobj->add(buf);
    }
    sizeobj->value(w+1);
  }
  textobj->redraw();
}

void size_cb(Fl_Widget *, long) {
  int i = sizeobj->value();
  if (!i) return;
  const char *c = sizeobj->text(i);
  while (*c < '0' || *c > '9') c++;
  pickedsize = atoi(c);
  textobj->size = pickedsize;
  textobj->redraw();
}

char label[0x1000];

unsigned char current_char = 'A';
unsigned char vec[255][128] = {
  { 0 }
};

class LetterBox : public Fl_Group
{
public:
  LetterBox(int x, int y, int w, int h, const char *l)
  : Fl_Group(x, y, w, h, l) { }
  void draw() {
    draw_box();
    fl_push_clip(x(), y(), w(), h());
    draw_label(x(), y()-5, w(), h()-16, FL_ALIGN_CENTER);

    fl_color(FL_BLUE);
    fl_line_style(FL_SOLID|FL_CAP_ROUND|FL_JOIN_ROUND, 10);
    bool rendering = false;
    unsigned char *fd = vec[current_char];
    double px, py;
    for (;;) {
      unsigned char cmd = *fd++;
      if (cmd==0) {
        if (rendering) {
          fl_end_line();
          rendering = false;
        }
        break;
      } else if (cmd>63) {
        if (cmd=='\100' && rendering) {
          fl_end_line();
          rendering = false;
        }
      } else {
        if (!rendering) { fl_begin_line(); rendering = true; }
        int vx = (cmd & '\70')>>3;
        int vy = (cmd & '\07');
        px = (vx*16+8+10);
        py = (vy*32+16+10);
        fl_vertex(px, py);
      }
    }
    fl_line_style(FL_SOLID, 1);

    draw_children();
    fl_pop_clip();
  }
};

void add_point_cb(Fl_Widget *w, void *d)
{
  unsigned char *fd = vec[current_char];
  while (*fd) fd++;
  *fd = (fl_intptr_t)(d);
  w->parent()->redraw();
}

void add_gap_cb(Fl_Widget *w, void *d)
{
  unsigned char *fd = vec[current_char];
  while (*fd) fd++;
  *fd = '\100';
  w->parent()->redraw();
}

void clear_cb(Fl_Widget *w, void *d)
{
  unsigned char *fd = vec[current_char];
  memset(fd, 0, 128);
  w->parent()->redraw();
}

void prev_cb(Fl_Widget *w, void *d)
{
  current_char--;
  char b[2] = { current_char, 0 };
  w->parent()->child(0)->copy_label(b);
  w->parent()->child(0)->redraw();
}

void next_cb(Fl_Widget *w, void *d)
{
  current_char++;
  char b[2] = { current_char, 0 };
  w->parent()->child(0)->copy_label(b);
  w->parent()->child(0)->redraw();
}

void back_cb(Fl_Widget *w, void *d)
{
  unsigned char *fd = vec[current_char];
  if (*fd==0) return;
  while (*fd) fd++;
  *(--fd) = 0;
  w->parent()->child(0)->redraw();
}

void save_cb(Fl_Widget *w, void *d)
{
  const char *filename = fl_file_chooser("Save font as:", 0, 0);
  if (!filename) return;
  FILE *f = fopen(filename, "wb");
  if (!f) {
    fl_alert("can't open file for writing");
    return;
  }
  fprintf(f, "\nstatic const char *font_data[128] = {\n  ");
  for (int i=0; i<128; i++) {
    unsigned char *fd = vec[i];
    if (i>=32 && i<127) fprintf(f, "/*%c*/", i); else fprintf(f, "/*%02X*/", i);
    if (*fd==0) {
      fprintf(f, "0");
    } else {
      fprintf(f, "\"");
      for (;;) {
        unsigned char c = *fd++;
        if (c==0) break;
        fprintf(f, "\\%02o", c);
      }
      fprintf(f, "\"");
    }
    if (i<127) fprintf(f, ", ");
    if ((i&3)==3)fprintf(f, "\n  ");
  }
  fprintf(f, "};\n\n");
  fclose(f);
}

Fl_Window *create_editor()
{
  Fl_Window *win = new Fl_Double_Window(400,400);
  LetterBox *c = new LetterBox(10, 10, 128, 256, "A");
  //c->labelfont(FL_COURIER);
  c->align(FL_ALIGN_CENTER);
  c->labelsize(200);
  c->labelcolor(FL_DARK3);
  c->box(FL_DOWN_BOX);
  Fl_Button *b;
  int i, j;
  for (i=0; i<8; i++) {
    for (j=0; j<8; j++) {
      b = new Fl_Button(i*16+8-5+10, j*32+16-5+10, 10, 10);
      b->box(FL_OVAL_BOX);
      b->callback(add_point_cb, (void*)(fl_intptr_t)(i*8+j));
    }
  }
  c->end();

  b = new Fl_Button(10, 290, 70, 20, "Gap");
  b->callback(add_gap_cb);
  b = new Fl_Button(90, 290, 70, 20, "Clear");
  b->callback(clear_cb);
  b = new Fl_Button(10, 315, 70, 20, "<-");
  b->callback(prev_cb);
  b->shortcut(FL_Left);
  b = new Fl_Button(90, 315, 70, 20, "->");
  b->callback(next_cb);
  b->shortcut(FL_Right);
  b = new Fl_Button(10, 340, 70, 20, "Back");
  b->callback(back_cb);
  b = new Fl_Button(90, 340, 70, 20, "Save");
  b->callback(save_cb);
  b->shortcut(FL_COMMAND+'s');
  return win;
}

class MainWindow : public Fl_Double_Window
{
public:
  MainWindow(int w, int h, const char *l=0)
  : Fl_Double_Window(w, h, l) { }
  int handle(int event) {
    if (event==FL_KEYBOARD && Fl::event_key()==FL_F+1) {
      if (!vector_font_editor) vector_font_editor = create_editor();
      vector_font_editor->show();
      return 1;
    } else {
      return Fl_Double_Window::handle(event);
    }
  }
};

void create_the_forms() {
  // create the sample string
  int n = 0;
  strcpy(label, "Hello, world!\n");
  int i = strlen(label);
  ulong c;
  for (c = ' '+1; c < 127; c++) {
    if (!(c&0x1f)) label[i++]='\n';
    if (c=='@') label[i++]=c;
    label[i++]=c;
  }
  label[i++] = '\n';
  for (c = 0xA1; c < 0x600; c += 9) {
    if (!(++n&(0x1f))) label[i++]='\n';
    i += fl_utf8encode((unsigned int)c, label + i);
  }
  label[i] = 0;

  // create the basic layout
  form = new MainWindow(550,370);

  tile = new Fl_Tile(0, 0, 550, 370);

  Fl_Group *textgroup = new Fl_Group(0, 0, 550, 185);
  textgroup->box(FL_FLAT_BOX);
  textobj = new FontDisplay(FL_FRAME_BOX,10,10,530,170,label);
  textobj->align(FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
  textobj->color(9,47);
  textgroup->resizable(textobj);
  textgroup->end();

  Fl_Group *fontgroup = new Fl_Group(0, 185, 550, 185);
  fontgroup->box(FL_FLAT_BOX);
  fontobj = new Fl_Hold_Browser(10, 190, 390, 170);
  fontobj->box(FL_FRAME_BOX);
  fontobj->color(53,3);
  fontobj->callback(font_cb);
  sizeobj = new Fl_Hold_Browser(410, 190, 130, 170);
  sizeobj->box(FL_FRAME_BOX);
  sizeobj->color(53,3);
  sizeobj->callback(size_cb);
  fontgroup->resizable(fontobj);
  fontgroup->end();

  tile->end();

  form->resizable(tile);
  form->end();
}

int main(int argc, char **argv) {
  Fl::scheme(NULL);
  Fl::args(argc, argv);
  Fl::get_system_colors();
  create_the_forms();

// For the Unicode test, get all fonts...
//#ifdef __APPLE__
  int i = 0;
//#else
//  int i = fl_choice("Which fonts:","-*","iso8859","All");
//#endif
  int k = Fl::set_fonts(i ? (i>1 ? "*" : 0) : "-*");
  sizes = new int*[k];
  numsizes = new int[k];
  for (i = 0; i < k; i++) {
    int t; const char *name = Fl::get_font_name((Fl_Font)i,&t);
    char buffer[128];
#if 1
    if (t) {
      char *p = buffer;
      if (t & FL_BOLD) {*p++ = '@'; *p++ = 'b';}
      if (t & FL_ITALIC) {*p++ = '@'; *p++ = 'i';}
	  *p++ = '@'; *p++ = '.'; // Suppress subsequent formatting - some MS fonts have '@' in their name
      strcpy(p,name);
      name = buffer;
    }
#else // this is neat, but really slow on some X servers:
    sprintf(buffer, "@F%d@.%s", i, name);
    name = buffer;
#endif
    fontobj->add(name);
    int *s; int n = Fl::get_font_sizes((Fl_Font)i, s);
    numsizes[i] = n;
    if (n) {
      sizes[i] = new int[n];
      for (int j=0; j<n; j++) sizes[i][j] = s[j];
    }
  }
  fontobj->value(1);
  font_cb(fontobj,0);
  form->show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
