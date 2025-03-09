//
// Code output routines for the Fast Light Tool Kit (FLTK).
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

#include "io/Code_Writer.h"

#include "Fluid.h"
#include "Project.h"
#include "nodes/Fl_Window_Type.h"
#include "nodes/Fl_Function_Type.h"

#include <FL/filename.H>
#include "../src/flstring.h"

#include <zlib.h>

using namespace fld;
using namespace fld::io;

/**
 Return true if c can be in a C identifier.
 I needed this so it is not messed up by locale settings.
 \param[in] c a character, or the start of a utf-8 sequence
 \return 1 if c is alphanumeric or '_'
 */
int is_id(char c) {
  return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || c=='_';
}

////////////////////////////////////////////////////////////////
// Generate unique but human-readable identifiers:

/** A binary searchable tree storing identifiers for quick retrieval. */
struct Fd_Identifier_Tree {
  char* text;
  void* object;
  Fd_Identifier_Tree *left, *right;
  Fd_Identifier_Tree (const char* t, void* o) : text(fl_strdup(t)), object(o) {left = right = 0;}
  ~Fd_Identifier_Tree();
};

Fd_Identifier_Tree::~Fd_Identifier_Tree() {
  delete left;
  free((void *)text);
  delete right;
}

/** \brief Return a unique name for the given object.

 This function combines the name and label into an identifier. It then checks
 if that id was already taken by another object, and if so, appends a
 hexadecimal value which is incremented until the id is unique in this file.

 If a new id was created, it is stored in the id tree.

 \param[in] o create an ID for this object
 \param[in] type is the first word of the ID
 \param[in] name if name is set, it is appended to the ID
 \param[in] label else if label is set, it is appended, skipping non-keyword characters
 \return buffer to a unique identifier, managed by Code_Writer, so caller must NOT free() it
 */
const char* Code_Writer::unique_id(void* o, const char* type, const char* name, const char* label) {
  char buffer[128];
  char* q = buffer;
  char* q_end = q + 128 - 8 - 1; // room for hex number and NUL
  while (*type) *q++ = *type++;
  *q++ = '_';
  const char* n = name;
  if (!n || !*n) n = label;
  if (n && *n) {
    while (*n && !is_id(*n)) n++;
    while (is_id(*n) && (q < q_end)) *q++ = *n++;
  }
  *q = 0;
  // okay, search the tree and see if the name was already used:
  Fd_Identifier_Tree** p = &id_root;
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
  *p = new Fd_Identifier_Tree(buffer, o);
  return (*p)->text;
}

////////////////////////////////////////////////////////////////
// return current indentation:


/**
 Return a C string that indents code to the given depth.

 Indentation can be changed by modifying the multiplicator (``*2`` to keep
 the FLTK indent style). Changing `spaces` to a list of tabs would generate
 tab indents instead. This function can also be used for fixed depth indents
 in the header file.

 Do *not* ever make this a user preference, or you will end up writing a
 fully featured code formatter.

 \param[in] set generate this indent depth
 \return pointer to a static string
 */
const char *Code_Writer::indent(int set) {
  static const char* spaces = "                                ";
  int i = set * 2;
  if (i>32) i = 32;
  if (i<0) i = 0;
  return spaces+32-i;
}

/**
 Return a C string that indents code to the current source file depth.
 \return pointer to a static string
 */
const char *Code_Writer::indent() {
  return indent(indentation);
}

/**
 Return a C string that indents code to the current source file depth plus an offset.
 \param[in] offset adds a temporary offset for this call only; this does not
    change the `indentation` variable; offset can be negative
 \return pointer to a static string
 */
const char *Code_Writer::indent_plus(int offset) {
  return indent(indentation+offset);
}


////////////////////////////////////////////////////////////////
// declarations/include files:
// Each string generated by write_h_once is written only once to
// the header file.  This is done by keeping a binary tree of all
// the calls so far and not printing it if it is in the tree.

/** A binary searchable tree storing text for quick retrieval. */
struct Fd_Text_Tree {
  char *text;
  Fd_Text_Tree *left, *right;
  Fd_Text_Tree(const char *t) {
    text = fl_strdup(t);
    left = right = 0;
  }
  ~Fd_Text_Tree();
};

Fd_Text_Tree::~Fd_Text_Tree() {
  delete left;
  free((void *)text);
  delete right;
}

/** A binary searchable tree storing pointers for quick retrieval. */
struct Fd_Pointer_Tree {
  void *ptr;
  Fd_Pointer_Tree *left, *right;
  Fd_Pointer_Tree(void *p) {
    ptr = p;
    left = right = 0;
  }
  ~Fd_Pointer_Tree();
};

Fd_Pointer_Tree::~Fd_Pointer_Tree() {
  delete left;
  delete right;
}

/**
 Print a formatted line to the header file, unless the same line was produced before in this header file.
 \note Resulting line is cropped at 1023 bytes.
 \param[in] format printf-style formatting text, followed by a vararg list
 \return 1 if the text was written to the file, 0 if it was previously written.
 */
int Code_Writer::write_h_once(const char *format, ...) {
  va_list args;
  char buf[1024];
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);
  Fd_Text_Tree **p = &text_in_header;
  while (*p) {
    int i = strcmp(buf,(*p)->text);
    if (!i) return 0;
    else if (i < 0) p = &((*p)->left);
    else p  = &((*p)->right);
  }
  fprintf(header_file,"%s\n",buf);
  *p = new Fd_Text_Tree(buf);
  return 1;
}

/**
 Print a formatted line to the source file, unless the same line was produced before in this code file.
 \note Resulting line is cropped at 1023 bytes.
 \param[in] format printf-style formatting text, followed by a vararg list
 \return 1 if the text was written to the file, 0 if it was previously written.
 */
int Code_Writer::write_c_once(const char *format, ...) {
  va_list args;
  char buf[1024];
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);
  Fd_Text_Tree **p = &text_in_header;
  while (*p) {
    int i = strcmp(buf,(*p)->text);
    if (!i) return 0;
    else if (i < 0) p = &((*p)->left);
    else p  = &((*p)->right);
  }
  p = &text_in_code;
  while (*p) {
    int i = strcmp(buf,(*p)->text);
    if (!i) return 0;
    else if (i < 0) p = &((*p)->left);
    else p  = &((*p)->right);
  }
  crc_printf("%s\n", buf);
  *p = new Fd_Text_Tree(buf);
  return 1;
}

/**
 Return true if this pointer was already included in the code file.
 If it was not, add it to the list and return false.
 \param[in] pp ay pointer
 \return true if found in the tree, false if added to the tree
 */
bool Code_Writer::c_contains(void *pp) {
  Fd_Pointer_Tree **p = &ptr_in_code;
  while (*p) {
    if ((*p)->ptr == pp) return true;
    else if ((*p)->ptr < pp) p = &((*p)->left);
    else p  = &((*p)->right);
  }
  *p = new Fd_Pointer_Tree(pp);
  return false;
}

/**
 Write a C string to the code file, escaping non-ASCII characters.

 Text is broken into lines of 78 character.
 FLUID " before and after every line text.

 A list of control characters and ", ', and \\ are escaped by adding a \\ in
 front of them. Escape ?? by writing ?\\?. All other characters that are not
 between 32 and 126 inclusive will be escaped as octal characters.

 This function is utf8 agnostic.

 \param[in] s write this string
 \param[in] length write so many bytes in this string

 \see f.write_cstring(const char*)
 */
void Code_Writer::write_cstring(const char *s, int length) {
  const char *next_line = "\"\n\"";
  if (varused_test) {
    varused = 1;
    return;
  }
  // if we are rendering to the source code preview window, and the text is
  // longer than four lines, we only render a placeholder.
  if (write_codeview && ((s==NULL) || (length>300))) {
    if (length>=0)
      crc_printf("\" ... %d bytes of text... \"", length);
    else
      crc_puts("\" ... text... \"");
    return;
  }
  if (length==-1 || s==0L) {
    crc_puts("\n#error  string not found\n");
    crc_puts("\" ... undefined size text... \"");
    return;
  }

  const char *p = s;
  const char *e = s+length;
  int linelength = 1;
  crc_putc('\"');
  for (; p < e;) {
    int c = *p++;
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
      if (linelength >= 77) { crc_puts(next_line); linelength = 0; }
      crc_putc('\\');
      crc_putc(c);
      linelength += 2;
      break;
    case '?': // prevent trigraphs by writing ?? as ?\?
      if (p-2 >= s && *(p-2) == '?') goto QUOTED;
      // else fall through:
    default:
      if (c >= ' ' && c < 127) {
        // a legal ASCII character
        if (linelength >= 78) { crc_puts(next_line); linelength = 0; }
        crc_putc(c);
        linelength++;
        break;
      }
      // if the UTF-8 option is checked, write unicode characters verbatim
      if (proj_.utf8_in_src && (c&0x80)) {
        if ((c&0x40)) {
          // This is the first character in a utf-8 sequence (0b11......).
          // A line break would be ok here. Do not put linebreak in front of
          // following characters (0b10......)
          if (linelength >= 78) { crc_puts(next_line); linelength = 0; }
        }
        crc_putc(c);
        linelength++;
        break;
      }
      // otherwise we must print it as an octal constant:
      c &= 255;
      if (linelength >= 74) { crc_puts(next_line); linelength = 0; }
      crc_printf("\\%03o", c);
      linelength += 4;
      break;
    }
  }
  crc_putc('\"');
}

/**
 Write a C string, escaping non-ASCII characters.
 \param[in] s write this string
 \see f.write_cstring(const char*, int)
 */
void Code_Writer::write_cstring(const char *s) {
  write_cstring(s, (int)strlen(s));
}

/**
 Write an array of C binary data (does not add a null).
 The output is bracketed in { and }. The content is written
 as decimal bytes, i.e. `{ 1, 2, 200 }`

 \param[in] s a block of binary data, interpreted as unsigned bytes
 \param[in] length size of the block in bytes
 */
void Code_Writer::write_cdata(const char *s, int length) {
  if (varused_test) {
    varused = 1;
    return;
  }
  if (write_codeview) {
    if (length>=0)
      crc_printf("{ /* ... %d bytes of binary data... */ }", length);
    else
      crc_puts("{ /* ... binary data... */ }");
    return;
  }
  if (length==-1) {
    crc_puts("\n#error  data not found\n");
    crc_puts("{ /* ... undefined size binary data... */ }");
    return;
  }
  const unsigned char *w = (const unsigned char *)s;
  const unsigned char *e = w+length;
  int linelength = 1;
  crc_putc('{');
  for (; w < e;) {
    unsigned char c = *w++;
    if (c>99) linelength += 4;
    else if (c>9) linelength += 3;
    else linelength += 2;
    if (linelength >= 77) {crc_puts("\n"); linelength = 0;}
    crc_printf("%d", c);
    if (w<e) crc_putc(',');
  }
  crc_putc('}');
}

/**
 Print a formatted line to the source file.
 \param[in] format printf-style formatting text
 \param[in] args list of arguments
 */
void Code_Writer::vwrite_c(const char* format, va_list args) {
  if (varused_test) {
    varused = 1;
    return;
  }
  crc_vprintf(format, args);
}

/**
 Print a formatted line to the source file.
 \param[in] format printf-style formatting text, followed by a vararg list
 */
void Code_Writer::write_c(const char* format,...) {
  va_list args;
  va_start(args, format);
  vwrite_c(format, args);
  va_end(args);
}

/**
 Write code (c) of size (n) to C file, with optional comment (com) w/o trailing space.
 if the code line does not end in a ';' or '}', a ';' will be added.
 \param[in] indent indentation string for all lines
 \param[in] n number of bytes in code line
 \param[in] c line of code
 \param[in] com optional commentary
 */
void Code_Writer::write_cc(const char *indent, int n, const char *c, const char *com) {
  write_c("%s%.*s", indent, n, c);
  char cc = c[n-1];
  if (cc!='}' && cc!=';')
    write_c(";");
  if (*com)
    write_c(" %s", com);
  write_c("\n");
}

/**
 Print a formatted line to the header file.
 \param[in] format printf-style formatting text, followed by a vararg list
 */
void Code_Writer::write_h(const char* format,...) {
  if (varused_test) return;
  va_list args;
  va_start(args, format);
  vfprintf(header_file, format, args);
  va_end(args);
}

/**
 Write code (c) of size (n) to H file, with optional comment (com) w/o trailing space.
 if the code line does not end in a ';' or '}', a ';' will be added.
 \param[in] indent indentation string for all lines
 \param[in] n number of bytes in code line
 \param[in] c line of code
 \param[in] com optional commentary
 */
void Code_Writer::write_hc(const char *indent, int n, const char* c, const char *com) {
  write_h("%s%.*s", indent, n, c);
  char cc = c[n-1];
  if (cc!='}' && cc!=';')
    write_h(";");
  if (*com)
    write_h(" %s", com);
  write_h("\n");
}

/**
 Write one or more lines of code, indenting each one of them.
 \param[in] textlines one or more lines of text, separated by \\n
 \param[in] inIndent increment indentation by this amount
 \param[in] inTrailWith append this character if the last line did not end with
            a newline, usually 0 or newline.
 */
void Code_Writer::write_c_indented(const char *textlines, int inIndent, char inTrailWith) {
  if (textlines) {
    indentation += inIndent;
    for (;;) {
      int line_len;
      const char *newline = strchr(textlines, '\n');
      if (newline)
        line_len = (int)(newline-textlines);
      else
        line_len = (int)strlen(textlines);
      if (textlines[0]=='\n') {
        // avoid trailing spaces
      } else if (textlines[0]=='#') {
        // don't indent preprocessor statments starting with '#'
        write_c("%.*s", line_len, textlines);
      } else {
        // indent all other text lines
        write_c("%s%.*s", indent(), line_len, textlines);
      }
      if (newline) {
        write_c("\n");
      } else {
        if (inTrailWith)
          write_c("%c", inTrailWith);
        break;
      }
      textlines = newline+1;
    }
    indentation -= inIndent;
  }
}

/**
 Return true if the type would be the member of a class.
 Some types are treated differently if they are inside class. Especially within
 a Widget Class, children that are widgets are written as part of the
 constructor whereas functions, declarations, and inline data are seen as
 members of the class itself.
 */
bool is_class_member(Fl_Type *t) {
  return    t->is_a(ID_Function)
         || t->is_a(ID_Decl)
         || t->is_a(ID_Data);
//         || t->is_a(ID_Class)         // FLUID can't handle a class inside a class
//         || t->is_a(ID_Widget_Class)
//         || t->is_a(ID_DeclBlock)     // Declaration blocks are generally not handled well
}

/**
 Return true, if this is a comment, and if it is followed by a class member.
 This must only be called if q is inside a widget class.
 Widget classes can have widgets and members (functions/methods, declarations,
 etc.) intermixed.
 \param[in] q should be a comment type
 \return true if this comment is followed by a class member
 \return false if it is followed by a widget or code
 \see is_class_member(Fl_Type *t)
 */
bool is_comment_before_class_member(Fl_Type *q) {
  if (q->is_a(ID_Comment) && q->next && q->next->level==q->level) {
    if (q->next->is_a(ID_Comment))
      return is_comment_before_class_member(q->next);
    if (is_class_member(q->next))
      return true;
  }
  return false;
}

/**
 Recursively write static code and declarations
 \param[in] p write this type and all its children
 \return pointer to the next sibling
 */
Fl_Type* Code_Writer::write_static(Fl_Type* p) {
  if (write_codeview) p->header_static_start = (int)ftell(header_file);
  if (write_codeview) p->code_static_start = (int)ftell(code_file);
  p->write_static(*this);
  if (write_codeview) p->code_static_end = (int)ftell(code_file);
  if (write_codeview) p->header_static_end = (int)ftell(header_file);

  Fl_Type* q;
  for (q = p->next; q && q->level > p->level;) {
    q = write_static(q);
  }

  p->write_static_after(*this);

  return q;
}

/**
 Recursively write code, putting children between the two parts of the parent code.
 \param[in] p write this type and all its children
 \return pointer to the next sibling
 */
Fl_Type* Code_Writer::write_code(Fl_Type* p) {
  // write all code that comes before the children code
  // (but don't write the last comment until the very end)
  if (!(p==Fl_Type::last && p->is_a(ID_Comment))) {
    if (write_codeview) p->code1_start = (int)ftell(code_file);
    if (write_codeview) p->header1_start = (int)ftell(header_file);
    p->write_code1(*this);
    if (write_codeview) p->code1_end = (int)ftell(code_file);
    if (write_codeview) p->header1_end = (int)ftell(header_file);
  }
  // recursively write the code of all children
  Fl_Type* q;
  if (p->is_widget() && p->is_class()) {
    // Handle widget classes specially
    for (q = p->next; q && q->level > p->level;) {
      // note: maybe declaration blocks should be handled like comments in the context
      if (!is_class_member(q) && !is_comment_before_class_member(q)) {
        q = write_code(q);
      } else {
        int level = q->level;
        do {
          q = q->next;
        } while (q && q->level > level);
      }
    }

    // write all code that come after the children
    if (write_codeview) p->code2_start = (int)ftell(code_file);
    if (write_codeview) p->header2_start = (int)ftell(header_file);
    p->write_code2(*this);
    if (write_codeview) p->code2_end = (int)ftell(code_file);
    if (write_codeview) p->header2_end = (int)ftell(header_file);

    for (q = p->next; q && q->level > p->level;) {
      if (is_class_member(q) || is_comment_before_class_member(q)) {
        q = write_code(q);
      } else {
        int level = q->level;
        do {
          q = q->next;
        } while (q && q->level > level);
      }
    }

    write_h("};\n");
    current_widget_class = 0L;
  } else {
    for (q = p->next; q && q->level > p->level;) q = write_code(q);
    // write all code that come after the children
    if (write_codeview) p->code2_start = (int)ftell(code_file);
    if (write_codeview) p->header2_start = (int)ftell(header_file);
    p->write_code2(*this);
    if (write_codeview) p->code2_end = (int)ftell(code_file);
    if (write_codeview) p->header2_end = (int)ftell(header_file);
  }
  return q;
}

/**
 Write the source and header files for the current design.

 If the files already exist, they will be overwritten.

 \note There is no true error checking here.

 \param[in] s filename of source code file
 \param[in] t filename of the header file
 \return 0 if the operation failed, 1 if it was successful
 */
int Code_Writer::write_code(const char *s, const char *t, bool to_codeview) {
  write_codeview = to_codeview;
  delete id_root; id_root = 0;
  indentation = 0;
  current_class = 0L;
  current_widget_class = 0L;
  if (!s) code_file = stdout;
  else {
    FILE *f = fl_fopen(s, "wb");
    if (!f) return 0;
    code_file = f;
  }
  if (!t) header_file = stdout;
  else {
    FILE *f = fl_fopen(t, "wb");
    if (!f) {fclose(code_file); return 0;}
    header_file = f;
  }
  // Remember the last code file location for MergeBack
  if (s && proj_.write_mergeback_data && !to_codeview) {
    std::string filename = proj_.projectfile_path() + proj_.projectfile_name();
    int i, n = (int)filename.size();
    for (i=0; i<n; i++) if (filename[i]=='\\') filename[i] = '/';
    Fl_Preferences build_records(Fl_Preferences::USER_L, "fltk.org", "fluid-build");
    Fl_Preferences path(build_records, filename.c_str());
    path.set("code", s);
  }
  // if the first entry in the Type tree is a comment, then it is probably
  // a copyright notice. We print that before anything else in the file!
  Fl_Type* first_type = Fl_Type::first;
  if (first_type && first_type->is_a(ID_Comment)) {
    if (write_codeview) {
      first_type->code1_start = first_type->code2_start = (int)ftell(code_file);
      first_type->header1_start = first_type->header2_start = (int)ftell(header_file);
    }
    // it is ok to write non-recursive code here, because comments have no children or code2 blocks
    first_type->write_code1(*this);
    if (write_codeview) {
      first_type->code1_end = first_type->code2_end = (int)ftell(code_file);
      first_type->header1_end = first_type->header2_end = (int)ftell(header_file);
    }
    first_type = first_type->next;
  }

  const char *hdr = "\
// generated by Fast Light User Interface Designer (fluid) version %.4f\n\n";
  fprintf(header_file, hdr, FL_VERSION);
  crc_printf(hdr, FL_VERSION);

  {char define_name[102];
  const char* a = fl_filename_name(t);
  char* b = define_name;
  if (!isalpha(*a)) {*b++ = '_';}
  while (*a) {*b++ = isalnum(*a) ? *a : '_'; a++;}
  *b = 0;
  fprintf(header_file, "#ifndef %s\n", define_name);
  fprintf(header_file, "#define %s\n", define_name);
  }

  if (proj_.avoid_early_includes==0) {
    write_h_once("#include <FL/Fl.H>");
  }
  if (t && proj_.include_H_from_C) {
    if (to_codeview) {
      write_c("#include \"CodeView.h\"\n");
    } else if (proj_.header_file_name[0] == '.' && strchr(proj_.header_file_name.c_str(), '/') == NULL) {
      write_c("#include \"%s\"\n", fl_filename_name(t));
    } else {
      write_c("#include \"%s\"\n", proj_.header_file_name.c_str());
    }
  }
  std::string loc_include, loc_conditional;
  if (proj_.i18n_type==fld::I18n_Type::GNU) {
    loc_include = proj_.i18n_gnu_include;
    loc_conditional = proj_.i18n_gnu_conditional;
  } else {
    loc_include = proj_.i18n_pos_include;
    loc_conditional = proj_.i18n_pos_conditional;
  }
  if ((proj_.i18n_type != fld::I18n_Type::NONE) && !loc_include.empty()) {
    int conditional = !loc_conditional.empty();
    if (conditional) {
      write_c("#ifdef %s\n", loc_conditional.c_str());
      indentation++;
    }
    if (loc_include[0] != '<' && loc_include[0] != '\"')
      write_c("#%sinclude \"%s\"\n", indent(), loc_include.c_str());
    else
      write_c("#%sinclude %s\n", indent(), loc_include.c_str());
    if (proj_.i18n_type == fld::I18n_Type::POSIX) {
      if (!proj_.i18n_pos_file.empty()) {
        write_c("extern nl_catd %s;\n", proj_.i18n_pos_file.c_str());
      } else {
        write_c("// Initialize I18N stuff now for menus...\n");
        write_c("#%sinclude <locale.h>\n", indent());
        write_c("static char *_locale = setlocale(LC_MESSAGES, \"\");\n");
        write_c("static nl_catd _catalog = catopen(\"%s\", 0);\n", proj_.basename().c_str());
      }
    }
    if (conditional) {
      write_c("#else\n");
      if (proj_.i18n_type == fld::I18n_Type::GNU) {
        if (!proj_.i18n_gnu_function.empty()) {
          write_c("#%sifndef %s\n", indent(), proj_.i18n_gnu_function.c_str());
          write_c("#%sdefine %s(text) text\n", indent_plus(1), proj_.i18n_gnu_function.c_str());
          write_c("#%sendif\n", indent());
        }
      }
      if (proj_.i18n_type == fld::I18n_Type::POSIX) {
        write_c("#%sifndef catgets\n", indent());
        write_c("#%sdefine catgets(catalog, set, msgid, text) text\n", indent_plus(1));
        write_c("#%sendif\n", indent());
      }
      indentation--;
      write_c("#endif\n");
    }
    if (proj_.i18n_type == fld::I18n_Type::GNU && proj_.i18n_gnu_static_function[0]) {
      write_c("#ifndef %s\n", proj_.i18n_gnu_static_function.c_str());
      write_c("#%sdefine %s(text) text\n", indent_plus(1), proj_.i18n_gnu_static_function.c_str());
      write_c("#endif\n");
    }
  }
  for (Fl_Type* p = first_type; p;) {
    // write all static data for this & all children first
    write_static(p);
    // then write the nested code:
    p = write_code(p);
  }

  if (!s) return 1;

  fprintf(header_file, "#endif\n");

  Fl_Type* last_type = Fl_Type::last;
  if (last_type && (last_type != Fl_Type::first) && last_type->is_a(ID_Comment)) {
    if (write_codeview) {
      last_type->code1_start = last_type->code2_start = (int)ftell(code_file);
      last_type->header1_start = last_type->header2_start = (int)ftell(header_file);
    }
    last_type->write_code1(*this);
    if (write_codeview) {
      last_type->code1_end = last_type->code2_end = (int)ftell(code_file);
      last_type->header1_end = last_type->header2_end = (int)ftell(header_file);
    }
  }
  int x = 0, y = 0;

  if (code_file != stdout)
    x = fclose(code_file);
  code_file = 0;
  if (header_file != stdout)
    y = fclose(header_file);
  header_file = 0;
  return x >= 0 && y >= 0;
}


/**
 Write the public/private/protected keywords inside the class.
 This avoids repeating these words if the mode is already set.
 \param[in] state 0 for private, 1 for public, 2 for protected
 */
void Code_Writer::write_public(int state) {
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

/**
 Create and initialize a new C++ source code writer.
 */
Code_Writer::Code_Writer(Project &proj)
: proj_ { proj }
{
  block_crc_ = crc32(0, NULL, 0);
}

/**
 Release all resources.
 */
Code_Writer::~Code_Writer()
{
  delete id_root;
  delete ptr_in_code;
  delete text_in_code;
  delete text_in_header;
  if (block_buffer_) ::free(block_buffer_);
}

/**
 Write a MergeBack tag as a separate line of C++ comment.
 The tag contains information about the type of tag that we are writing, a
 link back to the type using its unique id, and the CRC of all code written
 after the previous tag up to this point.
 \param[in] type FD_TAG_GENERIC, FD_TAG_CODE, FD_TAG_MENU_CALLBACK, or FD_TAG_WIDGET_CALLBACK
 \param[in] uid the unique id of the current type
 */
void Code_Writer::tag(int type, unsigned short uid) {
  if (proj_.write_mergeback_data)
    fprintf(code_file, "//~fl~%d~%04x~%08x~~\n", type, (int)uid, (unsigned int)block_crc_);
  block_crc_ = crc32(0, NULL, 0);
}

/**
 Static function to calculate the CRC32 of a block of C source code.
 Calculation of the CRC ignores leading whitespace in a line and all linefeed
 characters ('\\r').
 \param[in] data a pointer to the data block
 \param[in] n the size of the data in bytes, or -1 to use strlen()
 \param[in] in_crc add to this CRC, 0 by default to start a new block
 \param[inout] inout_line_start optional pointer to flag that determines
            if we are the start of a line, used to find leading whitespace
 \return the new CRC
 */
unsigned long Code_Writer::block_crc(const void *data, int n, unsigned long in_crc, bool *inout_line_start) {
  if (!data) return 0;
  if (n==-1) n = (int)strlen((const char*)data);
  bool line_start = true;
  if (inout_line_start) line_start = *inout_line_start;
  const char *s = (const char*)data;
  for ( ; n>0; --n, ++s) {
    if (line_start) {
      // don't count leading spaces and tabs in a line
      while (n>0 && *s>0 && isspace(*s)) { s++; n--; }
      if (*s) line_start = false;
    }
    // don't count '\r' that may be introduced by Windows
    if (n>0 && *s=='\r') { s++; n--; }
    if (n>0 && *s=='\n') line_start = true;
    if (n>0) {
      in_crc = crc32(in_crc, (const Bytef*)s, 1);
    }
  }
  if (inout_line_start) *inout_line_start = line_start;
  return in_crc;
}

/** Add the following block of text to the CRC of this class.
 \param[in] data a pointer to the data block
 \param[in] n the size of the data in bytes, or -1 to use strlen()
 */
void Code_Writer::crc_add(const void *data, int n) {
  block_crc_ = block_crc(data, n, block_crc_, &block_line_start_);
}

/** Write formatted text to the code file.
 If MergeBack is enabled, the CRC calculation is continued.
 \param[in] format printf style formatting string
 \return see fprintf(FILE *, *const char*, ...)
 */
int Code_Writer::crc_printf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  int ret = crc_vprintf(format, args);
  va_end(args);
  return ret;
}

/** Write formatted text to the code file.
 If MergeBack is enabled, the CRC calculation is continued.
 \param[in] format printf style formatting string
 \param[in] args list of arguments
 \return see fprintf(FILE *, *const char*, ...)
 */
int Code_Writer::crc_vprintf(const char *format, va_list args) {
  if (proj_.write_mergeback_data) {
    int n = vsnprintf(block_buffer_, block_buffer_size_, format, args);
    if (n > block_buffer_size_) {
      block_buffer_size_ = n + 128;
      if (block_buffer_) ::free(block_buffer_);
      block_buffer_ = (char*)::malloc(block_buffer_size_+1);
      n = vsnprintf(block_buffer_, block_buffer_size_, format, args);
    }
    crc_add(block_buffer_, n);
    return fputs(block_buffer_, code_file);
  } else {
    return vfprintf(code_file, format, args);
  }
}

/** Write some text to the code file.
 If MergeBack is enabled, the CRC calculation is continued.
 \param[in] text any text, no requirements to end in a newline or such
 \return see fputs(const char*, FILE*)
 */
int Code_Writer::crc_puts(const char *text) {
  if (proj_.write_mergeback_data) {
    crc_add(text);
  }
  return fputs(text, code_file);
}

/** Write a single ASCII character to the code file.
 If MergeBack is enabled, the CRC calculation is continued.
 \note to write UTF-8 characters, use Code_Writer::crc_puts(const char *text)
 \param[in] c any character between 0 and 127 inclusive
 \return see fputc(int, FILE*)
 */
int Code_Writer::crc_putc(int c) {
  if (proj_.write_mergeback_data) {
    uchar uc = (uchar)c;
    crc_add(&uc, 1);
  }
  return fputc(c, code_file);
}

