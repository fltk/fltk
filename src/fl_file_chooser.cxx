//
// "$Id: fl_file_chooser.cxx,v 1.10.2.10 2001/05/05 23:39:01 spitzak Exp $"
//
// File chooser widget for the Fast Light Tool Kit (FLTK).
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

#include <config.h>
#include <FL/fl_file_chooser.H>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Browser_.H>
#include <FL/Fl_Input.H>
#include <FL/fl_draw.H>
#include <FL/filename.H>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

static void default_callback(const char*) {}
static void (*current_callback)(const char*) = default_callback;
void fl_file_chooser_callback(void (*cb)(const char*)) {
  current_callback = cb ? cb : default_callback;
}

// "File Chooser Browser" widget:
class FCB : public Fl_Browser_ {
  void* item_first() const ;
  void* item_next(void*) const ;
  void* item_prev(void*) const ;
  int item_height(const dirent*, int) const ;
  int item_height(void*) const ;
  int item_width(const dirent*) const ;
  int item_width(void*) const ;
  int item_quick_height(void*) const ;
  int incr_height() const ;
  void item_draw(void*, int, int, int, int) const ;
  int checkdir(const dirent*, char*) const ;
  void draw();
  void clear_prev();
public:
  char listed[FL_PATH_MAX];// current dir & starname
  int dirend;		// points after last / before starname
  int nameend;		// length to trailing '*' or '\0'
  const char* pattern;	// default pattern
  dirent** list;	// the file names
  dirent** last;	// pointer after end of list
  const char* message;	// message if no file names
  char preved[FL_PATH_MAX];// directory listed in prev
  dirent** prev;	// cached list of another directory
  dirent** prev_last;	// end of that list
  int prev_count;
  FCB(int x, int y, int w, int h) : Fl_Browser_(x, y, w, h, 0) {
    type(FL_HOLD_BROWSER);
    listed[0] = 0;
    dirend = nameend = 1;
    pattern = 0;
    list = prev = 0;
    message = 0;
  }
  // ~FCB nyi
  void clear();
  void set(const char*);
  int get(char*);
};

// "File Chooser Window" widget:
class FCW : public Fl_Window {
public:
  int handle(int);
  Fl_Input input;
  Fl_Button* ok_button;
  Fl_Button* cancel_button;
  Fl_Button* normal_button;
  FCB browser;
  FCW();
};

/* Files are marked as being directories by replacing the trailing null
   with a '/' if it is a directory, a '\001' if it is *not* a directory.
   An item has height (and is thus selectable) if it is either a directory
   or if it matches the pattern.  Quick-height assummes all unknown files
   are directories, and thus saves the time needed to do a stat().
*/

// return pointer to last character:
static const char* end_of_name(const dirent* d) {
#if HAVE_DIRENT_H
  const char* e;
  for (e = d->d_name; ;e++) switch (*e) {
  case 0: case 1: case '/': return e;
  }
#else
  // warning: clobbers byte after end of name
  return d->d_name + d->d_namelen;
#endif
}

// return true if item is directory, when given pointer to last character:
int FCB::checkdir(const dirent* d, char* e) const {
  if (*e == 1) return 0;
  if (*e == '/') return 1;
  char buf[FL_PATH_MAX];
  memcpy(buf, listed, dirend);
  memcpy(buf+dirend, d->d_name, e-d->d_name);
  *(buf+dirend+(e-d->d_name)) = 0;
  if (filename_isdir(buf)) {
    *e = '/'; return 1;
  } else {
    *e = 1; return 0;
  }
}

void* FCB::item_first() const {return list;}

void* FCB::item_next(void* p) const {
  if ((dirent**)p+1 >= last) return 0;
  return (dirent**)p+1;
}

void* FCB::item_prev(void* p) const {
  if ((dirent**)p <= list) return 0;
  return ((dirent**)p)-1;
}

#ifdef _MSC_VER
#pragma optimize("a",off) // without this it does not change *e
#endif
static int ido_matching(const dirent* p, const char* e, const char* n) {
  // replace / or 1 at end with 0 and do match, then put back.  yukko
  int save = *e; *(char*)e = 0;
  int r = filename_match(p->d_name, n);
  *(char*)e = save;
  return(r);
}
#ifdef _MSC_VER
#pragma optimize("",on)
#endif

int FCB::incr_height() const {return textsize()+2;}

int FCB::item_height(const dirent* p, int slow) const {
  const char* e = end_of_name(p);
  if (listed[dirend]) {
//  if (p->d_name[0]=='.' && listed[dirend]!='.') return 0;
    if (listed[nameend-1]=='/') {
      if (slow ? !checkdir(p, (char*)e) : *e==1) return 0;
      ((char*)listed)[nameend-1] = 0;
      int r = ido_matching(p, e, listed+dirend);
      ((char*)listed)[nameend-1] = '/';
      if (!r) return 0;
    } else {
      if (!ido_matching(p, e, listed+dirend)) return 0;
    }
  } else {
    if (p->d_name[0]=='.') return 0;
    if (pattern && (slow ? !checkdir(p, (char*)e) : *e==1) &&
	!ido_matching(p, e, pattern)) return 0;
  }
  return textsize()+2;
}

int FCB::item_height(void* x) const {
  return item_height(*(const dirent**)x, 1);
}

int FCB::item_quick_height(void* x) const {
  return item_height(*(const dirent**)x, 0);
}

void FCB::item_draw(void* v, int x, int y, int, int h) const {
  const dirent* p = *(const dirent**)v;
  const char* e = end_of_name(p);
  if (checkdir(p, (char*)e)) e++;
  if (v == selection()) fl_color(contrast(textcolor(), selection_color()));
  else fl_color(textcolor());
  fl_font(textfont(), textsize());
  fl_draw(p->d_name, e-p->d_name, x+4, y+h-3);
}

int FCB::item_width(const dirent* p) const {
  const char* e = end_of_name(p); if (*e == '/') e++;
  fl_font(textfont(), textsize());
  return (int)fl_width(p->d_name, e-p->d_name)+4;
}

int FCB::item_width(void* x) const {
  return item_width(*(const dirent**)x);
}

// "get" the current value by copying the name of the selected file
// or if none are selected, by copying as many common letters as
// possible of the matched file list:
int FCB::get(char* buf) {
  dirent** q = (dirent**)selection(); // the file to copy from
  int n = 0;	// number of letters
  if (q) {	// a file is selected
    const char* e = end_of_name(*q);
    n = e - (*q)->d_name;
    if (*e == '/') n++;
  } else {	// do filename completion
    for (q = list; q < last && !item_height(*q, 0); q++);
    if (q < last) {
      const char* e = end_of_name(*q);
      n = e - (*q)->d_name;
      if (*e == '/') n++;
      for (dirent** r = q+1; n && r < last; r++) {
	if (!item_height(*r, 0)) continue;
	int i;
#ifdef WIN32
	for (i=0; i<n && tolower((*q)->d_name[i])==tolower((*r)->d_name[i]); i++) {}
#else
	for (i=0; i<n && (*q)->d_name[i]==(*r)->d_name[i]; i++) {}
#endif
	n = i;
      }
    }
  }
  if (n) {
    memcpy(buf, listed, dirend);
    memcpy(buf+dirend, (*q)->d_name, n);
    buf[dirend+n]=0;
  }
  return n;
}

// "set" the current value by changing the directory being listed and
// changing the highlighted item, if possible:
void FCB::set(const char* buf) {

  int bufdirend;
  int ispattern = 0;
  const char* c = buf;
  for (bufdirend=0; *c;) switch(*c++) {
  case '?': case '[': case '*': case '{': ispattern = 1; goto BREAK;
#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
  case '\\':
#endif
  case '/': bufdirend=c-buf; break;
  }
#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
  if ((!bufdirend) && isalpha(buf[0]) && (buf[1]==':')) bufdirend = 2;
#endif
 BREAK:
  int bufend = strlen(buf);
  if (bufend<=bufdirend) ispattern = 1;

  // if directory is different, change list to xxx/ :
  if (bufdirend != dirend || strncmp(buf, listed, bufdirend)) {
    if (prev &&
	preved[bufdirend]==0 && !strncmp(buf, preved, bufdirend)) {
      strcpy(preved, listed); preved[dirend] = 0;
      dirent** t;
      t = prev; prev = list; list = t;
      t = prev_last; prev_last = last; last = t;
      strcpy(listed, buf);
      dirend = nameend = bufdirend;
      message = 0;
    } else {
      if (list) {
	clear_prev();
	strcpy(preved, listed); preved[dirend]=0;
	prev = list;
	prev_last = last;
      }
      list = last = 0;
      message = "reading..."; redraw(); Fl::flush(); redraw();
      strcpy(listed, buf);
      dirend = nameend = bufdirend;
      listed[dirend] = listed[dirend+1] = 0;
      int n = filename_list(dirend ? listed : ".", &list);
      if (n < 0) {
	if (errno==ENOENT) message = "No such directory";
	else message = strerror(errno);
	n = 0; list = 0;
      } else message = 0;
      last = list+n;
    }
    if (list && last <= list+2) message = "Empty directory";
    new_list();
  }

  dirent** q = 0; // will point to future selection
  int any = 0; // true if any names shown

  // do we match one item in the previous list?
  if (!ispattern && bufend >= nameend) {
    for (q = list; ; q++) {
      if (q >= last) {q = 0; break;}
      if (item_height(*q, 0)==0) continue;
      any = 1;
      const char* a = (*q)->d_name;
      const char* b = buf+bufdirend;
#if defined(WIN32) && !defined(__CYGWIN__)
      while (*b && tolower(*a)==tolower(*b)) {a++; b++;}
#else
      while (*b && *a==*b) {a++; b++;}
#endif
      if (!*b && (*a==0 || /* *a=='/' ||*/ *a==1)) break;
    }
  }

  // no, change the list pattern to the new text + a star:
  if (!q) {
    strcpy(listed+dirend, buf+bufdirend);
    nameend = bufend;
    if (!ispattern) {listed[nameend]='*'; listed[nameend+1]=0;}
    any = 0;
    // search again for an exact match:
    for (q = list; ; q++) {
      if (q >= last) {q = 0; break;}
      if (item_height(*q, 0)==0) continue;
      any = 1;
      const char* a = (*q)->d_name;
      const char* b = buf+bufdirend;
#if defined(WIN32) && !defined(__CYGWIN__)
      while (*b && tolower(*a)==tolower(*b)) {a++; b++;}
#else
      while (*b && *a==*b) {a++; b++;}
#endif
      if (!*b && (*a==0 || /* *a=='/' ||*/ *a==1)) break;
    }
    new_list();
  }

  if (any) message = 0;
  else if (!message) message = "No matching files";
  select_only(q);
  if (q) current_callback(buf);
}

void FCB::draw() {
  if (!message) {
    Fl_Browser_::draw();
    if (full_height() > 0) return;
    message = "No matching files";
  }
  Fl_Boxtype b = box(); if (!b) b = FL_DOWN_BOX;
  draw_box(b,color());
  fl_color(FL_INACTIVE_COLOR);
  fl_font(textfont(), textsize());
  fl_draw(message, x()+7, y()+3, w(), h()-3, FL_ALIGN_TOP_LEFT);
  // insure scrollbars are redrawn if error message goes away:
  scrollbar.redraw();
  hscrollbar.redraw();
}

void FCB::clear_prev() {
  if (prev) {
    for (dirent**p=prev_last-1; p>=prev; p--) free((void*)*p);
    free((void*)prev);
    prev = prev_last = 0;
  }
}

void FCB::clear() {
  if (list) {
    for (dirent**p=last-1; p>=list; p--) free((void*)*p);
    free((void*)list);
    list = last = 0;
  }
  clear_prev();
  listed[0] = 0; dirend = 1;
}

////////////////////////////////////////////////////////////////

static void fcb_cb(Fl_Widget*, void* v) {
  FCW* w = (FCW*)v;
  char buf[FL_PATH_MAX];
  if (w->browser.get(buf)) {
    w->input.value(buf);
    w->input.position(10000);
//  w->input.position(10000, w->browser.dirend);
    if (Fl::event_button()==1) {
      if (Fl::event_clicks()) w->ok_button->do_callback();
      else w->browser.set(buf);
    } else {
      current_callback(buf);
    }
  }
}

static void tab_cb(Fl_Widget*, void* v) {
  FCW* w = (FCW*)v;
  char buf[FL_PATH_MAX];
  if (w->browser.get(buf)) {
    w->input.value(buf);
    w->input.position(10000);
    w->browser.set(buf);
  }
}

#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
// ':' needs very special handling!
static inline int isdirsep(char c) {return c=='/' || c=='\\';}
#else
#define isdirsep(c) ((c)=='/')
#endif

static void input_cb(Fl_Widget*, void* v) {
  FCW* w = (FCW*)v;
  const char* buf = w->input.value();
  char localbuf[FL_PATH_MAX];
  if (buf[0] && isdirsep(buf[w->input.size()-1])
      && filename_expand(localbuf, buf)) {
    buf = localbuf;
    w->input.value(localbuf);
    w->input.position(10000);
  }
  w->browser.set(buf);
}

static void up_cb(Fl_Widget*wd, void*) { // the .. button
  FCW* w = (FCW*)(wd->window());
  char* p;
  const char* newname;
  char buf[FL_PATH_MAX];
  p = w->browser.listed+w->browser.dirend-1; // point right before last '/'
  if (p < w->browser.listed)
    newname = "../"; // go up from current directory
  else {
    for (; p>w->browser.listed; p--) if (isdirsep(*(p-1))) break;
    if (isdirsep(*p) || *p=='.' &&
	(isdirsep(p[1]) || p[1]=='.' && isdirsep(p[2]))) {
      p = w->browser.listed+w->browser.dirend;
      memcpy(buf, w->browser.listed, p-w->browser.listed);
      strcpy(buf+(p-w->browser.listed), "../");
    } else {
      memcpy(buf, w->browser.listed, p-w->browser.listed);
      buf[p-w->browser.listed] = 0;
    }
    newname = buf;
  }
  w->input.value(newname);
  w->input.position(10000);
  w->browser.set(newname);
}

static void dir_cb(Fl_Widget* obj, void* v) { // directory buttons
  FCW* w = (FCW*)(obj->window());
  char buf[FL_PATH_MAX];
  filename_expand(buf, (const char*)v);
  w->input.value(buf);
  w->input.position(10000);
  w->browser.set(buf);
}

static void working_cb(Fl_Widget* obj, void*) { // directory buttons
  FCW* w = (FCW*)(obj->window());
  char buf[FL_PATH_MAX];
  filename_absolute(buf, "");
  w->input.value(buf);
  w->input.position(10000);
  w->browser.set(buf);
}

static void files_cb(Fl_Widget* obj, void* v) { // file pattern buttons
  FCW* w = (FCW*)(obj->window());
  char buf[FL_PATH_MAX];
  strcpy(buf, w->input.value());
  char* q = buf+w->browser.dirend;
  if (v) strcpy(q, (char*)v); else *q = 0;
  w->input.value(buf);
  w->input.position(10000, w->browser.dirend);
  w->browser.set(buf);
}

/*----------------------- The Main Routine ----------------------*/
#define HEIGHT_BOX	(4*WIDTH_SPC+HEIGHT_BUT+HEIGHT_INPUT+HEIGHT_BROWSER)
#define HEIGHT_BUT	23
#define HEIGHT_INPUT	23
#define HEIGHT_BROWSER	(9*HEIGHT_BUT+2) // must be > buttons*HEIGHT_BUT
#define WIDTH_BOX	(3*WIDTH_SPC+WIDTH_BUT+WIDTH_BROWSER)
#define WIDTH_BROWSER	350
#define WIDTH_BUT	125
#define WIDTH_OK	60
#define WIDTH_SPC	5

int FCW::handle(int event) {
  if (Fl_Window::handle(event)) return 1;
  if (event==FL_KEYBOARD && Fl::event_key()==FL_Tab) {
    tab_cb(this, this);
    return 1;
  }
  return 0;
}

// set this to make extra directory-jumping button:
static Fl_Button* extra_button;
const char* fl_file_chooser_button;
const char* fl_file_chooser_data;
extern const char* fl_ok;
extern const char* fl_cancel;

FCW::FCW() : Fl_Window(WIDTH_BOX, HEIGHT_BOX),
	input(WIDTH_SPC, HEIGHT_BOX-HEIGHT_BUT-2*WIDTH_SPC-HEIGHT_INPUT,
	      WIDTH_BOX-2*WIDTH_SPC, HEIGHT_INPUT, 0),
	browser(2*WIDTH_SPC+WIDTH_BUT, WIDTH_SPC,
		WIDTH_BROWSER, HEIGHT_BROWSER)
{
  int but_y = WIDTH_SPC;
  input.callback(input_cb, this);
  input.when(FL_WHEN_CHANGED);
  //  add(browser);
  browser.callback(fcb_cb, this);

  begin();
  Fl_Widget* obj;
  obj = ok_button = new Fl_Return_Button(
    WIDTH_BOX-2*(WIDTH_SPC+WIDTH_OK), HEIGHT_BOX-WIDTH_SPC-HEIGHT_BUT,
    WIDTH_OK, HEIGHT_BUT, fl_ok);
  obj = cancel_button = new Fl_Button(
    WIDTH_BOX-WIDTH_SPC-WIDTH_OK, HEIGHT_BOX-WIDTH_SPC-HEIGHT_BUT,
    WIDTH_OK, HEIGHT_BUT, fl_cancel);
  cancel_button->shortcut("^[");

  obj=new Fl_Button(WIDTH_SPC,but_y,WIDTH_BUT,HEIGHT_BUT, "&Up one directory");
  obj->callback(up_cb);
  but_y += HEIGHT_BUT;

  obj = new Fl_Button(WIDTH_SPC, but_y, WIDTH_BUT, HEIGHT_BUT, "&~ Home");
  obj->callback(dir_cb, (void*)"~/");
  but_y += HEIGHT_BUT;

  obj = new Fl_Button(WIDTH_SPC, but_y, WIDTH_BUT, HEIGHT_BUT, "&/ Root");
  obj->callback(dir_cb, (void*)"/");
  but_y += HEIGHT_BUT;

  obj=new Fl_Button(WIDTH_SPC, but_y, WIDTH_BUT, HEIGHT_BUT, "&Current dir");
  obj->callback(working_cb);
  but_y += HEIGHT_BUT;

  if (fl_file_chooser_button) {
    obj = extra_button = new Fl_Button(WIDTH_SPC,but_y,WIDTH_BUT,HEIGHT_BUT,fl_file_chooser_button);
    obj->callback(dir_cb, (void*)fl_file_chooser_button);
    but_y += HEIGHT_BUT;
  }

  normal_button = new Fl_Button(WIDTH_SPC, but_y, WIDTH_BUT, HEIGHT_BUT, "");
  normal_button->callback(files_cb, 0);
  but_y += HEIGHT_BUT;

  obj = new Fl_Button(WIDTH_SPC,but_y, WIDTH_BUT, HEIGHT_BUT, "&All files");
  obj->callback(files_cb, (void*)"*");
  but_y += HEIGHT_BUT;

  obj = new Fl_Button(WIDTH_SPC,but_y,WIDTH_BUT,HEIGHT_BUT, "&Hidden files");
  obj->callback(files_cb, (void*)".");
  but_y += HEIGHT_BUT;

  obj = new Fl_Button(WIDTH_SPC,but_y,WIDTH_BUT,HEIGHT_BUT, "&Directories");
  obj->callback(files_cb, (void*)"*/");
  but_y += HEIGHT_BUT;

  resizable(new Fl_Box(browser.x(), but_y,
		       ok_button->x()-browser.x(),
		       browser.y()+browser.h()-but_y));
  // add(input); // put last for better draw() speed
  end();
  set_modal();
}

char* fl_file_chooser(const char* message, const char* pat, const char* fname)
{
  static FCW* f; if (!f) f = new FCW();
  f->ok_button->label(fl_ok);
  f->cancel_button->label(fl_cancel);
  if (extra_button) {
    extra_button->label(fl_file_chooser_button);
    extra_button->user_data((void*)(fl_file_chooser_data ? fl_file_chooser_data : fl_file_chooser_button));
  }

  if (pat && !*pat) pat = 0;
  if (fname && *fname) {
    f->input.value(fname);
  } else if (f->browser.pattern != pat && (!pat || !f->browser.pattern ||
					   strcmp(pat,f->browser.pattern))) {
    // if pattern is different, remove name but leave old directory:
    const char* p = f->input.value();
    const char* q = filename_name(p);
    f->input.value(p, q-p);
  }
  f->browser.pattern = pat;
  f->normal_button->label(pat ? pat : "visible files");
  f->normal_button->user_data((void*)(pat ? pat : 0));
  f->browser.set(f->input.value());
  f->input.position(10000, f->browser.dirend);

  f->label(message);
  f->hotspot(f);
  f->show();
  int ok = 0;
  for (;;) {
    Fl::wait();
    Fl_Widget* o = Fl::readqueue();
    if (o == f->ok_button) {ok = 1; break;}
    else if (o == f->cancel_button || o == f) break;
  }
  f->hide();
  f->browser.clear();

  if (!ok) return 0;
  const char* r = f->input.value();
  const char* p;
  for (p=r+f->browser.dirend; *p; p++)
    if (*p=='*' || *p=='?' || *p=='[' || *p=='{') return 0;
  return (char*)r;
}

//
// End of "$Id: fl_file_chooser.cxx,v 1.10.2.10 2001/05/05 23:39:01 spitzak Exp $".
//
