//
// Fluid Project File Reader code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

// You may find the basic read_* and write_* routines to
// be useful for other programs.  I have used them many times.
// They are somewhat similar to tcl, using matching { and }
// to quote strings.

#include "io/Project_Reader.h"

#include "Fluid.h"
#include "Project.h"
#include "app/shell_command.h"
#include "proj/undo.h"
#include "app/Snap_Action.h"
#include "nodes/factory.h"
#include "nodes/Function_Node.h"
#include "nodes/Widget_Node.h"
#include "nodes/Grid_Node.h"
#include "nodes/Window_Node.h"
#include "widgets/Node_Browser.h"

#include <FL/Fl_Window.H>
#include <FL/fl_message.H>

/// \defgroup flfile .fl Project File Operations
/// \{

using namespace fld;
using namespace fld::io;

// This file contains code to read and write .fl files.

/// If set, we read an old fdesign file and widget y coordinates need to be flipped.
int fld::io::fdesign_flip = 0;

/** \brief Read a .fl project file.

 The .fl file format is documented in `fluid/README_fl.txt`.

 \param[in] filename read this file
 \param[in] merge if this is set, merge the file into an existing project
    at Fluid.proj.tree.current
 \param[in] strategy add new nodes after current or as last child
 \return 0 if the operation failed, 1 if it succeeded
 */
int fld::io::read_file(Project &proj, const char *filename, int merge, Strategy strategy) {
  Project_Reader f(proj);
  strategy.source(Strategy::FROM_FILE);
  return f.read_project(filename, merge, strategy);
}

/**
 Convert a single ASCII char, assumed to be a hex digit, into its decimal value.
 \param[in] x ASCII character
 \return decimal value or 20 if character is not a valid hex digit (0..9,a..f,A..F)
 */
static int hexdigit(int x) {
  if ((x < 0) || (x > 127)) return 20;
  if (isdigit(x)) return x-'0';
  if (isupper(x)) return x-'A'+10;
  if (islower(x)) return x-'a'+10;
  return 20;
}

// ---- Project_Reader ---------------------------------------------- MARK: -

/**
 A simple growing buffer.
 Oh how I wish sometimes we would upgrade to modern C++.
 \param[in] length minimum length in bytes
 */
void Project_Reader::expand_buffer(int length) {
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

/** \brief Construct local project reader. */
Project_Reader::Project_Reader(Project &proj)
: proj_(proj)
{
}

/** \brief Release project reader resources. */
Project_Reader::~Project_Reader()
{
  // fname is not copied, so do not free it
  if (buffer)
    ::free(buffer);
}

/**
 Open an .fl file for reading.
 \param[in] s filename, if nullptr, read from stdin instead
 \return 0 if the operation failed, 1 if it succeeded
 */
int Project_Reader::open_read(const char *s) {
  lineno = 1;
  if (!s) {
    fin = stdin;
    fname = "stdin";
  } else {
    FILE *f = fl_fopen(s, "rb");
    if (!f)
      return 0;
    fin = f;
    fname = s;
  }
  return 1;
}

/**
 Close the .fl file.
 \return 0 if the operation failed, 1 if it succeeded
 */
int Project_Reader::close_read() {
  if (fin != stdin) {
    int x = fclose(fin);
    fin = nullptr;
    return x >= 0;
  }
  return 1;
}

/**
 Return the name part of the current filename and path.
 \return a pointer into a string that is not owned by this class
 */
const char *Project_Reader::filename_name() {
  return fl_filename_name(fname);
}

/**
 Convert an ASCII sequence from the \.fl file following a previously read `\\` into a single character.
 Conversion includes the common C style \\ characters like \\n, \\x## hex
 values, and \\o### octal values.
 \return a character in the ASCII range
 */
int Project_Reader::read_quoted() {      // read whatever character is after a \ .
  int c,d,x;
  switch(c = nextchar()) {
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
        int ch = nextchar();
        d = hexdigit(ch);
        if (d > 15) {ungetc(ch,fin); break;}
        c = (c<<4)+d;
      }
      break;
    default:              /* read octal */
      if (c<'0' || c>'7') break;
      c -= '0';
      for (x=0; x<2; x++) {
        int ch = nextchar();
        d = hexdigit(ch);
        if (d>7) {ungetc(ch,fin); break;}
        c = (c<<3)+d;
      }
      break;
  }
  return(c);
}

/**
 Recursively read child nodes in the .fl design file.

 If this is the first call, also read the global settings for this design.

 \param[in] p parent node or nullptr
 \param[in] merge if set, merge into existing design, else replace design
 \param[in] strategy add nodes after current or as last child
 \param[in] skip_options this is set if the options were already found in
 a previous call, and there is no need to waste time searching for them.
 \return the last node that was created
 */
Node *Project_Reader::read_children(Node *p, int merge, Strategy strategy, char skip_options) {
  Fluid.proj.tree.current = p;
  Node *last_child_read = nullptr;
  Node *t = nullptr;
  for (;;) {
    const char *c = read_word();
  REUSE_C:
    if (!c) {
      if (p && !merge)
        read_error("Missing '}'");
      break;
    }

    if (!strcmp(c,"}")) {
      if (!p) read_error("Unexpected '}'");
      break;
    }

    // Make sure that we don't go through the list of options for child nodes
    if (!skip_options) {
      // this is the first word in a .fd file:
      if (!strcmp(c,"Magic:")) {
        read_fdesign();
        return nullptr;
      }

      if (!strcmp(c,"version")) {
        c = read_word();
        read_version = strtod(c,nullptr);
        if (read_version<=0 || read_version>double(FL_VERSION+0.00001))
          read_error("unknown version '%s'",c);
        continue;
      }

      // back compatibility with Vincent Penne's original class code:
      if (!p && !strcmp(c,"define_in_struct")) {
        Node *t = add_new_widget_from_file("class", Strategy::FROM_FILE_AS_LAST_CHILD);
        t->name(read_word());
        Fluid.proj.tree.current = p = t;
        merge = 1; // stops "missing }" error
        continue;
      }

      if (!strcmp(c,"do_not_include_H_from_C")) {
        proj_.include_H_from_C=0;
        goto CONTINUE;
      }
      if (!strcmp(c,"use_FL_COMMAND")) {
        proj_.use_FL_COMMAND=1;
        goto CONTINUE;
      }
      if (!strcmp(c,"utf8_in_src")) {
        proj_.utf8_in_src=1;
        goto CONTINUE;
      }
      if (!strcmp(c,"avoid_early_includes")) {
        proj_.avoid_early_includes=1;
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_type")) {
        proj_.i18n_type = static_cast<fld::I18n_Type>(atoi(read_word()));
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_gnu_function")) {
        proj_.i18n_gnu_function = read_word();
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_gnu_static_function")) {
        proj_.i18n_gnu_static_function = read_word();
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_pos_file")) {
        proj_.i18n_pos_file = read_word();
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_pos_set")) {
        proj_.i18n_pos_set = read_word();
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_include")) {
        if (proj_.i18n_type == fld::I18n_Type::GNU)
          proj_.i18n_gnu_include = read_word();
        else if (proj_.i18n_type == fld::I18n_Type::POSIX)
          proj_.i18n_pos_include = read_word();
        goto CONTINUE;
      }
      if (!strcmp(c,"i18n_conditional")) {
        if (proj_.i18n_type == fld::I18n_Type::GNU)
          proj_.i18n_gnu_conditional = read_word();
        else if (proj_.i18n_type == fld::I18n_Type::POSIX)
          proj_.i18n_pos_conditional = read_word();
        goto CONTINUE;
      }
      if (!strcmp(c,"header_name")) {
        if (!proj_.header_file_set) proj_.header_file_name = read_word();
        else read_word();
        goto CONTINUE;
      }

      if (!strcmp(c,"code_name")) {
        if (!proj_.code_file_set) proj_.code_file_name = read_word();
        else read_word();
        goto CONTINUE;
      }

      if (!strcmp(c, "snap")) {
        Fluid.layout_list.read(this);
        goto CONTINUE;
      }

      if (!strcmp(c, "gridx") || !strcmp(c, "gridy")) {
        // grid settings are now global
        read_word();
        goto CONTINUE;
      }

      if (strcmp(c, "shell_commands")==0) {
        if (g_shell_config) {
          g_shell_config->read(this);
        } else {
          read_word();
        }
        goto CONTINUE;
      }

      if (!strcmp(c, "mergeback")) {
        proj_.write_mergeback_data = read_int();
        goto CONTINUE;
      }
    }
    t = add_new_widget_from_file(c, strategy);
    if (!t) {
      read_error("Unknown word \"%s\"", c);
      continue;
    }
    last_child_read = t;
    // After reading the first widget, we no longer need to look for options
    skip_options = 1;

    t->name(read_word());

    c = read_word(1);
    if (strcmp(c,"{") && t->is_class()) {   // <prefix> <name>
      ((Class_Node*)t)->prefix(t->name());
      t->name(c);
      c = read_word(1);
    }

    if (strcmp(c,"{")) {
      read_error("Missing property list for %s\n",t->title());
      goto REUSE_C;
    }

    t->folded_ = 1;
    for (;;) {
      const char *cc = read_word();
      if (!cc || !strcmp(cc,"}")) break;
      t->read_property(*this, cc);
    }

    if (t->can_have_children()) {
      c = read_word(1);
      if (strcmp(c,"{")) {
        read_error("Missing child list for %s\n",t->title());
        goto REUSE_C;
      }
      read_children(t, 0, Strategy::FROM_FILE_AS_LAST_CHILD, skip_options);
      t->postprocess_read();
      // FIXME: this has no business in the file reader!
      // TODO: this is called whenever something is pasted from the top level into a grid
      //    It makes sense to make this more universal for other widget types too.
      if (merge && t && t->parent && t->parent->is_a(Type::Grid)) {
        if (Window_Node::popupx != 0x7FFFFFFF) {
          ((Grid_Node*)t->parent)->insert_child_at(((Widget_Node*)t)->o, Window_Node::popupx, Window_Node::popupy);
        } else {
          ((Grid_Node*)t->parent)->insert_child_at_next_free_cell(((Widget_Node*)t)->o);
        }
      }

      t->layout_widget();
    }

    if (strategy.placement() == Strategy::AS_FIRST_CHILD) {
      strategy.placement(Strategy::AFTER_CURRENT);
    }
    if (strategy.placement() == Strategy::AFTER_CURRENT) {
      Fluid.proj.tree.current = t;
    } else {
      Fluid.proj.tree.current = p;
    }

  CONTINUE:;
  }
  if (merge && last_child_read && last_child_read->parent) {
    last_child_read->parent->postprocess_read();
    last_child_read->parent->layout_widget();
  }
  return last_child_read;
}

/** \brief Read a .fl project file.
 \param[in] filename read this file
 \param[in] merge if this is set, merge the file into an existing project
 at Fluid.proj.tree.current
 \param[in] strategy add new nodes after current or as last child
 \return 0 if the operation failed, 1 if it succeeded
 */
int Project_Reader::read_project(const char *filename, int merge, Strategy strategy) {
  Node *o;
  proj_.undo.suspend();
  read_version = 0.0;
  if (!open_read(filename)) {
    proj_.undo.resume();
    return 0;
  }
  if (merge)
    deselect();
  else
    proj_.reset();
  read_children(Fluid.proj.tree.current, merge, strategy);
  // clear this
  Fluid.proj.tree.current = nullptr;
  // Force menu items to be rebuilt...
  for (o = Fluid.proj.tree.first; o; o = o->next) {
    if (o->is_a(Type::Menu_Manager_)) {
      o->add_child(nullptr,nullptr);
    }
  }
  for (o = Fluid.proj.tree.first; o; o = o->next) {
    if (o->selected) {
      Fluid.proj.tree.current = o;
      break;
    }
  }
  selection_changed(Fluid.proj.tree.current);
  if (g_shell_config) {
    g_shell_config->rebuild_shell_menu();
    g_shell_config->update_settings_dialog();
  }
  Fluid.layout_list.update_dialogs();
  proj_.update_settings_dialog();
  int ret = close_read();
  proj_.undo.resume();
  return ret;
}

/**
 Display an error while reading the file.
 If the .fl file isn't opened for reading, pop up an FLTK dialog, otherwise
 print to stdout.
 \note Matt: I am not sure why it is done this way. Shouldn't this depend on \c Fluid.batch_mode?
 \todo Not happy about this function. Output channel should depend on `Fluid.batch_mode`
       as the note above already states. I want to make all file readers and writers
       depend on an error handling base class that outputs a useful analysis of file
       operations.
 \param[in] format printf style format string, followed by an argument list
 */
void Project_Reader::read_error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  if (!fin) { // FIXME: this line suppresses any error messages in interactive mode
    char buffer[1024]; // TODO: hides class member "buffer"
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
 Return a word read from the .fl file, or nullptr at the EOF.

 This will skip all comments (# to end of line), and evaluate
 all \\xxx sequences and use \\ at the end of line to remove the newline.

 A word is any one of:
 - a continuous string of non-space chars except { and } and #
 - everything between matching {...} (unless wantbrace != 0)
 - the characters '{' and '}'

 \param[in] wantbrace if set, reading a `{` as the first non-space character
    will return the string `"{"`, if clear, a `{` is seen as the start of a word
 \return a pointer to the internal buffer, containing a copy of the word.
    Don't free the buffer! Note that most (all?) other file operations will
    overwrite this buffer. If wantbrace is not set, but we read a leading '{',
    the returned string will be stripped of its leading and trailing braces.
 */
const char *Project_Reader::read_word(int wantbrace) {
  int x;

  // skip all the whitespace before it:
  for (;;) {
    x = nextchar();
    if (x < 0 && feof(fin)) {   // eof
      return nullptr;
    } else if (x == '#') {      // comment
      do x = nextchar(); while (x >= 0 && x != '\n');
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
      x = nextchar();
      if (x<0) {read_error("Missing '}'"); break;}
      else if (x == '#') { // embedded comment
        do x = nextchar(); while (x >= 0 && x != '\n');
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
      x = nextchar();
    }
    ungetc(x, fin);
    buffer[length] = 0;
    return buffer;

  }
}

/** Read a word and interpret it as an integer value.
 \return integer value, or 0 if the word is not an integer
 */
int Project_Reader::read_int() {
  const char *word = read_word();
  if (word) {
    return atoi(word);
  } else {
    return 0;
  }
}

/** Read fdesign name/value pairs.
 Fdesign is the file format of the XForms UI designer. It stores lists of name
 and value pairs separated by a colon: `class: FL_LABELFRAME`.
 \param[out] name string
 \param[out] value string
 \return 0 if end of file, else 1
 */
int Project_Reader::read_fdesign_line(const char*& name, const char*& value) {
  int length = 0;
  int x;
  // find a colon:
  for (;;) {
    x = nextchar();
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
    x = nextchar();
    if ((x < 0 && feof(fin)) || x == '\n' || !isspace(x & 255)) break;
  }

  // read the value:
  for (;;) {
    if (x == '\\') {x = read_quoted(); if (x<0) continue;}
    else if (x == '\n') break;
    buffer[length++] = x;
    expand_buffer(length);
    x = nextchar();
  }
  buffer[length] = 0;
  name = buffer;
  value = buffer+valueoffset;
  return 1;
}

/// Lookup table from fdesign .fd files to .fl files
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
  nullptr};


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
void Project_Reader::read_fdesign() {
  int fdesign_magic = atoi(read_word());
  fdesign_flip = (fdesign_magic < 13000);
  Widget_Node *window = nullptr;
  Widget_Node *group = nullptr;
  Widget_Node *widget = nullptr;
  if (!Fluid.proj.tree.current) {
    Node *t = add_new_widget_from_file("Function", Strategy::FROM_FILE_AS_LAST_CHILD);
    t->name("create_the_forms()");
    Fluid.proj.tree.current = t;
  }
  for (;;) {
    const char *name;
    const char *value;
    if (!read_fdesign_line(name, value)) break;

    if (!strcmp(name,"Name")) {

      window = (Widget_Node*)add_new_widget_from_file("Fl_Window", Strategy::FROM_FILE_AS_LAST_CHILD);
      window->name(value);
      window->label(value);
      Fluid.proj.tree.current = widget = window;

    } else if (!strcmp(name,"class")) {

      if (!strcmp(value,"FL_BEGIN_GROUP")) {
        group = widget = (Widget_Node*)add_new_widget_from_file("Fl_Group", Strategy::FROM_FILE_AS_LAST_CHILD);
        Fluid.proj.tree.current = group;
      } else if (!strcmp(value,"FL_END_GROUP")) {
        if (group) {
          Fl_Group* g = (Fl_Group*)(group->o);
          g->begin();
          forms_end(g, fdesign_flip);
          Fl_Group::current(nullptr);
        }
        group = widget = nullptr;
        Fluid.proj.tree.current = window;
      } else {
        for (int i = 0; class_matcher[i]; i += 2)
          if (!strcmp(value,class_matcher[i])) {
            value = class_matcher[i+1]; break;}
        widget = (Widget_Node*)add_new_widget_from_file(value, Strategy::FROM_FILE_AS_LAST_CHILD);
        if (!widget) {
          printf("class %s not found, using Fl_Button\n", value);
          widget = (Widget_Node*)add_new_widget_from_file("Fl_Button", Strategy::FROM_FILE_AS_LAST_CHILD);
        }
      }

    } else if (widget) {
      if (!widget->read_fdesign(name, value))
        printf("Ignoring \"%s: %s\"\n", name, value);
    }
  }
}

/// \}
