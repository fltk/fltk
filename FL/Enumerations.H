//
// "$Id: Enumerations.H,v 1.18.2.14 2001/01/22 15:13:37 easysw Exp $"
//
// Enumerations for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Enumerations_H
#define Fl_Enumerations_H

//
// The following is only used when building DLLs under WIN32 with Visual C++...
//

#if defined(FL_DLL) && defined(_MSC_VER)
#  ifdef FL_LIBRARY
#    define FL_EXPORT	__declspec(dllexport)
#  else
#    define FL_EXPORT	__declspec(dllimport)
#  endif /* FL_LIBRARY */
#else
#  define FL_EXPORT
#endif /* FL_DLL && _MSC_VER */

//
// The FLTK version number; this is changed slightly from the beta versions
// because the old "const double" definition would not allow for conditional
// compilation...
//
// FL_VERSION is a double that describes the major and minor version numbers.
// Version 1.1 is actually stored as 1.01 to allow for more than 9 minor
// releases.
//
// The FL_MAJOR_VERSION, FL_MINOR_VERSION, and FL_PATCH_VERSION constants
// give the integral values for the major, minor, and patch releases
// respectively.
//

#define FL_MAJOR_VERSION	1
#define FL_MINOR_VERSION	0
#define FL_PATCH_VERSION	11
#define FL_VERSION		((double)FL_MAJOR_VERSION + \
				 (double)FL_MINOR_VERSION * 0.01 + \
				 (double)FL_PATCH_VERSION * 0.0001)

typedef unsigned char uchar;
typedef unsigned long ulong;

enum Fl_Event {	// events
  FL_NO_EVENT		= 0,
  FL_PUSH		= 1,
  FL_RELEASE		= 2,
  FL_ENTER		= 3,
  FL_LEAVE		= 4,
  FL_DRAG		= 5,
  FL_FOCUS		= 6,
  FL_UNFOCUS		= 7,
  FL_KEYBOARD		= 8,
  FL_CLOSE		= 9,
  FL_MOVE		= 10,
  FL_SHORTCUT		= 11,
  FL_DEACTIVATE		= 13,
  FL_ACTIVATE		= 14,
  FL_HIDE		= 15,
  FL_SHOW		= 16,
  FL_PASTE		= 17,
  FL_SELECTIONCLEAR	= 18
};

enum Fl_When { // Fl_Widget::when():
  FL_WHEN_NEVER		= 0,
  FL_WHEN_CHANGED	= 1,
  FL_WHEN_RELEASE	= 4,
  FL_WHEN_RELEASE_ALWAYS= 6,
  FL_WHEN_ENTER_KEY	= 8,
  FL_WHEN_ENTER_KEY_ALWAYS=10,
  FL_WHEN_ENTER_KEY_CHANGED=11,
  FL_WHEN_NOT_CHANGED	= 2 // modifier bit to disable changed() test
};

// Fl::event_key() and Fl::get_key(n) (use ascii letters for all other keys):
#define FL_Button	0xfee8 // use Fl_Button+FL_*_MOUSE
#define FL_BackSpace	0xff08
#define FL_Tab		0xff09
#define FL_Enter	0xff0d
#define FL_Pause	0xff13
#define FL_Scroll_Lock	0xff14
#define FL_Escape	0xff1b
#define FL_Home		0xff50
#define FL_Left		0xff51
#define FL_Up		0xff52
#define FL_Right	0xff53
#define FL_Down		0xff54
#define FL_Page_Up	0xff55
#define FL_Page_Down	0xff56
#define FL_End		0xff57
#define FL_Print	0xff61
#define FL_Insert	0xff63
#define FL_Menu		0xff67 // the "menu/apps" key on XFree86
#define FL_Num_Lock	0xff7f
#define FL_KP		0xff80 // use FL_KP+'x' for 'x' on numeric keypad
#define FL_KP_Enter	0xff8d // same as Fl_KP+'\r'
#define FL_KP_Last	0xffbd // use to range-check keypad
#define FL_F		0xffbd // use FL_F+n for function key n
#define FL_F_Last	0xffe0 // use to range-check function keys
#define FL_Shift_L	0xffe1
#define FL_Shift_R	0xffe2
#define FL_Control_L	0xffe3
#define FL_Control_R	0xffe4
#define FL_Caps_Lock	0xffe5
#define FL_Meta_L	0xffe7 // the left MSWindows key on XFree86
#define FL_Meta_R	0xffe8 // the right MSWindows key on XFree86
#define FL_Alt_L	0xffe9
#define FL_Alt_R	0xffea
#define FL_Delete	0xffff

// Fl::event_button():
#define FL_LEFT_MOUSE	1
#define FL_MIDDLE_MOUSE	2
#define FL_RIGHT_MOUSE	3

// Fl::event_state():
#define FL_SHIFT	0x00010000
#define FL_CAPS_LOCK	0x00020000
#define FL_CTRL		0x00040000
#define FL_ALT		0x00080000
#define FL_NUM_LOCK	0x00100000 // most X servers do this?
#define FL_META		0x00400000 // correct for XFree86
#define FL_SCROLL_LOCK	0x00800000 // correct for XFree86
#define FL_BUTTON1	0x01000000
#define FL_BUTTON2	0x02000000
#define FL_BUTTON3	0x04000000

enum Fl_Boxtype { // boxtypes (if you change these you must fix fl_boxtype.C):
  FL_NO_BOX = 0,	FL_FLAT_BOX,

  FL_UP_BOX,		FL_DOWN_BOX,
  FL_UP_FRAME,		FL_DOWN_FRAME,
  FL_THIN_UP_BOX,	FL_THIN_DOWN_BOX,
  FL_THIN_UP_FRAME,	FL_THIN_DOWN_FRAME,
  FL_ENGRAVED_BOX,	FL_EMBOSSED_BOX,
  FL_ENGRAVED_FRAME,	FL_EMBOSSED_FRAME,
  FL_BORDER_BOX,	_FL_SHADOW_BOX,
  FL_BORDER_FRAME,	_FL_SHADOW_FRAME,
  _FL_ROUNDED_BOX,	_FL_RSHADOW_BOX,
  _FL_ROUNDED_FRAME,	_FL_RFLAT_BOX,
  _FL_ROUND_UP_BOX,	_FL_ROUND_DOWN_BOX,
  _FL_DIAMOND_UP_BOX,	_FL_DIAMOND_DOWN_BOX,
  _FL_OVAL_BOX,		_FL_OSHADOW_BOX,
  _FL_OVAL_FRAME,	_FL_OFLAT_BOX,
  FL_FREE_BOXTYPE
};
extern FL_EXPORT Fl_Boxtype define_FL_ROUND_UP_BOX();
#define FL_ROUND_UP_BOX define_FL_ROUND_UP_BOX()
#define FL_ROUND_DOWN_BOX (Fl_Boxtype)(define_FL_ROUND_UP_BOX()+1)
extern FL_EXPORT Fl_Boxtype define_FL_SHADOW_BOX();
#define FL_SHADOW_BOX define_FL_SHADOW_BOX()
#define FL_SHADOW_FRAME (Fl_Boxtype)(define_FL_SHADOW_BOX()+2)
extern FL_EXPORT Fl_Boxtype define_FL_ROUNDED_BOX();
#define FL_ROUNDED_BOX define_FL_ROUNDED_BOX()
#define FL_ROUNDED_FRAME (Fl_Boxtype)(define_FL_ROUNDED_BOX()+2)
extern FL_EXPORT Fl_Boxtype define_FL_RFLAT_BOX();
#define FL_RFLAT_BOX define_FL_RFLAT_BOX()
extern FL_EXPORT Fl_Boxtype define_FL_RSHADOW_BOX();
#define FL_RSHADOW_BOX define_FL_RSHADOW_BOX()
extern FL_EXPORT Fl_Boxtype define_FL_DIAMOND_BOX();
#define FL_DIAMOND_UP_BOX define_FL_DIAMOND_BOX()
#define FL_DIAMOND_DOWN_BOX (Fl_Boxtype)(define_FL_DIAMOND_BOX()+1)
extern FL_EXPORT Fl_Boxtype define_FL_OVAL_BOX();
#define FL_OVAL_BOX define_FL_OVAL_BOX()
#define FL_OSHADOW_BOX (Fl_Boxtype)(define_FL_OVAL_BOX()+1)
#define FL_OVAL_FRAME (Fl_Boxtype)(define_FL_OVAL_BOX()+2)
#define FL_OFLAT_BOX (Fl_Boxtype)(define_FL_OVAL_BOX()+3)

// conversions of box types to other boxtypes:
inline Fl_Boxtype down(Fl_Boxtype b) {return (Fl_Boxtype)(b|1);}
inline Fl_Boxtype frame(Fl_Boxtype b) {return (Fl_Boxtype)(b|2);}

// back-compatability box types:
#define FL_FRAME FL_ENGRAVED_FRAME
#define FL_FRAME_BOX FL_ENGRAVED_BOX
#define FL_CIRCLE_BOX FL_ROUND_DOWN_BOX
#define FL_DIAMOND_BOX FL_DIAMOND_DOWN_BOX

enum Fl_Labeltype {	// labeltypes:
  FL_NORMAL_LABEL	= 0,
  FL_NO_LABEL,
  _FL_SYMBOL_LABEL,
  _FL_SHADOW_LABEL,
  _FL_ENGRAVED_LABEL,
  _FL_EMBOSSED_LABEL,
  _FL_BITMAP_LABEL,
  _FL_PIXMAP_LABEL,
  _FL_IMAGE_LABEL,
  _FL_MULTI_LABEL,
  FL_FREE_LABELTYPE
};
extern Fl_Labeltype FL_EXPORT define_FL_SYMBOL_LABEL();
#define FL_SYMBOL_LABEL define_FL_SYMBOL_LABEL()
extern Fl_Labeltype FL_EXPORT define_FL_SHADOW_LABEL();
#define FL_SHADOW_LABEL define_FL_SHADOW_LABEL()
extern Fl_Labeltype FL_EXPORT define_FL_ENGRAVED_LABEL();
#define FL_ENGRAVED_LABEL define_FL_ENGRAVED_LABEL()
extern Fl_Labeltype FL_EXPORT define_FL_EMBOSSED_LABEL();
#define FL_EMBOSSED_LABEL define_FL_EMBOSSED_LABEL()

enum Fl_Align {	// align() values
  FL_ALIGN_CENTER	= 0,
  FL_ALIGN_TOP		= 1,
  FL_ALIGN_BOTTOM	= 2,
  FL_ALIGN_LEFT		= 4,
  FL_ALIGN_RIGHT	= 8,
  FL_ALIGN_INSIDE	= 16,
  FL_ALIGN_CLIP		= 64,
  FL_ALIGN_WRAP		= 128,
  FL_ALIGN_TOP_LEFT	= FL_ALIGN_TOP | FL_ALIGN_LEFT,
  FL_ALIGN_TOP_RIGHT	= FL_ALIGN_TOP | FL_ALIGN_RIGHT,
  FL_ALIGN_BOTTOM_LEFT	= FL_ALIGN_BOTTOM | FL_ALIGN_LEFT,
  FL_ALIGN_BOTTOM_RIGHT	= FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT,
  FL_ALIGN_LEFT_TOP	= FL_ALIGN_TOP_LEFT,
  FL_ALIGN_RIGHT_TOP	= FL_ALIGN_TOP_RIGHT,
  FL_ALIGN_LEFT_BOTTOM	= FL_ALIGN_BOTTOM_LEFT,
  FL_ALIGN_RIGHT_BOTTOM	= FL_ALIGN_BOTTOM_RIGHT,
  FL_ALIGN_NOWRAP	= 0 // for back compatability
};

enum Fl_Font {	// standard fonts
  FL_HELVETICA		= 0,
  FL_HELVETICA_BOLD,
  FL_HELVETICA_ITALIC,
  FL_HELVETICA_BOLD_ITALIC,
  FL_COURIER,
  FL_COURIER_BOLD,
  FL_COURIER_ITALIC,
  FL_COURIER_BOLD_ITALIC,
  FL_TIMES,
  FL_TIMES_BOLD,
  FL_TIMES_ITALIC,
  FL_TIMES_BOLD_ITALIC,
  FL_SYMBOL,
  FL_SCREEN,
  FL_SCREEN_BOLD,
  FL_ZAPF_DINGBATS,

  FL_FREE_FONT		= 16,	// first one to allocate
  FL_BOLD		= 1,	// add this to helvetica, courier, or times
  FL_ITALIC		= 2	// add this to helvetica, courier, or times
};

extern FL_EXPORT int FL_NORMAL_SIZE;

enum Fl_Color {	// standard colors
  FL_BLACK		= 0,
  FL_RED		= 1,
  FL_GREEN		= 2,
  FL_YELLOW		= 3,
  FL_BLUE		= 4,
  FL_MAGENTA		= 5,
  FL_CYAN		= 6,
  FL_WHITE		= 7,
  FL_INACTIVE_COLOR	= 8,
  FL_SELECTION_COLOR	= 15,

  FL_FREE_COLOR		= 16,
  FL_NUM_FREE_COLOR	= 16,

  FL_GRAY_RAMP		= 32,

  // boxtypes limit themselves to these colors so whole ramp is not allocated:
  FL_GRAY0		= 32,	// 'A'
  FL_DARK3		= 39,	// 'H'
  FL_DARK2		= 45,   // 'N'
  FL_DARK1		= 47,	// 'P'
  FL_GRAY		= 49,	// 'R' default color
  FL_LIGHT1		= 50,	// 'S'
  FL_LIGHT2		= 52,	// 'U'
  FL_LIGHT3		= 54,	// 'W'

  FL_COLOR_CUBE		= 56
};

FL_EXPORT Fl_Color inactive(Fl_Color c);
FL_EXPORT Fl_Color contrast(Fl_Color fg, Fl_Color bg);
FL_EXPORT Fl_Color fl_color_average(Fl_Color c1, Fl_Color c2, float weight);
inline Fl_Color fl_lighter(Fl_Color c) { return fl_color_average(c, FL_WHITE, .67f); }
inline Fl_Color fl_darker(Fl_Color c) { return fl_color_average(c, FL_BLACK, .67f); }

#define FL_NUM_GRAY	24
inline Fl_Color fl_gray_ramp(int i) {return (Fl_Color)(i+FL_GRAY_RAMP);}
#define FL_NUM_RED	5
#define FL_NUM_GREEN	8
#define FL_NUM_BLUE	5
inline Fl_Color fl_color_cube(int r, int g, int b) {
  return (Fl_Color)((b*FL_NUM_RED + r) * FL_NUM_GREEN + g + FL_COLOR_CUBE);}

enum Fl_Cursor {	// standard cursors
  FL_CURSOR_DEFAULT	= 0,
  FL_CURSOR_ARROW	= 35,
  FL_CURSOR_CROSS	= 66,
  FL_CURSOR_WAIT	= 76,
  FL_CURSOR_INSERT	= 77,
  FL_CURSOR_HAND	= 31,
  FL_CURSOR_HELP	= 47,
  FL_CURSOR_MOVE	= 27,
  // fltk provides bitmaps for these:
  FL_CURSOR_NS		= 78,
  FL_CURSOR_WE		= 79,
  FL_CURSOR_NWSE	= 80,
  FL_CURSOR_NESW	= 81,
  FL_CURSOR_NONE	= 255,
  // for back compatability (non MSWindows ones):
  FL_CURSOR_N		= 70,
  FL_CURSOR_NE		= 69,
  FL_CURSOR_E		= 49,
  FL_CURSOR_SE		= 8,
  FL_CURSOR_S		= 9,
  FL_CURSOR_SW		= 7,
  FL_CURSOR_W		= 36,
  FL_CURSOR_NW		= 68
  //FL_CURSOR_NS	= 22,
  //FL_CURSOR_WE	= 55,
};

enum { // values for "when" passed to Fl::add_fd()
  FL_READ = 1,
  FL_WRITE = 4,
  FL_EXCEPT = 8
};

enum Fl_Mode { // visual types and Fl_Gl_Window::mode() (values match Glut)
  FL_RGB	= 0,
  FL_INDEX	= 1,
  FL_SINGLE	= 0,
  FL_DOUBLE	= 2,
  FL_ACCUM	= 4,
  FL_ALPHA	= 8,
  FL_DEPTH	= 16,
  FL_STENCIL	= 32,
  FL_RGB8	= 64,
  FL_MULTISAMPLE= 128
};

// damage masks

enum Fl_Damage {
  FL_DAMAGE_CHILD    = 0x01,
  FL_DAMAGE_EXPOSE   = 0x02,
  FL_DAMAGE_SCROLL   = 0x04,
  FL_DAMAGE_OVERLAY  = 0x08,
  FL_DAMAGE_ALL      = 0x80
};

#endif

//
// End of "$Id: Enumerations.H,v 1.18.2.14 2001/01/22 15:13:37 easysw Exp $".
//
