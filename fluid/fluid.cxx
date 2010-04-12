//
// "$Id$"
//
// FLUID main entry for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
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

#define IDE_SUPPORT

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Icon.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Plugin.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_message.H>
#include <FL/filename.H>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h> // time(), localtime(), etc.

#include "../src/flstring.h"
#include "alignment_panel.h"
#include "function_panel.h"
#include "template_panel.h"
#if !defined(WIN32) || defined(__CYGWIN__)
#  include "print_panel.cxx"
#endif // !WIN32 || __CYGWIN__

#if defined(WIN32) && !defined(__CYGWIN__)
#  include <direct.h>
#  include <windows.h>
#  include <io.h>
#  include <fcntl.h>
#  include <commdlg.h>
#  include <FL/x.H>
#  ifndef __WATCOMC__
// Visual C++ 2005 incorrectly displays a warning about the use of POSIX APIs
// on Windows, which is supposed to be POSIX compliant...
#    define access _access
#    define chdir _chdir
#    define getcwd _getcwd
#  endif // !__WATCOMC__
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
int show_comments = 1;
int show_coredevmenus = 1;

// File history info...
char	absolute_history[10][1024];
char	relative_history[10][1024];

void	load_history();
void	update_history(const char *);

// Shell command support...
void	show_shell_window();

Fl_Menu_Item *save_item = 0L;
Fl_Menu_Item *history_item = 0L;
Fl_Menu_Item *widgetbin_item = 0L;
Fl_Menu_Item *sourceview_item = 0L;

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

void save_cb(Fl_Widget *, void *v) {
  const char *c = filename;
  if (v || !c || !*c) {
    fl_file_chooser_ok_label("Save");
    c=fl_file_chooser("Save To:", "FLUID Files (*.f[ld])", c);
    fl_file_chooser_ok_label(NULL);
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

    if (v != (void *)2) set_filename(c);
  }
  if (!write_file(c)) {
    fl_alert("Error writing %s: %s", c, strerror(errno));
    return;
  }

  if (v != (void *)2) {
    set_modflag(0);
    undo_save = undo_current;
  }
}

void save_template_cb(Fl_Widget *, void *) {
  // Setup the template panel...
  if (!template_panel) make_template_panel();

  template_clear();
  template_browser->add("New Template");
  template_load();

  template_name->show();
  template_name->value("");

  template_instance->hide();

  template_delete->show();
  template_delete->deactivate();

  template_submit->label("Save");
  template_submit->deactivate();

  template_panel->label("Save Template");

  // Show the panel and wait for the user to do something...
  template_panel->show();
  while (template_panel->shown()) Fl::wait();

  // Get the template name, return if it is empty...
  const char *c = template_name->value();
  if (!c || !*c) return;

  // Convert template name to filename_with_underscores
  char safename[1024], *safeptr;
  strlcpy(safename, c, sizeof(safename));
  for (safeptr = safename; *safeptr; safeptr ++) {
    if (isspace(*safeptr)) *safeptr = '_';
  }

  // Find the templates directory...
  char filename[1024];
  fluid_prefs.getUserdataPath(filename, sizeof(filename));

  strlcat(filename, "templates", sizeof(filename));
#if defined(WIN32) && !defined(__CYGWIN__)
  if (access(filename, 0)) mkdir(filename);
#else
  if (access(filename, 0)) mkdir(filename, 0777);
#endif // WIN32 && !__CYGWIN__

  strlcat(filename, "/", sizeof(filename));
  strlcat(filename, safename, sizeof(filename));

  char *ext = filename + strlen(filename);
  if (ext >= (filename + sizeof(filename) - 5)) {
    fl_alert("The template name \"%s\" is too long!", c);
    return;
  }

  // Save the .fl file...
  strcpy(ext, ".fl");

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

  // Save to a PNG file...
  strcpy(ext, ".png");

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

void revert_cb(Fl_Widget *,void *) {
  if (modflag) {
    if (!fl_choice("This user interface has been changed. Really revert?",
                   "Cancel", "Revert", NULL)) return;
  }
  undo_suspend();
  if (!read_file(filename, 0)) {
    undo_resume();
    fl_message("Can't read %s: %s", filename, strerror(errno));
    return;
  }
  undo_resume();
  set_modflag(0);
  undo_clear();
}

void exit_cb(Fl_Widget *,void *) {
  if (modflag)
    switch (fl_choice("Do you want to save changes to this user\n"
                      "interface before exiting?", "Cancel",
                      "Save", "Don't Save"))
    {
      case 0 : /* Cancel */
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
  if (sourceview_panel) {
    Fl_Preferences svp(fluid_prefs, "sourceview");
    svp.set("autorefresh", sv_autorefresh->value());
    svp.set("autoposition", sv_autoposition->value());
    svp.set("tab", sv_tab->find(sv_tab->value()));
    save_position(sourceview_panel,"sourceview_pos");
    delete sourceview_panel;
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
  if (modflag) {
    switch (fl_choice("Do you want to save changes to this user\n"
                      "interface before opening another one?", "Don't Save",
                      "Save", "Cancel"))
    {
      case 0 : /* Cancel */
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
  if (!v && modflag) {
    switch (fl_choice("Do you want to save changes to this user\n"
                      "interface before opening another one?", "Cancel",
                      "Save", "Don't Save"))
    {
      case 0 : /* Cancel */
          return;
      case 1 : /* Save */
          save_cb(NULL, NULL);
	  if (modflag) return;	// Didn't save!
    }
  }
  const char *c;
  const char *oldfilename;
  fl_file_chooser_ok_label("Open");
  c = fl_file_chooser("Open:", "FLUID Files (*.f[ld])", filename);
  fl_file_chooser_ok_label(NULL);
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
    if (main_window) set_modflag(modflag);
    return;
  }
  undo_resume();
  if (v) {
    // Inserting a file; restore the original filename...
    free((void *)filename);
    filename = oldfilename;
    set_modflag(1);
  } else {
    // Loaded a file; free the old filename...
    set_modflag(0);
    undo_clear();
    if (oldfilename) free((void *)oldfilename);
  }
}

void open_history_cb(Fl_Widget *, void *v) {
  if (modflag) {
    switch (fl_choice("Do you want to save changes to this user\n"
                      "interface before opening another one?", "Cancel",
                      "Save", "Don't Save"))
    {
      case 0 : /* Cancel */
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
  // Check if the current file has been modified...
  if (!v && modflag) {
    // Yes, ask the user what to do...
    switch (fl_choice("Do you want to save changes to this user\n"
                      "interface before creating a new one?", "Cancel",
                      "Save", "Don't Save"))
    {
      case 0 : /* Cancel */
          return;
      case 1 : /* Save */
          save_cb(NULL, NULL);
	  if (modflag) return;	// Didn't save!
    }
  }

  // Setup the template panel...
  if (!template_panel) make_template_panel();

  template_clear();
  template_browser->add("Blank");
  template_load();

  template_name->hide();
  template_name->value("");

  template_instance->show();
  template_instance->deactivate();
  template_instance->value("");

  template_delete->hide();

  template_submit->label("New");
  template_submit->deactivate();

  template_panel->label("New");

  // Show the panel and wait for the user to do something...
  template_panel->show();
  while (template_panel->shown()) Fl::wait();

  // See if the user chose anything...
  int item = template_browser->value();
  if (item < 1) return;

  // Clear the current data...
  delete_all();
  set_filename(NULL);

  // Load the template, if any...
  const char *tname = (const char *)template_browser->data(item);

  if (tname) {
    // Grab the instance name...
    const char *iname = template_instance->value();

    if (iname && *iname) {
      // Copy the template to a temp file, then read it in...
      char line[1024], *ptr, *next;
      FILE *infile, *outfile;

      if ((infile = fopen(tname, "r")) == NULL) {
	fl_alert("Error reading template file \"%s\":\n%s", tname,
        	 strerror(errno));
	set_modflag(0);
	undo_clear();
	return;
      }

      if ((outfile = fopen(cutfname(1), "w")) == NULL) {
	fl_alert("Error writing buffer file \"%s\":\n%s", cutfname(1),
        	 strerror(errno));
	fclose(infile);
	set_modflag(0);
	undo_clear();
	return;
      }

      while (fgets(line, sizeof(line), infile)) {
	// Replace @INSTANCE@ with the instance name...
	for (ptr = line; (next = strstr(ptr, "@INSTANCE@")) != NULL; ptr = next + 10) {
	  fwrite(ptr, next - ptr, 1, outfile);
	  fputs(iname, outfile);
	}

	fputs(ptr, outfile);
      }

      fclose(infile);
      fclose(outfile);

      undo_suspend();
      read_file(cutfname(1), 0);
      unlink(cutfname(1));
      undo_resume();
    } else {
      // No instance name, so read the template without replacements...
      undo_suspend();
      read_file(tname, 0);
      undo_resume();
    }
  }

  set_modflag(0);
  undo_clear();
}

int exit_early = 0;
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
      fl_message("Wrote %s", cname);
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
  unlink(cutfname(1));
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
  show_help("main.html");
}


////////////////////////////////////////////////////////////////

#if defined(WIN32) && !defined(__CYGWIN__)
// Draw a shaded box...
static void win_box(int x, int y, int w, int h) {
  fl_color(0xc0, 0xc0, 0xc0);
  fl_rectf(x, y, w, h);
  fl_color(0, 0, 0);
  fl_rect(x, y, w, h);
  fl_color(0xf0, 0xf0, 0xf0);
  fl_rectf(x + 1, y + 1, 4, h - 2);
  fl_rectf(x + 1, y + 1, w - 2, 4);
  fl_color(0x90, 0x90, 0x90);
  fl_rectf(x + w - 5, y + 1, 4, h - 2);
  fl_rectf(x + 1, y + h - 5, w - 2, 4);
}

// Load and show the print dialog...
void print_menu_cb(Fl_Widget *, void *) {
  PRINTDLG	dialog;			// Print dialog
  DOCINFO	docinfo;		// Document info
  int		first, last;		// First and last page
  int		page;			// Current page
  int		winpage;		// Current window page
  int		num_pages;		// Number of pages
  Fl_Type	*t;			// Current widget
  int		num_windows;		// Number of windows
  Fl_Window_Type *windows[1000];	// Windows to print


  // Show print dialog...
  for (t = Fl_Type::first, num_pages = 0; t; t = t->next) {
    if (t->is_window()) num_pages ++;
  }

  memset(&dialog, 0, sizeof(dialog));
  dialog.lStructSize = sizeof(dialog);
  dialog.hwndOwner   = fl_xid(main_window);
  dialog.Flags       = PD_ALLPAGES |
                       PD_RETURNDC;
  dialog.nFromPage   = 1;
  dialog.nToPage     = num_pages;
  dialog.nMinPage    = 1;
  dialog.nMaxPage    = num_pages;
  dialog.nCopies     = 1;

  if (!PrintDlg(&dialog)) return;

  // Get the base filename...
  const char *basename = strrchr(filename, '/');
  if (basename) basename ++;
  else basename = filename;

  // Do the print job...
  memset(&docinfo, 0, sizeof(docinfo));
  docinfo.cbSize      = sizeof(docinfo);
  docinfo.lpszDocName = basename;

  StartDoc(dialog.hDC, &docinfo);

  // Figure out how many pages we'll have to print...
  if (dialog.Flags & PD_PAGENUMS) {
    // Get from and to page numbers...
    first = dialog.nFromPage;
    last  = dialog.nToPage;

    if (first > last) {
      // Swap first/last page
      page  = first;
      first = last;
      last  = page;
    }
  } else {
    // Print everything...
    first = 1;
    last  = dialog.nMaxPage;
  }

  for (t = Fl_Type::first, num_windows = 0, winpage = 0; t; t = t->next) {
    if (t->is_window()) {
      winpage ++;
      windows[num_windows] = (Fl_Window_Type *)t;
      num_windows ++;
#if 0
      if (dialog.Flags & PD_ALLPAGES) num_windows ++;
      else if ((dialog.Flags & PD_PAGENUMS) && winpage >= first &&
               winpage <= last) num_windows ++;
      else if ((dialog.Flags & PD_SELECTION) && t->selected) num_windows ++;
#endif // 0
    }
  }

  num_pages = num_windows;

  // Figure out the page size and margins...
  int	width, length;			// Size of page
  int   xdpi, ydpi;			// Output resolution
  char	buffer[1024];

  width  = GetDeviceCaps(dialog.hDC, HORZRES);
  length = GetDeviceCaps(dialog.hDC, VERTRES);
  xdpi   = GetDeviceCaps(dialog.hDC, LOGPIXELSX);
  ydpi   = GetDeviceCaps(dialog.hDC, LOGPIXELSY);

//  fl_message("width=%d, length=%d, xdpi=%d, ydpi=%d, num_windows=%d\n",
//             width, length, xdpi, ydpi, num_windows);

  HDC	save_dc = fl_gc;
  HWND	save_win = fl_window;
  int	fontsize = 14 * ydpi / 72;

  fl_gc = dialog.hDC;
  fl_window = (HWND)dialog.hDC;
  fl_push_no_clip();

  // Get the time and date...
  time_t curtime = time(NULL);
  struct tm *curdate = localtime(&curtime);
  char date[1024];

  strftime(date, sizeof(date), "%c", curdate);

  // Print each of the windows...
  for (winpage = 0; winpage < num_windows; winpage ++) {
    // Draw header...
    StartPage(dialog.hDC);

    fl_font(FL_HELVETICA_BOLD, fontsize);
    fl_color(0, 0, 0);

    fl_draw(basename, 0, fontsize);

    fl_draw(date, (width - (int)fl_width(date)) / 2, fontsize);

    sprintf(buffer, "%d/%d", winpage + 1, num_windows);
    fl_draw(buffer, width - (int)fl_width(buffer), fontsize);

    // Get window image...
    uchar	*pixels;		// Window image data
    int		w, h;			// Window image dimensions
    int		ww, hh;			// Scaled size
    int		ulx, uly;		// Upper-lefthand corner
    Fl_Window	*win;			// Window widget
    BITMAPINFO	info;			// Bitmap information

    win    = (Fl_Window *)(windows[winpage]->o);
    pixels = windows[winpage]->read_image(w, h);

    // Swap colors: FLTK uses R-G-B --> Windows GDI uses B-G-R

    { uchar *p = pixels;
      for (int i=0; i<w*h; i++, p+=3) {
	uchar temp = p[0]; p[0] = p[2]; p[2] = temp;
      }
    }

    // Figure out the window size, first at 100 PPI and then scaled
    // down if that is too big...
    ww = w * xdpi / 100;
    hh = h * ydpi / 100;

    if (ww > width) {
      ww = width;
      hh = h * ww * ydpi / xdpi / w;
    }

    if (hh > (length - ydpi / 2)) {
      hh = length - ydpi / 2;
      ww = w * hh / h;
    }

    // Position the window in the center...
    ulx = (width - ww) / 2;
    uly = (length - hh) / 2;

//    fl_message("winpage=%d, ulx=%d, uly=%d, ww=%d, hh=%d",
//               winpage, ulx, uly, ww, hh);

    // Draw a simulated window border...
    int xborder = 4 * ww / w;
    int yborder = 4 * hh / h;

    win_box(ulx - xborder, uly - 5 * yborder,
            ww + 2 * xborder, hh + 6 * yborder);

    fl_color(0, 0, 255);
    fl_rectf(ulx, uly - 4 * yborder, ww, 4 * yborder);

    fl_font(FL_HELVETICA_BOLD, 2 * yborder);
    fl_color(255, 255, 255);
    fl_draw(win->label() ? win->label() : "Window",
            ulx + xborder, uly - 3 * yborder);

    int x = ulx + ww - 4 * xborder;

    win_box(x, uly - 4 * yborder, 4 * xborder, 4 * yborder);
    fl_color(0, 0, 0);
    fl_line(x + xborder, uly - yborder,
            x + 3 * xborder, uly - 3 * yborder);
    fl_line(x + xborder, uly - 3 * yborder,
            x + 3 * xborder, uly - yborder);
    x -= 4 * xborder;

    if (win->resizable()) {
      win_box(x, uly - 4 * yborder, 4 * xborder, 4 * yborder);
      fl_color(0, 0, 0);
      fl_rect(x + xborder, uly - 3 * yborder, 2 * xborder, 2 * yborder);
      x -= 4 * xborder;
    }

    if (!win->modal()) {
      win_box(x, uly - 4 * yborder, 4 * xborder, 4 * yborder);
      fl_color(0, 0, 0);
      fl_line(x + xborder, uly - yborder, x + 3 * xborder, uly - yborder);
      x -= 4 * xborder;
    }

    // Color image...
    memset(&info, 0, sizeof(info));
    info.bmiHeader.biSize        = sizeof(info);
    info.bmiHeader.biWidth       = w;
    info.bmiHeader.biHeight      = 1;
    info.bmiHeader.biPlanes      = 1;
    info.bmiHeader.biBitCount    = 24;
    info.bmiHeader.biCompression = BI_RGB;

    for (int y = 0; y < h; y ++) {
      StretchDIBits(dialog.hDC, ulx, uly + y * hh / h, ww, (hh + h - 1) / h, 0, 0, w, 1,
                    pixels + y * w * 3, &info, DIB_RGB_COLORS, SRCCOPY);
    }

    delete[] pixels;

    // Show the page...
    EndPage(dialog.hDC);
  }

  // Finish up...
  EndDoc(dialog.hDC);

  fl_gc = save_dc;
  fl_window = save_win;
  fl_pop_clip();

  // Free the print DC and return...
  DeleteDC(dialog.hDC);
}
#else
// Load and show the print dialog...
void print_menu_cb(Fl_Widget *, void *) {
  if (!print_panel) make_print_panel();

  print_load();

  print_selection->deactivate();

  for (Fl_Type *t = Fl_Type::first; t; t = t->next) {
    if (t->selected && t->is_window()) {
      print_selection->activate();
      break;
    }
  }

  print_all->setonly();
  print_all->do_callback();

  print_panel->show();
}

// Quote a string for PostScript printing
static const char *ps_string(const char *s) {
  char *bufptr;
  static char buffer[2048];


  if (!s) {
    buffer[0] = '\0';
  } else {
    for (bufptr = buffer; bufptr < (buffer + sizeof(buffer) - 3) && *s;) {
      if (*s == '(' || *s == ')' || *s == '\\') *bufptr++ = '\\';
      *bufptr++ = *s++;
    }

    *bufptr = '\0';
  }

  return (buffer);
}

// Actually print...
void print_cb(Fl_Return_Button *, void *) {
  FILE		*outfile;		// Output file or pipe to print command
  char		command[1024];		// Print command
  int		copies;			// Collated copies
  int		first, last;		// First and last page
  int		page;			// Current page
  int		winpage;		// Current window page
  int		num_pages;		// Number of pages
  Fl_Type	*t;			// Current widget
  int		num_windows;		// Number of windows
  Fl_Window_Type *windows[1000];	// Windows to print

  // Show progress, deactivate controls...
  print_panel_controls->deactivate();
  print_progress->show();

  // Figure out how many pages we'll have to print...
  if (print_collate_button->value()) copies = (int)print_copies->value();
  else copies = 1;

  if (print_pages->value()) {
    // Get from and to page numbers...
    if ((first = atoi(print_from->value())) < 1) first = 1;
    if ((last = atoi(print_to->value())) < 1) last = 1000;

    if (first > last) {
      // Swap first/last page
      page  = first;
      first = last;
      last  = page;
    }
  } else {
    // Print everything...
    first = 1;
    last  = 1000;
  }

  for (t = Fl_Type::first, num_windows = 0, winpage = 0; t; t = t->next) {
    if (t->is_window()) {
      winpage ++;
      windows[num_windows] = (Fl_Window_Type *)t;

      if (print_all->value()) num_windows ++;
      else if (print_pages->value() && winpage >= first &&
               winpage <= last) num_windows ++;
      else if (print_selection->value() && t->selected) num_windows ++;
    }
  }

  num_pages = num_windows * copies;

  print_progress->minimum(0);
  print_progress->maximum(num_pages);
  print_progress->value(0);
  Fl::check();

  // Get the base filename...
  const char *basename = strrchr(filename, '/');
  if (basename) basename ++;
  else basename = filename;

  // Open the print stream...
  if (print_choice->value()) {
    // Pipe the output into the lp command...
    const char *printer = (const char *)print_choice->menu()[print_choice->value()].user_data();

    snprintf(command, sizeof(command), "lp -s -d %s -n %.0f -t '%s' -o media=%s",
             printer, print_collate_button->value() ? 1.0 : print_copies->value(),
	     basename, print_page_size->text(print_page_size->value()));
    outfile = popen(command, "w");
  } else {
    // Print to file...
    fl_file_chooser_ok_label("Print");
    const char *outname = fl_file_chooser("Print To", "PostScript (*.ps)", NULL, 1);
    fl_file_chooser_ok_label(NULL);

    if (outname && !access(outname, 0)) {
      if (fl_choice("The file \"%s\" already exists.\n"
                    "Do you want to replace it?", "Cancel",
		    "Replace", NULL, outname) == 0) outname = NULL;
    }

    if (outname) outfile = fopen(outname, "w");
    else outfile = NULL;
  }

  if (outfile) {
    // Figure out the page size and margins...
    int	width, length;			// Size of page
    int	left, bottom,			// Bottom lefthand corner
	right, top;			// Top righthand corner

    if (print_page_size->value()) {
      // A4
      width  = 595;
      length = 842;
    } else {
      // Letter
      width  = 612;
      length = 792;
    }

    int output_mode;
    for (output_mode = 0; output_mode < 4; output_mode ++) {
      if (print_output_mode[output_mode]->value()) break;
    }

    if (output_mode & 1) {
      // Landscape
      left   = 36;
      bottom = 18;
      right  = length - 36;
      top    = width - 18;
    } else {
      // Portrait
      left   = 18;
      bottom = 36;
      right  = width - 18;
      top    = length - 36;
    }

    // Get the time and date...
    time_t curtime = time(NULL);
    struct tm *curdate = localtime(&curtime);
    char date[1024];

    strftime(date, sizeof(date), "%c", curdate);

    // Write the prolog...
    fprintf(outfile,
            "%%!PS-Adobe-3.0\n"
	    "%%%%BoundingBox: 18 36 %d %d\n"
	    "%%%%Pages: %d\n"
	    "%%%%LanguageLevel: 1\n"
	    "%%%%DocumentData: Clean7Bit\n"
	    "%%%%DocumentNeededResources: font Helvetica-Bold\n"
	    "%%%%Creator: FLUID %.4f\n"
	    "%%%%CreationDate: %s\n"
	    "%%%%Title: (%s)\n"
	    "%%%%EndComments\n"
	    "%%%%BeginProlog\n"
	    "%%languagelevel 1 eq {\n"
	    "  /rectfill {\n"
	    "    newpath 4 2 roll moveto dup 0 exch rlineto exch 0 rlineto\n"
	    "    neg 0 exch rlineto closepath fill\n"
	    "  } bind def\n"
	    "  /rectstroke {\n"
	    "    newpath 4 2 roll moveto dup 0 exch rlineto exch 0 rlineto\n"
	    "    neg 0 exch rlineto closepath stroke\n"
	    "  } bind def\n"
	    "%%} if\n"
	    "%%%%EndProlog\n"
	    "%%%%BeginSetup\n"
	    "%%%%BeginFeature: *PageSize %s\n"
	    "languagelevel 1 ne {\n"
	    "  <</PageSize[%d %d]/ImagingBBox null>>setpagedevice\n"
	    "} {\n"
	    "  %s\n"
	    "} ifelse\n"
	    "%%%%EndFeature\n"
	    "%%%%EndSetup\n",
	    width - 18, length - 36,
	    num_pages,
	    FL_VERSION,
	    date,
	    basename,
	    print_page_size->text(print_page_size->value()),
	    width, length,
	    print_page_size->value() ? "a4tray" : "lettertray");

    // Print each of the windows...
    char	progress[255];		// Progress text
    int		copy;			// Current copy

    for (copy = 0, page = 0; copy < copies; copy ++) {
      for (winpage = 0; winpage < num_pages; winpage ++) {
        // Start next page...
        page ++;
	sprintf(progress, "Printing page %d/%d...", page, num_pages);
	print_progress->value(page);
	print_progress->label(progress);
	Fl::check();

        // Add common page stuff...
	fprintf(outfile,
	        "%%%%Page: %d %d\n"
		"gsave\n",
		page, page);

        if (output_mode & 1) {
	  // Landscape...
	  fprintf(outfile, "%d 0 translate 90 rotate\n", width);
	}

        // Draw header...
	fprintf(outfile,
		"0 setgray\n"
		"/Helvetica-Bold findfont 14 scalefont setfont\n"
		"%d %d moveto (%s) show\n"
		"%.1f %d moveto (%s) dup stringwidth pop -0.5 mul 0 rmoveto show\n"
		"%d %d moveto (%d/%d) dup stringwidth pop neg 0 rmoveto show\n",
	        left, top - 15, ps_string(basename),
		0.5 * (left + right), top - 15, date,
		right, top - 15, winpage + 1, num_windows);

        // Get window image...
	uchar	*pixels;		// Window image data
        int	w, h;			// Window image dimensions
	float	ww, hh;			// Scaled size
	float	border;			// Width of 1 pixel
        float	llx, lly,		// Lower-lefthand corner
		urx, ury;		// Upper-righthand corner
	Fl_Window *win;			// Window widget

        win    = (Fl_Window *)(windows[winpage]->o);
	pixels = windows[winpage]->read_image(w, h);

        // Figure out the window size, first at 100 PPI and then scaled
	// down if that is too big...
        ww = w * 72.0 / 100.0;
	hh = h * 72.0 / 100.0;

        if (ww > (right - left)) {
	  ww = right - left;
	  hh = h * ww / w;
	}

        if (hh > (top - bottom - 36)) {
	  hh = top - bottom;
	  ww = w * hh / h;
	}

        border = ww / w;

	// Position the window in the center...
	llx = 0.5 * (right - left - ww);
	lly = 0.5 * (top - bottom - hh);
	urx = 0.5 * (right - left + ww);
	ury = 0.5 * (top - bottom + hh);

        // Draw a simulated window border...
        fprintf(outfile,
	        "0.75 setgray\n"			// Gray background
	        "newpath %.2f %.2f %.2f 180 90 arcn\n"	// Top left
		"%.2f %.2f %.2f 90 0 arcn\n"		// Top right
		"%.2f %.2f %.2f 0 -90 arcn\n"		// Bottom right
		"%.2f %.2f %.2f -90 -180 arcn\n"	// Bottom left
		"closepath gsave fill grestore\n"	// Fill
		"0 setlinewidth 0 setgray stroke\n",	// Outline
		llx, ury + 12 * border, 4 * border,
		urx, ury + 12 * border, 4 * border,
		urx, lly, 4 * border,
		llx, lly, 4 * border);

        // Title bar...
	if (output_mode & 2) {
	  fputs("0.25 setgray\n", outfile);
	} else {
	  fputs("0.1 0.2 0.6 setrgbcolor\n", outfile);
	}

        fprintf(outfile, "%.2f %.2f %.2f %.2f rectfill\n",
	        llx + 12 * border, ury,
		ww - (24 + 16 * (!win->modal() || win->resizable()) +
		      16 * (!win->modal() && win->resizable())) * border,
		16 * border);

        if (win->resizable()) {
	  fprintf(outfile,
		  "%.2f %.2f %.2f -90 -180 arcn\n"	// Bottom left
	          "0 %.2f rlineto %.2f 0 rlineto 0 -%.2f rlineto closepath fill\n"
		  "%.2f %.2f %.2f 0 -90 arcn\n"	// Bottom right
	          "-%.2f 0 rlineto 0 %.2f rlineto %.2f 0 rlineto closepath fill\n",
		  llx, lly, 4 * border,
		  12 * border, 16 * border, 16 * border,
		  urx, lly, 4 * border,
		  12 * border, 16 * border, 16 * border);
	}

        // Inside outline and button shading...
        fprintf(outfile,
	        "%.2f setlinewidth 0.5 setgray\n"
		"%.2f %.2f %.2f %.2f rectstroke\n"
		"%.2f %.2f moveto 0 %.2f rlineto\n"
		"%.2f %.2f moveto 0 %.2f rlineto\n",
		border,
		llx - 0.5 * border, lly - 0.5 * border, ww + border, hh + border,
		llx + 12 * border, ury, 16 * border,
		urx - 12 * border, ury, 16 * border);

        if (!win->modal() || win->resizable()) {
	  fprintf(outfile, "%.2f %.2f moveto 0 %.2f rlineto\n",
	          urx - 28 * border, ury, 16 * border);
	}

        if (!win->modal() && win->resizable()) {
	  fprintf(outfile, "%.2f %.2f moveto 0 %.2f rlineto\n",
	          urx - 44 * border, ury, 16 * border);
	}

        fprintf(outfile, "%.2f %.2f moveto %.2f 0 rlineto stroke\n",
		llx - 3.5 * border, ury + 0.5 * border, ww + 7 * border);

        // Button icons...
        fprintf(outfile,
	        "%.2f setlinewidth 0 setgray\n"
		"%.2f %.2f moveto %.2f -%.2f rlineto %.2f %.2f rlineto\n"
		"%.2f %.2f moveto -%.2f -%.2f rlineto 0 %.2f rmoveto %.2f -%.2f rlineto\n",
		2 * border,
		llx, ury + 10 * border, 4 * border, 4 * border, 4 * border, 4 * border,
		urx, ury + 12 * border, 8 * border, 8 * border, 8 * border, 8 * border, 8 * border);

        float x = urx - 16 * border;

        if (win->resizable()) {
	  // Maximize button
	  fprintf(outfile,
	          "%.2f %.2f moveto -%.2f 0 rlineto 0 -%.2f rlineto "
		  "%.2f 0 rlineto 0 %.2f rlineto\n",
		  x, ury + 12 * border, 8 * border, 8 * border,
		  8 * border, 8 * border);

          x -= 16 * border;
        }

        if (!win->modal()) {
	  // Minimize button
	  fprintf(outfile,
	          "%.2f %.2f moveto -%.2f 0 rlineto\n",
		  x, ury + 4 * border, 8 * border);
        }

        fputs("stroke\n", outfile);

        if (win->label()) {
	  // Add window title...
	  fprintf(outfile,
		  "1 setgray\n"
		  "/Helvetica-Bold findfont %.2f scalefont setfont\n"
		  "(%s) %.2f %.2f moveto show\n",
		  12 * border,
		  ps_string(win->label()), llx + 16 * border, ury + 4 * border);
	}

        fprintf(outfile,
	        "gsave\n"
		"%.2f %.2f translate %.2f %.2f scale\n",
		llx, ury - border, border, border);

        if (output_mode & 2) {
	  // Grayscale image...
	  fprintf(outfile,
	          "/imgdata %d string def\n"
		  "%d %d 8[1 0 0 -1 0 1] "
		  "{currentfile imgdata readhexstring pop} image\n",
		  w,
		  w, h);

          uchar *ptr = pixels;
	  int i, count = w * h;

          for (i = 0; i < count; i ++, ptr += 3) {
	    fprintf(outfile, "%02X",
	            (31 * ptr[0] + 61 * ptr[1] + 8 * ptr[2]) / 100);
	    if (!(i % 40)) putc('\n', outfile);
	  }
	} else {
	  // Color image...
	  fprintf(outfile,
	          "/imgdata %d string def\n"
		  "%d %d 8[1 0 0 -1 0 1] "
		  "{currentfile imgdata readhexstring pop} false 3 colorimage\n",
		  w * 3,
		  w, h);

          uchar *ptr = pixels;
	  int i, count = w * h;

          for (i = 0; i < count; i ++, ptr += 3) {
	    fprintf(outfile, "%02X%02X%02X", ptr[0], ptr[1], ptr[2]);
	    if (!(i % 13)) putc('\n', outfile);
	  }
	}

        fputs("\ngrestore\n", outfile);

        delete[] pixels;

        // Show the page...
	fputs("grestore showpage\n", outfile);
      }
    }

    // Finish up...
    fputs("%%EOF\n", outfile);

    if (print_choice->value()) pclose(outfile);
    else fclose(outfile);
  } else {
    // Unable to print...
    fl_alert("Error printing: %s", strerror(errno));
  }

  // Hide progress, activate controls, hide print panel...
  print_panel_controls->activate();
  print_progress->hide();
  print_panel->hide();
}
#endif // WIN32 && !__CYGWIN__

void fltkdb_cb(Fl_Widget*, void*) {
  Fl_Plugin_Manager pm("commandline");  
  Fl_Commandline_Plugin *pi = (Fl_Commandline_Plugin*)pm.plugin("FltkDB.fluid.fltk.org");
  if (pi) pi->test("/Users/matt/dev/fltk-1.3.0/fltk.db");
}

void dbxcode_cb(Fl_Widget*, void*) {
  Fl_Plugin_Manager pm("commandline");  
  Fl_Commandline_Plugin *pi = (Fl_Commandline_Plugin*)pm.plugin("ideXcode.fluid.fltk.org");
  if (pi) pi->test("/Users/matt/dev/fltk-1.3.0/fltk.db", "/Users/matt/dev/fltk-1.3.0");
}

void dbvisualc_cb(Fl_Widget*, void*) {
  Fl_Plugin_Manager pm("commandline");  
  Fl_Commandline_Plugin *pi = (Fl_Commandline_Plugin*)pm.plugin("ideVisualC.fluid.fltk.org");
  if (pi) pi->test("/Users/matt/dev/fltk-1.3.0/fltk.db", "/Users/matt/dev/fltk-1.3.0");
}

void dbmake_cb(Fl_Widget*, void*) {
  Fl_Plugin_Manager pm("commandline");  
  Fl_Commandline_Plugin *pi = (Fl_Commandline_Plugin*)pm.plugin("ideMaketools.fluid.fltk.org");
  if (pi) pi->test("/Users/matt/dev/fltk-1.3.0/fltk.db", "/Users/matt/dev/fltk-1.3.0");
}

////////////////////////////////////////////////////////////////

extern Fl_Menu_Item New_Menu[];

void toggle_widgetbin_cb(Fl_Widget *, void *);
void toggle_sourceview_cb(Fl_Double_Window *, void *);

Fl_Menu_Item Main_Menu[] = {
{"&File",0,0,0,FL_SUBMENU},
  {"&New...", FL_COMMAND+'n', new_cb, 0},
  {"&Open...", FL_COMMAND+'o', open_cb, 0},
  {"&Insert...", FL_COMMAND+'i', open_cb, (void*)1, FL_MENU_DIVIDER},
  {"&Save", FL_COMMAND+'s', save_cb, 0},
  {"Save &As...", FL_COMMAND+FL_SHIFT+'s', save_cb, (void*)1},
  {"Sa&ve A Copy...", 0, save_cb, (void*)2},
  {"Save &Template...", 0, save_template_cb},
  {"&Revert...", 0, revert_cb, 0, FL_MENU_DIVIDER},
  {"&Print...", FL_COMMAND+'p', print_menu_cb},
  {"Write &Code...", FL_COMMAND+FL_SHIFT+'c', write_cb, 0},
  {"&Write Strings...", FL_COMMAND+FL_SHIFT+'w', write_strings_cb, 0, FL_MENU_DIVIDER},
  {relative_history[0], FL_COMMAND+'0', open_history_cb, absolute_history[0]},
  {relative_history[1], FL_COMMAND+'1', open_history_cb, absolute_history[1]},
  {relative_history[2], FL_COMMAND+'2', open_history_cb, absolute_history[2]},
  {relative_history[3], FL_COMMAND+'3', open_history_cb, absolute_history[3]},
  {relative_history[4], FL_COMMAND+'4', open_history_cb, absolute_history[4]},
  {relative_history[5], FL_COMMAND+'5', open_history_cb, absolute_history[5]},
  {relative_history[6], FL_COMMAND+'6', open_history_cb, absolute_history[6]},
  {relative_history[7], FL_COMMAND+'7', open_history_cb, absolute_history[7]},
  {relative_history[8], FL_COMMAND+'8', open_history_cb, absolute_history[8]},
  {relative_history[9], FL_COMMAND+'9', open_history_cb, absolute_history[9], FL_MENU_DIVIDER},
  {"&Quit", FL_COMMAND+'q', exit_cb},
  {0},
{"&Edit",0,0,0,FL_SUBMENU},
  {"&Undo", FL_COMMAND+'z', undo_cb},
  {"&Redo", FL_COMMAND+FL_SHIFT+'z', redo_cb, 0, FL_MENU_DIVIDER},
  {"C&ut", FL_COMMAND+'x', cut_cb},
  {"&Copy", FL_COMMAND+'c', copy_cb},
  {"&Paste", FL_COMMAND+'v', paste_cb},
  {"Dup&licate", FL_COMMAND+'u', duplicate_cb},
  {"&Delete", FL_Delete, delete_cb, 0, FL_MENU_DIVIDER},
  {"Select &All", FL_COMMAND+'a', select_all_cb},
  {"Select &None", FL_COMMAND+FL_SHIFT+'a', select_none_cb, 0, FL_MENU_DIVIDER},
  {"Pr&operties...", FL_F+1, openwidget_cb},
  {"&Sort",0,sort_cb},
  {"&Earlier", FL_F+2, earlier_cb},
  {"&Later", FL_F+3, later_cb},
  {"&Group", FL_F+7, group_cb},
  {"Ung&roup", FL_F+8, ungroup_cb,0, FL_MENU_DIVIDER},
  {"Hide O&verlays",FL_COMMAND+FL_SHIFT+'o',toggle_overlays},
  {"Show Widget &Bin...",FL_ALT+'b',toggle_widgetbin_cb},
  {"Show Source Code...",FL_ALT+FL_SHIFT+'s', (Fl_Callback*)toggle_sourceview_cb, 0, FL_MENU_DIVIDER},
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
  {"Set &Widget Size",0,0,0,FL_SUBMENU|FL_MENU_DIVIDER},
    {"&Tiny",FL_ALT+'1',(Fl_Callback *)widget_size_cb,(void*)8,0,FL_NORMAL_LABEL,FL_HELVETICA,8},
    {"&Small",FL_ALT+'2',(Fl_Callback *)widget_size_cb,(void*)11,0,FL_NORMAL_LABEL,FL_HELVETICA,11},
    {"&Normal",FL_ALT+'3',(Fl_Callback *)widget_size_cb,(void*)14,0,FL_NORMAL_LABEL,FL_HELVETICA,14},
    {"&Medium",FL_ALT+'4',(Fl_Callback *)widget_size_cb,(void*)18,0,FL_NORMAL_LABEL,FL_HELVETICA,18},
    {"&Large",FL_ALT+'5',(Fl_Callback *)widget_size_cb,(void*)24,0,FL_NORMAL_LABEL,FL_HELVETICA,24},
    {"&Huge",FL_ALT+'6',(Fl_Callback *)widget_size_cb,(void*)32,0,FL_NORMAL_LABEL,FL_HELVETICA,32},
    {0},
  {"&Grid and Size Settings...",FL_COMMAND+'g',show_grid_cb},
  {0},
{"&Shell",0,0,0,FL_SUBMENU},
  {"Execute &Command...",FL_ALT+'x',(Fl_Callback *)show_shell_window},
  {"Execute &Again...",FL_ALT+'g',(Fl_Callback *)do_shell_command},
#ifdef IDE_SUPPORT
  {"--fltkdb",0,(Fl_Callback *)fltkdb_cb},
  {"--dbxcode3",0,(Fl_Callback *)dbxcode_cb},
  {"--dbvisualc6",0,(Fl_Callback *)dbvisualc_cb},
  {"--dbmake",0,(Fl_Callback *)dbmake_cb},
#endif
  {0},
{"&Help",0,0,0,FL_SUBMENU},
  {"&Rapid development with FLUID...",0,help_cb},
  {"&FLTK Programmers Manual...",0,manual_cb, 0, FL_MENU_DIVIDER},
  {"&About FLUID...",0,about_cb},
  {0},
{0}};

#define BROWSERWIDTH 300
#define BROWSERHEIGHT 500
#define WINWIDTH 300
#define MENUHEIGHT 25
#define WINHEIGHT (BROWSERHEIGHT+MENUHEIGHT)

extern void fill_in_New_Menu();

void scheme_cb(Fl_Choice *, void *) {
  if (compile_only)
    return;

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
    case 3 : // GTK+
      Fl::scheme("gtk+");
      break;
  }

  fluid_prefs.set("scheme", scheme_choice->value());
}

void toggle_widgetbin_cb(Fl_Widget *, void *) {
  if (!widgetbin_panel) {
    make_widgetbin();
    if (!position_window(widgetbin_panel,"widgetbin_pos", 1, 320, 30)) return;
  }

  if (widgetbin_panel->visible()) {
    widgetbin_panel->hide();
    widgetbin_item->label("Show Widget &Bin...");
  } else {
    widgetbin_panel->show();
    widgetbin_item->label("Hide Widget &Bin");
  }
}


void toggle_sourceview_cb(Fl_Double_Window *, void *) {
  if (!sourceview_panel) {
    make_sourceview();
    sourceview_panel->callback((Fl_Callback*)toggle_sourceview_cb);
    Fl_Preferences svp(fluid_prefs, "sourceview");
    int autorefresh;
    svp.get("autorefresh", autorefresh, 1);
    sv_autorefresh->value(autorefresh);
    int autoposition;
    svp.get("autoposition", autoposition, 1);
    sv_autoposition->value(autoposition);
    int tab;
    svp.get("tab", tab, 0);
    if (tab>=0 && tab<sv_tab->children()) sv_tab->value(sv_tab->child(tab));
    if (!position_window(sourceview_panel,"sourceview_pos", 0, 320, 120, 550, 500)) return;
  }

  if (sourceview_panel->visible()) {
    sourceview_panel->hide();
    sourceview_item->label("Show Source Code...");
  } else {
    sourceview_panel->show();
    sourceview_item->label("Hide Source Code...");
    update_sourceview_cb(0,0);
  }
}

void toggle_sourceview_b_cb(Fl_Button*, void *) {
  toggle_sourceview_cb(0,0);
}

void make_main_window() {
  if (!compile_only) {
    fluid_prefs.get("snap", snap, 1);
    fluid_prefs.get("gridx", gridx, 5);
    fluid_prefs.get("gridy", gridy, 5);
    fluid_prefs.get("show_guides", show_guides, 0);
    fluid_prefs.get("widget_size", Fl_Widget_Type::default_size, 14);
    fluid_prefs.get("show_comments", show_comments, 1);
    make_layout_window();
    make_shell_window();
  }

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
    // quick access to all dynamic menu items
    save_item = (Fl_Menu_Item*)main_menubar->find_item(save_cb);
    history_item = (Fl_Menu_Item*)main_menubar->find_item(open_history_cb);
    widgetbin_item = (Fl_Menu_Item*)main_menubar->find_item(toggle_widgetbin_cb);
    sourceview_item = (Fl_Menu_Item*)main_menubar->find_item((Fl_Callback*)toggle_sourceview_cb);
    main_menubar->global();
    fill_in_New_Menu();
    main_window->end();
  }

  if (!compile_only) {
    load_history();
    make_settings_window();
  }
}

// Load file history from preferences...
void load_history() {
  int	i;		// Looping var
  int	max_files;


  fluid_prefs.get("recent_files", max_files, 5);
  if (max_files > 10) max_files = 10;

  for (i = 0; i < max_files; i ++) {
    fluid_prefs.get( Fl_Preferences::Name("file%d", i), absolute_history[i], "", sizeof(absolute_history[i]));
    if (absolute_history[i][0]) {
      // Make a relative version of the filename for the menu...
      fl_filename_relative(relative_history[i], sizeof(relative_history[i]),
                           absolute_history[i]);

      if (i == 9) history_item[i].flags = FL_MENU_DIVIDER;
      else history_item[i].flags = 0;
    } else break;
  }

  for (; i < 10; i ++) {
    if (i) history_item[i-1].flags |= FL_MENU_DIVIDER;
    history_item[i].hide();
  }
}

// Update file history from preferences...
void update_history(const char *flname) {
  int	i;		// Looping var
  char	absolute[1024];
  int	max_files;


  fluid_prefs.get("recent_files", max_files, 5);
  if (max_files > 10) max_files = 10;

  fl_filename_absolute(absolute, sizeof(absolute), flname);

  for (i = 0; i < max_files; i ++)
#if defined(WIN32) || defined(__APPLE__)
    if (!strcasecmp(absolute, absolute_history[i])) break;
#else
    if (!strcmp(absolute, absolute_history[i])) break;
#endif // WIN32 || __APPLE__

  if (i == 0) return;

  if (i >= max_files) i = max_files - 1;

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
  for (i = 0; i < max_files; i ++) {
    fluid_prefs.set( Fl_Preferences::Name("file%d", i), absolute_history[i]);
    if (absolute_history[i][0]) {
      if (i == 9) history_item[i].flags = FL_MENU_DIVIDER;
      else history_item[i].flags = 0;
    } else break;
  }

  for (; i < 10; i ++) {
    fluid_prefs.set( Fl_Preferences::Name("file%d", i), "");
    if (i) history_item[i-1].flags |= FL_MENU_DIVIDER;
    history_item[i].hide();
  }
}

// ********** portable process class definition **********

class Fl_Process {
public:
  // construction / destruction
  Fl_Process() {_fpt= NULL;}
  ~Fl_Process() {if (_fpt) close();}

  FILE * popen	(const char *cmd, const char *mode="r");
  //not necessary here: FILE * fopen	(const char *file, const char *mode="r");
  int  close();

  FILE * desc() const { return _fpt;} // non null if file is open
  char * get_line(char * line, size_t s) const {return _fpt ? fgets(line, s, _fpt) : NULL;}

#if defined(WIN32)  && !defined(__CYGWIN__)
protected:
  HANDLE pin[2], pout[2], perr[2];
  char ptmode;
  PROCESS_INFORMATION pi;
  STARTUPINFO si;

  static bool createPipe(HANDLE * h, BOOL bInheritHnd=TRUE);

private:
  FILE * freeHandles()  {
    clean_close(pin[0]);    clean_close(pin[1]);
    clean_close(pout[0]);   clean_close(pout[1]);
    clean_close(perr[0]);   clean_close(perr[1]);
    return NULL; // convenient for error management
  }
  static void clean_close(HANDLE& h);
#endif

protected:
  FILE * _fpt;
};

#if defined(WIN32)  && !defined(__CYGWIN__)
bool Fl_Process::createPipe(HANDLE * h, BOOL bInheritHnd) {
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = bInheritHnd;
  return CreatePipe (&h[0],&h[1],&sa,0) ? true : false;
}
#endif
// portable open process:
FILE * Fl_Process::popen(const char *cmd, const char *mode) {
#if defined(WIN32)  && !defined(__CYGWIN__)
  // PRECONDITIONS
  if (!mode || !*mode || (*mode!='r' && *mode!='w') ) return NULL;
  if (_fpt) close(); // close first before reuse

  ptmode = *mode;
  pin[0] = pin[1] = pout[0] = pout[1] = perr[0] = perr[1] = INVALID_HANDLE_VALUE;
  // stderr to stdout wanted ?
  int fusion = (strstr(cmd,"2>&1") !=NULL);

  // Create windows pipes
  if (!createPipe(pin) || !createPipe(pout) || (!fusion && !createPipe(perr) ) )
	return freeHandles(); // error

  // Initialize Startup Info
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb           = sizeof(STARTUPINFO);
  si.dwFlags    = STARTF_USESTDHANDLES;
  si.hStdInput    = pin[0];
  si.hStdOutput   = pout[1];
  si.hStdError  = fusion ? pout[1] : perr [1];

  if ( CreateProcess(NULL, (LPTSTR) cmd,NULL,NULL,TRUE,
		     DETACHED_PROCESS,NULL,NULL, &si, &pi)) {
    // don't need theses handles inherited by child process:
    clean_close(pin[0]); clean_close(pout[1]); clean_close(perr[1]);
    HANDLE & h = *mode == 'r' ? pout[0] : pin[1];
    _fpt = _fdopen(_open_osfhandle((long) h,_O_BINARY),mode);
    h= INVALID_HANDLE_VALUE;  // reset the handle pointer that is shared
    // with _fpt so we don't free it twice
  }

  if (!_fpt)  freeHandles();
  return _fpt;
#else
  _fpt=::popen(cmd,mode);
  return _fpt;
#endif
}

int Fl_Process::close() {
#if defined(WIN32)  && !defined(__CYGWIN__)
  if (_fpt) {
    fclose(_fpt);
    clean_close(perr[0]);
    clean_close(pin[1]);
    clean_close(pout[0]);
    _fpt = NULL;
    return 0;
  }
  return -1;
#else
  int ret = ::pclose(_fpt);
  _fpt=NULL;
  return ret;
#endif
}

#if defined(WIN32)  && !defined(__CYGWIN__)
void Fl_Process::clean_close(HANDLE& h) {
  if (h!= INVALID_HANDLE_VALUE) CloseHandle(h);
  h = INVALID_HANDLE_VALUE;
}
#endif
// ********** Fl_Process class end **********

static Fl_Process s_proc;

// Shell command support...

static bool prepare_shell_command(const char * &command)  { // common pre-shell command code all platforms
  shell_window->hide();
  if (s_proc.desc()) {
    fl_alert("Previous shell command still running!");
    return false;
  }
  if ((command = shell_command_input->value()) == NULL || !*command) {
    fl_alert("No shell command entered!");
    return false;
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
  return true;
}

#if !defined(__MWERKS__)
// Support the full piped shell command...
void
shell_pipe_cb(int, void*) {
  char	line[1024]="";		// Line from command output...

  if (s_proc.get_line(line, sizeof(line)) != NULL) {
    // Add the line to the output list...
    shell_run_buffer->append(line);
  } else {
    // End of file; tell the parent...
    Fl::remove_fd(fileno(s_proc.desc()));
    s_proc.close();
    shell_run_buffer->append("... END SHELL COMMAND ...\n");
  }

  shell_run_display->scroll(shell_run_display->count_lines(0,
                            shell_run_buffer->length(), 1), 0);
}

void
do_shell_command(Fl_Return_Button*, void*) {
  const char	*command=NULL;	// Command to run

  if (!prepare_shell_command(command)) return;

  // Show the output window and clear things...
  shell_run_buffer->text("");
  shell_run_buffer->append(command);
  shell_run_buffer->append("\n");
  shell_run_window->label("Shell Command Running...");

  if (s_proc.popen((char *)command) == NULL) {
    fl_alert("Unable to run shell command: %s", strerror(errno));
    return;
  }

  shell_run_button->deactivate();
  shell_run_window->hotspot(shell_run_display);
  shell_run_window->show();

  Fl::add_fd(fileno(s_proc.desc()), shell_pipe_cb);

  while (s_proc.desc()) Fl::wait();

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

  if (!prepare_shell_command(command)) return;

  if ((status = system(command)) != 0) {
    fl_alert("Shell command returned status %d!", status);
  } else if (completion_button->value()) {
    fl_message("Shell command completed successfully!");
  }
}
#endif // !__MWERKS__

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

//
// The Source View system offers an immediate preview of the code
// files that will be generated by FLUID. It also marks the code
// generated for the last selected item in the header and the source
// file.
//
// Can we patent this?  ;-)  - Matt, mm@matthiasm.com
//

//
// Update the header and source code highlighting depending on the
// currently selected object
//
void update_sourceview_position()
{
  if (!sourceview_panel || !sourceview_panel->visible())
    return;
  if (sv_autoposition->value()==0)
    return;
  if (sourceview_panel && sourceview_panel->visible() && Fl_Type::current) {
    int pos0, pos1;
    if (sv_source->visible_r()) {
      pos0 = Fl_Type::current->code_line;
      pos1 = Fl_Type::current->code_line_end;
      if (pos0>=0) {
        if (pos1<pos0)
          pos1 = pos0;
        sv_source->buffer()->highlight(pos0, pos1);
        int line = sv_source->buffer()->count_lines(0, pos0);
        sv_source->scroll(line, 0);
      }
    }
    if (sv_header->visible_r()) {
      pos0 = Fl_Type::current->header_line;
      pos1 = Fl_Type::current->header_line_end;
      if (pos0>=0) {
        if (pos1<pos0)
          pos1 = pos0;
        sv_header->buffer()->highlight(pos0, pos1);
        int line = sv_header->buffer()->count_lines(0, pos0);
        sv_header->scroll(line, 0);
      }
    }
  }
}

void update_sourceview_position_cb(Fl_Tabs*, void*)
{
  update_sourceview_position();
}

static char *sv_source_filename = 0;
static char *sv_header_filename = 0;

//
// Generate a header and source file in a temporary directory and
// load those into the Code Viewer widgets.
//
void update_sourceview_cb(Fl_Button*, void*)
{
  if (!sourceview_panel || !sourceview_panel->visible())
    return;
  // generate space for the source and header file filenames
  if (!sv_source_filename) {
    sv_source_filename = (char*)malloc(FL_PATH_MAX);
    fluid_prefs.getUserdataPath(sv_source_filename, FL_PATH_MAX);
    strlcat(sv_source_filename, "source_view_tmp.cxx", FL_PATH_MAX);
  }
  if (!sv_header_filename) {
    sv_header_filename = (char*)malloc(FL_PATH_MAX);
    fluid_prefs.getUserdataPath(sv_header_filename, FL_PATH_MAX);
    strlcat(sv_header_filename, "source_view_tmp.h", FL_PATH_MAX);
  }

  strlcpy(i18n_program, fl_filename_name(sv_source_filename), sizeof(i18n_program));
  fl_filename_setext(i18n_program, sizeof(i18n_program), "");
  const char *code_file_name_bak = code_file_name;
  code_file_name = sv_source_filename;
  const char *header_file_name_bak = header_file_name;
  header_file_name = sv_header_filename;

  // generate the code and load the files
  write_sourceview = 1;
  // generate files
  if (write_code(sv_source_filename, sv_header_filename))
  {
    // load file into source editor
    int pos = sv_source->top_line();
    sv_source->buffer()->loadfile(sv_source_filename);
    sv_source->scroll(pos, 0);
    // load file into header editor
    pos = sv_header->top_line();
    sv_header->buffer()->loadfile(sv_header_filename);
    sv_header->scroll(pos, 0);
    // update the source code highlighting
    update_sourceview_position();
  }
  write_sourceview = 0;

  code_file_name = code_file_name_bak;
  header_file_name = header_file_name_bak;
}

void update_sourceview_timer(void*)
{
  update_sourceview_cb(0,0);
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
  // if the UI was modified in any way, update the Source View panel
  if (sourceview_panel && sourceview_panel->visible() && sv_autorefresh->value())
  {
    // we will only update ealiest 0.5 seconds after the last change, and only
    // if no other change was made, so dragging a widget will not generate any
    // CPU load
    Fl::remove_timeout(update_sourceview_timer, 0);
    Fl::add_timeout(0.5, update_sourceview_timer, 0);
  }

  // Enable/disable the Save menu item...
  if (modflag) save_item->activate();
  else save_item->deactivate();
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
  Fl_Plugin_Manager pm("commandline");
  int j, n = pm.plugins();
  for (j=0; j<n; j++) {
    Fl_Commandline_Plugin *pi = (Fl_Commandline_Plugin*)pm.plugin(j);
    int r = pi->arg(argc, argv, i);
    if (r) return r;
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
    fprintf(stderr,
"usage: %s <switches> name.fl\n"
" -c : write .cxx and .h and exit\n"
" -cs : write .cxx and .h and strings and exit\n"
" -o <name> : .cxx output filename, or extension if <name> starts with '.'\n"
" -h <name> : .h output filename, or extension if <name> starts with '.'\n"
            , argv[0]);
    Fl_Plugin_Manager pm("commandline");
    int i, n = pm.plugins();
    for (i=0; i<n; i++) {
      Fl_Commandline_Plugin *pi = (Fl_Commandline_Plugin*)pm.plugin(i);
      if (pi) puts(pi->help());
    }
    fprintf(stderr, "%s\n", Fl::help);
    return 1;
  }
  if (exit_early)
    exit(0);
  
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
    toggle_sourceview_cb(0,0);
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
