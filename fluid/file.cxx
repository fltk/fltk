//
// Fluid file routines for the Fast Light Tool Kit (FLTK).
//
// You may find the basic read_* and write_* routines to
// be useful for other programs.  I have used them many times.
// They are somewhat similar to tcl, using matching { and }
// to quote strings.
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

#include "file.h"

#include "fluid.h"
#include "factory.h"
#include "Fl_Function_Type.h"
#include "Fl_Widget_Type.h"
#include "Fl_Window_Type.h"
#include "alignment_panel.h"
#include "widget_browser.h"
#include "shell_command.h"
#include "code.h"

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/fl_string_functions.h>
#include <FL/fl_message.H>
#include "../src/flstring.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/// \defgroup flfile .fl Design File Operations
/// \{

// This file contains code to read and write .fl files.
// TODO: there is a name confusion with routines that write to the C and Header
// TODO: files vs. those that write to the .fl file which should be fixed.

static FILE *fout;
static FILE *fin;

static int needspace;
static int lineno;
static const char *fname;

int fdesign_flip;
int fdesign_magic;

double read_version;

////////////////////////////////////////////////////////////////
// BASIC FILE WRITING:

/**
 Open teh .fl design file for writing.
 If the filename is NULL, associate stdout instead.
 \param[in] s the filename or NULL for stdout
 \return 1 if successful. 0 if the operation failed
 */
static int open_write(const char *s) {
  if (!s) {fout = stdout; return 1;}
  FILE *f = fl_fopen(s,"w");
  if (!f) return 0;
  fout = f;
  return 1;
}

/**
 Close the .fl design file.
 Don't close, if data was sent to stdout.
 */
static int close_write() {
  if (fout != stdout) {
    int x = fclose(fout);
    fout = stdout;
    return x >= 0;
  }
  return 1;
}

/**
 Write a string to the .fl file, quoting characters if necessary.
 */
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

/**
 Write an arbitrary formatted word to the .fl file, or a comment, etc .
 If needspace is set, then one space is written before the string
 unless the format starts with a newline character \\n.
 */
void write_string(const char *format, ...) {
  va_list args;
  va_start(args, format);
  if (needspace && *format != '\n') fputc(' ',fout);
  vfprintf(fout, format, args);
  va_end(args);
  needspace = !isspace(format[strlen(format)-1] & 255);
}

/**
 Start a new line in the .fl file and indent it for a given nesting level.
 */
void write_indent(int n) {
  fputc('\n',fout);
  while (n--) {fputc(' ',fout); fputc(' ',fout);}
  needspace = 0;
}

/**
 Write a '{' to the .fl file at the given indenting level.
 */
void write_open(int) {
  if (needspace) fputc(' ',fout);
  fputc('{',fout);
  needspace = 0;
}

/**
 Write a '}' to the .fl file at the given indenting level.
 */
void write_close(int n) {
  if (needspace) write_indent(n);
  fputc('}',fout);
  needspace = 1;
}


////////////////////////////////////////////////////////////////
// BASIC FILE READING:

/**
 Open an .fl file for reading.
 \param[in] s filename, if NULL, read from stdin instead
 \return 0 if the operation failed, 1 if it succeeded
 */
static int open_read(const char *s) {
  lineno = 1;
  if (!s) {fin = stdin; fname = "stdin"; return 1;}
  FILE *f = fl_fopen(s,"r");
  if (!f) return 0;
  fin = f;
  fname = s;
  return 1;
}

/**
 Close the .fl file.
 \return 0 if the operation failed, 1 if it succeeded
 */
static int close_read() {
  if (fin != stdin) {
    int x = fclose(fin);
    fin = 0;
    return x >= 0;
  }
  return 1;
}

/**
 Display an error while reading the file.
 If the .fl file isn't opened for reading, pop up an FLTK dialog, otherwise
 print to stdout.
 \note Matt: I am not sure why it is done this way. Shouldn;t this depend on \c batch_mode?
 */
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

/**
 Convert a single ASCII char, assumed to be a hex digit, into its decimal value.
 */
static int hexdigit(int x) {
  if (isdigit(x)) return x-'0';
  if (isupper(x)) return x-'A'+10;
  if (islower(x)) return x-'a'+10;
  return 20;
}

/**
 Convert an ASCII sequence form the \.fl file that starts with a \\ into a single character.
 Conversion includes the common C style \\ characters like \\n, \\x## hex
 values, and \\o### octal values.
 */
static int read_quoted() {      // read whatever character is after a \ .
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
  case 'x' :    /* read hex */
    for (c=x=0; x<3; x++) {
      int ch = fgetc(fin);
      d = hexdigit(ch);
      if (d > 15) {ungetc(ch,fin); break;}
      c = (c<<4)+d;
    }
    break;
  default:              /* read octal */
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

static char *buffer;
static int buflen;

/**
 A simple growing buffer.
 Oh how I wish sometimes we would upgrade to modern C++.
 */
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

/**
 Return a word read from the .fl file, or NULL at the EOF.

 This will skip all comments (# to end of line), and evaluate
 all \\xxx sequences and use \\ at the end of line to remove the newline.

 A word is any one of:
  - a continuous string of non-space chars except { and } and #
  - everything between matching {...} (unless wantbrace != 0)
  - the characters '{' and '}'
 */
const char *read_word(int wantbrace) {
  int x;

  // skip all the whitespace before it:
  for (;;) {
    x = getc(fin);
    if (x < 0 && feof(fin)) {   // eof
      return 0;
    } else if (x == '#') {      // comment
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

/**
 Write an .fl design description file.
 \param[in] filename create this file, and if it exists, overwrite it
 \param[in] selected_only write only the selected nodes in the widget_tree. This
    is used to implement copy and paste.
 */
int write_file(const char *filename, int selected_only) {
  if (!open_write(filename)) return 0;
  write_string("# data file for the Fltk User Interface Designer (fluid)\n"
               "version %.4f",FL_VERSION);
  if(!include_H_from_C)
    write_string("\ndo_not_include_H_from_C");
  if(use_FL_COMMAND)
    write_string("\nuse_FL_COMMAND");
  if (utf8_in_src)
    write_string("\nutf8_in_src");
  if (avoid_early_includes)
    write_string("\navoid_early_includes");
  if (i18n_type) {
    write_string("\ni18n_type %d", i18n_type);
    write_string("\ni18n_include"); write_word(i18n_include);
    write_string("\ni18n_conditional"); write_word(i18n_conditional);
    switch (i18n_type) {
    case 1 : /* GNU gettext */
        write_string("\ni18n_function"); write_word(i18n_function);
        write_string("\ni18n_static_function"); write_word(i18n_static_function);
        break;
    case 2 : /* POSIX catgets */
        if (i18n_file[0]) {
          write_string("\ni18n_file");
          write_word(i18n_file);
        }
        write_string("\ni18n_set"); write_word(i18n_set);
        break;
    }
  }

  if (!selected_only) {
    write_string("\nheader_name"); write_word(header_file_name);
    write_string("\ncode_name"); write_word(code_file_name);

#if 0
    // https://github.com/fltk/fltk/issues/328
    // Project wide settings require a redesign.
    shell_settings_write();
    if (shell_settings_windows.command) {
      write_string("\nwin_shell_cmd"); write_word(shell_settings_windows.command);
      write_string("\nwin_shell_flags"); write_string("%d", shell_settings_windows.flags);
    }
    if (shell_settings_linux.command) {
      write_string("\nlinux_shell_cmd"); write_word(shell_settings_linux.command);
      write_string("\nlinux_shell_flags"); write_string("%d", shell_settings_linux.flags);
    }
    if (shell_settings_macos.command) {
      write_string("\nmac_shell_cmd"); write_word(shell_settings_macos.command);
      write_string("\nmac_shell_flags"); write_string("%d", shell_settings_macos.flags);
    }
#endif
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

/**
 Recursively read child nodes in the .fl design file.

 If this is the first call, also read the global settings for this design.

 \param[in] p parent node or NULL
 \param[in] paste if set, merge into existing design, else replace design
 \param[in] strategy add nodes after current or as last child
 \param[in] skip_options this is set if the options were already found in
    a previous call, and there is no need to waste time searchingg for them.
 */
static void read_children(Fl_Type *p, int paste, Strategy strategy, char skip_options=0) {
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

    // Make sure that we don;t go through the list of options for child nodes
    if (!skip_options) {
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
        Fl_Type *t = add_new_widget_from_file("class", kAddAsLastChild);
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
      if (!strcmp(c,"utf8_in_src")) {
        utf8_in_src=1;
        goto CONTINUE;
      }
      if (!strcmp(c,"avoid_early_includes")) {
        avoid_early_includes=1;
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_type")) {
        i18n_type = atoi(read_word());
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_function")) {
        i18n_function = fl_strdup(read_word());
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_static_function")) {
        i18n_static_function = fl_strdup(read_word());
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_file")) {
        i18n_file = fl_strdup(read_word());
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_set")) {
        i18n_set = fl_strdup(read_word());
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_include")) {
        i18n_include = fl_strdup(read_word());
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_conditional")) {
        i18n_conditional = fl_strdup(read_word());
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_type"))
      {
        i18n_type = atoi(read_word());
        goto CONTINUE;
      }
      if (!strcmp(c,"header_name")) {
        if (!header_file_set) header_file_name = fl_strdup(read_word());
        else read_word();
        goto CONTINUE;
      }

      if (!strcmp(c,"code_name")) {
        if (!code_file_set) code_file_name = fl_strdup(read_word());
        else read_word();
        goto CONTINUE;
      }

      if (!strcmp(c, "snap") || !strcmp(c, "gridx") || !strcmp(c, "gridy")) {
        // grid settings are now global
        read_word();
        goto CONTINUE;
      }

      if (strcmp(c, "win_shell_cmd")==0) {
        if (shell_settings_windows.command)
          free((void*)shell_settings_windows.command);
        shell_settings_windows.command = fl_strdup(read_word());
        goto CONTINUE;
      } else if (strcmp(c, "win_shell_flags")==0) {
        shell_settings_windows.flags = atoi(read_word());
        goto CONTINUE;
      } else if (strcmp(c, "linux_shell_cmd")==0) {
        if (shell_settings_linux.command)
          free((void*)shell_settings_linux.command);
        shell_settings_linux.command = fl_strdup(read_word());
        goto CONTINUE;
      } else if (strcmp(c, "linux_shell_flags")==0) {
        shell_settings_linux.flags = atoi(read_word());
        goto CONTINUE;
      } else if (strcmp(c, "mac_shell_cmd")==0) {
        if (shell_settings_macos.command)
          free((void*)shell_settings_macos.command);
        shell_settings_macos.command = fl_strdup(read_word());
        goto CONTINUE;
      } else if (strcmp(c, "mac_shell_flags")==0) {
        shell_settings_macos.flags = atoi(read_word());
        goto CONTINUE;
      }
    }
    {
      Fl_Type *t = add_new_widget_from_file(c, strategy);
      if (!t) {
        read_error("Unknown word \"%s\"", c);
        continue;
      }
      // After reading the first widget, we no longer need to look for options
      skip_options = 1;

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
      read_children(t, 0, strategy, skip_options);
    }

    Fl_Type::current = p;

  CONTINUE:;
  }
}

/**
 Read a .fl design file.
 \param[in] filename read this file
 \param[in] merge if this is set, merge the file into an existing design
    at Fl_Type::current
 \param[in] strategy add new nodes after current or as last child
 \return 0 if the operation failed, 1 if it succeeded
 */
int read_file(const char *filename, int merge, Strategy strategy) {
  Fl_Type *o;
  read_version = 0.0;
  if (!open_read(filename))
    return 0;
  if (merge)
    deselect();
  else
    delete_all();
  read_children(Fl_Type::current, merge, strategy);
  Fl_Type::current = 0;
  // Force menu items to be rebuilt...
  for (o = Fl_Type::first; o; o = o->next)
    if (o->is_menu_button())
      o->add_child(0,0);
  for (o = Fl_Type::first; o; o = o->next)
    if (o->selected) {
      Fl_Type::current = o;
      break;
    }
  selection_changed(Fl_Type::current);
  shell_settings_read();
  return close_read();
}

////////////////////////////////////////////////////////////////
// Read Forms and XForms fdesign files:

static int read_fdesign_line(const char*& name, const char*& value) {
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


/**
  Finish a group of widgets and optionally transform its children's coordinates.

  Implements the same functionality as Fl_Group::forms_end() from the forms
  compatibility library would have done:

  - resize the group to surround its children if the group's w() == 0
  - optionally flip the \p y coordinates of all children relative to the group's window
  - Fl_Group::end() the group

  \note Copied from forms_compatibility.cxx and modified as a static fluid
    function so we don't have to link to fltk_forms.

  \param[in]  g     the Fl_Group widget
  \param[in]  flip  flip children's \p y coordinates if true (non-zero)
*/
static void forms_end(Fl_Group *g, int flip) {
  // set the dimensions of a group to surround its contents
  const int nc = g->children();
  if (nc && !g->w()) {
    Fl_Widget*const* a = g->array();
    Fl_Widget* o = *a++;
    int rx = o->x();
    int ry = o->y();
    int rw = rx+o->w();
    int rh = ry+o->h();
    for (int i = nc - 1; i--;) {
      o = *a++;
      if (o->x() < rx) rx = o->x();
      if (o->y() < ry) ry = o->y();
      if (o->x() + o->w() > rw) rw = o->x() + o->w();
      if (o->y() + o->h() > rh) rh = o->y() + o->h();
    }
    g->Fl_Widget::resize(rx, ry, rw-rx, rh-ry);
  }
  // flip all the children's coordinate systems:
  if (nc && flip) {
    Fl_Widget* o = (g->as_window()) ? g : g->window();
    int Y = o->h();
    Fl_Widget*const* a = g->array();
    for (int i = nc; i--;) {
      Fl_Widget* ow = *a++;
      int newy = Y - ow->y() - ow->h();
      ow->Fl_Widget::resize(ow->x(), newy, ow->w(), ow->h());
    }
  }
  g->end();
}

/**
 Read a XForms design file.
 .fl and .fd file start with the same header. Fluid can recognize .fd XForms
 Design files by a magic number. It will read them and map XForms widgets onto
 FLTK widgets.
 \see http://xforms-toolkit.org
 */
void read_fdesign() {
  fdesign_magic = atoi(read_word());
  fdesign_flip = (fdesign_magic < 13000);
  Fl_Widget_Type *window = 0;
  Fl_Widget_Type *group = 0;
  Fl_Widget_Type *widget = 0;
  if (!Fl_Type::current) {
    Fl_Type *t = add_new_widget_from_file("Function", kAddAsLastChild);
    t->name("create_the_forms()");
    Fl_Type::current = t;
  }
  for (;;) {
    const char *name;
    const char *value;
    if (!read_fdesign_line(name, value)) break;

    if (!strcmp(name,"Name")) {

      window = (Fl_Widget_Type*)add_new_widget_from_file("Fl_Window", kAddAsLastChild);
      window->name(value);
      window->label(value);
      Fl_Type::current = widget = window;

    } else if (!strcmp(name,"class")) {

      if (!strcmp(value,"FL_BEGIN_GROUP")) {
        group = widget = (Fl_Widget_Type*)add_new_widget_from_file("Fl_Group", kAddAsLastChild);
        Fl_Type::current = group;
      } else if (!strcmp(value,"FL_END_GROUP")) {
        if (group) {
          Fl_Group* g = (Fl_Group*)(group->o);
          g->begin();
          forms_end(g, fdesign_flip);
          Fl_Group::current(0);
        }
        group = widget = 0;
        Fl_Type::current = window;
      } else {
        for (int i = 0; class_matcher[i]; i += 2)
          if (!strcmp(value,class_matcher[i])) {
            value = class_matcher[i+1]; break;}
        widget = (Fl_Widget_Type*)add_new_widget_from_file(value, kAddAsLastChild);
        if (!widget) {
          printf("class %s not found, using Fl_Button\n", value);
          widget = (Fl_Widget_Type*)add_new_widget_from_file("Fl_Button", kAddAsLastChild);
        }
      }

    } else if (widget) {
      if (!widget->read_fdesign(name, value))
        printf("Ignoring \"%s: %s\"\n", name, value);
    }
  }
}

/// \}
