//
// X Color Browser demo program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//


#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Box.H>

#include <FL/fl_ask.H>
#include <FL/filename.H>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// some constants

#define MAX_RGB 3000

#define FL_FREE_COL4    ((Fl_Color)(FL_FREE_COLOR+3))
#define FL_INDIANRED    ((Fl_Color)(164))


static Fl_Double_Window *cl;
static Fl_Box *rescol;
static Fl_Button *dbobj;
static Fl_Hold_Browser *colbr;
static Fl_Value_Slider *rs, *gs, *bs;

static char dbname[FL_PATH_MAX];

static void create_form_cl(void);
static int load_browser(const char *);

typedef struct { int r, g, b; } RGBdb;

static RGBdb rgbdb[MAX_RGB];


int main(int argc, char *argv[]) {
  int i;
  if (!Fl::args(argc, argv, i)) Fl::fatal(Fl::help);
  strcpy(dbname, (i < argc) ? argv[i] : "rgb.txt");

  create_form_cl();

  if (load_browser(dbname))
    dbobj->label(dbname);
  else
    dbobj->label("None");
  dbobj->redraw();

  cl->size_range(cl->w(),cl->h(),2*cl->w(),2*cl->h());

  cl->label("RGB Browser");
  cl->free_position();
  cl->show(argc,argv);

  return Fl::run();
}


static void set_entry(int i)
{
  RGBdb *db = rgbdb + i;
  Fl::set_color(FL_FREE_COL4, db->r, db->g, db->b);
  rs->value(db->r);
  gs->value(db->g);
  bs->value(db->b);
  rescol->redraw();
}


static void br_cb(Fl_Widget *ob, long)
{
  int r = ((Fl_Browser *)ob)->value();

  if (r <= 0)
    return;
  set_entry(r - 1);
}


static int read_entry(FILE * fp, int *r, int *g, int *b, char *name)
{
  int  n;
  char buf[512], *p;

  if (!fgets(buf, sizeof(buf) - 1, fp))
    return 0;

  if(buf[0] == '!') {
    if (fgets(buf,sizeof(buf)-1,fp)==0) {
      /* ignore */
    }
  }

  if(sscanf(buf, " %d %d %d %n", r, g, b, &n) < 3)
    return 0;

  p = buf + n;

  /* squeeze out all spaces */
  while (*p)
  {
    if (*p != ' ' && *p != '\n')
      *name++ = *p;
    p++;
  }
  *name = 0;

  return (feof(fp) || ferror(fp)) ? 0 : 1;
}


static int load_browser(const char *fname)
{
  FILE *fp;
  RGBdb *db = rgbdb, *dbs = db + MAX_RGB;
  int r, g, b,  lr  = -1 , lg = -1, lb = -1;
  char name[256], buf[300];

#ifdef __EMX__
  if (!(fp = fl_fopen(__XOS2RedirRoot(fname), "r")))
#else
    if (!(fp = fl_fopen(fname, "r")))
#endif
  {
    fl_alert("Load:\nCan't open '%s'", fname);
    return 0;
  }

  /* read the items */

  for (; db < dbs && read_entry(fp, &r, &g, &b, name);)
  {
    db->r = r;
    db->g = g;
    db->b = b;

    /* unique the entries on the fly */
    if (lr != r || lg != g || lb != b)
    {
      db++;
      lr = r;
      lg = g;
      lb = b;
      sprintf(buf, "(%3d %3d %3d) %s", r, g, b, name);
      colbr->add(buf);
    }
  }
  fclose(fp);

  if (db < dbs)
    db->r = 1000;               /* sentinel */
  else
  {
    db--;
    db->r = 1000;
  }

  colbr->topline(1);
  colbr->select(1,1);
  set_entry(0);

  return 1;
}


static int search_entry(int r, int g, int b)
{
  RGBdb *db = rgbdb;
  int i, j, diffr, diffg, diffb;
  unsigned int diff, mindiff;

  mindiff = (unsigned int)~0;
  for (i = j = 0; db->r < 256; db++, i++)
  {
    diffr = r - db->r;
    diffg = g - db->g;
    diffb = b - db->b;

#ifdef FL_LINEAR
    diff = unsigned(3.0 * (FL_abs(r - db->r)) +
                    (5.9 * FL_abs(g - db->g)) +
                    (1.1 * (FL_abs(b - db->b))));
#else
    diff = unsigned(3.0 * (diffr *diffr) +
                    5.9 * (diffg *diffg) +
                    1.1 * (diffb *diffb));
#endif

    if (mindiff > diff)
    {
      mindiff = diff;
      j = i;
    }
  }

  return j;
}


static void search_rgb(Fl_Widget *, long)
{
  int r, g, b, i;
  int top  = colbr->topline();

  r = int(rs->value());
  g = int(gs->value());
  b = int(bs->value());

  // fl_freeze_form(cl);
  Fl::set_color(FL_FREE_COL4, r, g, b);
  rescol->redraw();
  i = search_entry(r, g, b);
  /* change topline only if necessary */
  if(i < top || i > (top+15))
    colbr->topline(i-8);
  colbr->select(i+1, 1);
  // fl_unfreeze_form(cl);
}


/* change database */
static void db_cb(Fl_Widget * ob, long)
{
  const char *p = fl_input("Enter New Database Name", dbname);
  char buf[512];

  if (!p || strcmp(p, dbname) == 0)
    return;

  strcpy(buf, p);
  if (load_browser(buf))
    strcpy(dbname, buf);
  else
    ob->label(dbname);
}


static void done_cb(Fl_Widget *, long)
{
  exit(0);
}


static void create_form_cl(void)
{
  if (cl)
    return;

  cl = new Fl_Double_Window(400,385);
  cl->box(FL_UP_BOX);
  cl->color(FL_INDIANRED, FL_GRAY);

  Fl_Box *title = new Fl_Box(40, 10, 300, 30, "Color Browser");
  title->box(FL_NO_BOX);
  title->labelcolor(FL_RED);
  title->labelsize(32);
  title->labelfont(FL_HELVETICA_BOLD);
  title->labeltype(FL_SHADOW_LABEL);

  dbobj = new Fl_Button(40, 50, 300, 25, "");
  dbobj->type(FL_NORMAL_BUTTON);
  dbobj->box(FL_BORDER_BOX);
  dbobj->color(FL_INDIANRED,FL_INDIANRED);
  dbobj->callback(db_cb, 0);

  colbr = new Fl_Hold_Browser(10, 90, 280, 240, "");
  colbr->textfont(FL_COURIER);
  colbr->callback(br_cb, 0);
  colbr->box(FL_DOWN_BOX);

  rescol = new Fl_Box(300, 90, 90, 35, "");
  rescol->color(FL_FREE_COL4, FL_FREE_COL4);
  rescol->box(FL_BORDER_BOX);

  rs = new Fl_Value_Slider(300, 130, 30, 200, "");
  rs->type(FL_VERT_FILL_SLIDER);
  rs->color(FL_INDIANRED, FL_RED);
  rs->bounds(0, 255);
  rs->precision(0);
  rs->callback(search_rgb, 0);
  rs->when(FL_WHEN_RELEASE);

  gs = new Fl_Value_Slider(330, 130, 30, 200, "");
  gs->type(FL_VERT_FILL_SLIDER);
  gs->color(FL_INDIANRED, FL_GREEN);
  gs->bounds(0, 255);
  gs->precision(0);
  gs->callback(search_rgb, 1);
  gs->when(FL_WHEN_RELEASE);

  bs = new Fl_Value_Slider(360, 130, 30, 200, "");
  bs->type(FL_VERT_FILL_SLIDER);
  bs->color(FL_INDIANRED, FL_BLUE);
  bs->bounds(0, 255);
  bs->precision(0);
  bs->callback(search_rgb, 2);
  bs->when(FL_WHEN_RELEASE);

  Fl_Button *done = new Fl_Button(160, 345, 80, 30, "Done");
  done->type(FL_NORMAL_BUTTON);
  done->callback(done_cb, 0);

  cl->end();
  cl->resizable(cl);
}
