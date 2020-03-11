//
// "$Id$"
//
// Main demo program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2020 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     https://www.fltk.org/str.php
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/filename.H>
#include <FL/platform.H>

/* Define a macro to decide whether a trailing 'd' needs to be removed
   from the executable file name. Previous versions of Visual Studio
   added a 'd' to the executable file name ('demod.exe') in Debug
   configurations that needed to be removed.
   This is no longer true with CMake-generated IDE's since FLTK 1.4.
   Just in case we add it again: leave macro DEBUG_EXE_WITH_D defined
   and leave the code using this macro as-is.
*/

// #if defined(_MSC_VER) && defined(_DEBUG) // Visual Studio in Debug mode
// # define DEBUG_EXE_WITH_D 1
// #else
# define DEBUG_EXE_WITH_D 0
// #endif

/* The form description */

void doexit(Fl_Widget *, void *);
void doback(Fl_Widget *, void *);
void dobut(Fl_Widget *, long);
void doscheme(Fl_Choice *c, void *) {
  Fl::scheme(c->text(c->value()));
}

Fl_Double_Window *form;
Fl_Button *but[9];

void create_the_forms() {
  Fl_Widget *obj;
  form = new Fl_Double_Window(350, 440);
  obj = new Fl_Box(FL_FRAME_BOX,10,15,330,40,"FLTK Demonstration");
  obj->color(FL_GRAY-4);
  obj->labelsize(24);
  obj->labelfont(FL_BOLD);
  obj->labeltype(FL_ENGRAVED_LABEL);
  obj = new Fl_Box(FL_FRAME_BOX,10,65,330,330,0);
  obj->color(FL_GRAY-8);
  obj = new Fl_Button(280,405,60,25,"Exit");
  obj->callback(doexit);
  Fl_Choice *choice = new Fl_Choice(75, 405, 100, 25, "Scheme:");
  choice->labelfont(FL_HELVETICA_BOLD);
  choice->add("none");
  choice->add("gtk+");
  choice->add("gleam");
  choice->add("plastic");
  choice->callback((Fl_Callback *)doscheme);
  Fl::scheme(NULL);
  if (!Fl::scheme()) choice->value(0);
  else if (!strcmp(Fl::scheme(), "gtk+")) choice->value(1);
  else if (!strcmp(Fl::scheme(), "gleam")) choice->value(2);
  else if (!strcmp(Fl::scheme(), "plastic")) choice->value(3);
  else choice->value(0);
  obj = new Fl_Button(10,15,330,380); obj->type(FL_HIDDEN_BUTTON);
  obj->callback(doback);
  obj = but[0] = new Fl_Button( 30, 85,90,90);
  obj = but[1] = new Fl_Button(130, 85,90,90);
  obj = but[2] = new Fl_Button(230, 85,90,90);
  obj = but[3] = new Fl_Button( 30,185,90,90);
  obj = but[4] = new Fl_Button(130,185,90,90);
  obj = but[5] = new Fl_Button(230,185,90,90);
  obj = but[6] = new Fl_Button( 30,285,90,90);
  obj = but[7] = new Fl_Button(130,285,90,90);
  obj = but[8] = new Fl_Button(230,285,90,90);
  for (int i=0; i<9; i++) {
    but[i]->align(FL_ALIGN_WRAP);
    but[i]->callback(dobut, i);
  }
  form->end();
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

/* Return the number of a given menu name. */
int find_menu(const char* nnn) {
  int i;
  for (i=0; i<mennumb; i++)
    if (strcmp(menus[i].name,nnn) == 0) return i;
  return -1;
}

/* Create a new menu with name nnn */
void create_menu(const char* nnn) {
  if (mennumb == MAXMENU -1) return;
  strcpy(menus[mennumb].name,nnn);
  menus[mennumb].numb = 0;
  mennumb++;
}

/* Add an item to a menu */
void addto_menu(const char* men, const char* item, const char* comm) {
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

/* Transform a button number to an item number when there are
  maxnumb items in total. -1 if the button should not exist. */
int but2numb(int bnumb, int maxnumb)
{ return b2n[maxnumb][bnumb]; }

/* Transform an item number to a button number when there are
  maxnumb items in total. -1 if the item should not exist. */
int numb2but(int inumb, int maxnumb)
{ return n2b[maxnumb][inumb]; }

/* Pushing and Popping menus */

char stack[64][32];
int stsize = 0;

/* Push a menu to be visible */
void push_menu(const char* nnn) {
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

/* Pop a menu */
void pop_menu() {
  if (stsize<=1) return;
  stsize -= 2;
  push_menu(stack[stsize]);
}

/* The callback Routines */

/* Handle a button push */
void dobut(Fl_Widget *, long arg) {
  int men = find_menu(stack[stsize-1]);
  int n = menus[men].numb;
  int bn = but2numb( (int) arg, n-1);
  if (menus[men].icommand[bn][0] == '@') {
    push_menu(menus[men].icommand[bn]);
  } else {

#ifdef _WIN32

    STARTUPINFO		suInfo;		// Process startup information
    PROCESS_INFORMATION	prInfo;		// Process information

# if DEBUG_EXE_WITH_D
    const char *exe = "d.exe";		// exe name with trailing 'd'
# else
    const char *exe = ".exe";		// exe name w/o trailing 'd'
# endif

    memset(&suInfo, 0, sizeof(suInfo));
    suInfo.cb = sizeof(suInfo);
    
    int icommand_length = strlen(menus[men].icommand[bn]);
    
    char* copy_of_icommand = new char[icommand_length+1];
    strcpy(copy_of_icommand,menus[men].icommand[bn]);
    
    // On Windows the .exe suffix needs to be appended to the command
    // whilst leaving any additional parameters unchanged - this
    // is required to handle the correct conversion of cases such as : 
    // `../fluid/fluid valuators.fl' to '../fluid/fluid.exe valuators.fl'.

    // skip leading spaces.
    char* start_command = copy_of_icommand;
    while (*start_command == ' ') ++start_command;

    // find the space between the command and parameters if one exists.
    char* start_parameters = strchr(start_command,' ');

    char* command = new char[icommand_length+6]; // 6 for extra 'd.exe\0'

    if (start_parameters==NULL) { // no parameters required.
      sprintf(command, "%s%s", start_command, exe);
    } else { // parameters required.
      // break the start_command at the intermediate space between
      // start_command and start_parameters.
      *start_parameters = 0;
      // move start_paremeters to skip over the intermediate space.
      ++start_parameters;
      
      sprintf(command, "%s%s %s", start_command, exe, start_parameters);
    }
    
    CreateProcess(NULL, command, NULL, NULL, FALSE,
                  NORMAL_PRIORITY_CLASS, NULL, NULL, &suInfo, &prInfo);
    
    delete[] command;
    delete[] copy_of_icommand;
    
#elif defined __APPLE__
    /*
     Starting with version 1.4.0, FLTK uses CMake as the only supported build
     system. On macOS, the app developer is expected to run CMake in a
     directory named './build/Xcode' or './build/Makefiles' to generate the
     build environment.

     When building FLTK in the next step, teh macOS app bundles are then
     stored in either:
     './build/Xcode/bin/examples/hello.app/' for Makefiles
     './build/Xcode/bin/examples/Debug/hello.app/' for XCode Debug
     or
     './build/Xcode/bin/examples/Release/hello.app/' as a symbolic
     into the Archive system of macOS

     'Demo' needs to find and run all of these app bundles, some requiring
     an additional file name and path for resource files.

     This is my attempt to find the bundles and resources so that Demo.app
     and all its dependencies will run without any further configuration.
     They will stop running however if any of the bundles or rresources
     are moved.
     */
    {
      char src_path[PATH_MAX];
      char app_path[PATH_MAX];
      char app_name[PATH_MAX];
      char command[2*PATH_MAX+2];
      char *cmd = strdup(menus[men].icommand[bn]);
      char *args = strchr(cmd, ' ');

      /*
       Get the path to the source code at compile time. This is where the other
       resources are located.
       */
      strcpy(src_path, __FILE__);
      char *src_path_end = (char*)fl_filename_name(src_path);
      if (src_path_end) *src_path_end = 0;

      /*
       All example app bundles are in the same directory as 'Demo', so set the
       current dir to the location of Demo.app .

       Starting with macOS 10.12, the actual location of the app has a randomized
       path to fix a vulnerability. This still works in Debug mode which is
       */
      {
        app_path[0] = 0;
        CFBundleRef app = CFBundleGetMainBundle();
        CFURLRef url = CFBundleCopyBundleURL(app);
        CFStringRef cc_app_path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
        CFRelease(url);
        CFStringGetCString(cc_app_path, app_path, 2048, kCFStringEncodingUTF8);
        CFRelease(cc_app_path);
        if (app_path[0]) {
          char *app_path_end = (char*)fl_filename_name(app_path);
          if (app_path_end) *app_path_end = 0;
          fl_chdir(app_path);
        }
      }

      // extract the executable name from the command in the menu file
      strcpy(app_name, cmd);
      // remove any additioanl command line arguments
      if (args) app_name[args-cmd] = 0;
      // make the file name into a bundle name
      strcat(app_name, ".app");

      if (args) {
        if (strcmp(app_name, "../fluid/fluid.app")==0) {
          // CMake -G 'Unix Makefiles' ... : ./bin/fluid.app
          // CMake -G 'Xcode' ... : ./bin/Debug/fluid.app or ./bin/Release/fluid.app
          // so removing the '/example' path segment from the app_path should
          // always work.
          char *examples = strstr(app_path, "/examples");
          if (examples) {
            memmove(examples, examples+9, strlen(examples+9)+1);
          }
          sprintf(command, "open '%sfluid.app' --args '%s%s'", app_path, src_path, args+1);
        } else {
          // we assume that we have only one argument which is a filename, so we add a path
          sprintf(command, "open '%s%s' --args '%s%s'", app_path, app_name, src_path, args+1);
        }
      } else {
        sprintf(command, "open '%s%s'", app_path, app_name);
      }
      system(command);
      free(cmd);
    }

#else // Non Windows systems.

    int icommand_length = strlen(menus[men].icommand[bn]);
    char* command = new char[icommand_length+5]; // 5 for extra './' and ' &\0' 

    sprintf(command, "./%s &", menus[men].icommand[bn]);
    if (system(command)==-1) { /* ignore */ }

    delete[] command;

#endif // _WIN32
  }
}

void doback(Fl_Widget *, void *) {pop_menu();}

void doexit(Fl_Widget *, void *) {exit(0);}

/* Load the menu file. Returns whether successful. */
int load_the_menu(char* fname) {
  FILE *fin = 0;
  char line[256], mname[64],iname[64],cname[64];
  int i, j;
  fin = fl_fopen(fname,"r");
#if defined ( __APPLE__ )
  if (fin == NULL) {
    // mac os bundle menu detection:
    char* pos = strrchr(fname,'/');
    if (!pos) return 0;
    *pos = '\0';
    pos = strrchr(fname,'/');
    if (!pos) return 0;
    strcpy(pos,"/Resources/demo.menu");
    fin  = fl_fopen(fname,"r");
  }
#endif
  if (fin == NULL) {
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
    // interpret the line
    j = 0; i = 0;
    while (line[i] == ' ' || line[i] == '\t') i++;
    if (line[i] == '\n') continue;
    if (line[i] == '#') continue;
    while (line[i] != ':' && line[i] != '\n') mname[j++] = line[i++];
    mname[j] = '\0';
    if (line[i] == ':') i++;
    j = 0; 
    while (line[i] != ':' && line[i] != '\n') {
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
  fl_putenv("FLTK_DOCDIR=../documentation/html");
  char buf[FL_PATH_MAX];
  strcpy(buf, argv[0]);
#if DEBUG_EXE_WITH_D
  // MS_Visual Studio appends a 'd' to debugging executables. Remove it.
  fl_filename_setext( buf, "" );
  buf[ strlen(buf)-1 ] = 0;
#endif
  fl_filename_setext(buf,".menu");
  char *fname = buf;
  int i = 0;
  if (!Fl::args(argc,argv,i) || i < argc-1)
    Fl::fatal("Usage: %s <switches> <menufile>\n%s",argv[0],Fl::help);
  if (i < argc) fname = argv[i];
  
  create_the_forms();
  
  if (!load_the_menu(fname)) Fl::fatal("Can't open %s",fname);
  if (buf != fname)
    strcpy(buf,fname);
  const char *c = fl_filename_name(buf);
  if (c > buf) {
    buf[c-buf] = 0; 
    if (fl_chdir(buf) == -1) { /* ignore */ }
  }
  push_menu("@main");
  form->show(argc,argv);
  Fl::run();
  return 0;
}

//
// End of "$Id$".
//
