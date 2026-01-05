//
// C function Node code for the Fast Light Tool Kit (FLTK).
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

#include "nodes/Function_Node.h"

#include "Fluid.h"
#include "proj/mergeback.h"
#include "proj/undo.h"
#include "io/Project_Reader.h"
#include "io/Project_Writer.h"
#include "io/Code_Writer.h"
#include "nodes/Window_Node.h"
#include "nodes/Group_Node.h"
#include "panels/function_panel.h"
#include "rsrcs/comments.h"
#include "widgets/Node_Browser.h"

#include <FL/fl_string_functions.h>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_ask.H>
#include "../src/flstring.h"

#include <zlib.h>

extern void open_panel();

using namespace fld;
using namespace fld::io;
using namespace fld::proj;

/// Set a current class, so that the code of the children is generated correctly.
Class_Node *current_class = nullptr;

/**
 \brief Return 1 if the list contains a function with the given signature at the top level.
 Widget_Node uses this to check if a callback by a certain signature is
 already defined by the user within this file. If not, Widget_Node will
 generate an `extern $sig$;` statement.
 \param[in] rtype return type, can be nullptr to avoid checking (not used by Widget_Node)
 \param[in] sig function signature
 \return 1 if found.
 */
int has_toplevel_function(const char *rtype, const char *sig) {
  Node *child;
  for (child = Fluid.proj.tree.first; child; child = child->next) {
    if (!child->is_in_class() && child->is_a(Type::Function)) {
      const Function_Node *fn = (const Function_Node*)child;
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
 \return nullptr if the character was found, else a pointer to a static string
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
      if (*(c-1) == type) return nullptr;
  }
}

/**
 Check normal code, match brackets and parenthesis.
 Recursively run a line of code and make sure that
 {, [, ", ', and ( are matched.
 \param[inout] c start searching here, return the end of the search
 \param[in] type find this character match
 \return nullptr if the character was found, else a pointer to a static string
    with an error message
 */
const char *_c_check(const char * & c, int type) {
  const char *d;
  for (;;) switch (*c++) {
    case 0:
      if (!type) return nullptr;
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
//      // Matt: C++ does allow {} inside () now
//      if (type==')') goto UNEXPECTED;
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
//    UNEXPECTED:
      if (type == *(c-1)) return nullptr;
      sprintf(buffer, "unexpected '%c'", *(c-1));
      return buffer;
  }
}

/**
 Check legality of c code (sort of) and return error:
 Make sure that {, ", ', and ( are matched.
 \param[in] c start searching here
 \param[in] type find this character match (default is 0)
 \return nullptr if the character was found, else a pointer to a static string
    with an error message
 \note This function checks every conceivable line of code, which is not
    always wanted. It can't differentiate characters in comments, and the
    user may well intend to leave a curly bracket open
    (i.e. namespace { ... } ). We should make this option user selectable.
 */
const char *c_check(const char *c, int type) {
  return _c_check(c,type);
}

// ---- Function_Node implementation

/** \class Function_Node
 Manage a C++ function node in the Fluid design.

 A function can have a signature (name followed by arguments), a return type
 and a comment section. If can be local or global, and it can be declared a C
 or C++ function.
 */

/// Prototype for a function to be used by the factory.
Function_Node Function_Node::prototype;

/**
 Create a new function for the widget tree.
 \param[in] strategy add new function after current or as last child
 \return the new node
 */
Node *Function_Node::make(Strategy strategy) {
  Node *anchor = Fluid.proj.tree.current, *p = anchor;
  if (p && (strategy.placement() == Strategy::AFTER_CURRENT))
    p = p->parent;
  while (p && !p->is_decl_block()) {
    anchor = p;
    strategy.placement(Strategy::AFTER_CURRENT);
    p = p->parent;
  }
  Function_Node *o = new Function_Node();
  o->name("make_window()");
  o->return_type_.clear();
  o->add(anchor, strategy);
  o->factory = this;
  o->public_ = 1;
  o->declare_c_ = 0;
  return o;
}

/**
 Write function specific properties to an .fl file.
  - "private"/"public" indicates the state of the function
  - "C" is written if we want a C signature instead of C++
  - "return_type" is followed by the return type of the function
 */
void Function_Node::write_properties(fld::io::Project_Writer &f) {
  Node::write_properties(f);
  switch (public_) {
    case 0: f.write_string("private"); break;
    case 2: f.write_string("protected"); break;
  }
  if (declare_c_) f.write_string("C");
  if (!return_type().empty()) {
    f.write_string("return_type");
    f.write_word(return_type().c_str());
  }
}

/**
 Read function specific properties fron an .fl file.
 \param[in] c read from this string
 */
void Function_Node::read_property(fld::io::Project_Reader &f, const char *c) {
  if (!strcmp(c,"private")) {
    public_ = 0;
  } else if (!strcmp(c,"protected")) {
    public_ = 2;
  } else if (!strcmp(c,"C")) {
    declare_c_ = 1;
  } else if (!strcmp(c,"return_type")) {
    return_type(f.read_word());
  } else {
    Node::read_property(f, c);
  }
}

/**
 Open the function_panel dialog box to edit this function.
 */
void Function_Node::open() {
  open_panel();
}

/**
 Return 1 if the function is global.
 \return 1 if public, 0 if local.
 */
int Function_Node::is_public() const {
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
 \see write_code2(fld::io::Code_Writer& f)
 */
void Function_Node::write_code1(fld::io::Code_Writer& f) {
  constructor=0;
  havewidgets = 0;
  Node *child;
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
    std::string rtype = return_type();
    std::string star = "";
    // from matt: let the user type "static " at the start of type
    // in order to declare a static method;
    int is_static = 0;
    int is_virtual = 0;
    if (!rtype.empty()) {
      if (rtype == "static") {
        is_static = 1;
        rtype.clear();
      } else if (rtype.compare(0, 7, "static ")==0) {
        is_static = 1;
        rtype.erase(0, 7);
      }
    }
    if (!rtype.empty()) {
      if (rtype == "virtual") {
        is_virtual = 1;
        rtype.clear();
      } else if (rtype.compare(0, 8, "virtual ")==0) {
        is_virtual = 1;
        rtype.erase(0, 8);
      }
    }
    if (rtype.empty()) {
      if (havewidgets) {
        rtype = subclassname(child);
        star = "*";
      } else {
        rtype = "void";
      }
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
        f.write_h("%s%s ", rtype.c_str(), star.c_str());
        if (havechildren)
          f.write_c("%s%s ", rtype.c_str(), star.c_str());
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
        if (declare_c_)
          f.write_h("extern \"C\" { %s%s %s; }\n", rtype.c_str(), star.c_str(), name());
        else
          f.write_h("%s%s %s;\n", rtype.c_str(), star.c_str(), name());
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
        f.write_c("%s%s %s {\n", rtype.c_str(), star.c_str(), s);
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
 \see write_code1(fld::io::Code_Writer& f)
 */
void Function_Node::write_code2(fld::io::Code_Writer& f) {
  Node *child;
  const char *var = "w";
  char havechildren = 0;
  for (child = next; child && child->level > level; child = child->next) {
    havechildren = 1;
    if (child->is_a(Type::Window) && child->name()) var = child->name();
  }

  if (ismain()) {
    if (havewidgets)
      f.write_c("%s%s->show(argc, argv);\n", f.indent(1), var);
    if (havechildren)
      f.write_c("%sreturn Fl::run();\n", f.indent(1));
  } else if (havewidgets && !constructor && return_type().empty()) {
    f.write_c("%sreturn %s;\n", f.indent(1), var);
  }
  if (havechildren)
    f.write_c("}\n");
  f.indentation = 0;
}

/**
 Check if the return type and signatures match.
 \param[in] rtype function return type
 \param[in] sig function name followed by arguments
 \return 1 if they match, 0 if not
 */
int Function_Node::has_signature(const char *rtype, const char *sig) const {
  if (rtype && return_type().empty())
    return 0;
  if (!name())
    return 0;
  if ( (rtype==nullptr || (return_type() == rtype)) && fl_filename_match(name(), sig)) {
    return 1;
  }
  return 0;
}

// ---- Code_Node declaration

/** \class Code_Node
 Manage a block of C++ code in the Fluid design.

 This node manages an arbitrary block of code inside a function that will
 be written into the source code file. Fl_Code_Block has no comment field.
 However, the first line of code will be shown in the widget browser.
 */

/// Prototype for code to be used by the factory.
Code_Node Code_Node::prototype;

/**
 Make a new code node.
 If the parent node is not a function, a message box will pop up and
 the request will be ignored.
 \param[in] strategy add code after current or as last child
 \return new Code node
 */
Node *Code_Node::make(Strategy strategy) {
  Node *anchor = Fluid.proj.tree.current, *p = anchor;
  if (p && (strategy.placement() == Strategy::AFTER_CURRENT))
    p = p->parent;
  while (p && !p->is_code_block()) {
    anchor = p;
    strategy.placement(Strategy::AFTER_CURRENT);
    p = p->parent;
  }
  if (!p) {
    fl_message("Please select a function");
    return nullptr;
  }
  Code_Node *o = new Code_Node();
  o->name("printf(\"Hello, World!\\n\");");
  o->add(anchor, strategy);
  o->factory = this;
  return o;
}

/**
 Open the code_panel or an external editor to edit this code section.
 */
void Code_Node::open() {
  // Using an external code editor? Open it..
  if ( Fluid.use_external_editor && Fluid.external_editor_command[0] ) {
    const char *cmd = Fluid.external_editor_command;
    const char *code = name();
    if (!code) code = "";
    if ( editor_.open_editor(cmd, code) == 0 )
      return;   // return if editor opened ok, fall thru to built-in if not
  }
  open_panel();
}

/**
 Grab changes from an external editor and write this node.
 */
void Code_Node::write(fld::io::Project_Writer &f) {
  // External editor changes? If so, load changes into ram, update mtime/size
  if ( handle_editor_changes() == 1 ) {
    Fluid.main_window->redraw();    // tell fluid to redraw; edits may affect tree's contents
  }
  Node::write(f);
}

/**
 Write the code block with the correct indentation.
 */
void Code_Node::write_code1(fld::io::Code_Writer& f) {
  // External editor changes? If so, load changes into ram, update mtime/size
  if ( handle_editor_changes() == 1 ) {
    Fluid.main_window->redraw();    // tell fluid to redraw; edits may affect tree's contents
  }
  f.tag(Mergeback::Tag::GENERIC, Mergeback::Tag::CODE, 0);
  f.write_c_indented(name(), 0, '\n');
  f.tag(Mergeback::Tag::CODE, Mergeback::Tag::GENERIC, get_uid());
}

/**
 See if external editor is open.
 */
int Code_Node::is_editing() {
  return editor_.is_editing();
}

/**
 Reap the editor's pid
 \return -2: editor not open
 \return -1: wait failed
 \return 0: process still running
 \return \>0: process finished + reaped (returns pid)
 */
int Code_Node::reap_editor() {
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
int Code_Node::handle_editor_changes() {
  const char *newcode = nullptr;
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

// ---- CodeBlock_Node implementation

/** \class CodeBlock_Node
 Manage two blocks of C++ code enclosing its children.

 This node manages two lines of code that enclose all children
 of this node. This is usually an if..then clause.

 \todo this node could support multiple lines of code for each block.
 */

/// Prototype for a block of code to be used by the factory.
CodeBlock_Node CodeBlock_Node::prototype;

/**
 Make a new code block.
 If the parent node is not a function or another codeblock, a message box will
 pop up and the request will be ignored.
 \param[in] strategy add after current or as last child
 \return new CodeBlock
 */
Node *CodeBlock_Node::make(Strategy strategy) {
  Node *anchor = Fluid.proj.tree.current, *p = anchor;
  if (p && (strategy.placement() == Strategy::AFTER_CURRENT))
    p = p->parent;
  while (p && !p->is_code_block()) {
    anchor = p;
    strategy.placement(Strategy::AFTER_CURRENT);
    p = p->parent;
  }
  if (!p) {
    fl_message("Please select a function");
    return nullptr;
  }
  CodeBlock_Node *o = new CodeBlock_Node();
  o->name("if (test())");
  o->end_code_.clear();
  o->add(anchor, strategy);
  o->factory = this;
  return o;
}

/**
 Write the specific properties for this node.
  - "after" is followed by the code that comes after the children
 The "before" code is stored in the name() field.
 */
void CodeBlock_Node::write_properties(fld::io::Project_Writer &f) {
  Node::write_properties(f);
  if (!end_code().empty()) {
    f.write_string("after");
    f.write_word(end_code().c_str());
  }
}

/**
 Read the node specific properties.
 */
void CodeBlock_Node::read_property(fld::io::Project_Reader &f, const char *c) {
  if (!strcmp(c,"after")) {
    end_code(f.read_word());
  } else {
    Node::read_property(f, c);
  }
}

/**
 Open the codeblock_panel.
 */
void CodeBlock_Node::open() {
  open_panel();
}

/**
 Write the "before" code.
 */
void CodeBlock_Node::write_code1(fld::io::Code_Writer& f) {
  const char* c = name();
  f.write_c("%s%s {\n", f.indent(), c ? c : "");
  f.indentation++;
}

/**
 Write the "after" code.
 */
void CodeBlock_Node::write_code2(fld::io::Code_Writer& f) {
  f.indentation--;
  if (!end_code().empty())
    f.write_c("%s} %s\n", f.indent(), end_code().c_str());
  else
    f.write_c("%s}\n", f.indent());
}

// ---- Decl_Node declaration

/** \class Decl_Node
 Manage the C/C++ declaration of a variable.

 This node manages a single line of code that can be in the header or the source
 code, and can be made static.

 \todo this node could support multiple lines.
 */

/// Prototype for a declaration to be used by the factory.
Decl_Node Decl_Node::prototype;

/**
 Return 1 if this declaration and its parents are public.
 */
int Decl_Node::is_public() const
{
  Node *p = parent;
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
Node *Decl_Node::make(Strategy strategy) {
  Node *anchor = Fluid.proj.tree.current, *p = anchor;
  if (p && (strategy.placement() == Strategy::AFTER_CURRENT))
    p = p->parent;
  while (p && !p->is_decl_block()) {
    anchor = p;
    strategy.placement(Strategy::AFTER_CURRENT);
    p = p->parent;
  }
  Decl_Node *o = new Decl_Node();
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
void Decl_Node::write_properties(fld::io::Project_Writer &f) {
  Node::write_properties(f);
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
void Decl_Node::read_property(fld::io::Project_Reader &f, const char *c) {
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
    Node::read_property(f, c);
  }
}

/**
 Open the decl_panel to edit this node.
 */
void Decl_Node::open() {
  open_panel();
}

/**
 Write the code to the source and header files.
 \todo There are a lot of side effect in this node depending on the given text
    and the parent node. They need to be understood and documented.
 */
void Decl_Node::write_code1(fld::io::Code_Writer& f) {
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

// ---- Data_Node declaration

/** \class Data_Node
 Manage data from an external arbitrary file.

 The content of the file will be stored in binary inside the generated
 code. This can be used to store images inline in the source code,
 */

/// Prototype for a data node to be used by the factory.
Data_Node Data_Node::prototype;

/**
 Create an empty inline data node.
 \param[in] strategy add after current or as last child
 \return new inline data node
 */
Node *Data_Node::make(Strategy strategy) {
  Node *anchor = Fluid.proj.tree.current, *p = anchor;
  if (p && (strategy.placement() == Strategy::AFTER_CURRENT))
    p = p->parent;
  while (p && !p->is_decl_block()) {
    anchor = p;
    strategy.placement(Strategy::AFTER_CURRENT);
    p = p->parent;
  }
  Data_Node *o = new Data_Node();
  o->public_ = 1;
  o->static_ = 1;
  o->filename_.clear();
  o->output_format_ = 0;
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
void Data_Node::write_properties(fld::io::Project_Writer &f) {
  Decl_Node::write_properties(f);
  if (!filename().empty()) {
    f.write_string("filename");
    f.write_word(filename().c_str());
  }
  switch (output_format_) {
    case 1: f.write_string("textmode"); break;
    case 2: f.write_string("compressed"); break;
    case 3: f.write_string("std_binary"); break;
    case 4: f.write_string("std_textmode"); break;
    case 5: f.write_string("std_compressed"); break;
  }
}

/**
 Read specific properties.
 */
void Data_Node::read_property(fld::io::Project_Reader &f, const char *c) {
  if (!strcmp(c,"filename")) {
    storestring(f.read_word(), filename_, 1);
  } else if (!strcmp(c,"textmode")) {
    output_format_ = 1;
  } else if (!strcmp(c,"compressed")) {
    output_format_ = 2;
  } else if (!strcmp(c,"std_binary")) {
    output_format_ = 3;
  } else if (!strcmp(c,"std_textmode")) {
    output_format_ = 4;
  } else if (!strcmp(c,"std_compressed")) {
    output_format_ = 5;
  } else {
    Decl_Node::read_property(f, c);
  }
}

/**
 Open the data_panel to edit this node.
 */
void Data_Node::open() {
  open_panel();
}

/**
 Write the content of the external file inline into the source code.
 */
void Data_Node::write_code1(fld::io::Code_Writer& f) {
  const char *message = nullptr;
  const char *c = name();
  if (!c) return;
  std::string fn = filename();
  char *data = nullptr;
  int nData = -1;
  int uncompressedDataSize = 0;
  // path should be set correctly already
  if (!filename().empty() && !f.write_codeview) {
    Fluid.proj.enter_project_dir();
    FILE *f = fl_fopen(filename().c_str(), "rb");
    Fluid.proj.leave_project_dir();
    if (!f) {
      message = "Can't include data from file. Can't open";
    } else {
      fseek(f, 0, SEEK_END);
      nData = (int)ftell(f);
      fseek(f, 0, SEEK_SET);
      if (nData) {
        data = (char*)calloc(nData, 1);
        if (fread(data, nData, 1, f)==0) { /* use default */ }
        if ((output_format_ == 2) || (output_format_ == 5)) {
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
    if (filename().empty())
      fn = "<no filename>";
  }
  if (is_in_class()) {
    f.write_public(public_);
    if ((output_format_ == 1) || (output_format_ == 4)) {
      f.write_c("\n");
      write_comment_c(f);
      if (output_format_ == 1) {
        f.write_h("%sstatic const char *%s;\n", f.indent(1), c);
        f.write_c("const char *%s::%s = /* text inlined from %s */\n", class_name(1), c, fn.c_str());
      } else {
        f.write_h_once("#include <string>");
        f.write_h("%sstatic const std::string %s;\n", f.indent(1), c);
        f.write_c("const std::string %s::%s = /* text inlined from %s */\n", class_name(1), c, fn.c_str());
      }
      if (message) f.write_c("#error %s %s\n", message, fn.c_str());
      f.write_cstring(data, nData);
    } else if ((output_format_ == 2) || (output_format_ == 5)) {
      f.write_h("%sstatic int %s_size;\n", f.indent(1), c);
      f.write_c("\n");
      write_comment_c(f);
      f.write_c("int %s::%s_size = %d;\n", class_name(1), c, uncompressedDataSize);
      if (output_format_ == 2) {
        f.write_h("%sstatic unsigned char %s[%d];\n", f.indent(1), c, nData);
        f.write_c("unsigned char %s::%s[%d] = /* data compressed and inlined from %s */\n", class_name(1), c, nData, fn.c_str());
      } else {
        f.write_h_once("#include <stdint.h>");
        f.write_h_once("#include <vector>");
        f.write_h("%sstatic std::vector<uint8_t> %s;\n", f.indent(1), c);
        f.write_c("std::vector<uint8_t> %s::%s = /* data compressed and inlined from %s */\n", class_name(1), c, fn.c_str());
      }
      if (message) f.write_c("#error %s %s\n", message, fn.c_str());
      f.write_cdata(data, nData);
    } else {
      f.write_c("\n");
      write_comment_c(f);
      if (output_format_ == 0) {
        f.write_h("%sstatic unsigned char %s[%d];\n", f.indent(1), c, nData);
        f.write_c("unsigned char %s::%s[%d] = /* data inlined from %s */\n", class_name(1), c, nData, fn.c_str());
      } else {
        f.write_h_once("#include <stdint.h>");
        f.write_h_once("#include <vector>");
        f.write_h("%sstatic std::vector<uint8_t> %s;\n", f.indent(1), c);
        f.write_c("std::vector<uint8_t> %s::%s = /* data inlined from %s */\n", class_name(1), c, fn.c_str());
      }
      if (message) f.write_c("#error %s %s\n", message, fn.c_str());
      f.write_cdata(data, nData);
    }
    f.write_c(";\n");
  } else {
    // the "header only" option does not apply here!
    if (public_) {
      if (static_) {
        if ((output_format_ == 1) || (output_format_ == 4)) {
          f.write_c("\n");
          write_comment_c(f);
          if (output_format_ == 1) {
            f.write_h("extern const char *%s;\n", c);
            f.write_c("const char *%s = /* text inlined from %s */\n", c, fn.c_str());
          } else {
            f.write_h_once("#include <string>");
            f.write_h("extern const std::string %s;\n", c);
            f.write_c("const std::string %s = /* text inlined from %s */\n", c, fn.c_str());
          }
          if (message) f.write_c("#error %s %s\n", message, fn.c_str());
          f.write_cstring(data, nData);
        } else if ((output_format_ == 2) || (output_format_ == 5)) {
          f.write_h("extern int %s_size;\n", c);
          f.write_c("\n");
          write_comment_c(f);
          f.write_c("int %s_size = %d;\n", c, uncompressedDataSize);
          if (output_format_ == 2) {
            f.write_h("extern unsigned char %s[%d];\n", c, nData);
            f.write_c("unsigned char %s[%d] = /* data compressed and inlined from %s */\n", c, nData, fn.c_str());
          } else {
            f.write_h_once("#include <stdint.h>");
            f.write_h_once("#include <vector>");
            f.write_h("extern std::vector<uint8_t> %s;\n", c);
            f.write_c("std::vector<uint8_t> %s = /* data compressed and inlined from %s */\n", c, fn.c_str());
          }
          if (message) f.write_c("#error %s %s\n", message, fn.c_str());
          f.write_cdata(data, nData);
        } else {
          f.write_c("\n");
          write_comment_c(f);
          if (output_format_ == 0) {
            f.write_h("extern unsigned char %s[%d];\n", c, nData);
            f.write_c("unsigned char %s[%d] = /* data inlined from %s */\n", c, nData, fn.c_str());
          } else {
            f.write_h_once("#include <stdint.h>");
            f.write_h_once("#include <vector>");
            f.write_h("extern std::vector<uint8_t> %s;\n", c);
            f.write_c("std::vector<uint8_t> %s = /* data inlined from %s */\n", c, fn.c_str());
          }
          if (message) f.write_c("#error %s %s\n", message, fn.c_str());
          f.write_cdata(data, nData);
        }
        f.write_c(";\n");
      } else {
        write_comment_h(f);
        f.write_h("#error Unsupported declaration loading inline data %s\n", fn.c_str());
        if (output_format_ == 1)
          f.write_h("const char *%s = \"abc...\";\n", c);
        else
          f.write_h("unsigned char %s[3] = { 1, 2, 3 };\n", c);
      }
    } else {
      f.write_c("\n");
      write_comment_c(f);
      if ((output_format_ == 1) || (output_format_ == 4)) {
        if (output_format_ == 1) {
          if (static_) f.write_c("static ");
          f.write_c("const char *%s = /* text inlined from %s */\n", c, fn.c_str());
        } else {
          f.write_c_once("#include <string>");
          if (static_) f.write_c("static ");
          f.write_c("const std::string %s = /* text inlined from %s */\n", c, fn.c_str());
        }
        if (message) f.write_c("#error %s %s\n", message, fn.c_str());
        f.write_cstring(data, nData);
      } else if ((output_format_ == 2) || (output_format_ == 5)) {
        if (static_) f.write_c("static ");
        f.write_c("int %s_size = %d;\n", c, uncompressedDataSize);
        if (output_format_ == 2) {
          if (static_) f.write_c("static ");
          f.write_c("unsigned char %s[%d] = /* data compressed and inlined from %s */\n", c, nData, fn.c_str());
        } else {
          f.write_c_once("#include <stdint.h>");
          f.write_c_once("#include <vector>");
          if (static_) f.write_c("static ");
          f.write_c("std::vector<uint8_t> %s = /* data compressed and inlined from %s */\n", c, fn.c_str());
        }
        if (message) f.write_c("#error %s %s\n", message, fn.c_str());
        f.write_cdata(data, nData);
      } else {
        if (output_format_ == 0) {
          if (static_) f.write_c("static ");
          f.write_c("unsigned char %s[%d] = /* data inlined from %s */\n", c, nData, fn.c_str());
        } else {
          f.write_c_once("#include <stdint.h>");
          f.write_c_once("#include <vector>");
          if (static_) f.write_c("static ");
          f.write_c("std::vector<uint8_t> %s = /* data inlined from %s */\n", c, fn.c_str());
        }
        if (message) f.write_c("#error %s %s\n", message, fn.c_str());
        f.write_cdata(data, nData);
      }
      f.write_c(";\n");
    }
  }
  // if we are in interactive mode, we pop up a warning dialog
  // giving the error: (Fluid.batch_mode && !write_codeview) ???
  if (message && !f.write_codeview) {
    if (Fluid.batch_mode)
      fprintf(stderr, "FLUID ERROR: %s %s\n", message, fn.c_str());
    else
      fl_alert("%s\n%s\n", message, fn.c_str());
  }
  if (data) free(data);
}


// ---- DeclBlock_Node declaration

/** \class DeclBlock_Node
 Manage a declaration block.

 Declaration blocks have two text field that are written before and after
 the children of this block. This block is located at the top level and
 is written to the source file, and to the header file, if declared public.
 */

/// Prototype for a declaration block to be used by the factory.
DeclBlock_Node DeclBlock_Node::prototype;

/**
 Return 1 if this block is public.
 */
int DeclBlock_Node::is_public() const {
  return ((write_map_&CODE_IN_HEADER) != 0);
}

/**
 Create a new declaration block.
 \param[in] strategy add after current or as last child
 \return new Declaration Block node
 */
Node *DeclBlock_Node::make(Strategy strategy) {
  Node *anchor = Fluid.proj.tree.current, *p = anchor;
  if (p && (strategy.placement() == Strategy::AFTER_CURRENT)) p = p->parent;
  while (p && !p->is_decl_block()) {
    anchor = p;
    strategy.placement(Strategy::AFTER_CURRENT);
    p = p->parent;
  }
  DeclBlock_Node *o = new DeclBlock_Node();
  o->name("#if 1");
  o->write_map_ = CODE_IN_SOURCE;
  o->end_code_ = "#endif";
  o->add(anchor, strategy);
  o->factory = this;
  return o;
}

/**
 Write the specific properties.
  - "public"/"protected"
  - "after" followed by the second code block.
 */
void DeclBlock_Node::write_properties(fld::io::Project_Writer &f) {
  Node::write_properties(f);
  // deprecated
  if (is_public()) f.write_string("public");
  // new way to map declaration block to various parts of the generated code
  if (write_map_ != CODE_IN_SOURCE)
    f.write_string("map %d", write_map_);
  f.write_string("after");
  f.write_word(end_code().c_str());
}

/**
 Read the specific properties.
 */
void DeclBlock_Node::read_property(fld::io::Project_Reader &f, const char *c) {
  if(!strcmp(c,"public")) {
    write_map_ |= CODE_IN_HEADER;
  } else if(!strcmp(c,"protected")) {
    //
  } else if(!strcmp(c,"map")) {
    write_map_ = (int)atol(f.read_word());
  } else  if (!strcmp(c,"after")) {
    end_code(f.read_word());
  } else {
    Node::read_property(f, c);
  }
}

/**
 Open the declblock_panel to edit this node.
 */
void DeclBlock_Node::open() {
  open_panel();
}

/**
 Write the \b before static code to the source file, and to the header file if declared public.
 The before code is stored in the name() field.
 */
void DeclBlock_Node::write_static(fld::io::Code_Writer& f) {
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
void DeclBlock_Node::write_static_after(fld::io::Code_Writer& f) {
  if (!end_code().empty()) {
    if (write_map_ & STATIC_IN_HEADER)
      f.write_h("%s\n", end_code().c_str());
    if (write_map_ & STATIC_IN_SOURCE)
      f.write_c("%s\n", end_code().c_str());
  }
}

/**
 Write the \b before code to the source file, and to the header file if declared public.
 The before code is stored in the name() field.
 */
void DeclBlock_Node::write_code1(fld::io::Code_Writer& f) {
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
void DeclBlock_Node::write_code2(fld::io::Code_Writer& f) {
  if (!end_code().empty()) {
    if (write_map_ & CODE_IN_HEADER)
      f.write_h("%s\n", end_code().c_str());
    if (write_map_ & CODE_IN_SOURCE)
      f.write_c("%s\n", end_code().c_str());
  }
}

// ---- Comment_Node declaration

/** \class Comment_Node
 Manage a comment node.

 The comment field takes one or more lines of ASCII text. If the text starts
 with a '/' and a '*', Fluid assumes that the text is already formatted. If not,
 every line will be preceded with "// ".
 */

/// Prototype for a comment node to be used by the factory.
Comment_Node Comment_Node::prototype;

/**
 Make a new comment node.
 \param[in] strategy add after current or as last child
 \return new Comment node
 */
Node *Comment_Node::make(Strategy strategy) {
  Node *anchor = Fluid.proj.tree.current, *p = anchor;
  if (p && (strategy.placement() == Strategy::AFTER_CURRENT))
    p = p->parent;
  while (p && !p->is_code_block()) {
    anchor = p;
    strategy.placement(Strategy::AFTER_CURRENT);
    p = p->parent;
  }
  Comment_Node *o = new Comment_Node();
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
void Comment_Node::write_properties(fld::io::Project_Writer &f) {
  Node::write_properties(f);
  if (in_c_) f.write_string("in_source"); else f.write_string("not_in_source");
  if (in_h_) f.write_string("in_header"); else f.write_string("not_in_header");
}

/**
 Read extra properties.
 */
void Comment_Node::read_property(fld::io::Project_Reader &f, const char *c) {
  if (!strcmp(c,"in_source")) {
    in_c_ = 1;
  } else if (!strcmp(c,"not_in_source")) {
    in_c_ = 0;
  } else if (!strcmp(c,"in_header")) {
    in_h_ = 1;
  } else if (!strcmp(c,"not_in_header")) {
    in_h_ = 0;
  } else {
    Node::read_property(f, c);
  }
}

/**
 Load available preset comments.
 Fluid comes with GPL and LGPL preset for comments. Users can
 add their own presets which are stored per user in a separate
 preferences database.
 */
void load_comments_preset(Fl_Preferences &menu) {
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
void Comment_Node::open() {
  open_panel();
}

/**
 Write the comment to the files.
 */
void Comment_Node::write_code1(fld::io::Code_Writer& f) {
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

// ---- Class_Node declaration

/** \class Class_Node
 Manage a class declaration and implementation.
 */

/// Prototype for a class node to be used by the factory.
Class_Node Class_Node::prototype;

/**
 Return 1 if this class is marked public.
 */
int Class_Node::is_public() const {
  return public_;
}

/**
 Make a new class node.
 \param[in] strategy add after current or as last child
 \return new Class node
 */
Node *Class_Node::make(Strategy strategy) {
  Node *anchor = Fluid.proj.tree.current, *p = anchor;
  if (p && (strategy.placement() == Strategy::AFTER_CURRENT))
    p = p->parent;
  while (p && !p->is_decl_block()) {
    anchor = p;
    strategy.placement(Strategy::AFTER_CURRENT);
    p = p->parent;
  }
  Class_Node *o = new Class_Node();
  o->name("UserInterface");
  o->prefix("");
  o->base_class("");
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
void Class_Node::write_properties(fld::io::Project_Writer &f) {
  Node::write_properties(f);
  if (!base_class().empty()) {
    f.write_string(":");
    f.write_word(base_class().c_str());
  }
  switch (public_) {
    case 0: f.write_string("private"); break;
    case 2: f.write_string("protected"); break;
  }
}

/**
 Read additional properties.
 */
void Class_Node::read_property(fld::io::Project_Reader &f, const char *c) {
  if (!strcmp(c,"private")) {
    public_ = 0;
  } else if (!strcmp(c,"protected")) {
    public_ = 2;
  } else if (!strcmp(c,":")) {
    base_class(f.read_word());
  } else {
    Node::read_property(f, c);
  }
}

/**
 Open the class_panel to edit the class name and superclass name.
 */
void Class_Node::open() {
  open_panel();
}

/**
 Write the header code that declares this class.
 */
void Class_Node::write_code1(fld::io::Code_Writer& f) {
  parent_class = current_class;
  current_class = this;
  write_public_state = 0;
  f.write_h("\n");
  write_comment_h(f);
  if (!prefix().empty())
    f.write_h("class %s %s ", prefix().c_str(), name());
  else
    f.write_h("class %s ", name());
  if (!base_class().empty()) {
    f.write_h(": %s ", base_class().c_str());
  }
  f.write_h("{\n");
}

/**
 Write the header code that ends the declaration of this class.
 */
void Class_Node::write_code2(fld::io::Code_Writer& f) {
  f.write_h("};\n");
  current_class = parent_class;
}

/**
 Return 1 if this class contains a function with the given signature.
 */
int Node::has_function(const char *rtype, const char *sig) const {
  Node *child;
  for (child = next; child && child->level > level; child = child->next) {
    if (child->level == level+1 && child->is_a(Type::Function)) {
      const Function_Node *fn = (const Function_Node*)child;
      if (fn->has_signature(rtype, sig))
        return 1;
    }
  }
  return 0;
}
