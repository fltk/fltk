
Fluid .fd file format version 1.4
=================================

This file explains the history of the Fluid .fd format and faithfully describes
all elements of the format and its caveats.


History
-------

Fluid, the Fast Light User Interface Designer was started in the 1990's loosely
based on 'fdesign', a GUI designer that came with the 'Forms Library', later
'XForms Library'. Fluid's .fd file format was originally compatible with XForms,
but later evolved somewhat ad'hoc to become what it is today.


Basics
======

.fd files describe a hierarchical graphical user interface for the 'fltk'
library. Fluid is a visual editor, storing the user interface description in
.fd files with the ability to create C++ files.

Elements in the tree can be fltk Widgets as well as functional component
like classes, C++ functions, variabes, etc. . Fluid calls all elements
in the hierarchy 'Type'.'

.fd files start with a 'Header', followed by a list of 'Options', followed
by the Type hierarchy 'Tree'. Options and Types are composed of `Words` and
`Strings'.


Line Endings
------------

Although Fluid writes all line endings as '\n', readers should tolerate '\r\n'
MSWindows line endings as well. Except for the header, the Fluid reader does not
differentiate between a line ending and a space character outside of a 'word'.


Unicode
-------

.fd files were originally pure ASCII. Fluid does not handle UTF8 characters in
.fd files in any special way, but it can be expected that the read and write
without issues.


Words
-----

Words are any sequence of ASCII characters. Words are always case-sensitive.

Simple words are composed of a-z, A-Z, 0-9, or _ and are writen verbatim,
followed by a space or newline.

All other ASCII sequences are bracketed between between ‘{‘ and ‘}’ without
spaces. For example, "" is written as ‘{}’, and ".hallo" is written as
`{.hallo}`.

The special characters ‘\’ and ‘#’ are escaped by prepending the ‘\’ character,
so "#define" is written as '{\#define}`.

The characters ‘{‘ and ‘}’ are also escaped with a '\' unless every opening
‘{‘ in the sequnece is matched with a closing ‘}’.

Note: line endings are copied verbatim and become significant within a Word.


Strings
-------

Strings are generated with 'fprintf' statements in the hopes that the generated
text can be read back as one or more word, or by using a corresponding 'fscanf'.

Note: As there are no start and end markers to a string, a reader must know
      when these strings appear and be prepared to read them correctly, even if
      the String itself is discared after reading.


Content
=======


Header
------

The header for any .fd file is

  # data file for the Fltk User Interface Designer (fluid)

followed by a newline, followed by

  version <float v>

wehere 'v' is the version number as in FL_VERSION (major*1.0 + minor * 0.01
+ patch * 0.0001).

Note: the version number corresponds not so much to the version of Fluid, but
      to the version of the underlying fltk library. So unless the version of
      fltk is finalised, the file format in the github master branch can still
      change unexpectedly.

Note: if the version number is above the internal version number, Fluid will
      abort reading the file. There are no other uses inside the Fluid reader
      except for fltk2 which is beyond the scope of thsi document.

Note: fdesign files start with the text "Magic:". Fluid can read thse files,
      but Forms/XFroms file are beyond the scope of this document.


Options
-------

Options are usually composed of a Word, two Words, or a Word and a String. If
an option is missing, a default value is assumed.

  "Magic:" : used by fdesign, not written by Fluid

  "define_in_struct" : no longer used

  "do_not_include_H_from_C" : don’t generate #include “myDesign.h”

  "use_FL_COMMAND" : use macOS CMD key instead of Ctrl

  "utf8_in_src" : allow utf8 when writing source code, otherwise
      escape utf8 sequences

  "avoid_early_includes" : generate the #include “myDesign.h” statement late

  "i18n_type" <int> : 0=default=none, 1=gettext, 2=catgets

  "i18n_function" <str> : e.g. “gettext”

  "i18n_static_function" <str> : e.g. “gettext_noop”

  "i18n_file" <str> :

  "i18n_set" <str> :

  "i18n_include" <str> : e.g. “<libintl.h>”

  "i18n_conditional" <str> :

  "header_name" <str> : can be the full filename, or just the
      extension e.g. “.h” in which case Fluid will use the same filename
      as the .fd file.

  "code_name" <str> : can be the full filename, or just the
      extension e.g. “.cxx”

  "snap" <int> : ignored

  "gridx" <int> : ignored

  "gridy" <int> : ignored

  "win_shell_cmd" <str> : this is the recommended command when calling the
      shell on an MSWindows machine. Implementation may be problematic.

  "win_shell_flags" <int> : interpreted as a bit filed: bit 0 is set if the
      .fl must be save before runing the shell command, bit 1 is set for
      saving the source code, bit 2 is set for saving text strings.

  "linux_shell_cmd" <str> : as above for Linux

  "linux_shell_flags" <int> :  : as above for Linux

  "mac_shell_cmd" <str> : as above for macOS

  "mac_shell_flags" <int> : as above for macOS

Note: There is no keyword that marks the ending of the options section. The
      Tree section starts when a word is not in the options list. If the word
      is not a vaild type, Fluid will give an error message and try to continue
      to read the file.


Tree
====

If a keyword is read that is not in the option list, we start reading types.
Types represent all possible entries in the hierarchy including C functions,
class definitions, and of course all widgets. A type is any of the supported
widgets class names, or one of the following:

Function, code, codeblock, decl, data, declblock, comment, class, widget_class

Every type keyword is followed by a word, which is usually interpreted as the
C++ name of the Type, followed by an opening `{`, a list of properties, and
a closing ‘}’. If the type has children, they are stored between another
opening ‘{‘, followed by a list of types, followed by a closing ‘}’.

The file end when there are no more types.

Note: the "class" type has an additional Word following immediatley after
      the keyword conatining the prefix for the class. So a class definition may
      be written as:

        class FL_EXPORT MyClass { ...properties... } { ...children... }


Types
-----

Type names are based on fltk class names. Types derive properties from super
types loosely similar to fltk.

Note: the hierarchical dependency is implemented twice and somewhat conflicting
      in Fluid via the Fl_..._Type hierarchy, and by using '::is_button()'
      virtual functions, which does not always match the type hierarchy.

The list of know types is:


  Fl_Type (note: can't be written)
   +-- Function
   +-- code
   +-- codeblock
   +-- decl
   +-- data
   +-- declblock
   +-- comment
   +-- class
   +-- Fl_Widget (note: can't be written)
   |    +-- Fl_Window
   |    |    +-- widget_class
   |    +-- Fl_Group
   |    |    +-- Fl_Pack
   |    |    +-- Fl_Flex
   |    |    +-- Fl_Table
   |    |    +-- Fl_Tabs
   |    |    +-- Fl_Scroll
   |    |    +-- Fl_Tile
   |    |    +-- Fl_Wizard
   |    +-- Fl_Menu_Type  (note: can't be written)
   |    |    +-- Fl_Menu_Button
   |    |    +-- Fl_Choice
   |    |    +-- Fl_Input_Choice
   |    |    +-- Fl_Menu_Bar
   |    |    +-- Fl_
   |    +-- Fl_Box
   |    +-- Fl_Button
   |    |    +-- Fl_Return_Button
   |    |    +-- Fl_Light_Button
   |    |    +-- Fl_Check_Button
   |    |    +-- Fl_Round_Button
   |    +-- Fl_Repeat_Button
   |    +-- Fl_Browser
   |    +-- Fl_Check_Browser
   |    +-- Fl_Tree
   |    +-- Fl_File_Browser
   |    +-- Fl_Counter
   |    +-- Fl_Spinner
   |    +-- Fl_Input
   |    |    +-- Fl_Output
   |    +-- Fl_File_Input
   |    +-- Fl_Text_Display
   |    +-- Fl_Text_Editor
   |    |    +-- Fl_Simple_Terminal
   |    +-- Fl_Clock
   |    +-- Fl_Help_View
   |    +-- Fl_Progress
   |    +-- Fl_Adjuster
   |    +-- Fl_Dial
   |    +-- Fl_Roller
   |    +-- Fl_Slider
   |    |    +-- Fl_Scrollbar
   |    |    +-- Fl_Value_Slider
   |    +-- Fl_Value_Input
   |    +-- Fl_Value_Output
   :
   :


Properties
----------

Properties are defined in a quite arbitrary manner and on a per-type basis.
They can be inherited from super types, or be limited by 'is_some_tyep()'
calls. All properties are optional, some are mutually exclusive.


All Types <word>

  “label” <word> : text
  “user_data” <word> : a value or an expression
  “user_data_type” <word> : usually “void*” or “long”
  “callback” <word> : a function name or a function body
  “comment” <word> : one or many lines of text
  “open” : group content visible in the Fluid tree browser
  “selected” : type was selected in tree view


Type "Function" <word> : function signature

  none or "private" or "protected" : for methods in classes, or to mark
      functions static in a file
  “C” : if set, function is extern “C”
  “return_type” <word> : C or C++ type descriptor, can start with “virtual” and/or “static”
  ... : inherit more from Fl_Type

Type codeblock <word> : C++ code, usually an 'if

  "after" <word> : C++ code or comment following the closing '}'
  ... : inherit more from Fl_Type

Type "decl" <word> : C++ code to decalre a variable or cllass member

  none or "public" or "private" or "protected" : for declarations within classes
      defaults to "prvate"
  none or "local" or "global": for declarition in the code body
      defaults to "global"
  ... : inherit more from Fl_Type

Type "data" <word> : C++ variable name

  "filename" <word> : name or path as entered by user, forward slashes
  "textmode" : defaults to binary mode
  ... : inherit more from decl

Type "declblock" <word> : C++ code

  none or "public" or "protected" : defaults to private
  "after" <word> : C++ code or comment following the block
  ... : inherit more from Fl_Type

Type "comment" <word> : comment text

  "in_source" or "not_in_source": default to in_source
  "in_header" or "not_in_header": default to in_header
  ... : inherit more from Fl_Type

Type "class" <word> <word> : prefix, class name

  none or "private" or "protected" : defaults to public
  ":" <word> : name of super class
  ... : inherit more from Fl_Type

Type "Fl_Widget" <word> : C++ variable name

  none or "private" or "protected" : ...
  "xywh" <word> : "{%d %d %d %d}" ...
  "tooltip" <word> : tooltip text
  "image" <word> : image name
  "deimage" <word> : deactivated image name
  "type" <word> : integer
  "box" <word> : text or integer (see fltk boxtypes)
  "down_box" <word> : (is_button() or Fl_Input_choice" or is_menu_button())
      text or integer (see fltk boxtypes)
  "value" <word> : (is_button()) integer
  "value" <word> : (is_valuator(), is_spinner()) double
  "color" <word> :
      If word starts with "0x", the rest of the field is a hex number.
      If two intergers follow, this is color and selection_color (not written).
      If one integer follows, it's the color index.
  "selection_color" <word> : integer color index
  "labeltype" <word> :
      If the word is "image", TBD.
      Or one of "NORMAL_LABEL", "SHADOW_LABEL", "ENGRAVED_LABEL",
      "EMBOSSED_LABEL", or "NO_LABEL"
  "labelfont" <word> : intger
  "labelsize" <word> : intger
  "labelcolor" <word> : intger
  "align" <word> : intger
  "when" <word> : intger
  "minimum" <word> : (is_valuator(), is_spinner()) double
  "maximum" <word> : (is_valuator(), is_spinner()) double
  "step" <word> : (is_valuator(), is_spinner()) double
  "slider_size" <word> : (is_valuator()==2) double
  "size" <word> : (is_valuator()==2) double
  "textfont" <word> : integer
  "textsize" <word> : integer
  "textcolor" <word> : integer
  "hide" : default visible
  "deactivate" : default active
  "resizable" : default fixed
  "hotspot" : make hotspot
  "divider" : make hotspot
  "class" <word> : superclass
  "shortcut" <word> : integer
  "code0" or "code1" or "code2" or "code3" <word> : C++ extra code lines
  "extra_code" <word> : C++ extra code lines
    Fl_Type::read_property(c);
  ... : inherit more from Fl_Group

Type "Fl_Flex" <word> : C++ variable name

  "margins" <word> : this word is written with fprintf as "{%d %d %d %d}",
      note no spaces after { or before }, left, top, right, bottom
  "gap" <word> : integer
  "set_size_tuples" <word> : this word is written fprintf "{%d", where %d
      encodes the number of tuples to follow, and zero or more " %d %d"
      containing the index and size of that child, followed by a '}'.
      Note no spaces after { or before }.
  ... : inherit more from Fl_Group

Type "Fl_Window" <word> : C++ variable name

  none or "modal", or "non_modal": defaults to not modal (which is
      different to non_modal!)
  "visible" : show window when opening file in FLuid
  "noborder" : borderless window
  "xclass" <word> : see fltk
  "size_range" : this word is written with fprintf as "{%d %d %d %d}",
      note no spaces after { or before }, min_w, min_h, max_w, max_h
  "xywh" <word> : this word is written with fprintf as "{%d %d %d %d}",
      note no spaces after { or before }, x, y, w, h. This as actually
      read in Fl_Widget Type, but here it ensures that window is
      not a subwindow.





