//
// C function type code for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Window.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_File_Chooser.H>
#include "Fl_Type.h"
#include <FL/fl_show_input.H>
#include <FL/Fl_File_Chooser.H>
#include "alignment_panel.h"
#include "../src/flstring.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
  #include "ExternalCodeEditor_WIN32.h"
#else
  #include "ExternalCodeEditor_UNIX.h"
#endif

extern int i18n_type;
extern const char* i18n_include;
extern const char* i18n_function;
extern const char* i18n_file;
extern const char* i18n_set;
extern char i18n_program[];

extern int batch_mode;

extern void redraw_browser();
extern void goto_source_dir();
extern void leave_source_dir();
extern Fl_Window *main_window;

////////////////////////////////////////////////////////////////
// quick check of any C code for legality, returns an error message

static char buffer[128]; // for error messages

// check a quoted string ending in either " or ' or >:
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

// check normal code, match braces and parenthesis:
const char *_c_check(const char * & c, int type) {
  const char *d;
  for (;;) switch (*c++) {
    case 0:
      if (!type) return 0;
      sprintf(buffer, "missing %c", type);
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
    case '#':
      // treat cpp directives as a comment:
      while (*c != '\n' && *c) c++;
      break;
    case '{':
      if (type==')') goto UNEXPECTED;
      d = _c_check(c,'}');
      if (d) return d;
      break;
    case '(':
      d = _c_check(c,')');
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
    UNEXPECTED:
      if (type == *(c-1)) return 0;
      sprintf(buffer, "unexpected %c", *(c-1));
      return buffer;
  }
}

const char *c_check(const char *c, int type) {
  return _c_check(c,type);
}

////////////////////////////////////////////////////////////////

int Fl_Function_Type::is_public() const {return public_;}

Fl_Type *Fl_Function_Type::make() {
  Fl_Type *p = Fl_Type::current;
  while (p && !p->is_decl_block()) p = p->parent;
  Fl_Function_Type *o = new Fl_Function_Type();
  o->name("make_window()");
  o->return_type = 0;
  o->add(p);
  o->factory = this;
  o->public_ = 1;
  o->cdecl_ = 0;
  return o;
}

void Fl_Function_Type::write_properties() {
  Fl_Type::write_properties();
  switch (public_) {
    case 0: write_string("private"); break;
    case 2: write_string("protected"); break;
  }
  if (cdecl_) write_string("C");
  if (return_type) {
    write_string("return_type");
    write_word(return_type);
  }
}

void Fl_Function_Type::read_property(const char *c) {
  if (!strcmp(c,"private")) {
    public_ = 0;
  } else if (!strcmp(c,"protected")) {
    public_ = 2;
  } else if (!strcmp(c,"C")) {
    cdecl_ = 1;
  } else if (!strcmp(c,"return_type")) {
    storestring(read_word(),return_type);
  } else {
    Fl_Type::read_property(c);
  }
}

#include "function_panel.h"
#include <FL/fl_ask.H>

void Fl_Function_Type::open() {
  if (!function_panel) make_function_panel();
  f_return_type_input->static_value(return_type);
  f_name_input->static_value(name());
  if (is_in_class()) {
    f_public_member_choice->value(public_);
    f_public_member_choice->show();
    f_public_choice->hide();
  } else {
    f_public_choice->value(public_>0);
    f_public_choice->show();
    f_public_member_choice->hide();
  }
  f_c_button->value(cdecl_);
  const char *c = comment();
  f_comment_input->buffer()->text(c?c:"");
  function_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    if (message) fl_alert("%s", message);
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == f_panel_cancel) goto BREAK2;
      else if (w == f_panel_ok) break;
      else if (!w) Fl::wait();
    }
    const char*c = f_name_input->value();
    while (isspace(*c)) c++;
    message = c_check(c); if (message) continue;
    const char *d = c;
    for (; *d != '('; d++) if (isspace(*d) || !*d) break;
    if (*c && *d != '(') {
      message = "must be name(arguments), try again:"; continue;
    }
    int mod = 0;
    c = f_return_type_input->value();
    message = c_check(c); if (message) continue;
    name(f_name_input->value());
    storestring(c, return_type);
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
      if (!comment() || strcmp(c, comment())) redraw_browser();
      comment(c);
    } else {
      if (comment()) redraw_browser();
      comment(0);
    }
    if (c) free((void*)c);
    if (mod) set_modflag(1);
    break;
  }
BREAK2:
  function_panel->hide();
}

Fl_Function_Type Fl_Function_type;

extern const char* subclassname(Fl_Type*);

void Fl_Function_Type::write_code1() {
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
    write_c("\n");
  if (ismain()) {
    if (havechildren)
      write_c("int main(int argc, char **argv) {\n");
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
      if (havechildren)
        write_comment_c();
      write_public(public_);
      if (name()[0] == '~')
        constructor = 1;
      else {
        size_t n = strlen(k);
        if (!strncmp(name(), k, n) && name()[n] == '(') constructor = 1;
      }
      write_h("  ");
      if (is_static) write_h("static ");
      if (is_virtual) write_h("virtual ");
      if (!constructor) {
        write_h("%s%s ", rtype, star);
        if (havechildren)
          write_c("%s%s ", rtype, star);
      }
      
      // if this is a subclass, only write_h() the part before the ':'
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
      
      write_h("%s;\n", s);
      // skip all function default param. init in body:
      int skips=0,skipc=0;
      int nc=0,plevel=0;
      for (sptr=s,nptr=(char*)name(); *nptr; nc++,nptr++) {
        if (!skips && *nptr=='(') plevel++;
        else if (!skips && *nptr==')') plevel--;
        if ( *nptr=='"' &&  !(nc &&  *(nptr-1)=='\\') ) 
          skips = skips ? 0 : 1;
        else if(!skips && *nptr=='\'' &&  !(nc &&  *(nptr-1)=='\\'))
          skipc = skipc ? 0 : 1;
        if(!skips && !skipc && plevel==1 && *nptr =='=' && 
           !(nc && *(nptr-1)=='\'') ) // ignore '=' case 
          while(*++nptr  && (skips || skipc || ( (*nptr!=',' && *nptr!=')') || plevel!=1) )) {
            if ( *nptr=='"' &&  *(nptr-1)!='\\' ) 
              skips = skips ? 0 : 1;
            else if(!skips && *nptr=='\'' &&  *(nptr-1)!='\\')
              skipc = skipc ? 0 : 1;
            if (!skips && !skipc && *nptr=='(') plevel++;
            else if (!skips && *nptr==')') plevel--;
          }
        
        if (sptr < (s + sizeof(s) - 1))	*sptr++ = *nptr;
      }
      *sptr = '\0';
      
      if (havechildren)
        write_c("%s::%s {\n", k, s);
    } else {
      if (havechildren)
        write_comment_c();
      if (public_) {
        if (cdecl_)
          write_h("extern \"C\" { %s%s %s; }\n", rtype, star, name());
        else
          write_h("%s%s %s;\n", rtype, star, name());
      } else {
        if (havechildren)
          write_c("static ");
      }
      
      // write everything but the default parameters (if any)
      char s[1024], *sptr;
      char *nptr;
      int skips=0,skipc=0;
      int nc=0,plevel=0;
      for (sptr=s,nptr=(char*)name(); *nptr; nc++,nptr++) {
        if (!skips && *nptr=='(') plevel++;
        else if (!skips && *nptr==')') plevel--;
        if ( *nptr=='"' &&  !(nc &&  *(nptr-1)=='\\') ) 
          skips = skips ? 0 : 1;
        else if(!skips && *nptr=='\'' &&  !(nc &&  *(nptr-1)=='\\'))
          skipc = skipc ? 0 : 1;
        if(!skips && !skipc && plevel==1 && *nptr =='=' && 
           !(nc && *(nptr-1)=='\'') ) // ignore '=' case 
          while(*++nptr  && (skips || skipc || ( (*nptr!=',' && *nptr!=')') || plevel!=1) )) {
            if ( *nptr=='"' &&  *(nptr-1)!='\\' ) 
              skips = skips ? 0 : 1;
            else if(!skips && *nptr=='\'' &&  *(nptr-1)!='\\')
              skipc = skipc ? 0 : 1;
            if (!skips && !skipc && *nptr=='(') plevel++;
            else if (!skips && *nptr==')') plevel--;
          }
        
        if (sptr < (s + sizeof(s) - 1))	*sptr++ = *nptr;
      }
      *sptr = '\0';
      
      if (havechildren)
        write_c("%s%s %s {\n", rtype, star, s);
    }
  }
  
  if (havewidgets && child && !child->name()) write_c("  %s* w;\n", subclassname(child));
  indentation += 2;
}

void Fl_Function_Type::write_code2() {
  Fl_Type *child;
  const char *var = "w";
  char havechildren = 0;
  for (child = next; child && child->level > level; child = child->next) {
    havechildren = 1;
    if (child->is_window() && child->name()) var = child->name();
  }
  
  if (ismain()) {
    if (havewidgets) write_c("  %s->show(argc, argv);\n", var);
    if (havechildren) write_c("  return Fl::run();\n");
  } else if (havewidgets && !constructor && !return_type) {
    write_c("  return %s;\n", var);
  }
  if (havechildren)
    write_c("}\n");
  indentation = 0;
}

int Fl_Function_Type::has_signature(const char *rtype, const char *sig) const {
  if (rtype && !return_type) return 0;
  if (!name()) return 0;
  if ( (rtype==0L || strcmp(return_type, rtype)==0) 
      && fl_filename_match(name(), sig)) {
    return 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////

Fl_Type *Fl_Code_Type::make() {
  Fl_Type *p = Fl_Type::current;
  while (p && !p->is_code_block()) p = p->parent;
  if (!p) {
    fl_message("Please select a function");
    return 0;
  }
  Fl_Code_Type *o = new Fl_Code_Type();
  o->name("printf(\"Hello, World!\\n\");");
  o->add(p);
  o->factory = this;
  return o;
}

void Fl_Code_Type::open() {
  // Using an external code editor? Open it..
  if ( G_use_external_editor && G_external_editor_command[0] ) {
    const char *cmd = G_external_editor_command;
    const char *code = name();
    if ( editor_.open_editor(cmd, code) == 0 )
      return;   // return if editor opened ok, fallthru to built-in if not
  }
  // Use built-in code editor..
  if (!code_panel) make_code_panel();
  const char *text = name();
  code_input->buffer()->text( text ? text : "" );
  code_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    if (message) fl_alert("%s", message);
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == code_panel_cancel) goto BREAK2;
      else if (w == code_panel_ok) break;
      else if (!w) Fl::wait();
    }
    char*c = code_input->buffer()->text();
    message = c_check(c); if (message) continue;
    name(c);
    free(c);
    break;
  }
BREAK2:
  code_panel->hide();
}

Fl_Code_Type Fl_Code_type;

void Fl_Code_Type::write() {
  // External editor changes? If so, load changes into ram, update mtime/size
  if ( handle_editor_changes() == 1 ) {
    main_window->redraw();    // tell fluid to redraw; edits may affect tree's contents
  }
  Fl_Type::write();
}

void Fl_Code_Type::write_code1() {
  // External editor changes? If so, load changes into ram, update mtime/size
  if ( handle_editor_changes() == 1 ) {
    main_window->redraw();    // tell fluid to redraw; edits may affect tree's contents
  }

  const char* c = name();
  if (!c) return;

  const char *pch;
  const char *ind = indent();
  while( (pch=strchr(c,'\n')) )
  {
    int line_len = pch - c;
    write_c("%s%.*s\n", ind, line_len, c);
    c = pch+1;
  }
  write_c("%s%s\n", ind, c);
}

void Fl_Code_Type::write_code2() {}

////////////////////////////////////////////////////////////////

Fl_Type *Fl_CodeBlock_Type::make() {
  Fl_Type *p = Fl_Type::current;
  while (p && !p->is_code_block()) p = p->parent;
  if (!p) {
    fl_message("Please select a function");
    return 0;
  }
  Fl_CodeBlock_Type *o = new Fl_CodeBlock_Type();
  o->name("if (test())");
  o->after = 0;
  o->add(p);
  o->factory = this;
  return o;
}

void Fl_CodeBlock_Type::write_properties() {
  Fl_Type::write_properties();
  if (after) {
    write_string("after");
    write_word(after);
  }
}

void Fl_CodeBlock_Type::read_property(const char *c) {
  if (!strcmp(c,"after")) {
    storestring(read_word(),after);
  } else {
    Fl_Type::read_property(c);
  }
}

void Fl_CodeBlock_Type::open() {
  if (!codeblock_panel) make_codeblock_panel();
  code_before_input->static_value(name());
  code_after_input->static_value(after);
  codeblock_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    if (message) fl_alert("%s", message);
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == codeblock_panel_cancel) goto BREAK2;
      else if (w == codeblock_panel_ok) break;
      else if (!w) Fl::wait();
    }
    const char*c = code_before_input->value();
    message = c_check(c); if (message) continue;
    name(c);
    c = code_after_input->value();
    message = c_check(c); if (message) continue;
    storestring(c, after);
    break;
  }
BREAK2:
  codeblock_panel->hide();
}

Fl_CodeBlock_Type Fl_CodeBlock_type;

void Fl_CodeBlock_Type::write_code1() {
  const char* c = name();
  write_c("%s%s {\n", indent(), c ? c : "");
  indentation += 2;
}

void Fl_CodeBlock_Type::write_code2() {
  indentation -= 2;
  if (after) write_c("%s} %s\n", indent(), after);
  else write_c("%s}\n", indent());
}

////////////////////////////////////////////////////////////////

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

Fl_Type *Fl_Decl_Type::make() {
  Fl_Type *p = Fl_Type::current;
  while (p && !p->is_decl_block()) p = p->parent;
  Fl_Decl_Type *o = new Fl_Decl_Type();
  o->public_ = 0;
  o->static_ = 1;
  o->name("int x;");
  o->add(p);
  o->factory = this;
  return o;
}

void Fl_Decl_Type::write_properties() {
  Fl_Type::write_properties();
  switch (public_) {
    case 0: write_string("private"); break;
    case 1: write_string("public"); break;
    case 2: write_string("protected"); break;
  }
  if (static_) 
    write_string("local");
  else
    write_string("global");
}

void Fl_Decl_Type::read_property(const char *c) {
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
    Fl_Type::read_property(c);
  }
}

void Fl_Decl_Type::open() {
  if (!decl_panel) make_decl_panel();
  decl_input->static_value(name());
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
    if (message) fl_alert("%s", message);
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == decl_panel_cancel) goto BREAK2;
      else if (w == decl_panel_ok) break;
      else if (!w) Fl::wait();
    }
    const char*c = decl_input->value();
    while (isspace(*c)) c++;
    message = c_check(c&&c[0]=='#' ? c+1 : c);
    if (message) continue;
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
      if (!comment() || strcmp(c, comment())) redraw_browser();
      comment(c);
    } else {
      if (comment()) redraw_browser();
      comment(0);
    }
    if (c) free((void*)c);
    break;
  }
BREAK2:
  decl_panel->hide();
}

Fl_Decl_Type Fl_Decl_type;

void Fl_Decl_Type::write_code1() {
  const char* c = name();
  if (!c) return;
  // handle a few keywords differently if inside a class
  if (is_in_class() && (   (!strncmp(c,"class",5) && isspace(c[5]))
                        || (!strncmp(c,"typedef",7) && isspace(c[7]))
                        || (!strncmp(c,"FL_EXPORT",9) && isspace(c[9]))
                        || (!strncmp(c,"struct",6) && isspace(c[6]))
                        ) ) {
    write_public(public_);
    write_comment_h("  ");
    write_h("  %s\n", c);
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
      write_comment_h();
      write_h("%s\n", c);
    } else {
      write_comment_c();
      write_c("%s\n", c);
    }
    return;
  }
  // find the first C++ style comment
  const char* e = c+strlen(c), *csc = c;
  while (csc<e && (csc[0]!='/' || csc[1]!='/')) csc++;
  if (csc!=e) e = csc; // comment found
                       // lose all trailing semicolons so I can add one:
  while (e>c && e[-1]==' ') e--;
  while (e>c && e[-1]==';') e--;
  if (class_name(1)) {
    write_public(public_);
    write_comment_h("  ");
    write_h("  %.*s; %s\n", (int)(e-c), c, csc);
  } else {
    if (public_) {
      if (static_) 
        write_h("extern ");
      else
        write_comment_h();
      write_h("%.*s; %s\n", (int)(e-c), c, csc);
      if (static_) {
        write_comment_c();
        write_c("%.*s; %s\n", (int)(e-c), c, csc);
      }
    } else {
      write_comment_c();
      if (static_) 
        write_c("static ");
      write_c("%.*s; %s\n", (int)(e-c), c, csc);
    }
  }
}

void Fl_Decl_Type::write_code2() {}

////////////////////////////////////////////////////////////////

Fl_Type *Fl_Data_Type::make() {
  Fl_Type *p = Fl_Type::current;
  while (p && !p->is_decl_block()) p = p->parent;
  Fl_Data_Type *o = new Fl_Data_Type();
  o->public_ = 1;
  o->static_ = 1;
  o->filename_ = 0;
  o->name("myBinaryData");
  o->add(p);
  o->factory = this;
  return o;
}

void Fl_Data_Type::write_properties() {
  Fl_Decl_Type::write_properties();
  if (filename_) {
    write_string("filename");
    write_word(filename_);
  }
}

void Fl_Data_Type::read_property(const char *c) {
  if (!strcmp(c,"filename")) {
    storestring(read_word(), filename_, 1);
  } else {
    Fl_Decl_Type::read_property(c);
  }
}

void Fl_Data_Type::open() {
  if (!data_panel) make_data_panel();
  data_input->static_value(name());
  if (is_in_class()) {
    data_class_choice->value(public_);
    data_class_choice->show();
    data_choice->hide();
  } else {
    data_choice->value((public_&1)|((static_&1)<<1));
    data_choice->show();
    data_class_choice->hide();
  }
  data_filename->value(filename_?filename_:"");
  const char *c = comment();
  data_comment_input->buffer()->text(c?c:"");
  data_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    if (message) fl_alert("%s", message);
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == data_panel_cancel) goto BREAK2;
      else if (w == data_panel_ok) break;
      else if (w == data_filebrowser) {
        goto_source_dir();
        const char *fn = fl_file_chooser("Load Binary Data", 0L, data_filename->value(), 1);
        leave_source_dir();
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
    char *s = strdup(c), *p = s, *q, *n;
    for (;;++p) {
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
    if (n==q) {
    OOPS: message = "variable name must be a C identifier";
      free((void*)s);
      continue;
    }
    *p = 0;
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
    // store the filename
    c = data_filename->value();
    if (filename_ && strcmp(filename_, data_filename->value()))
      set_modflag(1); 
    else if (!filename_ && *c)
      set_modflag(1);
    if (filename_) { free((void*)filename_); filename_ = 0L; }
    if (c && *c) filename_ = strdup(c);
    // store the comment
    c = data_comment_input->buffer()->text();
    if (c && *c) {
      if (!comment() || strcmp(c, comment())) redraw_browser();
      comment(c);
    } else {
      if (comment()) redraw_browser();
      comment(0);
    }
    if (c) free((void*)c);
    break;
  }
BREAK2:
  data_panel->hide();
}

Fl_Data_Type Fl_Data_type;

void Fl_Data_Type::write_code1() {
  const char *message = 0;
  const char *c = name();
  if (!c) return;
  const char *fn = filename_;
  char *data = 0;
  int nData = -1;
  // path should be set correctly already
  if (filename_ && !write_sourceview) {
    FILE *f = fl_fopen(filename_, "rb");
    if (!f) {
      message = "Can't include binary file. Can't open";
    } else {
      fseek(f, 0, SEEK_END);
      nData = ftell(f);
      fseek(f, 0, SEEK_SET);
      if (nData) {
        data = (char*)calloc(nData, 1);
        if (fread(data, nData, 1, f)==0) { /* use default */ }
      }
      fclose(f);
    }
  } else {
    fn = "<no filename>";
  }
  if (is_in_class()) {
    write_public(public_);
    write_comment_h("  ");
    write_h("  static unsigned char %s[%d];\n", c, nData);
    write_c("unsigned char %s::%s[%d] = /* binary data included from %s */\n", class_name(1), c, nData, fn);
    if (message) write_c("#error %s %s\n", message, fn);
    write_cdata(data, nData);
    write_c(";\n");
  } else {
    // the "header only" option does not apply here!
    if (public_) {
      if (static_) {
        write_h("extern unsigned char %s[%d];\n", c, nData);
        write_comment_c();
        write_c("unsigned char %s[%d] = /* binary data included from %s */\n", c, nData, fn);
        if (message) write_c("#error %s %s\n", message, fn);
        write_cdata(data, nData);
        write_c(";\n");
      } else {
        write_comment_h();
        write_h("#error Unsupported declaration loading binary data %s\n", fn);
        write_h("unsigned char %s[3] = { 1, 2, 3 };\n", c);
      }
    } else {
      write_comment_c();
      if (static_) 
        write_c("static ");
      write_c("unsigned char %s[%d] = /* binary data included from %s */\n", c, nData, fn);
      if (message) write_c("#error %s %s\n", message, fn);
      write_cdata(data, nData);
      write_c(";\n");
    }
  }
  // if we are in interactive mode, we pop up a warning dialog 
  // giving the error: (batch_mode && !write_sourceview) ???
  if (message && !write_sourceview) {
    if (batch_mode)
      fprintf(stderr, "FLUID ERROR: %s %s\n", message, fn);
    else
      fl_alert("%s\n%s\n", message, fn);
  }
  if (data) free(data);
}

void Fl_Data_Type::write_code2() {}

////////////////////////////////////////////////////////////////

int Fl_DeclBlock_Type::is_public() const {return public_;}

Fl_Type *Fl_DeclBlock_Type::make() {
  Fl_Type *p = Fl_Type::current;
  while (p && !p->is_decl_block()) p = p->parent;
  Fl_DeclBlock_Type *o = new Fl_DeclBlock_Type();
  o->name("#if 1");
  o->public_ = 0;
  o->after = strdup("#endif");
  o->add(p);
  o->factory = this;
  return o;
}

void Fl_DeclBlock_Type::write_properties() {
  Fl_Type::write_properties();
  switch (public_) {
    case 1: write_string("public"); break;
    case 2: write_string("protected"); break;
  }
  write_string("after");
  write_word(after);
}

void Fl_DeclBlock_Type::read_property(const char *c) {
  if(!strcmp(c,"public")) {
    public_ = 1;
  } else if(!strcmp(c,"protected")) {
    public_ = 2;
  } else  if (!strcmp(c,"after")) {
    storestring(read_word(),after);
  } else {
    Fl_Type::read_property(c);
  }
}

void Fl_DeclBlock_Type::open() {
  if (!declblock_panel) make_declblock_panel();
  decl_before_input->static_value(name());
  declblock_public_choice->value((public_>0));
  decl_after_input->static_value(after);
  declblock_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    if (message) fl_alert("%s", message);
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == declblock_panel_cancel) goto BREAK2;
      else if (w == declblock_panel_ok) break;
      else if (!w) Fl::wait();
    }
    const char*c = decl_before_input->value();
    while (isspace(*c)) c++;
    message = c_check(c&&c[0]=='#' ? c+1 : c);
    if (message) continue;
    name(c);
    c = decl_after_input->value();
    while (isspace(*c)) c++;
    message = c_check(c&&c[0]=='#' ? c+1 : c);
    if (message) continue;
    storestring(c,after);
    if (public_ != declblock_public_choice->value()) {
      set_modflag(1);
      public_ = declblock_public_choice->value();
      redraw_browser();
    }
    break;
  }
BREAK2:
  declblock_panel->hide();
}

Fl_DeclBlock_Type Fl_DeclBlock_type;

void Fl_DeclBlock_Type::write_code1() {
  const char* c = name();
  if (public_)
    write_h("%s\n", c);
  write_c("%s\n", c);
}

void Fl_DeclBlock_Type::write_code2() {
  const char* c = after;
  if (public_)
    write_h("%s\n", c);
  write_c("%s\n", c);
}

////////////////////////////////////////////////////////////////

Fl_Type *Fl_Comment_Type::make() {
  Fl_Type *p = Fl_Type::current;
  while (p && !p->is_code_block()) p = p->parent;
  Fl_Comment_Type *o = new Fl_Comment_Type();
  o->in_c_ = 1;
  o->in_h_ = 1;
  o->style_ = 0;
  o->name("my comment");
  o->add(p);
  o->factory = this;
  o->title_buf[0] = 0;
  return o;
}

void Fl_Comment_Type::write_properties() {
  Fl_Type::write_properties();
  if (in_c_) write_string("in_source"); else write_string("not_in_source"); 
  if (in_h_) write_string("in_header"); else write_string("not_in_header");
}

void Fl_Comment_Type::read_property(const char *c) {
  if (!strcmp(c,"in_source")) {
    in_c_ = 1;
  } else if (!strcmp(c,"not_in_source")) {
    in_c_ = 0;
  } else if (!strcmp(c,"in_header")) {
    in_h_ = 1;
  } else if (!strcmp(c,"not_in_header")) {
    in_h_ = 0;
  } else {
    Fl_Type::read_property(c);
  }
}

#include "comments.h"

static void load_comments_preset(Fl_Preferences &menu) {
  static const char * const predefined_comment[] = {
    "GNU Public License/GPL Header",  "GNU Public License/GPL Footer",
    "GNU Public License/LGPL Header", "GNU Public License/LGPL Footer",
    "FLTK/Header", "FLTK/Footer" };
  int i;
  menu.set("n", 6);
  Fl_Preferences db(Fl_Preferences::USER, "fltk.org", "fluid_comments");
  for (i=0; i<6; i++) {
    menu.set(Fl_Preferences::Name(i), predefined_comment[i]);
    db.set(predefined_comment[i], comment_text[i]);
  }
}

void Fl_Comment_Type::open() {
  if (!comment_panel) make_comment_panel();
  const char *text = name();
  {
    int i=0, n=0;
    Fl_Preferences menu(Fl_Preferences::USER, "fltk.org", "fluid_comments_menu");
    comment_predefined->clear();
    comment_predefined->add("_Edit/Add current comment...");
    comment_predefined->add("_Edit/Remove last selection...");
    menu.get("n", n, -1);
    if (n==-1) load_comments_preset(menu);
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
  const char* message = 0;
  char itempath[FL_PATH_MAX]; itempath[0] = 0;
  int last_selected_item = 0;
  for (;;) { // repeat as long as there are errors
    if (message) fl_alert("%s", message);
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
            char *name = strdup(xname);
            for (char*s=name;*s;s++) if (*s==':') *s = ';';
            int n;
            Fl_Preferences db(Fl_Preferences::USER, "fltk.org", "fluid_comments");
            db.set(name, comment_input->buffer()->text());
            Fl_Preferences menu(Fl_Preferences::USER, "fltk.org", "fluid_comments_menu");
            menu.get("n", n, 0);
            menu.set(Fl_Preferences::Name(n), name);
            menu.set("n", ++n);
            comment_predefined->add(name);
            free(name);
          }
        } else if (comment_predefined->value()==2) {
          // remove the last selected comment from the database
          if (itempath[0]==0 || last_selected_item==0) {
            fl_message("Please select an entry form this menu first.");
          } else if (fl_choice("Are you sure that you want to delete the entry\n"
	                       "\"%s\"\nfrom the database?", "Cancel", "Delete",
			       NULL, itempath)) {
            Fl_Preferences db(Fl_Preferences::USER, "fltk.org", "fluid_comments");
            db.deleteEntry(itempath);
            comment_predefined->remove(last_selected_item);
            Fl_Preferences menu(Fl_Preferences::USER, "fltk.org", "fluid_comments_menu");
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
            Fl_Preferences db(Fl_Preferences::USER, "fltk.org", "fluid_comments");
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
  title_buf[0] = 0;
  comment_panel->hide();
}

const char *Fl_Comment_Type::title() {
  const char* n = name(); 
  if (!n || !*n) return type_name();
  if (title_buf[0]==0) {
    const char *s = n;
    char *d = title_buf;
    int i = 50;
    while (--i > 0) {
      char n = *s++;
      if (n==0) break;
      if (n=='\r') { *d++ = '\\'; *d++ = 'r'; i--; }
      else if (n=='\n') { *d++ = '\\'; *d++ = 'n'; i--; }
      else if (n<32) { *d++ = '^'; *d++ = 'A'+n; i--; }
      else *d++ = n;
    }
    if (i<=0) { *d++ = '.'; *d++ = '.'; *d++ = '.'; }
    *d++ = 0;
  }
  return title_buf;
}

Fl_Comment_Type Fl_Comment_type;

void Fl_Comment_Type::write_code1() {
  const char* c = name();
  if (!c) return;
  if (!in_c_ && !in_h_) return;
  // find out if there is already a valid comment:
  const char *s = c;
  while (isspace(*s)) s++;
  // if this seems to be a C style comment, copy the block as is
  // (it's up to the user to correctly close the comment)
  if (s[0]=='/' && s[1]=='*') {
    if (in_h_) write_h("%s\n", c);
    if (in_c_) write_c("%s\n", c);
    return;
  }
  // copy the comment line by line, add the double slash if needed
  char *txt = strdup(c);
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
      if (in_h_) write_h("// ");
      if (in_c_) write_c("// ");
    }
    // now copy the rest of the line
    if (in_h_) write_h("%s\n", b);
    if (in_c_) write_c("%s\n", b);
    if (eol==0) break;
    *e++ = eol;
    b = e;
  }
  free(txt);
}

void Fl_Comment_Type::write_code2() {}

////////////////////////////////////////////////////////////////

const char* Fl_Type::class_name(const int need_nest) const {
  Fl_Type* p = parent;
  while (p) {
    if (p->is_class()) {
      // see if we are nested in another class, we must fully-qualify name:
      // this is lame but works...
      const char* q = 0;
      if(need_nest) q=p->class_name(need_nest);
      if (q) {
	static char s[256];
	if (q != s) strlcpy(s, q, sizeof(s));
	strlcat(s, "::", sizeof(s));
	strlcat(s, p->name(), sizeof(s));
	return s;
      }
      return p->name();
    }
    p = p->parent;
  }
  return 0;
}

/**
 * If this Type resides inside a class, this function returns the class type, or null.
 */
const Fl_Class_Type *Fl_Type::is_in_class() const {
  Fl_Type* p = parent;
  while (p) {
    if (p->is_class()) {
      return (Fl_Class_Type*)p;
    }
    p = p->parent;
  }
  return 0;
}

int Fl_Class_Type::is_public() const {return public_;}

void Fl_Class_Type::prefix(const char*p) {
  free((void*) class_prefix);
  class_prefix=strdup(p ? p : "" );
}

Fl_Type *Fl_Class_Type::make() {
  Fl_Type *p = Fl_Type::current;
  while (p && !p->is_decl_block()) p = p->parent;
  Fl_Class_Type *o = new Fl_Class_Type();
  o->name("UserInterface");
  o->class_prefix=0;
  o->subclass_of = 0;
  o->public_ = 1;
  o->add(p);
  o->factory = this;
  return o;
}

void Fl_Class_Type::write_properties() {
  Fl_Type::write_properties();
  if (subclass_of) {
    write_string(":");
    write_word(subclass_of);
  }
  switch (public_) {
    case 0: write_string("private"); break;
    case 2: write_string("protected"); break;
  }
}

void Fl_Class_Type::read_property(const char *c) {
  if (!strcmp(c,"private")) {
    public_ = 0;
  } else if (!strcmp(c,"protected")) {
    public_ = 2;
  } else if (!strcmp(c,":")) {
    storestring(read_word(), subclass_of);
  } else {
    Fl_Type::read_property(c);
  }
}

void Fl_Class_Type::open() {
  if (!class_panel) make_class_panel();
  char fullname[FL_PATH_MAX]="";
  if (prefix() && strlen(prefix())) 
    sprintf(fullname,"%s %s",prefix(),name());
  else 
    strcpy(fullname, name());
  c_name_input->static_value(fullname);
  c_subclass_input->static_value(subclass_of);
  c_public_button->value(public_);
  const char *c = comment();
  c_comment_input->buffer()->text(c?c:"");
  class_panel->show();
  const char* message = 0;
  
  char *na=0,*pr=0,*p=0; // name and prefix substrings
  
  for (;;) { // repeat as long as there are errors
    if (message) fl_alert("%s", message);
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == c_panel_cancel) goto BREAK2;
      else if (w == c_panel_ok) break;
      else if (!w) Fl::wait();
    }
    const char*c = c_name_input->value();
    char *s = strdup(c);
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
      if (!comment() || strcmp(c, comment())) redraw_browser();
      comment(c);
    } else {
      if (comment()) redraw_browser();
      comment(0);
    }
    if (c) free((void*)c);
    break;
  }
BREAK2:
  class_panel->hide();
}

Fl_Class_Type Fl_Class_type;

Fl_Class_Type *current_class;
extern Fl_Widget_Class_Type *current_widget_class;
void write_public(int state) {
  if (!current_class && !current_widget_class) return;
  if (current_class && current_class->write_public_state == state) return;
  if (current_widget_class && current_widget_class->write_public_state == state) return;
  if (current_class) current_class->write_public_state = state;
  if (current_widget_class) current_widget_class->write_public_state = state;
  switch (state) {
    case 0: write_h("private:\n"); break;
    case 1: write_h("public:\n"); break;
    case 2: write_h("protected:\n"); break;
  }
}

void Fl_Class_Type::write_code1() {
  parent_class = current_class;
  current_class = this;
  write_public_state = 0;
  write_h("\n");
  write_comment_h();
  if (prefix() && strlen(prefix()))
    write_h("class %s %s ", prefix(), name());
  else
    write_h("class %s ", name());
  if (subclass_of) write_h(": %s ", subclass_of);
  write_h("{\n");
}

void Fl_Class_Type::write_code2() {
  write_h("};\n");
  current_class = parent_class;
}

/**
 * Return 1 if this class contains a function with the given signature.
 */
int Fl_Class_Type::has_function(const char *rtype, const char *sig) const {
  Fl_Type *child;
  for (child = next; child && child->level > level; child = child->next) {
    if (child->level == level+1 && strcmp(child->type_name(), "Function")==0) {
      const Fl_Function_Type *fn = (const Fl_Function_Type*)child;
      if (fn->has_signature(rtype, sig))
        return 1;
    }
  }
  return 0;
}

//
// End of "$Id$".
//
