//
// "$Id$"
//
// Fl_Help_Dialog test program.
//
// Copyright 1999-2010 by Easy Software Products.
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
// Contents:
//
//   main() - Display the help GUI...
//

//
// Include necessary headers...
//

#include <FL/Fl_Help_Dialog.H>


#ifdef USING_XCODE
#include <ApplicationServices/ApplicationServices.h>
void set_app_dir() {
  char app_path[2048];
  CFBundleRef app = CFBundleGetMainBundle();
  CFURLRef url = CFBundleCopyBundleURL(app);    
  CFStringRef cc_app_path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
  CFStringGetCString(cc_app_path, app_path, 2048, kCFStringEncodingUTF8);
  if (*app_path) {
    char *n = strrchr(app_path, '/');
    if (n) {
      *n = 0;
      chdir(app_path);
    }
  }
}
#endif


//
// 'main()' - Display the help GUI...
//

int				// O - Exit status
main(int  argc,			// I - Number of command-line arguments
     char *argv[])		// I - Command-line arguments
{
  Fl_Help_Dialog	*help;		// Help dialog


  help = new Fl_Help_Dialog;

  int argn = 1;
  
#ifdef USING_XCODE
  
  if (argc>argn && strncmp(argv[1], "-psn_", 5)==0)
    argn++;
  set_app_dir();
  
  if (argc <= argn)
    help->load("../../../../documentation/html/intro.html");
  else
    help->load(argv[argn]);
  
#else
  
  if (argc <= argn)
    help->load("../documentation/html/intro.html");
  else
    help->load(argv[1]);
  
#endif
  
  help->show(1, argv);

  Fl::run();

  delete help;

  return (0);
}


//
// End of "$Id$".
//
