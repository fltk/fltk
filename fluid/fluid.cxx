//
// "$Id$"
//
// FLUID main entry for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2005 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Icon.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Input.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_message.H>
#include <FL/filename.H>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "../src/flstring.h"
#include "alignment_panel.h"
#include "function_panel.h"

#if defined(WIN32) && !defined(__CYGWIN__)
#  include <direct.h>
#  include <windows.h>
#  include <io.h>
#else
#  include <unistd.h>
#endif
#ifdef __EMX__
#  include <X11/Xlibint.h>
#endif

#include "about_panel.h"
#include "undo.h"

#include "Fl_Type.h"

extern "C"
{
#if defined(HAVE_LIBPNG) && defined(HAVE_LIBZ)
#  include <zlib.h>
#  ifdef HAVE_PNG_H
#    include <png.h>
#  else
#    include <libpng/png.h>
#  endif // HAVE_PNG_H
#endif // HAVE_LIBPNG && HAVE_LIBZ
}

static Fl_Help_Dialog *help_dialog = 0;

Fl_Preferences	fluid_prefs(Fl_Preferences::USER, "fltk.org", "fluid");
int gridx = 5;
int gridy = 5;
int snap = 1;
int show_guides = 1;

// File history info...
char	absolute_history[10][1024];
char	relative_history[10][1024];

void	load_history();
void	update_history(const char *);

// Shell command support...
void	show_shell_window();

////////////////////////////////////////////////////////////////

static const char *filename;
void set_filename(const char *c);
void set_modflag(int mf);
int modflag;

static char* pwd;
static char in_source_dir;
void goto_source_dir() {
  if (in_source_dir) return;
  if (!filename || !*filename) return;
  const char *p = fl_filename_name(filename);
  if (p <= filename) return; // it is in the current directory
  char buffer[1024];
  strlcpy(buffer, filename, sizeof(buffer));
  int n = p-filename; if (n>1) n--; buffer[n] = 0;
  if (!pwd) {
    pwd = getcwd(0,1024);
    if (!pwd) {fprintf(stderr,"getwd : %s\n",strerror(errno)); return;}
  }
  if (chdir(buffer)<0) {fprintf(stderr, "Can't chdir to %s : %s\n",
				buffer, strerror(errno)); return;}
  in_source_dir = 1;
}

void leave_source_dir() {
  if (!in_source_dir) return;
  if (chdir(pwd)<0) {fprintf(stderr, "Can't chdir to %s : %s\n",
			     pwd, strerror(errno));}
  in_source_dir = 0;
}
  
char position_window(Fl_Window *w, const char *prefsName, int Visible, int X, int Y, int W=0, int H=0 ) {
  Fl_Preferences pos(fluid_prefs, prefsName);
  if (prevpos_button->value()) {
    pos.get("x", X, X);
    pos.get("y", Y, Y);
    if ( W!=0 ) {
      pos.get("w", W, W);
      pos.get("h", H, H);
      w->resize( X, Y, W, H );
    }
    else
      w->position( X, Y );
  }
  pos.get("visible", Visible, Visible);
  return Visible;
}

void save_position(Fl_Window *w, const char *prefsName) {
  Fl_Preferences pos(fluid_prefs, prefsName);
  pos.set("x", w->x());
  pos.set("y", w->y());
  pos.set("w", w->w());
  pos.set("h", w->h());
  pos.set("visible", (int)(w->shown() && w->visible()));
}

Fl_Window *main_window;
Fl_Menu_Bar *main_menubar;

void save_cb(Fl_Widget *, void *v) {
  const char *c = filename;
  if (v || !c || !*c) {
    fl_ok = "Save";
    c=fl_file_chooser("Save To:", "FLUID Files (*.f[ld])", c);
    fl_ok = "OK";
    if (!c) return;

    if (!access(c, 0)) {
      const char *basename;
      if ((basename = strrchr(c, '/')) != NULL)
        basename ++;
#if defined(WIN32) || defined(__EMX__)
      if ((basename = strrchr(c, '\\')) != NULL)
        basename ++;
#endif // WIN32 || __EMX__
      else
        basename = c;

      if (fl_choice("The file \"%s\" already exists.\n"
                    "Do you want to replace it?", "Cancel",
		    "Replace", NULL, basename) == 0) return;
    }

    set_filename(c);
  }
  if (!write_file(c)) {
    fl_alert("Error writing %s: %s", c, strerror(errno));
    return;
  }
  set_modflag(0);
  undo_save = undo_current;
}

void save_template_cb(Fl_Widget *, void *) {
  const char *c = fl_input("Template Name:");
  if (!c) return;

  char filename[1024];
  fluid_prefs.getUserdataPath(filename, sizeof(filename));

  strlcat(filename, "templates", sizeof(filename));
  if (access(filename, 0)) mkdir(filename, 0777);

  strlcat(filename, "/", sizeof(filename));
  strlcat(filename, c, sizeof(filename));

  char *ext = filename + strlen(filename);
  if (ext >= (filename + sizeof(filename) - 5)) {
    fl_alert("The template name \"%s\" is too long!", c);
    return;
  }

  strcpy(ext, ".fl");

//  printf("save_template_cb: template filename=\"%s\"\n", filename);

  if (!access(filename, 0)) {
    if (fl_choice("The template \"%s\" already exists.\n"
                  "Do you want to replace it?", "Cancel",
		  "Replace", NULL, c) == 0) return;
  }

  if (!write_file(filename)) {
    fl_alert("Error writing %s: %s", filename, strerror(errno));
    return;
  }

#if defined(HAVE_LIBPNG) && defined(HAVE_LIBZ)
  // Get the screenshot, if any...
  Fl_Type *t;

  for (t = Fl_Type::first; t; t = t->next) {
    // Find the first window...
    if (t->is_window()) break;
  }

  if (!t) return;

  // Grab a screenshot...
  Fl_Window_Type *wt = (Fl_Window_Type *)t;
  uchar *pixels;
  int w, h;

  if ((pixels = wt->read_image(w, h)) == NULL) return;

//  printf("save_template_cb: pixels=%p, w=%d, h=%d...\n", pixels, w, h);

  // Save to a PNG file...
  strcpy(ext, ".png");

//  printf("save_template_cb: screenshot filename=\"%s\"\n", filename);

  FILE *fp;

  if ((fp = fopen(filename, "wb")) == NULL) {
    delete[] pixels;
    fl_alert("Error writing %s: %s", filename, strerror(errno));
    return;
  }

  png_structp pptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  png_infop iptr = png_create_info_struct(pptr);
  png_bytep ptr = (png_bytep)pixels;

  png_init_io(pptr, fp);
  png_set_IHDR(pptr, iptr, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_sRGB(pptr, iptr, PNG_sRGB_INTENT_PERCEPTUAL);

  png_write_info(pptr, iptr);

  for (int i = h; i > 0; i --, ptr += w * 3) {
    png_write_row(pptr, ptr);
  }

  png_write_end(pptr, iptr);
  png_destroy_write_struct(&pptr, &iptr);

  fclose(fp);

#  if 0 // The original PPM output code...
  strcpy(ext, ".ppm");
  fp = fopen(filename, "wb");
  fprintf(fp, "P6\n%d %d 255\n", w, h);
  fwrite(pixels, w * h, 3, fp);
  fclose(fp);
#  endif // 0

  delete[] pixels;
#endif // HAVE_LIBPNG && HAVE_LIBZ
}

void exit_cb(Fl_Widget *,void *) {
  if (modflag)
    switch (fl_choice("Do you want to save changes to this user\n"
                      "interface before exiting?", "Don't Save",
                      "Save", "Cancel"))
    {
      case 2 : /* Cancel */
          return;
      case 1 : /* Save */
          save_cb(NULL, NULL);
	  if (modflag) return;	// Didn't save!
    }

  save_position(main_window,"main_window_pos");

  if (widgetbin_panel) {
    save_position(widgetbin_panel,"widgetbin_pos");
    delete widgetbin_panel;
  }
  if (about_panel)
    delete about_panel;
  if (help_dialog)
    delete help_dialog;

  undo_clear();

  exit(0);
}

#ifdef __APPLE__
#  include <FL/x.H>

void
apple_open_cb(const char *c) {
  if (modflag)
  {
    switch (fl_choice("Do you want to save changes to this user\n"
                      "interface before opening another one?", "Don't Save",
                      "Save", "Cancel"))
    {
      case 2 : /* Cancel */
          return;
      case 1 : /* Save */
          save_cb(NULL, NULL);
	  if (modflag) return;	// Didn't save!
    }
  }
  const char *oldfilename;
  oldfilename = filename;
  filename    = NULL;
  set_filename(c);
  undo_suspend();
  if (!read_file(c, 0)) {
    undo_resume();
    fl_message("Can't read %s: %s", c, strerror(errno));
    free((void *)filename);
    filename = oldfilename;
    if (main_window) main_window->label(filename);
    return;
  }

  // Loaded a file; free the old filename...
  set_modflag(0);
  undo_resume();
  undo_clear();
  if (oldfilename) free((void *)oldfilename);
}
#endif // __APPLE__

void open_cb(Fl_Widget *, void *v) {
  if (!v && modflag)
  {
    switch (fl_choice("Do you want to save changes to this user\n"
                      "interface before opening another one?", "Don't Save",
                      "Save", "Cancel"))
    {
      case 2 : /* Cancel */
          return;
      case 1 : /* Save */
          save_cb(NULL, NULL);
	  if (modflag) return;	// Didn't save!
    }
  }
  const char *c;
  const char *oldfilename;
  fl_ok = "Open";
  c = fl_file_chooser("Open:", "FLUID Files (*.f[ld])", filename);
  fl_ok = "OK";
  if (!c) return;
  oldfilename = filename;
  filename    = NULL;
  set_filename(c);
  if (v != 0) undo_checkpoint();
  undo_suspend();
  if (!read_file(c, v!=0)) {
    undo_resume();
    fl_message("Can't read %s: %s", c, strerror(errno));
    free((void *)filename);
    filename = oldfilename;
    if (main_window) main_window->label(filename);
    return;
  }
  undo_resume();
  if (v) {
    // Inserting a file; restore the original filename...
    set_modflag(1);
    free((void *)filename);
    filename = oldfilename;
    if (main_window) main_window->label(filename);
  } else {
    // Loaded a file; free the old filename...
    set_modflag(0);
    undo_clear();
    if (oldfilename) free((void *)oldfilename);
  }
}

void open_history_cb(Fl_Widget *, void *v) {
  if (modflag)
  {
    switch (fl_choice("Do you want to save changes to this user\n"
                      "interface before opening another one?", "Don't Save",
                      "Save", "Cancel"))
    {
      case 2 : /* Cancel */
          return;
      case 1 : /* Save */
          save_cb(NULL, NULL);
	  if (modflag) return;	// Didn't save!
    }
  }
  const char *oldfilename = filename;
  filename = NULL;
  set_filename((char *)v);
  undo_suspend();
  if (!read_file(filename, 0)) {
    undo_resume();
    undo_clear();
    fl_message("Can't read %s: %s", filename, strerror(errno));
    free((void *)filename);
    filename = oldfilename;
    if (main_window) main_window->label(filename);
    return;
  }
  set_modflag(0);
  undo_resume();
  undo_clear();
  if (oldfilename) free((void *)oldfilename);
}

void new_cb(Fl_Widget *, void *v) {
  if (!v && modflag)
  {
    switch (fl_choice("Do you want to save changes to this user\n"
                      "interface before creating a new one?", "Don't Save",
                      "Save", "Cancel"))
    {
      case 2 : /* Cancel */
          return;
      case 1 : /* Save */
          save_cb(NULL, NULL);
	  if (modflag) return;	// Didn't save!
    }
  }
  delete_all();
  set_filename(NULL);
  set_modflag(0);
  undo_clear();
}

int compile_only = 0;
int compile_strings = 0;
int header_file_set = 0;
int code_file_set = 0;
const char* header_file_name = ".h";
const char* code_file_name = ".cxx";
int i18n_type = 0;
const char* i18n_include = "";
const char* i18n_function = "";
const char* i18n_file = "";
const char* i18n_set = "";
char i18n_program[1024] = "";

void write_cb(Fl_Widget *, void *) {
  if (!filename) {
    save_cb(0,0);
    if (!filename) return;
  }
  char cname[1024];
  char hname[1024];
  strlcpy(i18n_program, fl_filename_name(filename), sizeof(i18n_program));
  fl_filename_setext(i18n_program, sizeof(i18n_program), "");
  if (*code_file_name == '.' && strchr(code_file_name, '/') == NULL) {
    strlcpy(cname, fl_filename_name(filename), sizeof(cname));
    fl_filename_setext(cname, sizeof(cname), code_file_name);
  } else {
    strlcpy(cname, code_file_name, sizeof(hname));
  }
  if (*header_file_name == '.' && strchr(header_file_name, '/') == NULL) {
    strlcpy(hname, fl_filename_name(filename), sizeof(hname));
    fl_filename_setext(hname, sizeof(hname), header_file_name);
  } else {
    strlcpy(hname, header_file_name, sizeof(hname));
  }
  if (!compile_only) goto_source_dir();
  int x = write_code(cname,hname);
  if (!compile_only) leave_source_dir();
  strlcat(cname, " and ", sizeof(cname));
  strlcat(cname, hname, sizeof(cname));
  if (compile_only) {
    if (!x) {fprintf(stderr,"%s : %s\n",cname,strerror(errno)); exit(1);}
  } else {
    if (!x) {
      fl_message("Can't write %s: %s", cname, strerror(errno));
    } else if (completion_button->value()) {
      fl_message("Wrote %s", cname, 0);
    }
  }
}

void write_strings_cb(Fl_Widget *, void *) {
  static const char *exts[] = { ".txt", ".po", ".msg" };
  if (!filename) {
    save_cb(0,0);
    if (!filename) return;
  }
  char sname[1024];
  strlcpy(sname, fl_filename_name(filename), sizeof(sname));
  fl_filename_setext(sname, sizeof(sname), exts[i18n_type]);
  if (!compile_only) goto_source_dir();
  int x = write_strings(sname);
  if (!compile_only) leave_source_dir();
  if (compile_only) {
    if (x) {fprintf(stderr,"%s : %s\n",sname,strerror(errno)); exit(1);}
  } else {
    if (x) {
      fl_message("Can't write %s: %s", sname, strerror(errno));
    } else if (completion_button->value()) {
      fl_message("Wrote %s", sname);
    }
  }
}

void openwidget_cb(Fl_Widget *, void *) {
  if (!Fl_Type::current) {
    fl_message("Please select a widget");
    return;
  }
  Fl_Type::current->open();
}

void toggle_overlays(Fl_Widget *,void *);

void select_all_cb(Fl_Widget *,void *);
void select_none_cb(Fl_Widget *,void *);

void group_cb(Fl_Widget *, void *);

void ungroup_cb(Fl_Widget *, void *);

extern int pasteoffset;
static int ipasteoffset;

static char* cutfname(int which = 0) {
  static char name[2][1024];
  static char beenhere = 0;

  if (!beenhere) {
    beenhere = 1;
    fluid_prefs.getUserdataPath(name[0], sizeof(name[0]));
    strlcat(name[0], "cut_buffer", sizeof(name[0]));
    fluid_prefs.getUserdataPath(name[1], sizeof(name[1]));
    strlcat(name[1], "dup_buffer", sizeof(name[1]));
  }

  return name[which];
}

void copy_cb(Fl_Widget*, void*) {
  if (!Fl_Type::current) {
    fl_beep();
    return;
  }
  ipasteoffset = 10;
  if (!write_file(cutfname(),1)) {
    fl_message("Can't write %s: %s", cutfname(), strerror(errno));
    return;
  }
}

extern void select_only(Fl_Type *);
void cut_cb(Fl_Widget *, void *) {
  if (!Fl_Type::current) {
    fl_beep();
    return;
  }
  if (!write_file(cutfname(),1)) {
    fl_message("Can't write %s: %s", cutfname(), strerror(errno));
    return;
  }
  undo_checkpoint();
  set_modflag(1);
  ipasteoffset = 0;
  Fl_Type *p = Fl_Type::current->parent;
  while (p && p->selected) p = p->parent;
  delete_all(1);
  if (p) select_only(p);
}

void delete_cb(Fl_Widget *, void *) {
  if (!Fl_Type::current) {
    fl_beep();
    return;
  }
  undo_checkpoint();
  set_modflag(1);
  ipasteoffset = 0;
  Fl_Type *p = Fl_Type::current->parent;
  while (p && p->selected) p = p->parent;
  delete_all(1);
  if (p) select_only(p);
}

extern int force_parent;

void paste_cb(Fl_Widget*, void*) {
  //if (ipasteoffset) force_parent = 1;
  pasteoffset = ipasteoffset;
  if (gridx>1) pasteoffset = ((pasteoffset-1)/gridx+1)*gridx;
  if (gridy>1) pasteoffset = ((pasteoffset-1)/gridy+1)*gridy;
  undo_checkpoint();
  undo_suspend();
  if (!read_file(cutfname(), 1)) {
    fl_message("Can't read %s: %s", cutfname(), strerror(errno));
  }
  undo_resume();
  pasteoffset = 0;
  ipasteoffset += 10;
  force_parent = 0;
}

// Duplicate the selected widgets...
void duplicate_cb(Fl_Widget*, void*) {
  if (!Fl_Type::current) {
    fl_beep();
    return;
  }

  if (!write_file(cutfname(1),1)) {
    fl_message("Can't write %s: %s", cutfname(1), strerror(errno));
    return;
  }

  pasteoffset  = 0;
  force_parent = 1;

  undo_checkpoint();
  undo_suspend();
  if (!read_file(cutfname(1), 1)) {
    fl_message("Can't read %s: %s", cutfname(1), strerror(errno));
  }
  undo_resume();

  force_parent = 0;
}

void earlier_cb(Fl_Widget*,void*);

void later_cb(Fl_Widget*,void*);

Fl_Type *sort(Fl_Type *parent);

static void sort_cb(Fl_Widget *,void *) {
  sort((Fl_Type*)0);
}

void show_project_cb(Fl_Widget *, void *);
void show_grid_cb(Fl_Widget *, void *);
void show_settings_cb(Fl_Widget *, void *);

void align_widget_cb(Fl_Widget *, long);
void widget_size_cb(Fl_Widget *, long);

void about_cb(Fl_Widget *, void *) {
  if (!about_panel) make_about_panel();
  about_panel->show();
}

void show_help(const char *name) {
  const char	*docdir;
  char		helpname[1024];
  
  if (!help_dialog) help_dialog = new Fl_Help_Dialog();

  if ((docdir = getenv("FLTK_DOCDIR")) == NULL) {
#ifdef __EMX__
    // Doesn't make sense to have a hardcoded fallback
    static char fltk_docdir[1024];

    strlcpy(fltk_docdir, __XOS2RedirRoot("/XFree86/lib/X11/fltk/doc"),
            sizeof(fltk_docdir));

    docdir = fltk_docdir;
#else
    docdir = FLTK_DOCDIR;
#endif // __EMX__
  }
  snprintf(helpname, sizeof(helpname), "%s/%s", docdir, name);  

  help_dialog->load(helpname);
  help_dialog->show();
}

void help_cb(Fl_Widget *, void *) {
  show_help("fluid.html");
}

void manual_cb(Fl_Widget *, void *) {
  show_help("index.html");
}

////////////////////////////////////////////////////////////////

extern Fl_Menu_Item New_Menu[];

void toggle_widgetbin_cb(Fl_Widget *, void *);

Fl_Menu_Item Main_Menu[] = {
{"&File",0,0,0,FL_SUBMENU},
  {"&New", FL_CTRL+'n', new_cb, 0},
  {"&Open...", FL_CTRL+'o', open_cb, 0},
  {"Open Pre&vious",0,0,0,FL_SUBMENU},
    {relative_history[0], FL_CTRL+'0', open_history_cb, absolute_history[0]},
    {relative_history[1], FL_CTRL+'1', open_history_cb, absolute_history[1]},
    {relative_history[2], FL_CTRL+'2', open_history_cb, absolute_history[2]},
    {relative_history[3], FL_CTRL+'3', open_history_cb, absolute_history[3]},
    {relative_history[4], FL_CTRL+'4', open_history_cb, absolute_history[4]},
    {relative_history[5], FL_CTRL+'5', open_history_cb, absolute_history[5]},
    {relative_history[6], FL_CTRL+'6', open_history_cb, absolute_history[6]},
    {relative_history[7], FL_CTRL+'7', open_history_cb, absolute_history[7]},
    {relative_history[8], FL_CTRL+'8', open_history_cb, absolute_history[8]},
    {relative_history[9], FL_CTRL+'9', open_history_cb, absolute_history[9]},
    {0},
  {"&Insert...", FL_CTRL+'i', open_cb, (void*)1, FL_MENU_DIVIDER},
  {"&Save", FL_CTRL+'s', save_cb, 0},
  {"Save &As...", FL_CTRL+FL_SHIFT+'s', save_cb, (void*)1},
  {"Save &Template...", 0, save_template_cb, (void*)2, FL_MENU_DIVIDER},
  {"&Print...", FL_CTRL+'p', 0},
  {"Write &Code...", FL_CTRL+FL_SHIFT+'c', write_cb, 0},
  {"&Write Strings...", FL_CTRL+FL_SHIFT+'w', write_strings_cb, 0, FL_MENU_DIVIDER},
  {"&Quit", FL_CTRL+'q', exit_cb},
  {0},
{"&Edit",0,0,0,FL_SUBMENU},
  {"&Undo", FL_CTRL+'z', undo_cb},
  {"&Redo", FL_CTRL+FL_SHIFT+'z', redo_cb, 0, FL_MENU_DIVIDER},
  {"C&ut", FL_CTRL+'x', cut_cb},
  {"&Copy", FL_CTRL+'c', copy_cb},
  {"&Paste", FL_CTRL+'v', paste_cb},
  {"Dup&licate", FL_CTRL+'u', duplicate_cb},
  {"&Delete", FL_Delete, delete_cb, 0, FL_MENU_DIVIDER},
  {"Select &All", FL_CTRL+'a', select_all_cb},
  {"Select &None", FL_CTRL+FL_SHIFT+'a', select_none_cb, 0, FL_MENU_DIVIDER},
  {"Pr&operties...", FL_F+1, openwidget_cb},
  {"&Sort",0,sort_cb},
  {"&Earlier", FL_F+2, earlier_cb},
  {"&Later", FL_F+3, later_cb},
  {"&Group", FL_F+7, group_cb},
  {"Ung&roup", FL_F+8, ungroup_cb,0, FL_MENU_DIVIDER},
  {"Hide O&verlays",FL_CTRL+FL_SHIFT+'o',toggle_overlays},
  {"Show Widget &Bin...",FL_ALT+'b',toggle_widgetbin_cb, 0, FL_MENU_DIVIDER},
  {"Pro&ject Settings...",FL_ALT+'p',show_project_cb},
  {"GU&I Settings...",FL_ALT+FL_SHIFT+'p',show_settings_cb},
  {0},
{"&New", 0, 0, (void *)New_Menu, FL_SUBMENU_POINTER},
{"&Layout",0,0,0,FL_SUBMENU},
  {"&Align",0,0,0,FL_SUBMENU},
    {"&Left",0,(Fl_Callback *)align_widget_cb,(void*)10},
    {"&Center",0,(Fl_Callback *)align_widget_cb,(void*)11},
    {"&Right",0,(Fl_Callback *)align_widget_cb,(void*)12},
    {"&Top",0,(Fl_Callback *)align_widget_cb,(void*)13},
    {"&Middle",0,(Fl_Callback *)align_widget_cb,(void*)14},
    {"&Bottom",0,(Fl_Callback *)align_widget_cb,(void*)15},
    {0},
  {"&Space Evenly",0,0,0,FL_SUBMENU},
    {"&Across",0,(Fl_Callback *)align_widget_cb,(void*)20},
    {"&Down",0,(Fl_Callback *)align_widget_cb,(void*)21},
    {0},
  {"&Make Same Size",0,0,0,FL_SUBMENU},
    {"&Width",0,(Fl_Callback *)align_widget_cb,(void*)30},
    {"&Height",0,(Fl_Callback *)align_widget_cb,(void*)31},
    {"&Both",0,(Fl_Callback *)align_widget_cb,(void*)32},
    {0},
  {"&Center In Group",0,0,0,FL_SUBMENU},
    {"&Horizontal",0,(Fl_Callback *)align_widget_cb,(void*)40},
    {"&Vertical",0,(Fl_Callback *)align_widget_cb,(void*)41},
    {0},
  {"&Set Widget Size",0,0,0,FL_SUBMENU|FL_MENU_DIVIDER},
    {"&Tiny",FL_ALT+'1',(Fl_Callback *)widget_size_cb,(void*)8,FL_MENU_RADIO,FL_NORMAL_LABEL,FL_HELVETICA,8},
    {"&Small",FL_ALT+'2',(Fl_Callback *)widget_size_cb,(void*)11,FL_MENU_RADIO,FL_NORMAL_LABEL,FL_HELVETICA,11},
    {"&Normal",FL_ALT+'3',(Fl_Callback *)widget_size_cb,(void*)14,FL_MENU_RADIO|FL_MENU_VALUE},
    {0},
  {"&Grid...",FL_CTRL+'g',show_grid_cb},
  {0},
{"&Shell",0,0,0,FL_SUBMENU},
  {"Execute &Command...",FL_ALT+'x',(Fl_Callback *)show_shell_window},
  {"Execute &Again...",FL_ALT+'g',(Fl_Callback *)do_shell_command},
  {0},
{"&Help",0,0,0,FL_SUBMENU},
  {"&About FLUID...",0,about_cb},
  {"&On FLUID...",0,help_cb},
  {"&Manual...",0,manual_cb},
  {0},
{0}};

#define BROWSERWIDTH 300
#define BROWSERHEIGHT 500
#define WINWIDTH 300
#define MENUHEIGHT 25
#define WINHEIGHT (BROWSERHEIGHT+MENUHEIGHT)

extern void fill_in_New_Menu();

void scheme_cb(Fl_Choice *, void *) {
  switch (scheme_choice->value()) {
    case 0 : // Default
      Fl::scheme(NULL);
      break;
    case 1 : // None
      Fl::scheme("none");
      break;
    case 2 : // Plastic
      Fl::scheme("plastic");
      break;
  }

  fluid_prefs.set("scheme", scheme_choice->value());
}

void toggle_widgetbin_cb(Fl_Widget *, void *) {
  if (!widgetbin_panel) {
    make_widgetbin();
    widgetbin_panel->callback(toggle_widgetbin_cb);
    if (!position_window(widgetbin_panel,"widgetbin_pos", 1, 320, 30)) return;
  }

  if (widgetbin_panel->visible()) {
    widgetbin_panel->hide();
    Main_Menu[41].label("Show Widget &Bin...");
  } else {
    widgetbin_panel->show();
    Main_Menu[41].label("Hide Widget &Bin");
  }
}


void make_main_window() {
  fluid_prefs.get("snap", snap, 1);
  fluid_prefs.get("gridx", gridx, 5);
  fluid_prefs.get("gridy", gridy, 5);
  fluid_prefs.get("show_guides", show_guides, 0);

  load_history();

  make_grid_window();
  make_settings_window();
  make_shell_window();

  if (!main_window) {
    Fl_Widget *o;
    main_window = new Fl_Double_Window(WINWIDTH,WINHEIGHT,"fluid");
    main_window->box(FL_NO_BOX);
    o = make_widget_browser(0,MENUHEIGHT,BROWSERWIDTH,BROWSERHEIGHT);
    o->box(FL_FLAT_BOX);
    o->tooltip("Double-click to view or change an item.");
    main_window->resizable(o);
    main_menubar = new Fl_Menu_Bar(0,0,BROWSERWIDTH,MENUHEIGHT);
    main_menubar->menu(Main_Menu);
    main_menubar->global();
    fill_in_New_Menu();
    main_window->end();
  }
}

// Load file history from preferences...
void load_history() {
  int	i;		// Looping var

  for (i = 0; i < 10; i ++) {
    fluid_prefs.get( Fl_Preferences::Name("file%d", i), absolute_history[i], "", sizeof(absolute_history[i]));
    if (absolute_history[i][0]) {
      // Make a relative version of the filename for the menu...
      fl_filename_relative(relative_history[i], sizeof(relative_history[i]),
                           absolute_history[i]);

      Main_Menu[i + 4].flags = 0;
    } else Main_Menu[i + 4].flags = FL_MENU_INVISIBLE;
  }

  if (!absolute_history[0][0]) Main_Menu[3].flags |= FL_MENU_INACTIVE;
}

// Update file history from preferences...
void update_history(const char *flname) {
  int	i;		// Looping var
  char	absolute[1024];

  fl_filename_absolute(absolute, sizeof(absolute), flname);

  for (i = 0; i < 10; i ++)
#if defined(WIN32) || defined(__APPLE__)
    if (!strcasecmp(absolute, absolute_history[i])) break;
#else
    if (!strcmp(absolute, absolute_history[i])) break;
#endif // WIN32 || __APPLE__

  if (i == 0) return;

  if (i >= 10) i = 9;

  // Move the other flnames down in the list...
  memmove(absolute_history + 1, absolute_history,
          i * sizeof(absolute_history[0]));
  memmove(relative_history + 1, relative_history,
          i * sizeof(relative_history[0]));

  // Put the new file at the top...
  strlcpy(absolute_history[0], absolute, sizeof(absolute_history[0]));

  fl_filename_relative(relative_history[0], sizeof(relative_history[0]),
                       absolute_history[0]);

  // Update the menu items as needed...
  for (i = 0; i < 10; i ++) {
    fluid_prefs.set( Fl_Preferences::Name("file%d", i), absolute_history[i]);
    if (absolute_history[i][0]) Main_Menu[i + 4].flags = 0;
    else Main_Menu[i + 4].flags = FL_MENU_INVISIBLE;
  }

  Main_Menu[3].flags &= ~FL_MENU_INACTIVE;
}

// Shell command support...
#if (!defined(WIN32) || defined(__CYGWIN__)) && !defined(__MWERKS__)
// Support the full piped shell command...
static FILE *shell_pipe = 0;

void
shell_pipe_cb(int, void*) {
  char	line[1024];		// Line from command output...

  if (fgets(line, sizeof(line), shell_pipe) != NULL) {
    // Add the line to the output list...
    shell_run_buffer->append(line);
  } else {
    // End of file; tell the parent...
    Fl::remove_fd(fileno(shell_pipe));

    pclose(shell_pipe);
    shell_pipe = NULL;
    shell_run_buffer->append("... END SHELL COMMAND ...\n");
  }

  shell_run_display->scroll(shell_run_display->count_lines(0,
                            shell_run_buffer->length(), 1), 0);
}

void
do_shell_command(Fl_Return_Button*, void*) {
  const char	*command;	// Command to run


  shell_window->hide();

  if (shell_pipe) {
    fl_alert("Previous shell command still running!");
    return;
  }

  if ((command = shell_command_input->value()) == NULL || !*command) {
    fl_alert("No shell command entered!");
    return;
  }

  if (shell_savefl_button->value()) {
    save_cb(0, 0);
  }

  if (shell_writecode_button->value()) {
    compile_only = 1;
    write_cb(0, 0);
    compile_only = 0;
  }

  if (shell_writemsgs_button->value()) {
    compile_only = 1;
    write_strings_cb(0, 0);
    compile_only = 0;
  }

  // Show the output window and clear things...
  shell_run_buffer->text("");
  shell_run_buffer->append(command);
  shell_run_buffer->append("\n");
  shell_run_window->label("Shell Command Running...");

  if ((shell_pipe = popen((char *)command, "r")) == NULL) {
    fl_alert("Unable to run shell command: %s", strerror(errno));
    return;
  }

  shell_run_button->deactivate();
  shell_run_window->hotspot(shell_run_display);
  shell_run_window->show();

  Fl::add_fd(fileno(shell_pipe), shell_pipe_cb);

  while (shell_pipe) Fl::wait();

  shell_run_button->activate();
  shell_run_window->label("Shell Command Complete");
  fl_beep();

  while (shell_run_window->shown()) Fl::wait();
}
#else
// Just do basic shell command stuff, no status window...
void
do_shell_command(Fl_Return_Button*, void*) {
  const char	*command;	// Command to run
  int		status;		// Status from command...


  shell_window->hide();

  if ((command = shell_command_input->value()) == NULL || !*command) {
    fl_alert("No shell command entered!");
    return;
  }

  if (shell_savefl_button->value()) {
    save_cb(0, 0);
  }

  if (shell_writecode_button->value()) {
    compile_only = 1;
    write_cb(0, 0);
    compile_only = 0;
  }

  if (shell_writemsgs_button->value()) {
    compile_only = 1;
    write_strings_cb(0, 0);
    compile_only = 0;
  }

  if ((status = system(command)) != 0) {
    fl_alert("Shell command returned status %d!", status);
  } else if (completion_button->value()) {
    fl_message("Shell command completed successfully!");
  }
}
#endif // (!WIN32 || __CYGWIN__) && !__MWERKS__


void
show_shell_window() {
  shell_window->hotspot(shell_command_input);
  shell_window->show();
}

void set_filename(const char *c) {
  if (filename) free((void *)filename);
  filename = c ? strdup(c) : NULL;

  if (filename) update_history(filename);

  set_modflag(modflag);
}

// Set the "modified" flag and update the title of the main window...
void set_modflag(int mf) {
  const char	*basename;
  static char	title[1024];

  modflag = mf;

  if (main_window) {
    if (!filename) basename = "Untitled.fl";
    else if ((basename = strrchr(filename, '/')) != NULL) basename ++;
#if defined(WIN32) || defined(__EMX__) 
    else if ((basename = strrchr(filename, '\\')) != NULL) basename ++;
#endif // WIN32 || __EMX__
    else basename = filename;

    if (modflag) {
      snprintf(title, sizeof(title), "%s (modified)", basename);
      main_window->label(title);
    } else main_window->label(basename);
  }

  // Enable/disable the Save menu item...
  if (modflag) Main_Menu[16].activate();
  else Main_Menu[16].deactivate();
}

////////////////////////////////////////////////////////////////

static int arg(int argc, char** argv, int& i) {
  if (argv[i][1] == 'c' && !argv[i][2]) {compile_only = 1; i++; return 1;}
  if (argv[i][1] == 'c' && argv[i][2] == 's' && !argv[i][3]) {compile_only = 1; compile_strings = 1; i++; return 1;}
  if (argv[i][1] == 'o' && !argv[i][2] && i+1 < argc) {
    code_file_name = argv[i+1];
    code_file_set  = 1;
    i += 2;
    return 2;
  }
  if (argv[i][1] == 'h' && !argv[i][2]) {
    header_file_name = argv[i+1];
    header_file_set  = 1;
    i += 2;
    return 2;
  }
  return 0;
}

#if ! (defined(WIN32) && !defined (__CYGWIN__))

int quit_flag = 0;
#include <signal.h>
#ifdef _sigargs
#define SIGARG _sigargs
#else
#ifdef __sigargs
#define SIGARG __sigargs
#else
#define SIGARG int // you may need to fix this for older systems
#endif
#endif

extern "C" {
static void sigint(SIGARG) {
  signal(SIGINT,sigint);
  quit_flag = 1;
}
}
#endif

int main(int argc,char **argv) {
  int i = 1;
  if (!Fl::args(argc,argv,i,arg) || i < argc-1) {
    fprintf(stderr,"usage: %s <switches> name.fl\n"
" -c : write .cxx and .h and exit\n"
" -cs : write .cxx and .h and strings and exit\n"
" -o <name> : .cxx output filename, or extension if <name> starts with '.'\n"
" -h <name> : .h output filename, or extension if <name> starts with '.'\n"
"%s\n", argv[0], Fl::help);
    return 1;
  }
  const char *c = argv[i];

  fl_register_images();

  make_main_window();

#ifdef __APPLE__
  fl_open_callback(apple_open_cb);
#endif // __APPLE__

  if (c) set_filename(c);
  if (!compile_only) {
    Fl::visual((Fl_Mode)(FL_DOUBLE|FL_INDEX));
    Fl_File_Icon::load_system_icons();
    main_window->callback(exit_cb);
    position_window(main_window,"main_window_pos", 1, 10, 30, WINWIDTH, WINHEIGHT );
    main_window->show(argc,argv);
    toggle_widgetbin_cb(0,0);
    if (!c && openlast_button->value() && absolute_history[0][0]) {
      // Open previous file when no file specified...
      open_history_cb(0, absolute_history[0]);
    }
  }
  undo_suspend();
  if (c && !read_file(c,0)) {
    if (compile_only) {
      fprintf(stderr,"%s : %s\n", c, strerror(errno));
      exit(1);
    }
    fl_message("Can't read %s: %s", c, strerror(errno));
  }
  undo_resume();
  if (compile_only) {
    if (compile_strings) write_strings_cb(0,0);
    write_cb(0,0);
    exit(0);
  }
  set_modflag(0);
  undo_clear();
#ifndef WIN32
  signal(SIGINT,sigint);
#endif

  grid_cb(horizontal_input, 0); // Makes sure that windows get snap params...

#ifdef WIN32
  Fl::run();
#else
  while (!quit_flag) Fl::wait();

  if (quit_flag) exit_cb(0,0);
#endif // WIN32

  undo_clear();

  return (0);
}

//
// End of "$Id$".
//
