//
// "$Id$"
//
// File chooser test program.
//
// Copyright 1999-2010 by Michael Sweet.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//
// Contents:
//
//   main()           - Create a file chooser and wait for a selection to
//                      be made.
//   close_callback() - Close the main window...
//   fc_callback()    - Handle choices in the file chooser...
//   pdf_check()      - Check for and load the first page of a PDF file.
//   ps_check()       - Check for and load the first page of a PostScript
//                      file.
//   show_callback()  - Show the file chooser...
//
//   extra_callback() - circle extra groups (none,group1,check_button);
//

//
// Include necessary headers...
//

#include <stdio.h>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_File_Icon.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_PNM_Image.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Double_Window.H>
#include <string.h>


//
// Globals...
//

Fl_Input		*filter;
Fl_File_Browser		*files;
Fl_File_Chooser		*fc;
Fl_Shared_Image		*image = 0;

// for choosing extra groups
Fl_Choice *ch_extra;
// first extra group
Fl_Group *encodings = (Fl_Group*)0;
Fl_Choice *ch_enc;
// second extra widget
Fl_Check_Button *version = (Fl_Check_Button*)0;

//
// Functions...
//

void		close_callback(void);
void		create_callback(void);
void		dir_callback(void);
void		fc_callback(Fl_File_Chooser *, void *);
void		multi_callback(void);
Fl_Image	*pdf_check(const char *, uchar *, int);
Fl_Image	*ps_check(const char *, uchar *, int);
void		show_callback(void);

void		extra_callback(Fl_Choice*,void*);

//
// 'main()' - Create a file chooser and wait for a selection to be made.
//

int			// O - Exit status
main(int  argc,		// I - Number of command-line arguments
     char *argv[])	// I - Command-line arguments
{
  Fl_Double_Window	*window;// Main window
  Fl_Button		*button;// Buttons
  Fl_File_Icon		*icon;	// New file icon


  // Make the file chooser...
  Fl::scheme(NULL);
  Fl_File_Icon::load_system_icons();

  fc = new Fl_File_Chooser(".", "*", Fl_File_Chooser::SINGLE, "Fl_File_Chooser Test");
  fc->callback(fc_callback);

  // Register the PS and PDF image types...
  Fl_Shared_Image::add_handler(pdf_check);
  Fl_Shared_Image::add_handler(ps_check);

  // Make the main window...
  window = new Fl_Double_Window(400, 215, "File Chooser Test");

  filter = new Fl_Input(50, 10, 315, 25, "Filter:");
  int argn = 1;
#ifdef __APPLE__
  // OS X may add the process number as the first argument - ignore
  if (argc>argn && strncmp(argv[1], "-psn_", 5)==0)
    argn++;
#endif
  if (argc > argn)
    filter->value(argv[argn]);
  else
    filter->value("PDF Files (*.pdf)\t"
                  "PostScript Files (*.ps)\t"
		  "Image Files (*.{bmp,gif,jpg,png})\t"
		  "C/C++ Source Files (*.{c,C,cc,cpp,cxx})");

  button = new Fl_Button(365, 10, 25, 25);
  button->labelcolor(FL_YELLOW);
  button->callback((Fl_Callback *)show_callback);

  icon   = Fl_File_Icon::find(".", Fl_File_Icon::DIRECTORY);
  icon->label(button);

  button = new Fl_Light_Button(50, 45, 80, 25, "MULTI");
  button->callback((Fl_Callback *)multi_callback);

  button = new Fl_Light_Button(140, 45, 90, 25, "CREATE");
  button->callback((Fl_Callback *)create_callback);

  button = new Fl_Light_Button(240, 45, 115, 25, "DIRECTORY");
  button->callback((Fl_Callback *)dir_callback);

  //
  ch_extra = new Fl_Choice(150, 75, 150, 25, "Extra Group:");
  ch_extra->add("none|encodings group|check button");
  ch_extra->value(0);
  ch_extra->callback((Fl_Callback *)extra_callback);
  //
  files = new Fl_File_Browser(50, 105, 340, 75, "Files:");
  files->align(FL_ALIGN_LEFT);

  button = new Fl_Button(340, 185, 50, 25, "Close");
  button->callback((Fl_Callback *)close_callback);

  window->resizable(files);
  window->end();
  window->show(1, argv);

  Fl::run();

  return (0);
}


void
extra_callback(Fl_Choice*w,void*)
{
  int val=w->value();
  if (0 == val) fc->add_extra(NULL);
  else if (1 == val) {
    if(!encodings){
      encodings=new Fl_Group(0,0,254,30);
      ch_enc=new Fl_Choice(152,2,100,25,"Choose Encoding:");
      ch_enc->add("ASCII|Koi8-r|win1251|Utf-8");
      encodings->end();
    }
    fc->add_extra(encodings);
  } else {
    if (!version) {
      version = new Fl_Check_Button(5,0,200,25,"Save binary 1.0 version");
    }
    fc->add_extra(version);
  }
}


//
// 'close_callback()' - Close the main window...
//

void
close_callback(void)
{
  exit(0);
}


//
// 'create_callback()' - Handle clicks on the create button.
//

void
create_callback(void)
{
  fc->type(fc->type() ^ Fl_File_Chooser::CREATE);
}


//
// 'dir_callback()' - Handle clicks on the directory button.
//

void
dir_callback(void)
{
  fc->type(fc->type() ^ Fl_File_Chooser::DIRECTORY);
}


//
// 'fc_callback()' - Handle choices in the file chooser...
//

void
fc_callback(Fl_File_Chooser *fc,	// I - File chooser
            void            *data)	// I - Data
{
  const char		*filename;	// Current filename


  printf("fc_callback(fc = %p, data = %p)\n", fc, data);

  filename = fc->value();

  printf("    filename = \"%s\"\n", filename ? filename : "(null)");
}


//
// 'multi_callback()' - Handle clicks on the multi button.
//

void
multi_callback(void)
{
  fc->type(fc->type() ^ Fl_File_Chooser::MULTI);
}


//
// 'pdf_check()' - Check for and load the first page of a PDF file.
//

Fl_Image *			// O - Page image or NULL
pdf_check(const char *name,	// I - Name of file
          uchar      *header,	// I - Header data
	  int)			// I - Length of header data (unused)
{
  const char	*home;		// Home directory
  char		preview[FL_PATH_MAX],	// Preview filename
		command[3 * FL_PATH_MAX]; // Command


  if (memcmp(header, "%PDF", 4) != 0)
    return 0;

  home = getenv("HOME");
  sprintf(preview, "%s/.preview.ppm", home ? home : "");

  sprintf(command,
          "gs -r100 -dFIXED -sDEVICE=ppmraw -dQUIET -dNOPAUSE -dBATCH "
	  "-sstdout=\"%%stderr\" -sOUTPUTFILE=\'%s\' "
	  "-dFirstPage=1 -dLastPage=1 \'%s\' 2>/dev/null", preview, name);

  if (system(command)) return 0;

  return new Fl_PNM_Image(preview);
}


//
// 'ps_check()' - Check for and load the first page of a PostScript file.
//

Fl_Image *			// O - Page image or NULL
ps_check(const char *name,	// I - Name of file
         uchar      *header,	// I - Header data
	 int)			// I - Length of header data (unused)
{
  const char	*home;		// Home directory
  char		preview[FL_PATH_MAX],	// Preview filename
		outname[FL_PATH_MAX],	// Preview PS file
		command[3 * FL_PATH_MAX]; // Command
  FILE		*in,		// Input file
		*out;		// Output file
  int		page;		// Current page
  char		line[256];	// Line from file


  if (memcmp(header, "%!", 2) != 0)
    return 0;

  home = getenv("HOME");
  sprintf(preview, "%s/.preview.ppm", home ? home : "");

  if (memcmp(header, "%!PS", 4) == 0) {
    // PS file has DSC comments; extract the first page...
    sprintf(outname, "%s/.preview.ps", home ? home : "");

    if (strcmp(name, outname) != 0) {
      in   = fl_fopen(name, "rb");
      out  = fl_fopen(outname, "wb");
      page = 0;

      while (fgets(line, sizeof(line), in) != NULL) {
	if (strncmp(line, "%%Page:", 7) == 0) {
          page ++;
	  if (page > 1) break;
	}

	fputs(line, out);
      }

      fclose(in);
      fclose(out);
    }
  } else {
    // PS file doesn't have DSC comments; do the whole file...
    strncpy(outname, name, sizeof(outname) - 1);
    outname[sizeof(outname) - 1] = '\0';
  }

  sprintf(command,
          "gs -r100 -dFIXED -sDEVICE=ppmraw -dQUIET -dNOPAUSE -dBATCH "
	  "-sstdout=\"%%stderr\" -sOUTPUTFILE=\'%s\' \'%s\' 2>/dev/null",
	  preview, outname);

  if (system(command)) return 0;

  return new Fl_PNM_Image(preview);
}


//
// 'show_callback()' - Show the file chooser...
//

void
show_callback(void)
{
  int	i;			// Looping var
  int	count;			// Number of files selected
  char	relative[FL_PATH_MAX];	// Relative filename


  if (filter->value()[0])
    fc->filter(filter->value());

  fc->show();

  while (fc->visible()) {
    Fl::wait();
  }

  count = fc->count();
  if (count > 0)
  {
    files->clear();

    for (i = 1; i <= count; i ++)
    {
      if (!fc->value(i))
        break;

      fl_filename_relative(relative, sizeof(relative), fc->value(i));

      files->add(relative,
                 Fl_File_Icon::find(fc->value(i), Fl_File_Icon::PLAIN));
    }

    files->redraw();
  }
}


//
// End of "$Id$".
//
