/*	code.C

	Code to write .C files from Fluid

*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <FL/Fl.H>
#include "Fl_Type.H"

static FILE *code_file;
static FILE *header_file;

// return true if c can be in a C identifier.  I needed this so
// it is not messed up by locale settings:
int is_id(char c) {
  return c>='a' && c<='z' || c>='A' && c<='Z' || c>='0' && c<='9' || c=='_';
}

////////////////////////////////////////////////////////////////
// Generate unique but human-readable identifiers:

struct id {
  char* text;
  void* object;
  id *left, *right;
  id (const char* t, void* o) : text(strdup(t)), object(o) {left = right = 0;}
  ~id();
};

id::~id() {
  delete left;
  free((void *)text);
  delete right;
}

static id* id_root;

const char* unique_id(void* o, const char* type, const char* name, const char* label) {
  char buffer[128];
  char* q = buffer;
  while (*type) *q++ = *type++;
  *q++ = '_';
  const char* n = name;
  if (!n || !*n) n = label;
  if (n && *n) {
    while (!is_id(*n)) n++;
    while (is_id(*n)) *q++ = *n++;
  }
  *q = 0;
  // okay, search the tree and see if the name was already used:
  id** p = &id_root;
  int which = 0;
  while (*p) {
    int i = strcmp(buffer, (*p)->text);
    if (!i) {
      if ((*p)->object == o) return (*p)->text;
      // already used, we need to pick a new name:
      sprintf(q,"%x",++which);
      p = &id_root;
      continue;
    }
    else if (i < 0) p = &((*p)->left);
    else p  = &((*p)->right);
  }
  *p = new id(buffer, o);
  return (*p)->text;
}

////////////////////////////////////////////////////////////////
// return current indentation:

static const char* spaces = "                ";
int indentation;
const char* indent() {
  int i = indentation; if (i>16) i = 16;
  return spaces+16-i;
}

////////////////////////////////////////////////////////////////
// declarations/include files:
// These are sorted in alphabetical order and only included once each:
// Relies on '#' being less than any letter to put #include first.
// I use a binary tree to sort these out.

struct included {
  char *text;
  included *left, *right;
  included(const char *t) {
    text = strdup(t);
    left = right = 0;
  }
  ~included();
};

included::~included() {
  delete left;
  fprintf(header_file,"%s\n",text);
  free((void *)text);
  delete right;
}
static included *included_root;

int write_declare(const char *format, ...) {
  va_list args;
  char buf[1024];
  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);
  included **p = &included_root;
  while (*p) {
    int i = strcmp(buf,(*p)->text);
    if (!i) return 0;
    else if (i < 0) p = &((*p)->left);
    else p  = &((*p)->right);
  }
  *p = new included(buf);
  return 1;
}

////////////////////////////////////////////////////////////////

// silly thing to prevent declaring unused variables:
// When this symbol is on, all attempts to write code don't write
// anything, but set a variable if it looks like the varaible "o" is used:
int varused_test;
int varused;

// write an array of C characters (adds a null):
void write_cstring(const char *w, int length) {
  if (varused_test) return;
  const char *e = w+length;
  int linelength = 1;
  putc('\"', code_file);
  for (; w < e;) {
    if (linelength >= 75) {fputs("\\\n",code_file); linelength = 0;}
    int c = *w++;
    switch (c) {
    case '\b': c = 'b'; goto QUOTED;
    case '\t': c = 't'; goto QUOTED;
    case '\n': c = 'n'; goto QUOTED;
    case '\f': c = 'f'; goto QUOTED;
    case '\r': c = 'r'; goto QUOTED;
    case '\"':
    case '\'':
    case '\\':
    QUOTED:
      putc('\\',code_file);
      putc(c,code_file);
      linelength += 2;
      break;
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
      if (*w < '0' || *w > '9') {c += '0'; goto QUOTED;}
      // else fall through:
    default:
      if (c < ' ' || c >= 127) {
      QUOTENEXT:
	fprintf(code_file, "\\x%02x",c&255);
	linelength += 4;
	c = *w;
	if (w < e && (c>='0'&&c<='9' || c>='a'&&c<='f' || c>='A'&&c<='F')) {
	  w++; goto QUOTENEXT;
	}
      } else {
	putc(c,code_file);
	linelength++;
      }
      break;
    }
  }
  putc('\"', code_file);
}

// write a C string, quoting characters if necessary:
void write_cstring(const char *w) {write_cstring(w,strlen(w));}

void write_c(const char* format,...) {
  if (varused_test) {varused = 1; return;}
  va_list args;
  va_start(args, format);
  vfprintf(code_file, format, args);
  va_end(args);
}

void write_h(const char* format,...) {
  if (varused_test) return;
  va_list args;
  va_start(args, format);
  vfprintf(header_file, format, args);
  va_end(args);
}

#include <FL/filename.H>
int write_number;

// recursively dump code, putting children between the two parts
// of the parent code:
static Fl_Type* write_code(Fl_Type* p) {
  p->write_code1();
  Fl_Type* q;
  for (q = p->next; q && q->level > p->level;) q = write_code(q);
  p->write_code2();
  return q;
}

int write_code(const char *s, const char *t) {
  write_number++;
  delete id_root; id_root = 0;
  indentation = 0;
  if (!s) code_file = stdout;
  else {
    FILE *f = fopen(s,"w");
    if (!f) return 0;
    code_file = f;
  }
  if (!t) header_file = stdout;
  else {
    FILE *f = fopen(t,"w");
    if (!f) {fclose(code_file); return 0;}
    header_file = f;
  }
  const char *hdr = "\
// generated by Fast Light User Interface Designer (fluid) version %.2f\n\n";
  fprintf(header_file, hdr, FL_VERSION);
  fprintf(code_file, hdr, FL_VERSION);
  Fl_Type *p;

  for (p = Fl_Type::first; p; p = p->next) p->write_declare();
  delete included_root; included_root = 0;

  if (t) write_c("#include \"%s\"\n", filename_name(t));
  for (p = Fl_Type::first; p; p = p->next) p->write_static();
  for (p = Fl_Type::first; p;) p = write_code(p);

  if (!s) return 1;
  int x = fclose(code_file);
  code_file = 0;
  int y = fclose(header_file);
  header_file = 0;
  return x >= 0 && y >= 0;
}

////////////////////////////////////////////////////////////////

void Fl_Type::write_declare() {}
void Fl_Type::write_static() {}
void Fl_Type::write_code1() {
  write_h("// Header for %s\n", title());
  write_c("// Code for %s\n", title());
}
void Fl_Type::write_code2() {}
