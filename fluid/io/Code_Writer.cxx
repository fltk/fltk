//
// Fluid C++ Code Writer code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2026 by Bill Spitzak and others.
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
#include "proj/mergeback.h"
#include "nodes/Window_Node.h"
#include "nodes/Function_Node.h"

#include <FL/filename.H>
#include "../../src/flstring.h"

#include <zlib.h>

#include <string>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace fluid;
using namespace fluid::io;
using namespace fluid::proj;

/**
 Return a string with the given value formatted as an 8-digit hexadecimal number.
 \param[in] value a 32-bit unsigned integer
 \return string with the hexadecimal representation of the value
 */
std::string fluid::io::to_string_8x(uint32_t value) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%08x", value);
  return std::string(buf);
}

/**
 Return a string with the given value formatted using the `%g` specifier.
 \param[in] value a double-precision floating-point number
 \return string with the formatted representation of the value
 */
std::string fluid::io::to_string_g(double value) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%g", value);
  return std::string(buf);
}

/**
 Return true if c can be in a C identifier.
 I needed this so it is not messed up by locale settings.
 \param[in] c a character, or the start of a utf-8 sequence
 \return 1 if c is alphanumeric or '_'
 */
int is_id(char c) {
  return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || c=='_';
}

/** \brief Return a unique name for the given object within this file.

 This is used to create unique identifiers for callback functions, menu items,
 and other objects that need a unique name in file scope.

 This function combines the name and label into an identifier. It then checks
 if that id was already taken by another object, and if so, appends a
 hexadecimal value which is incremented until the id is unique in this file.

 If a new id was created, it is stored in the id tree.

 \param[in] o create an ID for this object
 \param[in] type is the first word of the ID
 \param[in] name if name is set, it is appended to the ID
 \param[in] label else if label is set, it is appended, skipping non-keyword characters
 \return a unique identifier for this object and type
 */
std::string Code_Writer::unique_id(void* o, const std::string& type, const std::string& name, const std::string& label) {
  char buffer[128];
  char* q = buffer;
  char* q_end = q + 128 - 8 - 1; // room for hex number and NUL
  const char* type_cstr = type.c_str();
  while (*type_cstr) *q++ = *type_cstr++;
  *q++ = '_';
  const char* n = name.c_str();
  if (name.empty()) n = label.c_str();
  if (n && *n) {
    while (*n && !is_id(*n)) n++;
    while (is_id(*n) && (q < q_end)) *q++ = *n++;
  }
  *q = 0;
  // okay, search the tree and see if the name was already used:
  int which = 0;
  for (;;) {
    auto it = unique_id_list.find(buffer);
    // If the id does not exist, add it to the map
    if (it == unique_id_list.end()) {
      it = unique_id_list.insert(std::make_pair(buffer, o)).first;
      return it->first;
    }
    // If it does exist, and the pointers are the same, just return it.
    if (it->second == o) {
      return it->first;
    }
    // Else repeat until we have a new id,
    sprintf(q,"%x",++which);
  }
}

/**
 Return a C string that indents code to the given depth.

 Indentation can be changed by modifying the multiplicator (``*2`` to keep
 the FLTK indent style). Changing `spaces` to a list of tabs would generate
 tab indents instead. This function can also be used for fixed depth indents
 in the header file.

 Do *not* ever make this a user preference, or you will end up writing a
 fully featured code formatter.

 \param[in] set generate this indent depth
 \return indenting string
 */
std::string Code_Writer::indent(int set) const {
  return std::string(set*2, ' ');
}

/**
 Return a string that indents code to the current source file depth.
 \return indenting string
 */
std::string Code_Writer::indent() const {
  return indent(indentation);
}

/**
 Return a string that indents code to the current source file depth plus an offset.
 \param[in] offset adds a temporary offset for this call only; this does not
    change the `indentation` variable; offset can be negative
 \return indenting string
 */
std::string Code_Writer::indent_plus(int offset) const {
  return indent(indentation+offset);
}


/**
 Write a line of code to the header file unless the same line was written before.
 \param[in] code line of code, a newline character will be appended
 \return 1 if the text was added to the file, 0 if it was previously written.
 */
int Code_Writer::write_h_once(const std::string& code) {
  if (text_in_header.find(code) != text_in_header.end()) {
    return 0;
  }
  header_buffer << code << "\n";
  text_in_header.insert(code);
  return 1;
}


/**
 Write a line of code to the source file unless the same line was written before to the header or source file.
 \param[in] code line of code, a newline character will be appended
 \return 1 if the text was added to the file, 0 if it was previously written.
 */
int Code_Writer::write_c_once(const std::string& code) {
  if (text_in_header.find(code) != text_in_header.end()) {
    return 0;
  }
  if (text_in_code.find(code) != text_in_code.end()) {
    return 0;
  }
  crc_puts(code);
  crc_putc('\n');
  text_in_code.insert(code);
  return 1;
}

/**
 Return true if this pointer was already included in the code file.
 If it was not, add it to the list and return false.
 \param[in] pp any pointer
 \return true if found in the tree, false if added to the tree
 */
bool Code_Writer::c_contains(void *pp) {
  if (ptr_in_code.find(pp) != ptr_in_code.end()) {
    return true;
  }
  ptr_in_code.insert(pp);
  return false;
}

/**
 Write a C string to the code file, escaping non-ASCII characters.

 Text is broken into lines of 78 character.
 Fluid adds quotation marks before and after every line of text.

 A list of control characters and ", ', and \\ are escaped by adding a \\ in
 front of them. Escape ?? by writing ?\\?. All other characters that are not
 between 32 and 126 inclusive will be escaped as octal characters.

 This function is utf8 agnostic.

 \param[in] text write this string
 */
void Code_Writer::write_cstring(fluid::string_view text) {
  const char *next_line = "\"\n\"";
  if (varused_test) {
    varused = 1;
    return;
  }
  // if we are rendering to the source code preview window, and the text is
  // longer than four lines, we only render a placeholder.
  if (write_codeview && (text.empty() || (text.size()>300))) {
    if (text.size()>=0)
      crc_puts("\" ... " + std::to_string(text.size()) + " bytes of text... \"");
    else
      crc_puts("\" ... text... \"");
    return;
  }
  if (text.data() == nullptr) {
    crc_puts("\n#error  string not found\n");
    crc_puts("\" ... undefined size text... \"");
    return;
  }

  const char *p = text.data();
  const char *e = text.data()+text.size();
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
      if ( (p-2 >= text.data()) && (*(p-2) == '?')) {
        // first question mark was alreaady written
        crc_putc('\\');
        crc_putc('?');
        linelength += 2;
        break;
      }
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
      char buf[8];
      snprintf(buf, sizeof(buf), "\\%03o", c);
      crc_puts(buf);
      linelength += 4;
      break;
    }
  }
  crc_putc('\"');
}

/**
 Write an array of C binary data (does not add a null).
 The output is bracketed in { and }. The content is written
 as decimal bytes, i.e. `{ 1, 2, 200 }`

 \param[in] block pointer to a block of binary data, interpreted as unsigned bytes
 */
void Code_Writer::write_cdata(fluid::string_view block) {
  if (varused_test) {
    varused = 1;
    return;
  }
  if (write_codeview) {
    if (!block.empty())
      crc_puts("{ /* ... " + std::to_string(block.size()) + "  bytes of binary data... */ }");
    else
      crc_puts("{ /* ... binary data... */ }");
    return;
  }
  if (block.data() == nullptr) {
    crc_puts("\n#error  data not found\n");
    crc_puts("{ /* ... undefined size binary data... */ }");
    return;
  }
  const unsigned char *w = (const unsigned char *)block.data();
  const unsigned char *e = w+block.size();
  int linelength = 1;
  crc_putc('{');
  for (; w < e;) {
    unsigned char c = *w++;
    if (c>99) linelength += 4;
    else if (c>9) linelength += 3;
    else linelength += 2;
    if (linelength >= 77) {
      crc_puts("\n");
      linelength = 0;
    }
    crc_puts(std::to_string(c));
    if (w<e) crc_putc(',');
  }
  crc_putc('}');
}

/**
 Write code to the source file.
 \param[in] code string containing the code to write
 */
void Code_Writer::write_c(const std::string& code) {
  if (varused_test) {
    varused = 1;
    return;
  }
  crc_puts(code);
}

/**
 Write a line of code to the source file, appending an optional comment.
 if the code line does not end in a ';' or '}', a ';' will be added.
 \param[in] indent indentation string for all lines
 \param[in] code line of code
 \param[in] comment optional commentary
 */
void Code_Writer::write_cc(const std::string& indent, const std::string& code, const std::string& comment) {
  write_c(indent + code);
  char cc = code.back();
  if (cc!='}' && cc!=';')
    write_c(";");
  if (!comment.empty())
    write_c(" " + comment);
  write_c("\n");
}

/**
 Write code to the header file.
 \param[in] code string containing the code to write
 */
void Code_Writer::write_h(const std::string& code) {
  if (varused_test) return;
  header_buffer << code;
}

/**
 Write a line of code to the header file, appending an optional comment.
 if the code line does not end in a ';' or '}', a ';' will be added.
 \param[in] indent indentation string for all lines
 \param[in] code line of code
 \param[in] comment optional commentary
 */
void Code_Writer::write_hc(const std::string& indent, const std::string& code, const std::string& comment) {
  write_h(indent + code);
  char cc = code.back();
  if (cc!='}' && cc!=';')
    write_h(";");
  if (!comment.empty())
    write_h(" " + comment);
  write_h("\n");
}

/**
 Write a block of code, indenting it as needed.
 \param[in] codeblock one or more lines of text, separated by \\n
 \param[in] additional_indent increment indentation by this amount
 \param[in] trail_char append this character if the last line did not end with
            a newline, usually 0 or newline.
 */
void Code_Writer::write_c_indented(const std::string& codeblock, int additional_indent, char trail_char)
{
  if (codeblock.empty()) return;

  std::string trailing_text;
  if (trail_char && codeblock.back() != '\n') {
    trailing_text = std::string(1, trail_char);
  }

  indentation += additional_indent;

  size_t line_start = 0;
  size_t line_end = 0;
  size_t line_length = 0;
  bool done = false;

  while (!done)
  {
    // get the length of the next line.
    line_end = codeblock.find('\n', line_start);
    if (line_end == std::string::npos) {
      line_length = codeblock.length() - line_start;
      done = true;
      if (line_length == 0) break;
    } else {
      line_length = line_end - line_start + 1; // include the newline character
    }

    if (codeblock[line_start] == '\n') {
      // no characters in the line, don't indent
      write_c("\n");
    } else if (codeblock[line_start]=='#') {
      // don't indent preprocessor statments starting with '#'
      write_c(codeblock.substr(line_start, line_length));
    } else {
      // indent all other text lines
      write_c(indent() + codeblock.substr(line_start, line_length));
    }
    line_start = line_end + 1;
  }

  if (!trailing_text.empty()) {
    write_c(trailing_text);
  }

  indentation -= additional_indent;
}

/**
 Return true if the type can be the member of a class.

 Some types are treated differently if they are inside class. Especially within
 a Widget Class, children that are widgets are written as part of the
 constructor whereas functions, declarations, and inline data are seen as
 members of the class itself.
 */
bool is_class_member(Node *t) {
  return    dynamic_cast<Function_Node*>(t)
         || dynamic_cast<Decl_Node*>(t)
         || dynamic_cast<Data_Node*>(t);
//         || dynamic_cast<Class_Node*>(t)          // FLUID can't handle a class inside a class
//         || dynamic_cast<Widget_Class_Node*>(t)   // ???
//         || dynamic_cast<DeclBlock_Node*>(t)      // Declaration blocks are generally not handled well
}

/**
 Return true, if this is a comment, and if it is followed by a class member.

 This must only be called if q is inside a widget class.
 Widget classes can have widgets and members (functions/methods, declarations,
 etc.) intermixed.

 \param[in] q should be a comment type
 \return true if this comment is followed by a class member
 \return false if it is followed by a widget or code
 \see is_class_member(Node *t)
 */
bool is_comment_before_class_member(Node *q) {
  if (dynamic_cast<Comment_Node*>(q) && q->next && q->next->level==q->level) {
    if (dynamic_cast<Comment_Node*>(q->next))
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
Node* Code_Writer::write_static(Node* p) {
  if (write_codeview) p->header_static.start = header_pos();
  if (write_codeview) p->code_static.start = code_pos();
  p->write_static(*this);
  if (write_codeview) p->code_static.end = code_pos();
  if (write_codeview) p->header_static.end = header_pos();

  Node* q;
  for (q = p->next; q && q->level > p->level;) {
    q = write_static(q);
  }

  p->write_static_after(*this);

  return q;
}

/**
 Recursively write code, putting children between the two parts of the parent code.
 \param[in] p write this node and all its children
 \return pointer to the next sibling
 */
Node* Code_Writer::write_code(Node* p) {
  // write all code that comes before the children code
  // (but don't write the last comment until the very end)
  if (!(p==Fluid.proj.tree.last && dynamic_cast<Comment_Node*>(p))) {
    if (write_codeview) p->code1.start = code_pos();
    if (write_codeview) p->header1.start = header_pos();
    p->write_code1(*this);
    if (write_codeview) p->code1.end = code_pos();
    if (write_codeview) p->header1.end = header_pos();
  }
  // recursively write the code of all children
  Node* q;
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
    if (write_codeview) p->code2.start = code_pos();
    if (write_codeview) p->header2.start = header_pos();
    p->write_code2(*this);
    if (write_codeview) p->code2.end = code_pos();
    if (write_codeview) p->header2.end = header_pos();

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
    current_widget_class = nullptr;
  } else {
    for (q = p->next; q && q->level > p->level;) q = write_code(q);
    // write all code that come after the children
    if (write_codeview) p->code2.start = code_pos();
    if (write_codeview) p->header2.start = header_pos();
    p->write_code2(*this);
    if (write_codeview) p->code2.end = code_pos();
    if (write_codeview) p->header2.end = header_pos();
  }
  return q;
}

/**
 Write the source and header files for the current design.

 If the files already exist, they will be overwritten only if the content
 has changed. This conservative approach helps reduce unnecessary recompilation.

 \note There is no true error checking here.

 \param[in] s filename of source code file
 \param[in] t filename of the header file
 \return 0 if the operation failed, 1 if it was successful
 */
int Code_Writer::write_code(const std::string& code_arg, const std::string& header_arg, bool to_codeview) {
  write_codeview = to_codeview;
  unique_id_list.clear();
  indentation = 0;
  current_class = nullptr;
  current_widget_class = nullptr;

  // Always use string stream buffers for output
  code_buffer.str("");
  code_buffer.clear();
  header_buffer.str("");
  header_buffer.clear();

  // Remember the last code file location for MergeBack
  if (!code_arg.empty() && proj_.write_mergeback_data && !to_codeview) {
    std::string filename = proj_.projectfile_path() + proj_.projectfile_name();
    int i, n = (int)filename.size();
    for (i=0; i<n; i++) if (filename[i]=='\\') filename[i] = '/';
    Fl_Preferences build_records(Fl_Preferences::USER_L, "fltk.org", "fluid-build");
    Fl_Preferences path(build_records, filename.c_str());
    path.set("code", code_arg);
  }
  // if the first entry in the Type tree is a comment, then it is probably
  // a copyright notice. We print that before anything else in the file!
  Node* first_node = Fluid.proj.tree.first;
  if (first_node && dynamic_cast<Comment_Node*>(first_node)) {
    if (write_codeview) {
      first_node->code1.start = first_node->code2.start = code_pos();
      first_node->header1.start = first_node->header2.start = header_pos();
    }
    // it is ok to write non-recursive code here, because comments have no children or code2 blocks
    first_node->write_code1(*this);
    if (write_codeview) {
      first_node->code1.end = first_node->code2.end = code_pos();
      first_node->header1.end = first_node->header2.end = header_pos();
    }
    first_node = first_node->next;
  }

  char version[128];
  fl_snprintf(version, sizeof(version),
    "// generated by Fast Light User Interface Designer (fluid) version %.4f\n\n",
    FL_VERSION);
  write_h(std::string(version));
  crc_puts(version);
  {
    // Creating the include guard is more involved than it seems at first glance.
    // The include guard is deduced from header filename. However, if the
    // filename contains unicode characters, they need to be encoded using
    // \Uxxxxxxxx or \\uxxxx encoding to form a valid macro identifier.
    //
    // But that approach is not portable. Windows does not normalize Unicode
    // (ö is the letter \u00F6). macOS normalizes to NFD (ö is \u006F\u0308,
    // o followed by a Combining Diaresis ¨).
    //
    // To make the include guard consistent across l=platforms, it can be
    // explicitly set by the user in the Project Settings.
    std::string macro_name_str = proj_.include_guard;
    if (macro_name_str.empty()) {
      std::ostringstream macro_name;
      std::string header_name;
      const char* a = nullptr;
      if (write_codeview) {
        header_name = proj_.headerfile_name();
        a = header_name.c_str();
      } else {
        a = fl_filename_name(header_arg.c_str());
      }
      const char* b = a + strlen(a);
      int len = 0;
      unsigned ucs = fl_utf8decode(a, b, &len);
      if ((ucs > 127) || (!isalpha(ucs) && (ucs != '_')))
        macro_name << '_';
      while (a < b) {
        ucs = fl_utf8decode(a, b, &len);
        if (ucs > 0x0000ffff) { // large unicode character
          macro_name << "\\U" << std::setw(8) << std::setfill('0') << std::hex << ucs;
        } else if (ucs > 127) { // small unicode character or not an ASCI letter or digit
          macro_name << "\\u" << std::setw(4) << std::setfill('0') << std::hex << ucs;
        } else if (!isalnum(ucs)) {
          macro_name << '_';
        } else {
          macro_name << (char)ucs;
        }
        a += len;
      }
      macro_name_str = macro_name.str();
    }
    write_h("#ifndef " + macro_name_str + "\n");
    write_h("#define " + macro_name_str + "\n");
  }

  if (proj_.avoid_early_includes==0) {
    write_h_once("#include <FL/Fl.H>");
  }
  if (!header_arg.empty() && proj_.include_H_from_C) {
    if (to_codeview) {
      write_c("#include \"CodeView.h\"\n");
    } else if (proj_.header_file_name[0] == '.' && strchr(proj_.header_file_name.c_str(), '/') == nullptr) {
      write_c("#include \"" + fl_filename_name_str(header_arg) + "\"\n");
    } else {
      write_c("#include \"" + proj_.header_file_name + "\"\n");
    }
  }
  std::string loc_include, loc_conditional;
  if (proj_.i18n.type==fluid::I18n_Type::GNU) {
    loc_include = proj_.i18n.gnu_include;
    loc_conditional = proj_.i18n.gnu_conditional;
  } else {
    loc_include = proj_.i18n.posix_include;
    loc_conditional = proj_.i18n.posix_conditional;
  }
  if ((proj_.i18n.type != fluid::I18n_Type::NONE) && !loc_include.empty()) {
    int conditional = !loc_conditional.empty();
    if (conditional) {
      write_c("#ifdef " + loc_conditional + "\n");
      indentation++;
    }
    if (loc_include[0] != '<' && loc_include[0] != '\"')
      write_c("#" + indent() + "include \"" + loc_include + "\"\n");
    else
      write_c("#" + indent() + "include " + loc_include + "\n");
    if (proj_.i18n.type == fluid::I18n_Type::POSIX) {
      if (!proj_.i18n.posix_file.empty()) {
        write_c("extern nl_catd " + proj_.i18n.posix_file + ";\n");
      } else {
        write_c("// Initialize I18N stuff now for menus...\n");
        write_c("#" + indent() + "include <locale.h>\n");
        write_c("static char* _locale = setlocale(LC_MESSAGES, \"\");\n");
        write_c("static nl_catd _catalog = catopen(\"" + proj_.basename() + "\", 0);\n");
      }
    }
    if (conditional) {
      write_c("#else\n");
      if (proj_.i18n.type == fluid::I18n_Type::GNU) {
        if (!proj_.i18n.gnu_function.empty()) {
          write_c("#" + indent() + "ifndef " + proj_.i18n.gnu_function + "\n");
          write_c("#" + indent() + "define " + proj_.i18n.gnu_function + "(text) text\n");
          write_c("#" + indent() + "endif\n");
        }
      }
      if (proj_.i18n.type == fluid::I18n_Type::POSIX) {
        write_c("#" + indent() + "ifndef catgets\n");
        write_c("#" + std::string(indent_plus(1)) + "define catgets(catalog, set, msgid, text) text\n");
        write_c("#" + indent() + "endif\n");
      }
      indentation--;
      write_c("#endif\n");
    }
    if (proj_.i18n.type == fluid::I18n_Type::GNU && proj_.i18n.gnu_static_function[0]) {
      write_c("#ifndef " + proj_.i18n.gnu_static_function + "\n");
      write_c("#" + std::string(indent_plus(1)) + "define " + proj_.i18n.gnu_static_function + "(text) text\n");
      write_c("#endif\n");
    }
  }
  for (Node* p = first_node; p;) {
    // write all static data for this & all children first
    write_static(p);
    // then write the nested code:
    p = write_code(p);
  }

  write_h("#endif\n");

  Node* last_node = Fluid.proj.tree.last;
  if (last_node && (last_node != Fluid.proj.tree.first) && dynamic_cast<Comment_Node*>(last_node)) {
    if (write_codeview) {
      last_node->code1.start = last_node->code2.start = code_pos();
      last_node->header1.start = last_node->header2.start = header_pos();
    }
    last_node->write_code1(*this);
    if (write_codeview) {
      last_node->code1.end = last_node->code2.end = code_pos();
      last_node->header1.end = last_node->header2.end = header_pos();
    }
  }

  // For codeview mode, strings are available via code_string() / header_string()
  if (write_codeview)
    return 1;

  // Write code output: to file if filename provided, to stdout otherwise
  bool code_ok = true;
  if (!code_arg.empty()) {
    code_ok = write_file_if_changed(code_arg, code_buffer.str());
  } else {
    fputs(code_buffer.str().c_str(), stdout);
  }

  // Write header output: to file if filename provided, to stdout otherwise
  bool header_ok = true;
  if (!header_arg.empty()) {
    header_ok = write_file_if_changed(header_arg, header_buffer.str());
  } else {
    fputs(header_buffer.str().c_str(), stdout);
  }

  return code_ok && header_ok ? 1 : 0;
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
}

/**
 Write a MergeBack tag as a separate line of C++ comment.
 The tag contains information about the type of tag that we are writing, a
 link back to the type using its unique id, and the CRC of all code written
 after the previous tag up to this point.
 \param[in] type FD_TAG_GENERIC, FD_TAG_CODE, FD_TAG_MENU_CALLBACK, or FD_TAG_WIDGET_CALLBACK
 \param[in] uid the unique id of the current type
 */
void Code_Writer::tag(proj::Mergeback::Tag prev_type, proj::Mergeback::Tag next_type, unsigned short uid) {
  if (proj_.write_mergeback_data) {
    code_buffer << Mergeback::format_tag(prev_type, next_type, uid, crc_.value());
  }
  crc_.reset();
}

/** Write some text to the code buffer.
 If MergeBack is enabled, the CRC calculation is continued.
 \param[in] text any text, no requirements to end in a newline or such
 \return always 0
 */
int Code_Writer::crc_puts(const std::string& text) {
  if (proj_.write_mergeback_data) {
    crc_.update(text);
  }
  code_buffer << text;
  return 0;
}

/** Write a single ASCII character to the code buffer.
 If MergeBack is enabled, the CRC calculation is continued.
 \note to write UTF-8 characters, use Code_Writer::crc_puts(const char *text)
 \param[in] c any character between 0 and 127 inclusive
 \return the character written
 */
int Code_Writer::crc_putc(int c) {
  if (proj_.write_mergeback_data) {
    char cc = (char)c;
    crc_.update(fluid::string_view((const char*)&cc, 1));
  }
  code_buffer << (char)c;
  return c;
}

/**
 Check if the content of a file matches a given string.
 \param[in] filename path to the file to compare
 \param[in] content the string content to compare with
 \return true if the file exists and its content matches exactly, false otherwise
 */
bool Code_Writer::file_content_matches(const std::string& filename, const std::string &content) {
  FILE *f = fl_fopen(filename.c_str(), "rb");
  if (!f) {
    return false;  // File doesn't exist
  }

  // Get file size
  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return false;  // Seek error
  }
  long file_size = ftell(f);
  if (file_size < 0) {
    fclose(f);
    return false;  // ftell error
  }
  if (fseek(f, 0, SEEK_SET) != 0) {
    fclose(f);
    return false;  // Seek error
  }

  // Quick check: if sizes don't match, content is different
  if ((size_t)file_size != content.size()) {
    fclose(f);
    return false;
  }

  // Read file content and compare
  std::string file_content((size_t)file_size, '\0');
  size_t bytes_read = fread(&file_content[0], 1, (size_t)file_size, f);
  fclose(f);

  if (bytes_read != (size_t)file_size) {
    return false;  // Read error
  }

  return file_content == content;
}

/**
 Write content to a file only if the content differs from the existing file.
 This is a conservative write that avoids unnecessary file modifications,
 which helps reduce recompilation in build systems.
 \param[in] filename path to the file to write
 \param[in] content the string content to write
 \return true if the file was written or already up-to-date, false on write error
 */
bool Code_Writer::write_file_if_changed(const std::string& filename, const std::string &content) {
  // If content matches, no need to write
  if (file_content_matches(filename, content)) {
    return true;  // File is already up-to-date
  }

  // Content differs or file doesn't exist - write the new content
  FILE *f = fl_fopen(filename.c_str(), "wb");
  if (!f) {
    return false;  // Cannot open file for writing
  }

  size_t written = fwrite(content.c_str(), 1, content.size(), f);
  int result = fclose(f);

  return (written == content.size()) && (result == 0);
}


/**
 Add a block of text to the CRC32 calculation.

 Ignore leading whitespace and linefeed characters.
 All spaces and tabs are treated as a single space.
 This is to avoid differences in whitespace affecting the CRC, which can
 happen when code blocks are indented while writing code.

 This class can be used without the Code_Writer class, for example to
 calculate a CRC of a string or file.

 \param block The text to add to the CRC calculation.
 */
void fluid::CRC32::update(fluid::string_view block) {
  const char *s = block.data();
  size_t n = block.size();
  for ( ; n>0; --n, ++s)
  {
    // don't count leading spaces and tabs in a line
    if (line_start_) {
      while (n>0 && fl_ascii_isspace(*s)) { s++; n--; }
      if (n==0) return;
      line_start_ = false;
    }
    if (*s=='\n') line_start_ = true;

    // skip '\r' that may be introduced by Windows
    if (*s=='\r') continue;

    // skip multiple spaces in a row, but only if the previous character was a space
    if (multi_space_) {
      // skip all further spaces
      if (fl_ascii_isspace(*s)) continue;
      multi_space_ = false;
    } else if (fl_ascii_isspace(*s)) {
      multi_space_ = true;
      crc_ = crc32(crc_, (const Bytef*)" ", 1);
      continue;
    }

    crc_ = crc32(crc_, (const Bytef*)s, 1);
  }
}

/**
 Calculate the CRC32 of a block of text.
 This is a convenience function that creates a CRC32 object, adds the block of
 text, and returns the resulting CRC32 value.

 \param block The text to calculate the CRC32 for.
 \return The CRC32 value of the block of text.
 */
uint32_t fluid::CRC32::block(fluid::string_view block) {
  fluid::CRC32 crc;
  crc.update(block);
  return crc.value();
}
