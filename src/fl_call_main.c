/*
 * "$Id: fl_call_main.c,v 1.1.2.1 1999/03/29 17:39:29 carl Exp $"
 *
 * Copyright 1998-1999 by Bill Spitzak and others.
 *
 * fl_call_main() calls main() for you Windows people.  Needs to be done in C
 * because Borland C++ won't let you call main() from C++.
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
 * Please report all bugs and problems to "fltk-bugs@easysw.com".
 */

#if defined(WIN32) && !defined(FL_DLL)
extern int main(int, char *[]);
extern int  __argc;
extern char **__argv;

int fl_call_main() {
  return main(__argc, __argv);
}
#endif

/*
 * End of "$Id: fl_call_main.c,v 1.1.2.1 1999/03/29 17:39:29 carl Exp $".
 */

