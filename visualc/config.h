/* config.h
   This is a replacement for the file ../config.h which is produced by
   the GNU configure script on Unix systems.  Most of these symbols
   cannot be turned on as they will turn on X-specific code.  Some
   should work, however. */

/* Thickness of FL_UP_BOX and FL_DOWN_BOX.  Current 1,2, and 3 are
   supported.  3 is the historic fltk look.  2 looks more like windoze
   (and KDE and Qt).  1 is a plausible future evolution...  Notice
   that this may be simulated at runtime by redefining the boxtypes
   using Fl::set_boxtype() */
#define BORDER_WIDTH 3

/* Do you have OpenGL?
   Set this to 0 if you don't plan to use OpenGL, and fltk will be smaller */
#define HAVE_GL 1
/* Turn this on if your GL card has overlay hardware: */
#define HAVE_GL_OVERLAY 1

/* Turning this on causes fltk to use it's own palette on 8-bit displays.
   Currently the result is similar to the X version with Fl::own_colormap,
   in that it may produce colormap flashing if only one palette is
   supported at a time (I could not test this because my driver supports
   a very large number of palettes!).
   Current bugs:
    Fl::set_color() does not work after first window shown()
    Probably causes colormap flashing.
   Turning this off will save a chunk of code, and it will still "sort of"
   work on an 8-bit display (you get dithering) */
#define USE_COLORMAP 1

/* X specific, leave off: */
#define HAVE_XDBE 0
#define USE_XDBE 0

/* X specific, leave off: */
#define HAVE_OVERLAY 0

/* Byte order of your machine: (not used by win32 code) */
#define WORDS_BIGENDIAN 0

/* Types that are 32 bits and 16 bits long.  Used by fl_draw_image only: */
#define U16 unsigned short
#define U32 unsigned
/* #undef U64 */

/* Where is <dirent.h> (used only by fl_file_chooser and scandir): */
#define HAVE_DIRENT_H 1
#define HAVE_SYS_NDIR_H 0
#define HAVE_SYS_DIR_H 0
#define HAVE_NDIR_H 0
#define HAVE_SCANDIR 0
