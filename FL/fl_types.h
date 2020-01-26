/*
 * "$Id$"
 *
 * Simple "C"-style types for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2020 by Bill Spitzak and others.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     https://www.fltk.org/COPYING.php
 *
 * Please report all bugs and problems on the following page:
 *
 *     https://www.fltk.org/str.php
 */

/** \file
 *  This file contains simple "C"-style type definitions.
 */

#ifndef FL_TYPES_H
#define FL_TYPES_H

/** \name	Miscellaneous */
/*@{*/	/* group: Miscellaneous */

/** unsigned char */
typedef unsigned char uchar;
/** unsigned long */
typedef unsigned long ulong;

/** Flexible length UTF-8 Unicode text.
 *
 *  \todo FIXME: temporary (?) typedef to mark UTF-8 and Unicode conversions
 */
typedef char *Fl_String;

/** Flexible length UTF-8 Unicode read-only string.
 *  \sa Fl_String
 */
typedef const char *Fl_CString;

/** 16-bit Unicode character + 8-bit indicator for keyboard flags.

  \note This \b should be 24-bit Unicode character + 8-bit indicator for
    keyboard flags. The upper 8 bits are currently unused but reserved.

  Due to compatibility issues this type and all FLTK \b shortcuts can only
  be used with 16-bit Unicode characters (<tt>U+0000 .. U+FFFF</tt>) and
  not with the full range of unicode characters (<tt>U+0000 .. U+10FFFF</tt>).

  This is caused by the bit flags \c FL_SHIFT, \c FL_CTRL, \c FL_ALT, and
  \c FL_META being all in the range <tt>0x010000 .. 0x400000</tt>.

  \todo Discuss and decide whether we can "shift" these special keyboard
    flags to the upper byte to enable full 21-bit Unicode characters
    (<tt>U+0000 .. U+10FFFF</tt>) plus the keyboard indicator bits as this
    was originally intended. This would be possible if we could rely on \b all
    programs being coded with symbolic names and not hard coded bit values.

  \internal Can we do the move for 1.4 or, if not, for any later version
    that is allowed to break the ABI?
*/
typedef unsigned int Fl_Shortcut;

/** 24-bit Unicode character - upper 8 bits are unused */
typedef unsigned int Fl_Char;

/*@}*/	/* group: Miscellaneous */

#endif

/*
 * End of "$Id$".
 */
