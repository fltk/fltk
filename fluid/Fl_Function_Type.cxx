//
// "$Id: Fl_Function_Type.cxx,v 1.15.2.16 2001/01/22 15:13:38 easysw Exp $"
//
// C function type code for the Fast Light Tool Kit (FLTK).
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
#include "Fl_Type.h"
#include <FL/fl_show_input.H>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

extern int i18n_type;
extern const char* i18n_include;
extern const char* i18n_function;
extern const char* i18n_file;
extern const char* i18n_set;
extern char i18n_program[];

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
  if (!public_) write_string("private");
  if (cdecl_) write_string("C");
  if (return_type) {
    write_string("return_type");
    write_word(return_type);
  }
}

void Fl_Function_Type::read_property(const char *c) {
  if (!strcmp(c,"private")) {
    public_ = 0;
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
  f_public_button->value(public_);
  f_c_button->value(cdecl_);
  function_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    if (message) fl_alert(message);
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
    c = f_return_type_input->value();
    message = c_check(c); if (message) continue;
    name(f_name_input->value());
    storestring(c, return_type);
    public_ = f_public_button->value();
    cdecl_ = f_c_button->value();
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
  for (child = next; child && child->level > level; child = child->next)
    if (child->is_widget()) {
      havewidgets = 1;
      break;
    }
  write_c("\n");
  if (ismain())
    write_c("int main(int argc, char **argv) {\n");
  else {
    const char* rtype = return_type;
    const char* star = "";
    // from matt: let the user type "static " at the start of type
    // in order to declare a static method;
    int is_static = 0;
    int is_virtual = 0;
    if (rtype) {
      if (!strcmp(rtype,"static")) {is_static = 1; rtype = 0;}
      else if (!strncmp(rtype, "static ",7)) {is_static = 1; rtype += 7;}
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
      write_c("%s::%s {\n", k, name());
    } else {
      if (public_) {
	if (cdecl_)
	  write_h("extern \"C\" { %s%s %s; }\n", rtype, star, name());
	else
	  write_h("%s%s %s;\n", rtype, star, name());
      }
      else write_c("static ");
      write_c("%s%s %s {\n", rtype, star, name());
    }
  }
  if (havewidgets) write_c("  %s* w;\n",subclassname(child));
  indentation += 2;
}

void Fl_Function_Type::write_code2() {
  if (ismain()) {
    if (havewidgets) write_c("  w->show(argc, argv);\n");
    write_c("  return Fl::run();\n");
  } else if (havewidgets && !constructor && !return_type)
    write_c("  return w;\n");
  write_c("}\n");
  indentation = 0;
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
  if (!code_panel) make_code_panel();
  code_input->static_value(name());
  code_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    if (message) fl_alert(message);
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == code_panel_cancel) goto BREAK2;
      else if (w == code_panel_ok) break;
      else if (!w) Fl::wait();
    }
    const char*c = code_input->value();
    message = c_check(c); if (message) continue;
    name(c);
    break;
  }
 BREAK2:
  code_panel->hide();
}

Fl_Code_Type Fl_Code_type;

void Fl_Code_Type::write_code1() {
  const char* c = name();
  if (!c) return;
  write_c("%s%s\n", indent(), c);
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
    if (message) fl_alert(message);
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
  indentation += 2;
  if (after) write_c("%s} %s\n", indent(), after);
  else write_c("%s}\n", indent());
}

////////////////////////////////////////////////////////////////

Fl_Type *Fl_Decl_Type::make() {
  Fl_Type *p = Fl_Type::current;
  while (p && !p->is_decl_block()) p = p->parent;
  Fl_Decl_Type *o = new Fl_Decl_Type();
  o->public_ = 0;
  o->name("int x;");
  o->add(p);
  o->factory = this;
  return o;
}

void Fl_Decl_Type::write_properties() {
  Fl_Type::write_properties();
  if (public_) write_string("public");
}

void Fl_Decl_Type::read_property(const char *c) {
  if (!strcmp(c,"public")) {
    public_ = 1;
  } else {
    Fl_Type::read_property(c);
  }
}

void Fl_Decl_Type::open() {
  if (!decl_panel) make_decl_panel();
  decl_input->static_value(name());
  decl_public_button->value(public_);
  decl_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    if (message) fl_alert(message);
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
    public_ = decl_public_button->value();
    break;
  }
 BREAK2:
  decl_panel->hide();
}

Fl_Decl_Type Fl_Decl_type;

void Fl_Decl_Type::write_code1() {
  const char* c = name();
  if (!c) return;
  // handle putting #include or extern or typedef into decl:
  if (!isalpha(*c) && *c != '~'
      || !strncmp(c,"extern",6) && isspace(c[6])
      || !strncmp(c,"class",5) && isspace(c[5])
      || !strncmp(c,"typedef",7) && isspace(c[7])
//    || !strncmp(c,"struct",6) && isspace(c[6])
      ) {
    if (public_)
      write_h("%s\n", c);
    else
      write_c("%s\n", c);
    return;
  }
  // lose all trailing semicolons so I can add one:
  const char* e = c+strlen(c);
  while (e>c && e[-1]==';') e--;
  if (class_name(1)) {
    write_public(public_);
    write_h("  %.*s;\n", e-c, c);
  } else {
    if (public_) {
      write_h("extern %.*s;\n", e-c, c);
      write_c("%.*s;\n", e-c, c);
    } else {
      write_c("static %.*s;\n", e-c, c);
    }
  }
}

void Fl_Decl_Type::write_code2() {}

////////////////////////////////////////////////////////////////

Fl_Type *Fl_DeclBlock_Type::make() {
  Fl_Type *p = Fl_Type::current;
  while (p && !p->is_decl_block()) p = p->parent;
  Fl_DeclBlock_Type *o = new Fl_DeclBlock_Type();
  o->name("#if 1");
  o->after = strdup("#endif");
  o->add(p);
  o->factory = this;
  return o;
}

void Fl_DeclBlock_Type::write_properties() {
  Fl_Type::write_properties();
  write_string("after");
  write_word(after);
}

void Fl_DeclBlock_Type::read_property(const char *c) {
  if (!strcmp(c,"after")) {
    storestring(read_word(),after);
  } else {
    Fl_Type::read_property(c);
  }
}

void Fl_DeclBlock_Type::open() {
  if (!declblock_panel) make_declblock_panel();
  decl_before_input->static_value(name());
  decl_after_input->static_value(after);
  declblock_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    if (message) fl_alert(message);
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
    break;
  }
 BREAK2:
  declblock_panel->hide();
}

Fl_DeclBlock_Type Fl_DeclBlock_type;

void Fl_DeclBlock_Type::write_code1() {
  const char* c = name();
  if (c) write_c("%s\n", c);
}

void Fl_DeclBlock_Type::write_code2() {
  const char* c = after;
  if (c) write_c("%s\n", c);
}

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
	static char buffer[256];
	if (q != buffer) strcpy(buffer, q);
	strcat(buffer, "::");
	strcat(buffer, p->name());
	return buffer;
      }
      return p->name();
    }
    p = p->parent;
  }
  return 0;
}

Fl_Type *Fl_Class_Type::make() {
  Fl_Type *p = Fl_Type::current;
  while (p && !p->is_decl_block()) p = p->parent;
  Fl_Class_Type *o = new Fl_Class_Type();
  o->name("UserInterface");
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
  if (!public_) write_string("private");
}

void Fl_Class_Type::read_property(const char *c) {
  if (!strcmp(c,"private")) {
    public_ = 0;
  } else if (!strcmp(c,":")) {
    storestring(read_word(), subclass_of);
  } else {
    Fl_Type::read_property(c);
  }
}

void Fl_Class_Type::open() {
  if (!class_panel) make_class_panel();
  c_name_input->static_value(name());
  c_subclass_input->static_value(subclass_of);
  c_public_button->value(public_);
  class_panel->show();
  const char* message = 0;
  for (;;) { // repeat as long as there are errors
    if (message) fl_alert(message);
    for (;;) {
      Fl_Widget* w = Fl::readqueue();
      if (w == c_panel_cancel) goto BREAK2;
      else if (w == c_panel_ok) break;
      else if (!w) Fl::wait();
    }
    const char*c = c_name_input->value();
    while (isspace(*c)) c++;
    if (!*c) goto OOPS;
    while (is_id(*c)) c++;
    while (isspace(*c)) c++;
    if (*c) {OOPS: message = "class name must be C++ identifier"; continue;}
    c = c_subclass_input->value();
    message = c_check(c); if (message) continue;
    name(c_name_input->value());
    storestring(c, subclass_of);
    public_ = c_public_button->value();
    break;
  }
 BREAK2:
  class_panel->hide();
}

Fl_Class_Type Fl_Class_type;

static Fl_Class_Type *current_class;
extern int varused_test;
void write_public(int state) {
  if (!current_class || varused_test) return;
  if (current_class->write_public_state == state) return;
  current_class->write_public_state = state;
  write_h(state ? "public:\n" : "private:\n");
}

void Fl_Class_Type::write_code1() {
  parent_class = current_class;
  current_class = this;
  write_public_state = 0;
  write_h("\nclass %s ", name());
  if (subclass_of) write_h(": %s ", subclass_of);
  write_h("{\n");
}

void Fl_Class_Type::write_code2() {
  write_h("};\n");
  current_class = parent_class;
}

//
// End of "$Id: Fl_Function_Type.cxx,v 1.15.2.16 2001/01/22 15:13:38 easysw Exp $".
//
