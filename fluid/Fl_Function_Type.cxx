//
// C function type code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

#include "Fl_Function_Type.h"

#include "fluid.h"
#include "Fl_Window_Type.h"
#include "Fl_Group_Type.h"
#include "widget_browser.h"
#include "file.h"
#include "code.h"
#include "function_panel.h"
#include "comments.h"
#include "mergeback.h"
#include "undo.h"

#include <FL/fl_string_functions.h>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_ask.H>
#include "../src/flstring.h"

#include <zlib.h>


/// Set a current class, so that the code of the children is generated correctly.
Fl_Class_Type *current_class = NULL;

/**
 \brief Return 1 if the list contains a function with the given signature at the top level.
 Fl_Widget_Type uses this to check if a callback by a certain signature is
 already defined by the user within this file. If not, Fl_Widget_Type will
 generate an `extern $sig$;` statement.
 \param[in] rtype return type, can be NULL to avoid checking (not used by Fl_Widget_Type)
 \param[in] sig function signature
 \return 1 if found.
 */
int has_toplevel_function(const char *rtype, const char *sig) {
  Fl_Type *child;
  for (child = Fl_Type::first; child; child = child->next) {
    if (!child->is_in_class() && child->is_a(ID_Function)) {
      const Fl_Function_Type *fn = (const Fl_Function_Type*)child;
      if (fn->has_signature(rtype, sig))
        return 1;
    }
  }
  return 0;
}


////////////////////////////////////////////////////////////////
// quick check of any C code for legality, returns an error message

static char buffer[128]; // for error messages

/**
 Check a quoted string contains a character.
 This is used to find a matching " or ' in a string.
 \param[inout] c start searching here, return where we found \c type
 \param[in] type find this character
 \return NULL if the character was found, else a pointer to a static string
    with an error message
 */
const char *_q_check(const char * & c, int type) {
  for (;;) switch (*c++) {
    case '\0':
      sprintf(buffer,"missing %c",type);
      return buffer;
    case '\\':
      if (*c) c++;
      break;
    default:
      if (*(c-1) == type) return 0;
  }
}

/**
 Check normal code, match brackets and parenthesis.
 Recursively run a line of code and make sure that
 {, [, ", ', and ( are matched.
 \param[inout] c start searching here, return the end of the search
 \param[in] type find this character match
 \return NULL if the character was found, else a pointer to a static string
    with an error message
 */
const char *_c_check(const char * & c, int type) {
  const char *d;
  for (;;) switch (*c++) {
    case 0:
      if (!type) return 0;
      sprintf(buffer, "missing '%c'", type);
      return buffer;
    case '/':
      // Skip comments as needed...
      if (*c == '/') {
        while (*c != '\n' && *c) c++;
      } else if (*c == '*') {
        c++;
        while ((*c != '*' || c[1] != '/') && *c) c++;
        if (*c == '*') c+=2;
        else {
          return "missing '*/'";
        }
      }
      break;
//    case '#':
//      // treat cpp directives as a comment:
//      // Matt: a '#' character can appear as a concatenation when defining macros
//      // Matt: so instead we just silently ignore the '#'
//      while (*c != '\n' && *c) c++;
//      break;
    case '{':
      if (type==')') goto UNEXPECTED;
      d = _c_check(c,'}');
      if (d) return d;
      break;
    case '(':
      d = _c_check(c,')');
      if (d) return d;
      break;
    case '[':
      d = _c_check(c,']');
      if (d) return d;
      break;
    case '\"':
      d = _q_check(c,'\"');
      if (d) return d;
      break;
    case '\'':
      d = _q_check(c,'\'');
      if (d) return d;
      break;
    case '}':
    case ')':
    case ']':
    UNEXPECTED:
      if (type == *(c-1)) return 0;
      sprintf(buffer, "unexpected '%c'", *(c-1));
      return buffer;
  }
}

/**
 Check legality of c code (sort of) and return error:
 Make sure that {, ", ', and ( are matched.
 \param[in] c start searching here
 \param[in] type find this character match (default is 0)
 \return NULL if the character was found, else a pointer to a static string
    with an error message
 \note This function checks every conceivable line of code, which is not
    always wanted. It can't differentiate characters in comments, and the
    user may well intend to leave a curly bracket open
    (i.e. namespace { ... } ). We should make this option user selectable.
 */
const char *c_check(const char *c, int type) {
  return _c_check(c,type);
}

// ---- Fl_Function_Type implementation

/** \class Fl_Function_Type
 Manage a C++ function node in the Fluid design.

 A function can have a signature (name followed by arguments), a return type
 and a comment section. If can be local or global, and it can be declared a C
 or C++ function.
 */

/// Prototype for a function to be used by the factory.
Fl_Function_Type Fl_Function_type;

/**
 Create a new function.
 */
Fl_Function_Type::Fl_Function_Type() :
  Fl_Type(),
  return_type(0L),
  public_(0),
  cdecl_(0),
  constructor(0),
  havewidgets(0)
{ }

/**
 Destructor.
 */
Fl_Function_Type::~Fl_Function_Type() {
  if (return_type) free((void*)return_type);
}

/**
 Create a new function for the widget tree.
 \param[in] strategy new function add after current or as last child
 \return the new node
 */
Fl_Type *Fl_Function_Type::make(Strategy strategy) {
  Fl_Type *anchor = Fl_Type::current, *p = anchor;
  if (p && (strategy == kAddAfterCurrent)) p = p->parent;
  while (p && !p->is_decl_block()) {
    anchor = p;
    strategy = kAddAfterCurrent;
    p = p->parent;
  }
  Fl_Function_Type *o = new Fl_Function_Type();
  o->name("make_window()");
  o->return_type = 0;
  o->add(anchor, strategy);
  o->factory = this;
  o->public_ = 1;
  o->cdecl_ = 0;
  return o;
}

/**
 Write function specific properties to an .fl file.
  - "private"/"public" indicates the state of the function
  - "C" is written if we want a C signature instead of C++
  - "return_type" is followed by the return type of the function
 */
void Fl_Function_Type::write_properties(Fd_Project_Writer &f) {
  Fl_Type::write_properties(f);
  switch (public_) {
    case 0: f.write_string("private"); break;
    case 2: f.write_string("protected"); break;
  }
  if (cdecl_) f.write_string("C");
  if (return_type) {
    f.write_string("return_type");
    f.write_word(return_type);
  }
}

/**
 Read function specific properties fron an .fl file.
 \param[in] c read from this string
 */
void Fl_Function_Type::read_property(Fd_Project_Reader &f, const char *c) {
  if (!strcmp(c,"private")) {
    public_ = 0;
  } else if (!strcmp(c,"protected")) {
    public_ = 2;
  } else if (!strcmp(c,"C")) {
    cdecl_ = 1;
  } else if (!strcmp(c,"return_type")) {
    storestring(f.read_word(),return_type);
  } else {
    Fl_Type::read_property(f, c);
  }
}

/**
 Open the function_panel dialog box to edit this function.
 */
void Fl_Function_Type::open() {
  // fill dialog box
  if (!function_panel) make_function_panel();
  f_return_type_input->value(return_type);
  f_name_input->value(name());
  if (is_in_class()) {
    f_public_member_choice->value(public_);
    f_public_member_choice->show();
    f_public_choice->hide();
    f_c_button->hide();
  } else {
    f_public_choice->value(public_);
    f_public_choice->show();
    f_public_member_choice->hide();
    f_c_button->show();
  }
  f_c_button->value(cdecl_);
  const char *c = comment();
  f_comment_input->buffer()->text(c?c:"");
  function_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    // - message loop until OK or cancel is pressed
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == f_panel_cancel) goto BREAK2;
      else if (w == f_panel_ok) break;
      else if (!w) Fl::wait();
    }
    // - check syntax
    const char *c = f_name_input->value();
    while (isspace(*c)) c++;
    message = c_check(c);
    if (!message) {
      const char *d = c;
      for (; *d != '('; d++) if (isspace(*d) || !*d) break;
      if (*c && *d != '(')
        message = "must be 'name(arguments)'";
    }
    if (!message) {
      c = f_return_type_input->value();
      message = c_check(c);
    }
    // - alert user
    if (message) {
      int v = fl_choice("Potential syntax error detected: %s",
                        "Continue Editing", "Ignore Error", NULL, message);
      if (v==0) continue;     // Continue Editing
      //if (v==1) { }         // Ignore Error and close dialog
    }
    // - copy dialog data to target variables
    int mod = 0;
    name(f_name_input->value());
    storestring(f_return_type_input->value(), return_type);
    if (is_in_class()) {
      if (public_ != f_public_member_choice->value()) {
        mod = 1;
        public_ = f_public_member_choice->value();
        redraw_browser();
      }
    } else {
      if (public_ != f_public_choice->value()) {
        mod = 1;
        public_ = f_public_choice->value();
        redraw_browser();
      }
    }
    if (cdecl_ != f_c_button->value()) {
      mod = 1;
      cdecl_ = f_c_button->value();
    }
    c = f_comment_input->buffer()->text();
    if (c && *c) {
      if (!comment() || strcmp(c, comment()))  { set_modflag(1); redraw_browser(); }
      comment(c);
    } else {
      if (comment())  { set_modflag(1); redraw_browser(); }
      comment(0);
    }
    if (c) free((void*)c);
    if (mod) set_modflag(1);
    break;
  }
BREAK2:
  function_panel->hide();
}

/**
 Return 1 if the function is global.
 \return 1 if public, 0 if local.
 */
int Fl_Function_Type::is_public() const {
  return public_;
}

static bool fd_isspace(int c) {
  return (c>0 && c<128 && isspace(c));
}

// code duplication: see int is_id(char c) in code.cxx
static bool fd_iskeyword(int c) {
  return (c>0 && c<128 && (isalnum(c) || c=='_'));
}

// remove all function default parameters and `override` keyword
static void clean_function_for_implementation(char *out, const char *function_name) {
  char *sptr = out;
  const char *nptr = function_name;
  int skips=0,skipc=0;
  int nc=0,plevel=0;
  bool arglist_done = false;
  for (;*nptr; nc++,nptr++) {
    if (arglist_done && fd_isspace(nptr[0])) {
      // skip `override` and `FL_OVERRIDE` keywords if they are following the list of arguments
      if (strncmp(nptr+1, "override", 8)==0 && !fd_iskeyword(nptr[9])) { nptr += 8; continue; }
      else if (strncmp(nptr+1, "FL_OVERRIDE", 11)==0 && !fd_iskeyword(nptr[12])) { nptr += 11; continue; }
    }
    if (!skips && *nptr=='(') plevel++;
    else if (!skips && *nptr==')') { plevel--; if (plevel==0) arglist_done = true; }
    if ( *nptr=='"' &&  !(nc &&  *(nptr-1)=='\\') )
      skips = skips ? 0 : 1;
    else if(!skips && *nptr=='\'' &&  !(nc &&  *(nptr-1)=='\\'))
      skipc = skipc ? 0 : 1;
    if(!skips && !skipc && plevel==1 && *nptr =='=' && !(nc && *(nptr-1)=='\'') ) { // ignore '=' case
      while(*++nptr  && (skips || skipc || ( (*nptr!=',' && *nptr!=')') || plevel!=1) )) {
        if ( *nptr=='"' &&  *(nptr-1)!='\\' )
          skips = skips ? 0 : 1;
        else if(!skips && *nptr=='\'' &&  *(nptr-1)!='\\')
          skipc = skipc ? 0 : 1;
        if (!skips && !skipc && *nptr=='(') plevel++;
        else if (!skips && *nptr==')') plevel--;
      }
      if (*nptr==')') if (--plevel==0) arglist_done = true;
    }
    if (sptr < (out + 1024 - 1)) *sptr++ = *nptr;
  }
  *sptr = '\0';
}


/**
 Write the code for the source and the header file.
 This writes the code that goes \b before all children of this class.
 \see write_code2(Fd_Code_Writer& f)
 */
void Fl_Function_Type::write_code1(Fd_Code_Writer& f) {
  constructor=0;
  havewidgets = 0;
  Fl_Type *child;
  // if the function has no children (hence no body), Fluid will not generate
  // the function either. This is great if you decide to implement that function
  // inside another module
  char havechildren = 0;
  for (child = next; child && child->level > level; child = child->next) {
    havechildren = 1;
    if (child->is_widget()) {
      havewidgets = 1;
      break;
    }
  }
  if (havechildren)
    f.write_c("\n");
  if (ismain()) {
    if (havechildren)
      f.write_c("int main(int argc, char **argv) {\n");
  } else {
    const char* rtype = return_type;
    const char* star = "";
    // from matt: let the user type "static " at the start of type
    // in order to declare a static method;
    int is_static = 0;
    int is_virtual = 0;
    if (rtype) {
      if (!strcmp(rtype,"static")) {is_static = 1; rtype = 0;}
      else if (!strncmp(rtype, "static ",7)) {is_static = 1; rtype += 7;}
    }
    if (rtype) {
      if (!strcmp(rtype, "virtual")) {is_virtual = 1; rtype = 0;}
      else if (!strncmp(rtype, "virtual ",8)) {is_virtual = 1; rtype += 8;}
    }
    if (!rtype) {
      if (havewidgets) {
        rtype = subclassname(child);
        star = "*";
      } else rtype = "void";
    }

    const char* k = class_name(0);
    if (k) {
      f.write_public(public_);
      if (havechildren)
        write_comment_c(f);
      if (name()[0] == '~')
        constructor = 1;
      else {
        size_t n = strlen(k);
        if (!strncmp(name(), k, n) && name()[n] == '(') constructor = 1;
      }
      f.write_h("%s", f.indent(1));
      if (is_static) f.write_h("static ");
      if (is_virtual) f.write_h("virtual ");
      if (!constructor) {
        f.write_h("%s%s ", rtype, star);
        if (havechildren)
          f.write_c("%s%s ", rtype, star);
      }

      // if this is a subclass, only f.write_h() the part before the ':'
      char s[1024], *sptr = s;
      char *nptr = (char *)name();

      while (*nptr) {
        if (*nptr == ':') {
          if (nptr[1] != ':') break;
          // Copy extra ":" for "class::member"...
          *sptr++ = *nptr++;
        }
        *sptr++ = *nptr++;
      }
      *sptr = '\0';

      if (s[strlen(s)-1] == '}') {  // special case for inlined functions
        f.write_h("%s\n", s);
      } else {
        f.write_h("%s;\n", s);
      }
      if (havechildren) {
        clean_function_for_implementation(s, name());
        f.write_c("%s::%s {\n", k, s);
      }
    } else {
      if (havechildren)
        write_comment_c(f);
      if (public_==1) {
        if (cdecl_)
          f.write_h("extern \"C\" { %s%s %s; }\n", rtype, star, name());
        else
          f.write_h("%s%s %s;\n", rtype, star, name());
      } else if (public_==2) {
        // write neither the prototype nor static, the function may be declared elsewhere
      } else {
        if (havechildren)
          f.write_c("static ");
      }

      // write everything but the default parameters (if any)
      char s[1024];
      if (havechildren) {
        clean_function_for_implementation(s, name());
        f.write_c("%s%s %s {\n", rtype, star, s);
      }
    }
  }

  if (havewidgets && child && !child->name())
    f.write_c("%s%s* w;\n", f.indent(1), subclassname(child));
  f.indentation++;
}

/**
 Write the code for the source and the header file.
 This writes the code that goes \b after all children of this class.
 \see write_code1(Fd_Code_Writer& f)
 */
void Fl_Function_Type::write_code2(Fd_Code_Writer& f) {
  Fl_Type *child;
  const char *var = "w";
  char havechildren = 0;
  for (child = next; child && child->level > level; child = child->next) {
    havechildren = 1;
    if (child->is_a(ID_Window) && child->name()) var = child->name();
  }

  if (ismain()) {
    if (havewidgets)
      f.write_c("%s%s->show(argc, argv);\n", f.indent(1), var);
    if (havechildren)
      f.write_c("%sreturn Fl::run();\n", f.indent(1));
  } else if (havewidgets && !constructor && !return_type) {
    f.write_c("%sreturn %s;\n", f.indent(1), var);
  }
  if (havechildren)
    f.write_c("}\n");
  f.indentation = 0;
}

/**
 Check if the return type and signature s match.
 \param[in] rtype function return type
 \param[in] sig function name followed by arguments
 \return 1 if they match, 0 if not
 */
int Fl_Function_Type::has_signature(const char *rtype, const char *sig) const {
  if (rtype && !return_type) return 0;
  if (!name()) return 0;
  if ( (rtype==0L || strcmp(return_type, rtype)==0)
      && fl_filename_match(name(), sig)) {
    return 1;
  }
  return 0;
}

// ---- Fl_Code_Type declaration

/** \class Fl_Code_Type
 Manage a block of C++ code in the Fluid design.

 This node manages an arbitrary block of code inside a function that will
 be written into the source code file. Fl_Code_Block has no comment field.
 However, the first line of code will be shown in the widget browser.
 */

/// Prototype for code to be used by the factory.
Fl_Code_Type Fl_Code_type;

/**
 Constructor.
 */
Fl_Code_Type::Fl_Code_Type() :
  cursor_position_(0),
  code_input_scroll_row(0),
  code_input_scroll_col(0)
{}

/**
 Make a new code node.
 If the parent node is not a function, a message box will pop up and
 the request will be ignored.
 \param[in] strategy add code after current or as last child
 \return new Code node
 */
Fl_Type *Fl_Code_Type::make(Strategy strategy) {
  Fl_Type *anchor = Fl_Type::current, *p = anchor;
  if (p && (strategy == kAddAfterCurrent)) p = p->parent;
  while (p && !p->is_code_block()) {
    anchor = p;
    strategy = kAddAfterCurrent;
    p = p->parent;
  }
  if (!p) {
    fl_message("Please select a function");
    return 0;
  }
  Fl_Code_Type *o = new Fl_Code_Type();
  o->name("printf(\"Hello, World!\\n\");");
  o->add(anchor, strategy);
  o->factory = this;
  return o;
}

/**
 Open the code_panel or an external editor to edit this code section.
 */
void Fl_Code_Type::open() {
  // Using an external code editor? Open it..
  if ( G_use_external_editor && G_external_editor_command[0] ) {
    const char *cmd = G_external_editor_command;
    const char *code = name();
    if (!code) code = "";
    if ( editor_.open_editor(cmd, code) == 0 )
      return;   // return if editor opened ok, fall thru to built-in if not
  }
  // Use built-in code editor..
  if (!code_panel) make_code_panel();
  const char *text = name();
  code_input->buffer()->text( text ? text : "" );
  code_input->insert_position(cursor_position_);
  code_input->scroll(code_input_scroll_row, code_input_scroll_col);
  code_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == code_panel_cancel) goto BREAK2;
      else if (w == code_panel_ok) break;
      else if (!w) Fl::wait();
    }
    char*c = code_input->buffer()->text();
    message = c_check(c);
    if (message) {
      int v = fl_choice("Potential syntax error detected: %s",
                        "Continue Editing", "Ignore Error", NULL, message);
      if (v==0) continue;     // Continue Editing
      //if (v==1) { }         // Ignore Error and close dialog
    }
    name(c);
    free(c);
    break;
  }
  cursor_position_ = code_input->insert_position();
  code_input_scroll_row = code_input->scroll_row();
  code_input_scroll_col = code_input->scroll_col();
BREAK2:
  code_panel->hide();
}

/**
 Grab changes from an external editor and write this node.
 */
void Fl_Code_Type::write(Fd_Project_Writer &f) {
  // External editor changes? If so, load changes into ram, update mtime/size
  if ( handle_editor_changes() == 1 ) {
    main_window->redraw();    // tell fluid to redraw; edits may affect tree's contents
  }
  Fl_Type::write(f);
}

/**
 Write the code block with the correct indentation.
 */
void Fl_Code_Type::write_code1(Fd_Code_Writer& f) {
  // External editor changes? If so, load changes into ram, update mtime/size
  if ( handle_editor_changes() == 1 ) {
    main_window->redraw();    // tell fluid to redraw; edits may affect tree's contents
  }
  // Matt: disabled f.tag(FD_TAG_GENERIC, 0);
  f.write_c_indented(name(), 0, '\n');
  // Matt: disabled f.tag(FD_TAG_CODE, get_uid());
}

/**
 See if external editor is open.
 */
int Fl_Code_Type::is_editing() {
  return editor_.is_editing();
}

/**
 Reap the editor's pid
 \return -2: editor not open
 \return -1: wait failed
 \return 0: process still running
 \return \>0: process finished + reaped (returns pid)
 */
int Fl_Code_Type::reap_editor() {
  return editor_.reap_editor();
}

/**
 Handle external editor file modifications.
 If changed, record keeping is updated and file's contents is loaded into ram
 \return 0: file unchanged or not editing
 \return 1: file changed, internal records updated, 'code' has new content
 \return -1: error getting file info (get_ms_errmsg() has reason)
 \todo Figure out how saving a fluid file can be intercepted to grab
    current contents of editor file..
 */
int Fl_Code_Type::handle_editor_changes() {
  const char *newcode = 0;
  switch ( editor_.handle_changes(&newcode) ) {
    case 1: {            // (1)=changed
      name(newcode);     // update value in ram
      free((void*)newcode);
      return 1;
    }
    case -1: return -1;  // (-1)=error -- couldn't read file (dialog showed reason)
    default: break;      // (0)=no change
  }
  return 0;
}

// ---- Fl_CodeBlock_Type implementation

/** \class Fl_CodeBlock_Type
 Manage two blocks of C++ code enclosing its children.

 This node manages two lines of code that enclose all children
 of this node. This is usually an if..then clause.

 \todo this node could support multiple lines of code for each block.
 */

/// Prototype for a block of code to be used by the factory.
Fl_CodeBlock_Type Fl_CodeBlock_type;

/**
 Constructor.
 */
Fl_CodeBlock_Type::Fl_CodeBlock_Type() :
  Fl_Type(),
  after(NULL)
{ }

/**
 Destructor.
 */
Fl_CodeBlock_Type::~Fl_CodeBlock_Type() {
  if (after)
    free((void*)after);
}

/**
 Make a new code block.
 If the parent node is not a function or another codeblock, a message box will
 pop up and the request will be ignored.
 \param[in] strategy add after current or as last child
 \return new CodeBlock
 */
Fl_Type *Fl_CodeBlock_Type::make(Strategy strategy) {
  Fl_Type *anchor = Fl_Type::current, *p = anchor;
  if (p && (strategy == kAddAfterCurrent)) p = p->parent;
  while (p && !p->is_code_block()) {
    anchor = p;
    strategy = kAddAfterCurrent;
    p = p->parent;
  }
  if (!p) {
    fl_message("Please select a function");
    return 0;
  }
  Fl_CodeBlock_Type *o = new Fl_CodeBlock_Type();
  o->name("if (test())");
  o->after = 0;
  o->add(anchor, strategy);
  o->factory = this;
  return o;
}

/**
 Write the specific properties for this node.
  - "after" is followed by the code that comes after the children
 The "before" code is stored in the name() field.
 */
void Fl_CodeBlock_Type::write_properties(Fd_Project_Writer &f) {
  Fl_Type::write_properties(f);
  if (after) {
    f.write_string("after");
    f.write_word(after);
  }
}

/**
 Read the node specific properties.
 */
void Fl_CodeBlock_Type::read_property(Fd_Project_Reader &f, const char *c) {
  if (!strcmp(c,"after")) {
    storestring(f.read_word(),after);
  } else {
    Fl_Type::read_property(f, c);
  }
}

/**
 Open the codeblock_panel.
 */
void Fl_CodeBlock_Type::open() {
  if (!codeblock_panel) make_codeblock_panel();
  code_before_input->value(name());
  code_after_input->value(after);
  codeblock_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    // event loop
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == codeblock_panel_cancel) goto BREAK2;
      else if (w == codeblock_panel_ok) break;
      else if (!w) Fl::wait();
    }
    // check for syntax errors
    message = c_check(code_before_input->value());
    if (!message) {
      message = c_check(code_after_input->value());
    }
    // alert user
    if (message) {
      int v = fl_choice("Potential syntax error detected: %s",
                        "Continue Editing", "Ignore Error", NULL, message);
      if (v==0) continue;     // Continue Editing
      //if (v==1) { }         // Ignore Error and close dialog
    }
    // write to variables
    name(code_before_input->value());
    storestring(code_after_input->value(), after);
    break;
  }
BREAK2:
  codeblock_panel->hide();
}

/**
 Write the "before" code.
 */
void Fl_CodeBlock_Type::write_code1(Fd_Code_Writer& f) {
  const char* c = name();
  f.write_c("%s%s {\n", f.indent(), c ? c : "");
  f.indentation++;
}

/**
 Write the "after" code.
 */
void Fl_CodeBlock_Type::write_code2(Fd_Code_Writer& f) {
  f.indentation--;
  if (after) f.write_c("%s} %s\n", f.indent(), after);
  else f.write_c("%s}\n", f.indent());
}

// ---- Fl_Decl_Type declaration

/** \class Fl_Decl_Type
 Manage the C/C++ declaration of a variable.

 This node manages a single line of code that can be in the header or the source
 code, and can be made static.

 \todo this node could support multiple lines.
 */

/// Prototype for a declaration to be used by the factory.
Fl_Decl_Type Fl_Decl_type;

/**
 Constructor.
 */
Fl_Decl_Type::Fl_Decl_Type() :
  public_(0),
  static_(1)
{ }

/**
 Return 1 if this declaration and its parents are public.
 */
int Fl_Decl_Type::is_public() const
{
  Fl_Type *p = parent;
  while (p && !p->is_decl_block()) p = p->parent;
  if(p && p->is_public() && public_)
    return public_;
  else if(!p)
    return public_;
  return 0;
}

/**
 Make a new declaration.
 \param[in] strategy add after current or as last child
 \return new Declaration node
 */
Fl_Type *Fl_Decl_Type::make(Strategy strategy) {
  Fl_Type *anchor = Fl_Type::current, *p = anchor;
  if (p && (strategy == kAddAfterCurrent)) p = p->parent;
  while (p && !p->is_decl_block()) {
    anchor = p;
    strategy = kAddAfterCurrent;
    p = p->parent;
  }
  Fl_Decl_Type *o = new Fl_Decl_Type();
  o->public_ = 0;
  o->static_ = 1;
  o->name("int x;");
  o->add(anchor, strategy);
  o->factory = this;
  return o;
}

/**
 Write the specific properties.
  - "private"/"public"/"protected"
  - "local"/"global" if this is static or not
 */
void Fl_Decl_Type::write_properties(Fd_Project_Writer &f) {
  Fl_Type::write_properties(f);
  switch (public_) {
    case 0: f.write_string("private"); break;
    case 1: f.write_string("public"); break;
    case 2: f.write_string("protected"); break;
  }
  if (static_)
    f.write_string("local");
  else
    f.write_string("global");
}

/**
 Read the specific properties.
 */
void Fl_Decl_Type::read_property(Fd_Project_Reader &f, const char *c) {
  if (!strcmp(c,"public")) {
    public_ = 1;
  } else if (!strcmp(c,"private")) {
    public_ = 0;
  } else if (!strcmp(c,"protected")) {
    public_ = 2;
  } else if (!strcmp(c,"local")) {
    static_ = 1;
  } else if (!strcmp(c,"global")) {
    static_ = 0;
  } else {
    Fl_Type::read_property(f, c);
  }
}

/**
 Open the decl_panel to edit this node.
 */
void Fl_Decl_Type::open() {
  if (!decl_panel) make_decl_panel();
  decl_input->buffer()->text(name());
  if (is_in_class()) {
    decl_class_choice->value(public_);
    decl_class_choice->show();
    decl_choice->hide();
  } else {
    decl_choice->value((public_&1)|((static_&1)<<1));
    decl_choice->show();
    decl_class_choice->hide();
  }
  const char *c = comment();
  decl_comment_input->buffer()->text(c?c:"");
  decl_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    // event loop
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == decl_panel_cancel) goto BREAK2;
      else if (w == decl_panel_ok) break;
      else if (!w) Fl::wait();
    }
    // check values
    const char*c = decl_input->buffer()->text();
    while (isspace(*c)) c++;
    message = c_check(c&&c[0]=='#' ? c+1 : c);
    // alert user
    if (message) {
      int v = fl_choice("Potential syntax error detected: %s",
                        "Continue Editing", "Ignore Error", NULL, message);
      if (v==0) continue;     // Continue Editing
      //if (v==1) { }         // Ignore Error and close dialog
    }
    // copy vlaues
    name(c);
    if (is_in_class()) {
      if (public_!=decl_class_choice->value()) {
        set_modflag(1);
        public_ = decl_class_choice->value();
      }
    } else {
      if (public_!=(decl_choice->value()&1)) {
        set_modflag(1);
        public_ = (decl_choice->value()&1);
      }
      if (static_!=((decl_choice->value()>>1)&1)) {
        set_modflag(1);
        static_ = ((decl_choice->value()>>1)&1);
      }
    }
    c = decl_comment_input->buffer()->text();
    if (c && *c) {
      if (!comment() || strcmp(c, comment()))  { set_modflag(1); redraw_browser(); }
      comment(c);
    } else {
      if (comment())  { set_modflag(1); redraw_browser(); }
      comment(0);
    }
    if (c) free((void*)c);
    break;
  }
BREAK2:
  decl_panel->hide();
}

/**
 Write the code to the source and header files.
 \todo There are a lot of side effect in this node depending on the given text
    and the parent node. They need to be understood and documented.
 */
void Fl_Decl_Type::write_code1(Fd_Code_Writer& f) {
  const char* c = name();
  if (!c) return;
  // handle a few keywords differently if inside a class
  if (is_in_class() && (   (!strncmp(c,"class",5) && isspace(c[5]))
                        || (!strncmp(c,"typedef",7) && isspace(c[7]))
                        || (!strncmp(c,"FL_EXPORT",9) && isspace(c[9]))
                        || (!strncmp(c,"struct",6) && isspace(c[6]))
                        || (!strncmp(c,"enum",4) && isspace(c[4]))
                        ) ) {
    f.write_public(public_);
    write_comment_h(f, f.indent(1));
    f.write_h("%s%s\n", f.indent(1), c);
    return;
  }
  // handle putting #include, extern, using or typedef into decl:
  if (   (!isalpha(*c) && *c != '~')
      || (!strncmp(c,"extern",6) && isspace(c[6]))
      || (!strncmp(c,"class",5) && isspace(c[5]))
      || (!strncmp(c,"typedef",7) && isspace(c[7]))
      || (!strncmp(c,"using",5) && isspace(c[5]))
      || (!strncmp(c,"FL_EXPORT",9) && isspace(c[9]))
      //    || !strncmp(c,"struct",6) && isspace(c[6])
      ) {
    if (public_) {
      write_comment_h(f);
      f.write_h("%s\n", c);
    } else {
      write_comment_c(f);
      f.write_c("%s\n", c);
    }
    return;
  }
  // find the first C++ style comment
  const char* e = c+strlen(c), *csc = c;
  while (csc<e && (csc[0]!='/' || csc[1]!='/')) csc++;
  if (csc!=e) e = csc; // comment found
  // lose spaces between text and comment, if any
  while (e>c && e[-1]==' ') e--;
  if (class_name(1)) {
    f.write_public(public_);
    write_comment_h(f, f.indent(1));
    f.write_hc(f.indent(1), int(e-c), c, csc);
  } else {
    if (public_) {
      if (static_)
        f.write_h("extern ");
      else
        write_comment_h(f);
      f.write_hc("", int(e-c), c, csc);

      if (static_) {
        write_comment_c(f);
        f.write_cc("", int(e-c), c, csc);
      }
    } else {
      write_comment_c(f);
      if (static_)
        f.write_c("static ");
      f.write_cc("", int(e-c), c, csc);
    }
  }
}

// ---- Fl_Data_Type declaration

/** \class Fl_Data_Type
 Manage data from an external arbitrary file.

 The content of the file will be stored in binary inside the generated
 code. This can be used to store images inline in the source code,
 */

/// Prototype for a data node to be used by the factory.
Fl_Data_Type Fl_Data_type;

/**
 Constructor.
 */
Fl_Data_Type::Fl_Data_Type() :
  Fl_Decl_Type(),
  filename_(NULL),
  text_mode_(0)
{ }

/**
 Destructor.
 */
Fl_Data_Type::~Fl_Data_Type() {
  if (filename_)
    free((void*)filename_);
}

/**
 Create an empty inline data node.
 \param[in] strategy add after current or as last child
 \return new inline data node
 */
Fl_Type *Fl_Data_Type::make(Strategy strategy) {
  Fl_Type *anchor = Fl_Type::current, *p = anchor;
  if (p && (strategy == kAddAfterCurrent)) p = p->parent;
  while (p && !p->is_decl_block()) {
    anchor = p;
    strategy = kAddAfterCurrent;
    p = p->parent;
  }
  Fl_Data_Type *o = new Fl_Data_Type();
  o->public_ = 1;
  o->static_ = 1;
  o->filename_ = 0;
  o->text_mode_ = 0;
  o->name("myInlineData");
  o->add(anchor, strategy);
  o->factory = this;
  return o;
}

/**
 Write additional properties.
  - "filename" followed by the filename of the file to inline
  - "textmode" if data is written in ASCII vs. binary
 */
void Fl_Data_Type::write_properties(Fd_Project_Writer &f) {
  Fl_Decl_Type::write_properties(f);
  if (filename_) {
    f.write_string("filename");
    f.write_word(filename_);
  }
  if (text_mode_ == 1) {
    f.write_string("textmode");
  }
  if (text_mode_ == 2) {
    f.write_string("compressed");
  }
}

/**
 Read specific properties.
 */
void Fl_Data_Type::read_property(Fd_Project_Reader &f, const char *c) {
  if (!strcmp(c,"filename")) {
    storestring(f.read_word(), filename_, 1);
  } else if (!strcmp(c,"textmode")) {
    text_mode_ = 1;
  } else if (!strcmp(c,"compressed")) {
    text_mode_ = 2;
  } else {
    Fl_Decl_Type::read_property(f, c);
  }
}

/**
 Open the data_panel to edit this node.
 */
void Fl_Data_Type::open() {
  if (!data_panel) make_data_panel();
  data_input->value(name());
  if (is_in_class()) {
    data_class_choice->value(public_);
    data_class_choice->show();
    data_choice->hide();
  } else {
    data_choice->value((public_&1)|((static_&1)<<1));
    data_choice->show();
    data_class_choice->hide();
  }
  data_mode->value(text_mode_);
  data_filename->value(filename_?filename_:"");
  const char *c = comment();
  data_comment_input->buffer()->text(c?c:"");
  data_panel->show();
  for (;;) { // repeat as long as there are errors
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == data_panel_cancel) goto BREAK2;
      else if (w == data_panel_ok) break;
      else if (w == data_filebrowser) {
        enter_project_dir();
        const char *fn = fl_file_chooser("Load Inline Data", 0L, data_filename->value(), 1);
        leave_project_dir();
        if (fn) {
          if (strcmp(fn, data_filename->value()))
            set_modflag(1);
          data_filename->value(fn);
        }
      }
      else if (!w) Fl::wait();
    }
    // store the variable name:
    const char*c = data_input->value();
    char *s = fl_strdup(c), *p = s, *q, *n;
    for (;;++p) { // remove leading spaces
      if (!isspace((unsigned char)(*p))) break;
    }
    n = p;
    if ( (!isalpha((unsigned char)(*p))) && ((*p)!='_') && ((*p)!=':') ) goto OOPS;
    ++p;
    for (;;++p) {
      if ( (!isalnum((unsigned char)(*p))) && ((*p)!='_') && ((*p)!=':') ) break;
    }
    q = p;
    for (;;++q) {
      if (!*q) break;
      if (!isspace((unsigned char)(*q))) goto OOPS;
    }
    *p = 0; // remove trailing spaces
    if (n==q) {
    OOPS:
      int v = fl_choice("%s",
                        "Continue Editing", "Ignore Error", NULL,
                        "Variable name must be a C identifier");
      if (v==0) { free(s); continue; }    // Continue Editing
      //if (v==1) { }                     // Ignore Error and close dialog
    }
    undo_checkpoint();
    name(n);
    free(s);
    // store flags
    if (is_in_class()) {
      if (public_!=data_class_choice->value()) {
        set_modflag(1);
        public_ = data_class_choice->value();
      }
    } else {
      if (public_!=(data_choice->value()&1)) {
        set_modflag(1);
        public_ = (data_choice->value()&1);
      }
      if (static_!=((data_choice->value()>>1)&1)) {
        set_modflag(1);
        static_ = ((data_choice->value()>>1)&1);
      }
    }
    text_mode_ = data_mode->value();
    if (text_mode_ < 0) text_mode_ = 0;
    if (text_mode_ > 2) text_mode_ = 2;
    // store the filename
    c = data_filename->value();
    if (filename_ && strcmp(filename_, data_filename->value()))
      set_modflag(1);
    else if (!filename_ && *c)
      set_modflag(1);
    if (filename_) { free((void*)filename_); filename_ = 0L; }
    if (c && *c) filename_ = fl_strdup(c);
    // store the comment
    c = data_comment_input->buffer()->text();
    if (c && *c) {
      if (!comment() || strcmp(c, comment()))  { set_modflag(1); redraw_browser(); }
      comment(c);
    } else {
      if (comment())  { set_modflag(1); redraw_browser(); }
      comment(0);
    }
    if (c) free((void*)c);
    set_modflag(1);
    break;
  }
BREAK2:
  data_panel->hide();
}

/**
 Write the content of the external file inline into the source code.
 */
void Fl_Data_Type::write_code1(Fd_Code_Writer& f) {
  const char *message = 0;
  const char *c = name();
  if (!c) return;
  const char *fn = filename_;
  char *data = 0;
  int nData = -1;
  int uncompressedDataSize = 0;
  // path should be set correctly already
  if (filename_ && !f.write_codeview) {
    enter_project_dir();
    FILE *f = fl_fopen(filename_, "rb");
    leave_project_dir();
    if (!f) {
      message = "Can't include data from file. Can't open";
    } else {
      fseek(f, 0, SEEK_END);
      nData = (int)ftell(f);
      fseek(f, 0, SEEK_SET);
      if (nData) {
        data = (char*)calloc(nData, 1);
        if (fread(data, nData, 1, f)==0) { /* use default */ }
        if (text_mode_ == 2) {
          uncompressedDataSize = nData;
          uLong nzData = compressBound(nData);
          Bytef *zdata = (Bytef*)::malloc(nzData);
          if (compress(zdata, &nzData, (Bytef*)data, nData) != Z_OK) { /* error */ }
          ::free(data);
          data = (char*)zdata;
          nData = (int)nzData;
        }
      }
      fclose(f);
    }
  } else {
    fn = filename_ ? filename_ : "<no filename>";
  }
  if (is_in_class()) {
    f.write_public(public_);
    if (text_mode_ == 1) {
      f.write_h("%sstatic const char *%s;\n", f.indent(1), c);
      f.write_c("\n");
      write_comment_c(f);
      f.write_c("const char *%s::%s = /* text inlined from %s */\n", class_name(1), c, fn);
      if (message) f.write_c("#error %s %s\n", message, fn);
      f.write_cstring(data, nData);
    } else if (text_mode_ == 2) {
      f.write_h("%sstatic int %s_size;\n", f.indent(1), c);
      f.write_h("%sstatic unsigned char %s[%d];\n", f.indent(1), c, nData);
      f.write_c("\n");
      write_comment_c(f);
      f.write_c("int %s::%s_size = %d;\n", class_name(1), c, uncompressedDataSize);
      f.write_c("unsigned char %s::%s[%d] = /* data compressed and inlined from %s */\n", class_name(1), c, nData, fn);
      if (message) f.write_c("#error %s %s\n", message, fn);
      f.write_cdata(data, nData);
    } else {
      f.write_h("%sstatic unsigned char %s[%d];\n", f.indent(1), c, nData);
      f.write_c("\n");
      write_comment_c(f);
      f.write_c("unsigned char %s::%s[%d] = /* data inlined from %s */\n", class_name(1), c, nData, fn);
      if (message) f.write_c("#error %s %s\n", message, fn);
      f.write_cdata(data, nData);
    }
    f.write_c(";\n");
  } else {
    // the "header only" option does not apply here!
    if (public_) {
      if (static_) {
        if (text_mode_ == 1) {
          f.write_h("extern const char *%s;\n", c);
          f.write_c("\n");
          write_comment_c(f);
          f.write_c("const char *%s = /* text inlined from %s */\n", c, fn);
          if (message) f.write_c("#error %s %s\n", message, fn);
          f.write_cstring(data, nData);
        } else if (text_mode_ == 2) {
          f.write_h("extern int %s_size;\n", c);
          f.write_h("extern unsigned char %s[%d];\n", c, nData);
          f.write_c("\n");
          write_comment_c(f);
          f.write_c("int %s_size = %d;\n", c, uncompressedDataSize);
          f.write_c("unsigned char %s[%d] = /* data compressed and inlined from %s */\n", c, nData, fn);
          if (message) f.write_c("#error %s %s\n", message, fn);
          f.write_cdata(data, nData);
        } else {
          f.write_h("extern unsigned char %s[%d];\n", c, nData);
          f.write_c("\n");
          write_comment_c(f);
          f.write_c("unsigned char %s[%d] = /* data inlined from %s */\n", c, nData, fn);
          if (message) f.write_c("#error %s %s\n", message, fn);
          f.write_cdata(data, nData);
        }
        f.write_c(";\n");
      } else {
        write_comment_h(f);
        f.write_h("#error Unsupported declaration loading inline data %s\n", fn);
        if (text_mode_ == 1)
          f.write_h("const char *%s = \"abc...\";\n", c);
        else
          f.write_h("unsigned char %s[3] = { 1, 2, 3 };\n", c);
      }
    } else {
      f.write_c("\n");
      write_comment_c(f);
      if (static_)
        f.write_c("static ");
      if (text_mode_ == 1) {
        f.write_c("const char *%s = /* text inlined from %s */\n", c, fn);
        if (message) f.write_c("#error %s %s\n", message, fn);
        f.write_cstring(data, nData);
      } else if (text_mode_ == 2) {
        f.write_c("int %s_size = %d;\n", c, uncompressedDataSize);
        if (static_) f.write_c("static ");
        f.write_c("unsigned char %s[%d] = /* data compressed and inlined from %s */\n", c, nData, fn);
        if (message) f.write_c("#error %s %s\n", message, fn);
        f.write_cdata(data, nData);
      } else {
        f.write_c("unsigned char %s[%d] = /* data inlined from %s */\n", c, nData, fn);
        if (message) f.write_c("#error %s %s\n", message, fn);
        f.write_cdata(data, nData);
      }
      f.write_c(";\n");
    }
  }
  // if we are in interactive mode, we pop up a warning dialog
  // giving the error: (batch_mode && !write_codeview) ???
  if (message && !f.write_codeview) {
    if (batch_mode)
      fprintf(stderr, "FLUID ERROR: %s %s\n", message, fn);
    else
      fl_alert("%s\n%s\n", message, fn);
  }
  if (data) free(data);
}

// ---- Fl_DeclBlock_Type declaration

/** \class Fl_DeclBlock_Type
 Manage a declaration block.

 Declaration blocks have two text field that are written before and after
 the children of this block. This block is located at the top level and
 is written to the source file, and to the header file, if declared public.
 */

/// Prototype for a declaration block to be used by the factory.
Fl_DeclBlock_Type Fl_DeclBlock_type;

/**
 Constructor.
 */
Fl_DeclBlock_Type::Fl_DeclBlock_Type() :
  Fl_Type(),
  after(NULL),
  write_map_(CODE_IN_SOURCE)
{ }

/**
 Destructor.
 */
Fl_DeclBlock_Type::~Fl_DeclBlock_Type() {
  if (after)
    ::free((void*)after);
}

/**
 Return 1 if this block is public.
 */
int Fl_DeclBlock_Type::is_public() const {
  return ((write_map_&CODE_IN_HEADER) != 0);
}

/**
 Create a new declaration block.
 \param[in] strategy add after current or as last child
 \return new Declaration Block node
 */
Fl_Type *Fl_DeclBlock_Type::make(Strategy strategy) {
  Fl_Type *anchor = Fl_Type::current, *p = anchor;
  if (p && (strategy == kAddAfterCurrent)) p = p->parent;
  while (p && !p->is_decl_block()) {
    anchor = p;
    strategy = kAddAfterCurrent;
    p = p->parent;
  }
  Fl_DeclBlock_Type *o = new Fl_DeclBlock_Type();
  o->name("#if 1");
  o->write_map_ = CODE_IN_SOURCE;
  o->after = fl_strdup("#endif");
  o->add(anchor, strategy);
  o->factory = this;
  return o;
}

/**
 Write the specific properties.
  - "public"/"protected"
  - "after" followed by the second code block.
 */
void Fl_DeclBlock_Type::write_properties(Fd_Project_Writer &f) {
  Fl_Type::write_properties(f);
  // deprecated
  if (is_public()) f.write_string("public");
  // new way to map declaration block to various parts of the generated code
  if (write_map_ != CODE_IN_SOURCE)
    f.write_string("map %d", write_map_);
  f.write_string("after");
  f.write_word(after);
}

/**
 Read the specific properties.
 */
void Fl_DeclBlock_Type::read_property(Fd_Project_Reader &f, const char *c) {
  if(!strcmp(c,"public")) {
    write_map_ |= CODE_IN_HEADER;
  } else if(!strcmp(c,"protected")) {
    //
  } else if(!strcmp(c,"map")) {
    write_map_ = (int)atol(f.read_word());
  } else  if (!strcmp(c,"after")) {
    storestring(f.read_word(),after);
  } else {
    Fl_Type::read_property(f, c);
  }
}

/**
 Open the declblock_panel to edit this node.
 */
void Fl_DeclBlock_Type::open() {
  // build dialog box
  if (!declblock_panel) make_declblock_panel();
  // preset all values
  declblock_before_input->value(name());
  declblock_after_input->value(after);
  declblock_static_header->value(write_map_ & STATIC_IN_HEADER);
  declblock_static_source->value(write_map_ & STATIC_IN_SOURCE);
  declblock_code_header->value(write_map_ & CODE_IN_HEADER);
  declblock_code_source->value(write_map_ & CODE_IN_SOURCE);
  const char *c = comment();
  declblock_comment_input->buffer()->text(c?c:"");
  // show modal dialog and loop until satisfied
  declblock_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == declblock_panel_cancel) goto BREAK2;
      else if (w == declblock_panel_ok) break;
      else if (!w) Fl::wait();
    }
    // verify user input
    const char* a = declblock_before_input->value();
    while (isspace(*a)) a++;
    const char* b = declblock_after_input->value();
    while (isspace(*b)) b++;
    message = c_check(a&&a[0]=='#' ? a+1 : a);
    if (!message)
      message = c_check(b&&b[0]=='#' ? b+1 : b);
    if (message) {
      int v = fl_choice("Potential syntax error detected: %s",
                        "Continue Editing", "Ignore Error", NULL, message);
      if (v==0) continue;     // Continue Editing
      //if (v==1) { }         // Ignore Error and close dialog
    }
    // store user choices in data structure
    name(a);
    storestring(b, after);
    if (write_map_ & STATIC_IN_HEADER) {
      if (declblock_static_header->value()==0) {
        write_map_ &= ~STATIC_IN_HEADER;
        set_modflag(1);
      }
    } else {
      if (declblock_static_header->value()) {
        write_map_ |= STATIC_IN_HEADER;
        set_modflag(1);
      }
    }
    if (write_map_ & STATIC_IN_SOURCE) {
      if (declblock_static_source->value()==0) {
        write_map_ &= ~STATIC_IN_SOURCE;
        set_modflag(1);
      }
    } else {
      if (declblock_static_source->value()) {
        write_map_ |= STATIC_IN_SOURCE;
        set_modflag(1);
      }
    }
    if (write_map_ & CODE_IN_HEADER) {
      if (declblock_code_header->value()==0) {
        write_map_ &= ~CODE_IN_HEADER;
        set_modflag(1);
      }
    } else {
      if (declblock_code_header->value()) {
        write_map_ |= CODE_IN_HEADER;
        set_modflag(1);
      }
    }
    if (write_map_ & CODE_IN_SOURCE) {
      if (declblock_code_source->value()==0) {
        write_map_ &= ~CODE_IN_SOURCE;
        set_modflag(1);
      }
    } else {
      if (declblock_code_source->value()) {
        write_map_ |= CODE_IN_SOURCE;
        set_modflag(1);
      }
    }
    c = declblock_comment_input->buffer()->text();
    if (c && *c) {
      if (!comment() || strcmp(c, comment())) { set_modflag(1); redraw_browser(); }
      comment(c);
    } else {
      if (comment()) { set_modflag(1); redraw_browser(); }
      comment(0);
    }
    if (c) free((void*)c);
    break;
  }
BREAK2:
  declblock_panel->hide();
}

/**
 Write the \b before static code to the source file, and to the header file if declared public.
 The before code is stored in the name() field.
 */
void Fl_DeclBlock_Type::write_static(Fd_Code_Writer& f) {
  const char* c = name();
  if (c && *c) {
    if (write_map_ & STATIC_IN_HEADER)
      f.write_h("%s\n", c);
    if (write_map_ & STATIC_IN_SOURCE)
      f.write_c("%s\n", c);
  }
}

/**
 Write the \b after static code to the source file, and to the header file if declared public.
 */
void Fl_DeclBlock_Type::write_static_after(Fd_Code_Writer& f) {
  const char* c = after;
  if (c && *c) {
    if (write_map_ & STATIC_IN_HEADER)
      f.write_h("%s\n", c);
    if (write_map_ & STATIC_IN_SOURCE)
      f.write_c("%s\n", c);
  }
}

/**
 Write the \b before code to the source file, and to the header file if declared public.
 The before code is stored in the name() field.
 */
void Fl_DeclBlock_Type::write_code1(Fd_Code_Writer& f) {
  const char* c = name();
  if (c && *c) {
    if (write_map_ & CODE_IN_HEADER)
      f.write_h("%s\n", c);
    if (write_map_ & CODE_IN_SOURCE)
      f.write_c("%s\n", c);
  }
}

/**
 Write the \b after code to the source file, and to the header file if declared public.
 */
void Fl_DeclBlock_Type::write_code2(Fd_Code_Writer& f) {
  const char* c = after;
  if (c && *c) {
    if (write_map_ & CODE_IN_HEADER)
      f.write_h("%s\n", c);
    if (write_map_ & CODE_IN_SOURCE)
      f.write_c("%s\n", c);
  }
}

// ---- Fl_Comment_Type declaration

/** \class Fl_Comment_Type
 Manage a comment node.

 The comment field takes one or more lines of ASCII text. If the text starts
 with a '/' and a '*', Fluid assumes that the text is already formatted. If not,
 every line will be preceded with "// ".
 */

/// Prototype for a comment node to be used by the factory.
Fl_Comment_Type Fl_Comment_type;

/**
 Constructor.
 */
Fl_Comment_Type::Fl_Comment_Type() :
  in_c_(1),
  in_h_(1),
  style_(0)
{ }

/**
 Make a new comment node.
 \param[in] strategy add after current or as last child
 \return new Comment node
 */
Fl_Type *Fl_Comment_Type::make(Strategy strategy) {
  Fl_Type *anchor = Fl_Type::current, *p = anchor;
  if (p && (strategy == kAddAfterCurrent)) p = p->parent;
  while (p && !p->is_code_block()) {
    anchor = p;
    strategy = kAddAfterCurrent;
    p = p->parent;
  }
  Fl_Comment_Type *o = new Fl_Comment_Type();
  o->in_c_ = 1;
  o->in_h_ = 1;
  o->style_ = 0;
  o->name("my comment");
  o->add(anchor, strategy);
  o->factory = this;
  return o;
}

/**
 Write respective properties.
  - "in_source"/"not_in_source" if the comment will be written to the source code
  - "in_header"/"not_in_header" if the comment will be written to the header file
 */
void Fl_Comment_Type::write_properties(Fd_Project_Writer &f) {
  Fl_Type::write_properties(f);
  if (in_c_) f.write_string("in_source"); else f.write_string("not_in_source");
  if (in_h_) f.write_string("in_header"); else f.write_string("not_in_header");
}

/**
 Read extra properties.
 */
void Fl_Comment_Type::read_property(Fd_Project_Reader &f, const char *c) {
  if (!strcmp(c,"in_source")) {
    in_c_ = 1;
  } else if (!strcmp(c,"not_in_source")) {
    in_c_ = 0;
  } else if (!strcmp(c,"in_header")) {
    in_h_ = 1;
  } else if (!strcmp(c,"not_in_header")) {
    in_h_ = 0;
  } else {
    Fl_Type::read_property(f, c);
  }
}

/**
 Load available preset comments.
 Fluid comes with GPL and LGPL preset for comments. Users can
 add their own presets which are stored per user in a separate
 preferences database.
 */
static void load_comments_preset(Fl_Preferences &menu) {
  static const char * const predefined_comment[] = {
    "GNU Public License v3/GPL Header",  "GNU Public License v3/GPL Footer",
    "GNU Public License v3/LGPL Header", "GNU Public License v3/LGPL Footer",
    "FLTK/Header" };
  int i, n;
  menu.get("n", n, -1);
  if (n == -1) menu.set("n", 5);
  menu.set("version", 10400);
  Fl_Preferences db(Fl_Preferences::USER_L, "fltk.org", "fluid_comments");
  for (i=0; i<5; i++) {
    menu.set(Fl_Preferences::Name(i), predefined_comment[i]);
    db.set(predefined_comment[i], comment_text[i]);
  }
}

/**
 Open the comment_panel to edit this node.
 */
void Fl_Comment_Type::open() {
  if (!comment_panel) make_comment_panel();
  const char *text = name();
  {
    int i=0, n=0, version = 0;
    Fl_Preferences menu(Fl_Preferences::USER_L, "fltk.org", "fluid_comments_menu");
    comment_predefined->clear();
    comment_predefined->add("_Edit/Add current comment...");
    comment_predefined->add("_Edit/Remove last selection...");
    menu.get("version", version, -1);
    if (version < 10400) load_comments_preset(menu);
    menu.get("n", n, 0);
    for (i=0;i<n;i++) {
      char *text;
      menu.get(Fl_Preferences::Name(i), text, "");
      comment_predefined->add(text);
      free(text);
    }
  }
  comment_input->buffer()->text( text ? text : "" );
  comment_in_source->value(in_c_);
  comment_in_header->value(in_h_);
  comment_panel->show();
  char itempath[FL_PATH_MAX]; itempath[0] = 0;
  int last_selected_item = 0;
  for (;;) { // repeat as long as there are errors
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == comment_panel_cancel) goto BREAK2;
      else if (w == comment_panel_ok) break;
      else if (w == comment_predefined) {
        if (comment_predefined->value()==1) {
          // add the current comment to the database
          const char *xname = fl_input(
                                       "Please enter a name to reference the current\ncomment in your database.\n\n"
                                       "Use forward slashes '/' to create submenus.",
                                       "My Comment");
          if (xname) {
            char *name = fl_strdup(xname);
            for (char*s=name;*s;s++) if (*s==':') *s = ';';
            int n;
            Fl_Preferences db(Fl_Preferences::USER_L, "fltk.org", "fluid_comments");
            db.set(name, comment_input->buffer()->text());
            Fl_Preferences menu(Fl_Preferences::USER_L, "fltk.org", "fluid_comments_menu");
            menu.get("n", n, 0);
            menu.set(Fl_Preferences::Name(n), name);
            menu.set("n", ++n);
            comment_predefined->add(name);
            free(name);
          }
        } else if (comment_predefined->value()==2) {
          // remove the last selected comment from the database
          if (itempath[0]==0 || last_selected_item==0) {
            fl_message("Please select an entry from this menu first.");
          } else if (fl_choice("Are you sure that you want to delete the entry\n"
                               "\"%s\"\nfrom the database?", "Cancel", "Delete",
                               NULL, itempath)) {
            Fl_Preferences db(Fl_Preferences::USER_L, "fltk.org", "fluid_comments");
            db.deleteEntry(itempath);
            comment_predefined->remove(last_selected_item);
            Fl_Preferences menu(Fl_Preferences::USER_L, "fltk.org", "fluid_comments_menu");
            int i, n;
            for (i=4, n=0; i<comment_predefined->size(); i++) {
              const Fl_Menu_Item *mi = comment_predefined->menu()+i;
              if (comment_predefined->item_pathname(itempath, 255, mi)==0) {
                if (itempath[0]=='/') memmove(itempath, itempath+1, 255);
                if (itempath[0]) menu.set(Fl_Preferences::Name(n++), itempath);
              }
            }
            menu.set("n", n);
          }
        } else {
          // load the selected comment from the database
          if (comment_predefined->item_pathname(itempath, 255)==0) {
            if (itempath[0]=='/') memmove(itempath, itempath+1, 255);
            Fl_Preferences db(Fl_Preferences::USER_L, "fltk.org", "fluid_comments");
            char *text;
            db.get(itempath, text, "(no text found in data base)");
            comment_input->buffer()->text(text);
            free(text);
            last_selected_item = comment_predefined->value();
          }
        }
      }
      else if (w == comment_load) {
        // load a comment from disk
        fl_file_chooser_ok_label("Use File");
        const char *fname = fl_file_chooser("Pick a comment", 0L, 0L);
        fl_file_chooser_ok_label(NULL);
        if (fname) {
          if (comment_input->buffer()->loadfile(fname)) {
            fl_alert("Error loading file\n%s", fname);
          }
        }
      }
      else if (!w) Fl::wait();
    }
    char*c = comment_input->buffer()->text();
    name(c);
    free(c);
    int mod = 0;
    if (in_c_ != comment_in_source->value()) {
      in_c_ = comment_in_source->value();
      mod = 1;
    }
    if (in_h_ != comment_in_header->value()) {
      in_h_ = comment_in_header->value();
      mod = 1;
    }
    if (mod) set_modflag(1);
    break;
  }
BREAK2:
  comment_panel->hide();
}

/**
 Write the comment to the files.
 */
void Fl_Comment_Type::write_code1(Fd_Code_Writer& f) {
  const char* c = name();
  if (!c) return;
  if (!in_c_ && !in_h_) return;
  // find out if there is already a valid comment:
  const char *s = c;
  while (isspace(*s)) s++;
  // if this seems to be a C style comment, copy the block as is
  // (it's up to the user to correctly close the comment)
  if (s[0]=='/' && s[1]=='*') {
    if (in_h_) f.write_h("%s\n", c);
    if (in_c_) f.write_c("%s\n", c);
    return;
  }
  // copy the comment line by line, add the double slash if needed
  char *txt = fl_strdup(c);
  char *b = txt, *e = txt;
  for (;;) {
    // find the end of the line and set it to NUL
    while (*e && *e!='\n') e++;
    char eol = *e;
    *e = 0;
    // check if there is a C++ style comment at the beginning of the line
    char *s = b;
    while (isspace(*s)) s++;
    if (s!=e && ( s[0]!='/' || s[1]!='/') ) {
      // if no comment marker was found, we add one ourselves
      if (in_h_) f.write_h("// ");
      if (in_c_) f.write_c("// ");
    }
    // now copy the rest of the line
    if (in_h_) f.write_h("%s\n", b);
    if (in_c_) f.write_c("%s\n", b);
    if (eol==0) break;
    *e++ = eol;
    b = e;
  }
  free(txt);
}

// ---- Fl_Class_Type declaration

/** \class Fl_Class_Type
 Manage a class declaration and implementation.
 */

/// Prototype for a class node to be used by the factory.
Fl_Class_Type Fl_Class_type;

/**
 Constructor.
 */
Fl_Class_Type::Fl_Class_Type() :
  Fl_Type(),
  subclass_of(NULL),
  public_(1),
  class_prefix(NULL)
{ }

/**
 Destructor.
 */
Fl_Class_Type::~Fl_Class_Type() {
  if (subclass_of)
    free((void*)subclass_of);
  if (class_prefix)
    free((void*)class_prefix);
}

/**
 Return 1 if this class is marked public.
 */
int Fl_Class_Type::is_public() const {
  return public_;
}

/**
 Set the prefixx string.
 */
void Fl_Class_Type::prefix(const char*p) {
  free((void*) class_prefix);
  class_prefix=fl_strdup(p ? p : "" );
}

/**
 Make a new class node.
 \param[in] strategy add after current or as last child
 \return new Class node
 */
Fl_Type *Fl_Class_Type::make(Strategy strategy) {
  Fl_Type *anchor = Fl_Type::current, *p = anchor;
  if (p && (strategy == kAddAfterCurrent)) p = p->parent;
  while (p && !p->is_decl_block()) {
    anchor = p;
    strategy = kAddAfterCurrent;
    p = p->parent;
  }
  Fl_Class_Type *o = new Fl_Class_Type();
  o->name("UserInterface");
  o->class_prefix = NULL;
  o->subclass_of = NULL;
  o->public_ = 1;
  o->add(anchor, strategy);
  o->factory = this;
  return o;
}

/**
 Write the respective properties.
  - ":" followed by the super class
  - "private"/"protected"
 */
void Fl_Class_Type::write_properties(Fd_Project_Writer &f) {
  Fl_Type::write_properties(f);
  if (subclass_of) {
    f.write_string(":");
    f.write_word(subclass_of);
  }
  switch (public_) {
    case 0: f.write_string("private"); break;
    case 2: f.write_string("protected"); break;
  }
}

/**
 Read additional properties.
 */
void Fl_Class_Type::read_property(Fd_Project_Reader &f, const char *c) {
  if (!strcmp(c,"private")) {
    public_ = 0;
  } else if (!strcmp(c,"protected")) {
    public_ = 2;
  } else if (!strcmp(c,":")) {
    storestring(f.read_word(), subclass_of);
  } else {
    Fl_Type::read_property(f, c);
  }
}

/**
 Open the class_panel to edit the class name and superclass name.
 */
void Fl_Class_Type::open() {
  if (!class_panel) make_class_panel();
  char fullname[FL_PATH_MAX]="";
  if (prefix() && strlen(prefix()))
    sprintf(fullname,"%s %s",prefix(),name());
  else
    strcpy(fullname, name());
  c_name_input->value(fullname);
  c_subclass_input->value(subclass_of);
  c_public_button->value(public_);
  const char *c = comment();
  c_comment_input->buffer()->text(c?c:"");
  class_panel->show();
  const char* message = 0;

  char *na=0,*pr=0,*p=0; // name and prefix substrings

  for (;;) { // repeat as long as there are errors
    // we don;t give the option to ignore this error here because code depends
    // on this being a C++ identifier
    if (message) fl_alert("%s", message);
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == c_panel_cancel) goto BREAK2;
      else if (w == c_panel_ok) break;
      else if (!w) Fl::wait();
    }
    const char*c = c_name_input->value();
    char *s = fl_strdup(c);
    size_t len = strlen(s);
    if (!*s) goto OOPS;
    p = (char*) (s+len-1);
    while (p>=s && isspace(*p)) *(p--)='\0';
    if (p<s) goto OOPS;
    while (p>=s && is_id(*p)) p--;
    if ( (p<s && !is_id(*(p+1))) || !*(p+1) ) {
    OOPS: message = "class name must be C++ identifier";
      free((void*)s);
      continue;
    }
    na=p+1; // now we have the name
    if(p>s) *p--='\0';
    while (p>=s && isspace(*p)) *(p--)='\0';
    while (p>=s && is_id(*p))   p--;
    if (p<s)                    p++;
    if (is_id(*p) && p<na)      pr=p; // prefix detected
    c = c_subclass_input->value();
    message = c_check(c);
    if (message) { free((void*)s);continue;}
    name(na);
    prefix(pr);
    free((void*)s);
    storestring(c, subclass_of);
    if (public_ != c_public_button->value()) {
      public_ = c_public_button->value();
      set_modflag(1);
    }
    c = c_comment_input->buffer()->text();
    if (c && *c) {
      if (!comment() || strcmp(c, comment()))  { set_modflag(1); redraw_browser(); }
      comment(c);
    } else {
      if (comment())  { set_modflag(1); redraw_browser(); }
      comment(0);
    }
    if (c) free((void*)c);
    break;
  }
BREAK2:
  class_panel->hide();
}

/**
 Write the header code that declares this class.
 */
void Fl_Class_Type::write_code1(Fd_Code_Writer& f) {
  parent_class = current_class;
  current_class = this;
  write_public_state = 0;
  f.write_h("\n");
  write_comment_h(f);
  if (prefix() && strlen(prefix()))
    f.write_h("class %s %s ", prefix(), name());
  else
    f.write_h("class %s ", name());
  if (subclass_of) f.write_h(": %s ", subclass_of);
  f.write_h("{\n");
}

/**
 Write the header code that ends the declaration of this class.
 */
void Fl_Class_Type::write_code2(Fd_Code_Writer& f) {
  f.write_h("};\n");
  current_class = parent_class;
}

/**
 Return 1 if this class contains a function with the given signature.
 */
int Fl_Type::has_function(const char *rtype, const char *sig) const {
  Fl_Type *child;
  for (child = next; child && child->level > level; child = child->next) {
    if (child->level == level+1 && child->is_a(ID_Function)) {
      const Fl_Function_Type *fn = (const Fl_Function_Type*)child;
      if (fn->has_signature(rtype, sig))
        return 1;
    }
  }
  return 0;
}
