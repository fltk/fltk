/*
 * "$Id: Fl_Export.H,v 1.3.2.4 2002/07/11 22:43:52 matthiaswm Exp $"
 *
 * WIN32 DLL export definitions for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2002 by Bill Spitzak and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "fltk-bugs@fltk.org".
 */

#ifndef Fl_Export_H
#  define Fl_Export_H

/*
 * The following is only used when building DLLs under WIN32 with Visual C++
 * or CodeWarrior...
 */

#  if defined(FL_DLL) && ( defined(_MSC_VER) || defined(__MWERKS__) )
#    ifdef FL_LIBRARY
#      define FL_EXPORT	__declspec(dllexport)
#    else
#      define FL_EXPORT	__declspec(dllimport)
#    endif /* FL_LIBRARY */
#  else
#    define FL_EXPORT
#  endif /* FL_DLL && ( _MSC_VER || __MWERKS__ ) */

#endif /* !Fl_Export_H */

/*
 * End of "$Id: Fl_Export.H,v 1.3.2.4 2002/07/11 22:43:52 matthiaswm Exp $".
 */
