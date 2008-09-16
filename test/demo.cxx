//
// "$Id$"
//
// Main demo program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2008 by Bill Spitzak and others.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(WIN32) && !defined(__CYGWIN__)
#  include <direct.h>
#  ifndef __WATCOMC__
// Visual C++ 2005 incorrectly displays a warning about the use of POSIX APIs
// on Windows, which is supposed to be POSIX compliant...
#    define chdir _chdir
#    define putenv _putenv
#  endif // !__WATCOMC__
#else
#  include <unistd.h>
#endif
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/filename.H>
#include <FL/x.H>

/* The form description */

void doexit(Fl_Widget *, void *);
void doback(Fl_Widget *, void *);
void dobut(Fl_Widget *, long);
void doscheme(Fl_Choice *c, void *) {
  Fl::scheme(c->text(c->value()));
}

Fl_Window *form;
Fl_Button *but[9];

void create_the_forms() {
  Fl_Widget *obj;
  form = new Fl_Window(350, 440);
  obj = new Fl_Box(FL_FRAME_BOX,10,385,330,40,"FLTK Demonstration");
  obj->color(FL_GRAY-4);
  obj->labelsize(24);
  obj->labelfont(FL_BOLD);
  obj->labeltype(FL_ENGRAVED_LABEL);
  obj = new Fl_Box(FL_FRAME_BOX,10,45,330,330,0);
  obj->color(FL_GRAY-8);
  obj = new Fl_Button(280,10,60,25,"Exit");
  obj->callback(doexit);
  Fl_Choice *choice = new Fl_Choice(75, 10, 100, 25, "Scheme:");
  choice->labelfont(FL_HELVETICA_BOLD);
  choice->add("none");
  choice->add("gtk+");
  choice->add("plastic");
  choice->callback((Fl_Callback *)doscheme);
  Fl::scheme(NULL);
  if (!Fl::scheme()) choice->value(0);
  else if (!strcmp(Fl::scheme(), "gtk+")) choice->value(1);
  else choice->value(2);
  obj = new Fl_Button(10,45,330,380); obj->type(FL_HIDDEN_BUTTON);
  obj->callback(doback);
  obj = but[0] = new Fl_Button(30,265,90,90);
  obj = but[1] = new Fl_Button(130,265,90,90);
  obj = but[2] = new Fl_Button(230,265,90,90);
  obj = but[5] = new Fl_Button(230,165,90,90);
  obj = but[4] = new Fl_Button(130,165,90,90);
  obj = but[3] = new Fl_Button(30,165,90,90);
  obj = but[6] = new Fl_Button(30,65,90,90);
  obj = but[7] = new Fl_Button(130,65,90,90);
  obj = but[8] = new Fl_Button(230,65,90,90);
  for (int i=0; i<9; i++) {
    but[i]->align(FL_ALIGN_WRAP);
    but[i]->callback(dobut, i);
  }
  form->forms_end();
}

/* Maintaining and building up the menus. */

typedef struct {
  char name[64];
  int numb;
  char iname[9][64];
  char icommand[9][64];
} MENU;

#define MAXMENU	32

MENU menus[MAXMENU];
int mennumb = 0;

int find_menu(const char* nnn)
/* Returns the number of a given menu name. */
{
  int i;
  for (i=0; i<mennumb; i++)
    if (strcmp(menus[i].name,nnn) == 0) return i;
  return -1;
}

void create_menu(const char* nnn)
/* Creates a new menu with name nnn */
{
  if (mennumb == MAXMENU -1) return;
  strcpy(menus[mennumb].name,nnn);
  menus[mennumb].numb = 0;
  mennumb++;
}

void addto_menu(const char* men, const char* item, const char* comm)
/* Adds an item to a menu */
{
  int n = find_menu(men);
  if (n<0) { create_menu(men); n = find_menu(men); }
  if (menus[n].numb == 9) return;
  strcpy(menus[n].iname[menus[n].numb],item);
  strcpy(menus[n].icommand[menus[n].numb],comm);
  menus[n].numb++;
}

/* Button to Item conversion and back. */

int b2n[][9] = { 
	{ -1, -1, -1, -1,  0, -1, -1, -1, -1},
	{ -1, -1, -1,  0, -1,  1, -1, -1, -1},
	{  0, -1, -1, -1,  1, -1, -1, -1,  2},
	{  0, -1,  1, -1, -1, -1,  2, -1,  3},
	{  0, -1,  1, -1,  2, -1,  3, -1,  4},
	{  0, -1,  1,  2, -1,  3,  4, -1,  5},
	{  0, -1,  1,  2,  3,  4,  5, -1,  6},
	{  0,  1,  2,  3, -1,  4,  5,  6,  7},
	{  0,  1,  2,  3,  4,  5,  6,  7,  8}
  };
int n2b[][9] = { 
	{  4, -1, -1, -1, -1, -1, -1, -1, -1},
	{  3,  5, -1, -1, -1, -1, -1, -1, -1},
	{  0,  4,  8, -1, -1, -1, -1, -1, -1},
	{  0,  2,  6,  8, -1, -1, -1, -1, -1},
	{  0,  2,  4,  6,  8, -1, -1, -1, -1},
	{  0,  2,  3,  5,  6,  8, -1, -1, -1},
	{  0,  2,  3,  4,  5,  6,  8, -1, -1},
	{  0,  1,  2,  3,  5,  6,  7,  8, -1},
	{  0,  1,  2,  3,  4,  5,  6,  7,  8}
  };

int but2numb(int bnumb, int maxnumb)
/* Transforms a button number to an item number when there are
   maxnumb items in total. -1 if the button should not exist. */
 { return b2n[maxnumb][bnumb]; }

int numb2but(int inumb, int maxnumb)
/* Transforms an item number to a button number when there are
   maxnumb items in total. -1 if the item should not exist. */
 { return n2b[maxnumb][inumb]; }

/* Pushing and Popping menus */

char stack[64][32];
int stsize = 0;

void push_menu(const char* nnn)
/* Pushes a menu to be visible */
{
  int n,i,bn;
  int men = find_menu(nnn);
  if (men < 0) return;
  n = menus[men].numb;
  for (i=0; i<9; i++) but[i]->hide();
  for (i=0; i<n; i++)
  {
    bn = numb2but(i,n-1);
    but[bn]->show();
    but[bn]->label(menus[men].iname[i]);
    if (menus[men].icommand[i][0] != '@') but[bn]->tooltip(menus[men].icommand[i]);
    else but[bn]->tooltip(0);
  }
  if (stack[stsize]!=nnn)
    strcpy(stack[stsize],nnn);
  stsize++;
}

void pop_menu()
/* Pops a menu */
{
  if (stsize<=1) return;
  stsize -= 2;
  push_menu(stack[stsize]);
}

/* The callback Routines */

void dobut(Fl_Widget *, long arg)
/* handles a button push */
{
  int men = find_menu(stack[stsize-1]);
  int n = menus[men].numb;
  int bn = but2numb( (int) arg, n-1);
  if (menus[men].icommand[bn][0] == '@')
    push_menu(menus[men].icommand[bn]);
  else {

#ifdef WIN32
    STARTUPINFO		suInfo;		// Process startup information
    PROCESS_INFORMATION	prInfo;		// Process information

    memset(&suInfo, 0, sizeof(suInfo));
    suInfo.cb = sizeof(suInfo);

    int icommand_length = strlen(menus[men].icommand[bn]);

    char* copy_of_icommand = new char[icommand_length+1];
    strcpy(copy_of_icommand,menus[men].icommand[bn]);

    // On WIN32 the .exe suffix needs to be appended to the command
    // whilst leaving any additional parameters unchanged - this
    // is required to handle the correct conversion of cases such as : 
    // `../fluid/fluid valuators.fl' to '../fluid/fluid.exe valuators.fl'.

    // skip leading spaces.
    char* start_command = copy_of_icommand;
    while(*start_command == ' ') ++start_command;

    // find the space between the command and parameters if one exists.
    char* start_parameters = strchr(start_command,' ');

    char* command = new char[icommand_length+6]; // 6 for extra 'd.exe\0'

    if (start_parameters==NULL) { // no parameters required.
#  ifdef _DEBUG
      sprintf(command, "%sd.exe", start_command);
#  else
      sprintf(command, "%s.exe", start_command);
#  endif // _DEBUG
    } else { // parameters required.
      // break the start_command at the intermediate space between
      // start_command and start_parameters.
      *start_parameters = 0;
      // move start_paremeters to skip over the intermediate space.
      ++start_parameters;

#  ifdef _DEBUG
      sprintf(command, "%sd.exe %s", start_command, start_parameters);
#  else
      sprintf(command, "%s.exe %s", start_command, start_parameters);
#  endif // _DEBUG
    }

    CreateProcess(NULL, command, NULL, NULL, FALSE,
                  NORMAL_PRIORITY_CLASS, NULL, NULL, &suInfo, &prInfo);
	
    delete[] command;
    delete[] copy_of_icommand;

#elif defined USING_XCODE

    int icommand_length = strlen(menus[men].icommand[bn]);
    char* command = new char[icommand_length+24]; // extraspace for  'open ../../(name).app &\0' 
    
    sprintf(command, "open ../../../%s.app &", menus[men].icommand[bn]);
    system(command);
    
    delete[] command;

#else // NON WIN32 systems.

    int icommand_length = strlen(menus[men].icommand[bn]);
    char* command = new char[icommand_length+5]; // 5 for extra './' and ' &\0' 

    sprintf(command, "./%s &", menus[men].icommand[bn]);
    system(command);

    delete[] command;
#endif // WIN32
  }
}

void doback(Fl_Widget *, void *) {pop_menu();}

void doexit(Fl_Widget *, void *) {exit(0);}

int load_the_menu(const char* fname)
/* Loads the menu file. Returns whether successful. */
{
  FILE *fin;
  char line[256], mname[64],iname[64],cname[64];
  int i,j;
  fin = fopen(fname,"r");
  if (fin == NULL)
  {
#if defined ( __APPLE__ )
	// mac os bundle menu detection:
	  char* pos = strrchr(fname,'/');
	  if (!pos) return 0;
	  *pos='\0';
	  pos = strrchr(fname,'/');
	  if (!pos) return 0;
	  strcpy(pos,"/Resources/demo.menu");
	  fin  = fopen(fname,"r");
	  if (fin == NULL)
#endif
	  return 0;
  }
  for (;;) {
    if (fgets(line,256,fin) == NULL) break;
    // remove all carriage returns that Cygwin may have inserted
    char *s = line, *d = line;
    for (;;++d) {
      while (*s=='\r') s++;
      *d = *s++;
      if (!*d) break;
    }
    // interprete the line
    j = 0; i = 0;
    while (line[i] == ' ' || line[i] == '\t') i++;
    if (line[i] == '\n') continue;
    if (line[i] == '#') continue;
    while (line[i] != ':' && line[i] != '\n') mname[j++] = line[i++];
    mname[j] = '\0';
    if (line[i] == ':') i++;
    j = 0; 
    while (line[i] != ':' && line[i] != '\n')
    {
      if (line[i] == '\\') {
	i++;
	if (line[i] == 'n') iname[j++] = '\n';
	else iname[j++] = line[i];
	i++;
      } else
        iname[j++] = line[i++];
    }
    iname[j] = '\0';
    if (line[i] == ':') i++;
    j = 0;
    while (line[i] != ':' && line[i] != '\n') cname[j++] = line[i++];
    cname[j] = '\0';
    addto_menu(mname,iname,cname);
  }
  fclose(fin);
  return 1;
}

int main(int argc, char **argv) {
  putenv((char *)"FLTK_DOCDIR=../documentation");
  char buf[256];
  strcpy(buf, argv[0]);
#if ( defined _MSC_VER || defined __MWERKS__ ) && defined _DEBUG
  // MS_VisualC appends a 'd' to debugging executables. remove it.
  fl_filename_setext( buf, "" );
  buf[ strlen(buf)-1 ] = 0;
#endif
  fl_filename_setext(buf,".menu");
  const char *fname = buf;
  int i = 0;
  if (!Fl::args(argc,argv,i) || i < argc-1)
    Fl::fatal("Usage: %s <switches> <menufile>\n%s",argv[0],Fl::help);
  if (i < argc) fname = argv[i];

  create_the_forms();

  if (!load_the_menu(fname)) Fl::fatal("Can't open %s",fname);
  if (buf!=fname)
    strcpy(buf,fname);
  const char *c = fl_filename_name(buf);
  if (c > buf) {buf[c-buf] = 0; chdir(buf);}
  push_menu("@main");
  form->show(argc,argv);
  Fl::run();
  return 0;
}

//
// End of "$Id$".
//

