//
// "$Id: help.cxx,v 1.1.2.1 2001/08/02 19:43:49 easysw Exp $"
//
// Fl_HelpDialog test program.
//
// Copyright 1999-2001 by Easy Software Products.
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
// Contents:
//
//   main() - Display the help GUI...
//

//
// Include necessary headers...
//

#include <FL/Fl_HelpDialog.H>


//
// 'main()' - Display the help GUI...
//

int				// O - Exit status
main(int  argc,			// I - Number of command-line arguments
     char *argv[])		// I - Command-line arguments
{
  Fl_HelpDialog	*help;		// Help dialog


  help = new Fl_HelpDialog;

  if (argc < 2)
    help->load("../documentation/index.html");
  else
    help->load(argv[1]);

  help->show();

  Fl::run();

  delete help;

  return (0);
}


//
// End of "$Id: help.cxx,v 1.1.2.1 2001/08/02 19:43:49 easysw Exp $".
//
