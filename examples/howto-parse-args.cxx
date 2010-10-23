//
// "$Id:$"
//
//     How to parse command line arguments - Duncan Gibson 2010-10-23
//     First posted in http://www.fltk.org/newsgroups.php?gfltk.general+v:31449
//
//     Shows how to decode additional command line arguments using Fl::args()
//     on top of the "standard" options used by the toolkit itself.
//
// Copyright 1998-2010 by Bill Spitzak and others.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include "stdio.h"
#include "string.h"

int helpFlag = 0;
char *optionString = 0;

int arg(int argc, char **argv, int &i)
{
  if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
    helpFlag = 1;
    i += 1;
    return 1;
  }
  if (strcmp("-o", argv[i]) == 0 || strcmp("--option", argv[i]) == 0) {
    if (i < argc-1 && argv[i+1] != 0) {
      optionString = argv[i+1];
      i += 2;
      return 2;
    }
  }
  return 0;
}

int main(int argc, char** argv)
{
  int i = 1;
  if (Fl::args(argc, argv, i, arg) < argc)
    // note the concatenated strings to give a single format string!
    Fl::fatal("error: unknown option: %s\n"
              "usage: %s [options]\n"
              " -h | --help     : print extended help message\n"
              " -o | --option # : example option with parameter\n"
              " plus standard fltk options\n",
              argv[i], argv[0]);
  if (helpFlag)
    Fl::fatal("usage: %s [options]\n"
              " -h | --help     : print extended help message\n"
              " -o | --option # : example option with parameter\n"
              " plus standard fltk options:\n"
              "%s\n",
              argv[0], Fl::help);

  Fl_Window* mainWin = new Fl_Window(300, 200);
  Fl_Box* textBox = new Fl_Box(0, 0, 300, 200);
  if (optionString != 0)
    textBox->label(optionString);
  mainWin->show(argc, argv);
  return Fl::run();
}

//
// End of "$Id:$".
//
