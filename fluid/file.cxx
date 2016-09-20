//
// "$Id$"
//
// Fluid file routines for the Fast Light Tool Kit (FLTK).
//
// You may find the basic read_* and write_* routines to
// be useful for other programs.  I have used them many times.
// They are somewhat similar to tcl, using matching { and }
// to quote strings.
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#include <stdio.h>
#include <stdlib.h>
#include "../src/flstring.h"
#include <stdarg.h>
#include "alignment_panel.h"
#include <FL/Fl.H>
#include "Fl_Widget_Type.h"

////////////////////////////////////////////////////////////////
// BASIC FILE WRITING:

static FILE *fout;

int open_write(const char *s) {
  if (!s) {fout = stdout; return 1;}
  FILE *f = fl_fopen(s,"w");
  if (!f) return 0;
  fout = f;
  return 1;
}

int close_write() {
  if (fout != stdout) {
    int x = fclose(fout);
    fout = stdout;
    return x >= 0;
  }
  return 1;
}

static int needspace;
int is_id(char); // in code.C

// write a string, quoting characters if necessary:
void write_word(const char *w) {
  if (needspace) putc(' ', fout);
  needspace = 1;
  if (!w || !*w) {fprintf(fout,"{}"); return;}
  const char *p;
  // see if it is a single word:
  for (p = w; is_id(*p); p++) ;
  if (!*p) {fprintf(fout,"%s",w); return;}
  // see if there are matching braces:
  int n = 0;
  for (p = w; *p; p++) {
    if (*p == '{') n++;
    else if (*p == '}') {n--; if (n<0) break;}
  }
  int mismatched = (n != 0);
  // write out brace-quoted string:
  putc('{', fout);
  for (; *w; w++) {
    switch (*w) {
    case '{':
    case '}':
      if (!mismatched) break;
    case '\\':
    case '#':
      putc('\\',fout);
      break;
    }
    putc(*w,fout);
  }
  putc('}', fout);
}

// write an arbitrary formatted word, or a comment, etc.
// if needspace is set, then one space is written before the string
// unless the format starts with a newline character ('\n'):
void write_string(const char *format, ...) {
  va_list args;
  va_start(args, format);
  if (needspace && *format != '\n') fputc(' ',fout);
  vfprintf(fout, format, args);
  va_end(args);
  needspace = !isspace(format[strlen(format)-1] & 255);
}

// start a new line and indent it for a given nesting level:
void write_indent(int n) {
  fputc('\n',fout);
  while (n--) {fputc(' ',fout); fputc(' ',fout);}
  needspace = 0;
}

// write a '{' at the given indenting level:
void write_open(int) {
  if (needspace) fputc(' ',fout);
  fputc('{',fout);
  needspace = 0;
}

// write a '}' at the given indenting level:
void write_close(int n) {
  if (needspace) write_indent(n);
  fputc('}',fout);
  needspace = 1;
}

////////////////////////////////////////////////////////////////
// BASIC FILE READING:

static FILE *fin;
static int lineno;
static const char *fname;

int open_read(const char *s) {
  lineno = 1;
  if (!s) {fin = stdin; fname = "stdin"; return 1;}
  FILE *f = fl_fopen(s,"r");
  if (!f) return 0;
  fin = f;
  fname = s;
  return 1;
}

int close_read() {
  if (fin != stdin) {
    int x = fclose(fin);
    fin = 0;
    return x >= 0;
  }
  return 1;
}

#include <FL/fl_message.H>

void read_error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  if (!fin) {
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    fl_message("%s", buffer);
  } else {
    fprintf(stderr, "%s:%d: ", fname, lineno);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
  }
  va_end(args);
}

static int hexdigit(int x) {
  if (isdigit(x)) return x-'0';
  if (isupper(x)) return x-'A'+10;
  if (islower(x)) return x-'a'+10;
  return 20;
}


static int read_quoted() {	// read whatever character is after a \ .
  int c,d,x;
  switch(c = fgetc(fin)) {
  case '\n': lineno++; return -1;
  case 'a' : return('\a');
  case 'b' : return('\b');
  case 'f' : return('\f');
  case 'n' : return('\n');
  case 'r' : return('\r');
  case 't' : return('\t');
  case 'v' : return('\v');
  case 'x' :	/* read hex */
    for (c=x=0; x<3; x++) {
      int ch = fgetc(fin);
      d = hexdigit(ch);
      if (d > 15) {ungetc(ch,fin); break;}
      c = (c<<4)+d;
    }
    break;
  default:		/* read octal */
    if (c<'0' || c>'7') break;
    c -= '0';
    for (x=0; x<2; x++) {
      int ch = fgetc(fin);
      d = hexdigit(ch);
      if (d>7) {ungetc(ch,fin); break;}
      c = (c<<3)+d;
    }
    break;
  }
  return(c);
}

// return a word read from the file, or NULL at the EOF:
// This will skip all comments (# to end of line), and evaluate
// all \xxx sequences and use \ at the end of line to remove the newline.
// A word is any one of:
//	a continuous string of non-space chars except { and } and #
//	everything between matching {...} (unless wantbrace != 0)
//	the characters '{' and '}'

static char *buffer;
static int buflen;
static void expand_buffer(int length) {
  if (length >= buflen) {
    if (!buflen) {
      buflen = length+1;
      buffer = (char*)malloc(buflen);
    } else {
      buflen = 2*buflen;
      if (length >= buflen) buflen = length+1;
      buffer = (char *)realloc((void *)buffer,buflen);
    }
  }
}

const char *read_word(int wantbrace) {
  int x;

  // skip all the whitespace before it:
  for (;;) {
    x = getc(fin);
    if (x < 0 && feof(fin)) {	// eof
      return 0;
    } else if (x == '#') {	// comment
      do x = getc(fin); while (x >= 0 && x != '\n');
      lineno++;
      continue;
    } else if (x == '\n') {
      lineno++;
    } else if (!isspace(x & 255)) {
      break;
    }
  }

  expand_buffer(100);

  if (x == '{' && !wantbrace) {

    // read in whatever is between braces
    int length = 0;
    int nesting = 0;
    for (;;) {
      x = getc(fin);
      if (x<0) {read_error("Missing '}'"); break;}
      else if (x == '#') { // embedded comment
	do x = getc(fin); while (x >= 0 && x != '\n');
	lineno++;
	continue;
      } else if (x == '\n') lineno++;
      else if (x == '\\') {x = read_quoted(); if (x<0) continue;}
      else if (x == '{') nesting++;
      else if (x == '}') {if (!nesting--) break;}
      buffer[length++] = x;
      expand_buffer(length);
    }
    buffer[length] = 0;
    return buffer;

  } else if (x == '{' || x == '}') {
    // all the punctuation is a word:
    buffer[0] = x;
    buffer[1] = 0;
    return buffer;

  } else {

    // read in an unquoted word:
    int length = 0;
    for (;;) {
      if (x == '\\') {x = read_quoted(); if (x<0) continue;}
      else if (x<0 || isspace(x & 255) || x=='{' || x=='}' || x=='#') break;
      buffer[length++] = x;
      expand_buffer(length);
      x = getc(fin);
    }
    ungetc(x, fin);
    buffer[length] = 0;
    return buffer;

  }
}

////////////////////////////////////////////////////////////////

// global int variables:
extern int i18n_type;
extern const char* i18n_include;
extern const char* i18n_function;
extern const char* i18n_file;
extern const char* i18n_set;


extern int header_file_set;
extern int code_file_set;
extern const char* header_file_name;
extern const char* code_file_name;

int write_file(const char *filename, int selected_only) {
  if (!open_write(filename)) return 0;
  write_string("# data file for the Fltk User Interface Designer (fluid)\n"
	       "version %.4f",FL_VERSION);
  if(!include_H_from_C)
    write_string("\ndo_not_include_H_from_C");
  if(use_FL_COMMAND)
    write_string("\nuse_FL_COMMAND");
  if (i18n_type) {
    write_string("\ni18n_type %d", i18n_type);
    write_string("\ni18n_include %s", i18n_include);
    switch (i18n_type) {
    case 1 : /* GNU gettext */
	write_string("\ni18n_function %s", i18n_function);
        break;
    case 2 : /* POSIX catgets */
        if (i18n_file[0]) write_string("\ni18n_file %s", i18n_file);
	write_string("\ni18n_set %s", i18n_set);
        break;
    }
  }
  if (!selected_only) {
    write_string("\nheader_name"); write_word(header_file_name);
    write_string("\ncode_name"); write_word(code_file_name);
  }
  for (Fl_Type *p = Fl_Type::first; p;) {
    if (!selected_only || p->selected) {
      p->write();
      write_string("\n");
      int q = p->level;
      for (p = p->next; p && p->level > q; p = p->next) {/*empty*/}
    } else {
      p = p->next;
    }
  }
  return close_write();
}

////////////////////////////////////////////////////////////////
// read all the objects out of the input file:

void read_fdesign();

double read_version;

extern Fl_Type *Fl_Type_make(const char *tn);

static void read_children(Fl_Type *p, int paste) {
  Fl_Type::current = p;
  for (;;) {
    const char *c = read_word();
  REUSE_C:
    if (!c) {
      if (p && !paste) read_error("Missing '}'");
      break;
    }

    if (!strcmp(c,"}")) {
      if (!p) read_error("Unexpected '}'");
      break;
    }

    // this is the first word in a .fd file:
    if (!strcmp(c,"Magic:")) {
      read_fdesign();
      return;
    }

    if (!strcmp(c,"version")) {
      c = read_word();
      read_version = strtod(c,0);
      if (read_version<=0 || read_version>double(FL_VERSION+0.00001))
        read_error("unknown version '%s'",c);
      continue;
    }

    // back compatibility with Vincent Penne's original class code:
    if (!p && !strcmp(c,"define_in_struct")) {
      Fl_Type *t = Fl_Type_make("class");
      t->name(read_word());
      Fl_Type::current = p = t;
      paste = 1; // stops "missing }" error
      continue;
    }

    if (!strcmp(c,"do_not_include_H_from_C")) {
      include_H_from_C=0;
      goto CONTINUE;
    }
    if (!strcmp(c,"use_FL_COMMAND")) {
      use_FL_COMMAND=1;
      goto CONTINUE;
    }
    if (!strcmp(c,"i18n_type")) {
      i18n_type = atoi(read_word());
      goto CONTINUE;
    }
    if (!strcmp(c,"i18n_function")) {
      i18n_function = strdup(read_word());
      goto CONTINUE;
    }
    if (!strcmp(c,"i18n_file")) {
      i18n_file = strdup(read_word());
      goto CONTINUE;
    }
    if (!strcmp(c,"i18n_set")) {
      i18n_set = strdup(read_word());
      goto CONTINUE;
    }
    if (!strcmp(c,"i18n_include")) {
      i18n_include = strdup(read_word());
      goto CONTINUE;
    }
    if (!strcmp(c,"i18n_type"))
    {
      i18n_type = atoi(read_word());
      goto CONTINUE;
    }
    if (!strcmp(c,"i18n_type"))
    {
      i18n_type = atoi(read_word());
      goto CONTINUE;
    }
    if (!strcmp(c,"header_name")) {
      if (!header_file_set) header_file_name = strdup(read_word());
      else read_word();
      goto CONTINUE;
    }

    if (!strcmp(c,"code_name")) {
      if (!code_file_set) code_file_name = strdup(read_word());
      else read_word();
      goto CONTINUE;
    }

    if (!strcmp(c, "snap") || !strcmp(c, "gridx") || !strcmp(c, "gridy")) {
      // grid settings are now global
      read_word();
      goto CONTINUE;
    }

    {Fl_Type *t = Fl_Type_make(c);
    if (!t) {
      read_error("Unknown word \"%s\"", c);
      continue;
    }
    t->name(read_word());

    c = read_word(1);
    if (strcmp(c,"{") && t->is_class()) {   // <prefix> <name>
      ((Fl_Class_Type*)t)->prefix(t->name());
      t->name(c);
      c = read_word(1);
    }

    if (strcmp(c,"{")) {
      read_error("Missing property list for %s\n",t->title());
      goto REUSE_C;
    }

    t->open_ = 0;
    for (;;) {
      const char *cc = read_word();
      if (!cc || !strcmp(cc,"}")) break;
      t->read_property(cc);
    }

    if (!t->is_parent()) continue;
    c = read_word(1);
    if (strcmp(c,"{")) {
      read_error("Missing child list for %s\n",t->title());
      goto REUSE_C;
    }
    read_children(t, 0);}
    Fl_Type::current = p;
  CONTINUE:;
  }
}

extern void deselect();

int read_file(const char *filename, int merge) {
  Fl_Type *o;
  read_version = 0.0;
  if (!open_read(filename)) return 0;
  if (merge) deselect(); else    delete_all();
  read_children(Fl_Type::current, merge);
  Fl_Type::current = 0;
  // Force menu items to be rebuilt...
  for (o = Fl_Type::first; o; o = o->next)
    if (o->is_menu_button()) o->add_child(0,0);
  for (o = Fl_Type::first; o; o = o->next)
    if (o->selected) {Fl_Type::current = o; break;}
  selection_changed(Fl_Type::current);
  return close_read();
}

////////////////////////////////////////////////////////////////
// Read Forms and XForms fdesign files:

int read_fdesign_line(const char*& name, const char*& value) {

  int length = 0;
  int x;
  // find a colon:
  for (;;) {
    x = getc(fin);
    if (x < 0 && feof(fin)) return 0;
    if (x == '\n') {length = 0; continue;} // no colon this line...
    if (!isspace(x & 255)) {
      buffer[length++] = x;
      expand_buffer(length);
    }
    if (x == ':') break;
  }
  int valueoffset = length;
  buffer[length-1] = 0;

  // skip to start of value:
  for (;;) {
    x = getc(fin);
    if ((x < 0 && feof(fin)) || x == '\n' || !isspace(x & 255)) break;
  }

  // read the value:
  for (;;) {
    if (x == '\\') {x = read_quoted(); if (x<0) continue;}
    else if (x == '\n') break;
    buffer[length++] = x;
    expand_buffer(length);
    x = getc(fin);
  }
  buffer[length] = 0;
  name = buffer;
  value = buffer+valueoffset;
  return 1;
}

int fdesign_flip;
int fdesign_magic;
#include <FL/Fl_Group.H>

static const char *class_matcher[] = {
"FL_CHECKBUTTON", "Fl_Check_Button",
"FL_ROUNDBUTTON", "Fl_Round_Button",
"FL_ROUND3DBUTTON", "Fl_Round_Button",
"FL_LIGHTBUTTON", "Fl_Light_Button",
"FL_FRAME", "Fl_Box",
"FL_LABELFRAME", "Fl_Box",
"FL_TEXT", "Fl_Box",
"FL_VALSLIDER", "Fl_Value_Slider",
"FL_MENU", "Fl_Menu_Button",
"3", "FL_BITMAP",
"1", "FL_BOX",
"71","FL_BROWSER",
"11","FL_BUTTON",
"4", "FL_CHART",
"42","FL_CHOICE",
"61","FL_CLOCK",
"25","FL_COUNTER",
"22","FL_DIAL",
"101","FL_FREE",
"31","FL_INPUT",
"12","Fl_Light_Button",
"41","FL_MENU",
"23","FL_POSITIONER",
"13","Fl_Round_Button",
"21","FL_SLIDER",
"2", "FL_BOX", // was FL_TEXT
"62","FL_TIMER",
"24","Fl_Value_Slider",
0};

void read_fdesign() {
  fdesign_magic = atoi(read_word());
  fdesign_flip = (fdesign_magic < 13000);
  Fl_Widget_Type *window = 0;
  Fl_Widget_Type *group = 0;
  Fl_Widget_Type *widget = 0;
  if (!Fl_Type::current) {
    Fl_Type *t = Fl_Type_make("Function");
    t->name("create_the_forms()");
    Fl_Type::current = t;
  }
  for (;;) {
    const char *name;
    const char *value;
    if (!read_fdesign_line(name, value)) break;

    if (!strcmp(name,"Name")) {

      window = (Fl_Widget_Type*)Fl_Type_make("Fl_Window");
      window->name(value);
      window->label(value);
      Fl_Type::current = widget = window;

    } else if (!strcmp(name,"class")) {

      if (!strcmp(value,"FL_BEGIN_GROUP")) {
	group = widget = (Fl_Widget_Type*)Fl_Type_make("Fl_Group");
	Fl_Type::current = group;
      } else if (!strcmp(value,"FL_END_GROUP")) {
	if (group) {
	  Fl_Group* g = (Fl_Group*)(group->o);
	  g->begin();
	  g->forms_end();
	  Fl_Group::current(0);
	}
	group = widget = 0;
	Fl_Type::current = window;
      } else {
	for (int i = 0; class_matcher[i]; i += 2)
	  if (!strcmp(value,class_matcher[i])) {
	    value = class_matcher[i+1]; break;}
	widget = (Fl_Widget_Type*)Fl_Type_make(value);
	if (!widget) {
	  printf("class %s not found, using Fl_Button\n", value);
	  widget = (Fl_Widget_Type*)Fl_Type_make("Fl_Button");
	}
      }

    } else if (widget) {
      if (!widget->read_fdesign(name, value))
	printf("Ignoring \"%s: %s\"\n", name, value);
    }
  }
}

//
// End of "$Id$".
//
