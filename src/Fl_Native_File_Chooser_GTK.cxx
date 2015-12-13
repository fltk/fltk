// "$Id$"
//
// FLTK native file chooser widget wrapper for GTK's GtkFileChooserDialog 
//
// Copyright 1998-2014 by Bill Spitzak and others.
// Copyright 2012 IMM
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#include <FL/x.H>
#if HAVE_DLSYM && HAVE_DLFCN_H
#include <dlfcn.h>   // for dlopen et al
#endif
#include <locale.h>  // for setlocale

/* --------------------- Type definitions from GLIB and GTK --------------------- */
/* all of this is from the public gnome API, so unlikely to change */
#ifndef	FALSE
#define	FALSE	(0)
#endif
#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif
typedef void* gpointer;
typedef int    gint;
typedef unsigned int    guint;
typedef unsigned long   gulong;
typedef gint   gboolean;
typedef char   gchar;
typedef struct _GSList GSList;
struct _GSList
{
  gpointer data;
  GSList *next;
};
#define  g_slist_next(slist)	         ((slist) ? (((GSList *)(slist))->next) : NULL)
typedef struct _GtkWidget      GtkWidget;
typedef struct _GtkFileChooser      GtkFileChooser;
typedef struct _GtkDialog        GtkDialog;
typedef struct _GtkWindow          GtkWindow;
typedef struct _GdkDrawable           GdkWindow;
typedef struct _GtkFileFilter     GtkFileFilter;
typedef struct _GtkToggleButton       GtkToggleButton;
typedef enum {
  GTK_FILE_FILTER_FILENAME     = 1 << 0,
  GTK_FILE_FILTER_URI          = 1 << 1,
  GTK_FILE_FILTER_DISPLAY_NAME = 1 << 2,
  GTK_FILE_FILTER_MIME_TYPE    = 1 << 3
} GtkFileFilterFlags;
struct _GtkFileFilterInfo
{
  GtkFileFilterFlags contains;
  
  const gchar *filename;
  const gchar *uri;
  const gchar *display_name;
  const gchar *mime_type;
};
typedef struct _GtkFileFilterInfo GtkFileFilterInfo;
typedef gboolean (*GtkFileFilterFunc) (const GtkFileFilterInfo *filter_info, gpointer data);
typedef void (*GDestroyNotify)(gpointer data);
typedef enum
{
  GTK_FILE_CHOOSER_ACTION_OPEN,
  GTK_FILE_CHOOSER_ACTION_SAVE,
  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
  GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER
} GtkFileChooserAction;
#define GTK_STOCK_CANCEL           "gtk-cancel"
#define GTK_STOCK_SAVE             "gtk-save"
#define GTK_STOCK_OPEN             "gtk-open"
const int   GTK_RESPONSE_NONE = -1;
const int   GTK_RESPONSE_ACCEPT = -3;
const int   GTK_RESPONSE_DELETE_EVENT = -4;
const int   GTK_RESPONSE_CANCEL = -6;
typedef void  (*GCallback)(void);
#define	G_CALLBACK(f)			 ((GCallback) (f))
typedef int GConnectFlags;
typedef struct _GClosure		 GClosure;
typedef void  (*GClosureNotify)(gpointer data, GClosure	*closure);

/* --------------------- End of Type definitions from GLIB and GTK --------------------- */

int Fl_GTK_File_Chooser::did_find_GTK_libs = 0;

/* These are the GTK/GLib methods we want to load, but not call by name...! */

// void g_free (gpointer mem);
typedef void (*XX_g_free)(gpointer);
static XX_g_free fl_g_free = NULL;

// gpointer g_slist_nth_data (GSList *list, guint n);
typedef gpointer (*XX_g_slist_nth_data) (GSList *, guint);
static XX_g_slist_nth_data fl_g_slist_nth_data = NULL;

// guint g_slist_length (GSList *list);
typedef guint (*XX_g_slist_length) (GSList *);
static XX_g_slist_length fl_g_slist_length = NULL;

// void g_slist_free (GSList *list);
typedef void (*XX_g_slist_free) (GSList *);
static XX_g_slist_free fl_g_slist_free = NULL;

// gboolean gtk_init_check (int *argc, char ***argv);
typedef gboolean (*XX_gtk_init_check)(int *, char ***);
static XX_gtk_init_check fl_gtk_init_check = NULL;

// void gtk_widget_destroy (GtkWidget *widget);
typedef void (*XX_gtk_widget_destroy) (GtkWidget *);
static XX_gtk_widget_destroy fl_gtk_widget_destroy = NULL;

// void gtk_file_chooser_set_select_multiple(GtkFileChooser *chooser, gboolean select_multiple);
typedef void (*XX_gtk_file_chooser_set_select_multiple)(GtkFileChooser *, gboolean);
static XX_gtk_file_chooser_set_select_multiple fl_gtk_file_chooser_set_select_multiple = NULL;

// void gtk_file_chooser_set_do_overwrite_confirmation(GtkFileChooser *chooser, gboolean do_overwrite_confirmation);
typedef void (*XX_gtk_file_chooser_set_do_overwrite_confirmation)(GtkFileChooser *, gboolean);
static XX_gtk_file_chooser_set_do_overwrite_confirmation fl_gtk_file_chooser_set_do_overwrite_confirmation = NULL;

// void gtk_file_chooser_set_current_name (GtkFileChooser *chooser, const gchar *name);
typedef void (*XX_gtk_file_chooser_set_current_name)(GtkFileChooser *, const gchar *);
static XX_gtk_file_chooser_set_current_name fl_gtk_file_chooser_set_current_name = NULL;

// void gtk_file_chooser_set_current_folder (GtkFileChooser *chooser, const gchar *name);
typedef void (*XX_gtk_file_chooser_set_current_folder)(GtkFileChooser *, const gchar *);
static XX_gtk_file_chooser_set_current_folder fl_gtk_file_chooser_set_current_folder = NULL;

// void gtk_file_chooser_set_create_folders (GtkFileChooser *chooser, gboolean create_folders);
typedef void (*XX_gtk_file_chooser_set_create_folders) (GtkFileChooser *, gboolean);
static XX_gtk_file_chooser_set_create_folders fl_gtk_file_chooser_set_create_folders = NULL;

// gboolean gtk_file_chooser_get_select_multiple(GtkFileChooser *chooser);
typedef gboolean (*XX_gtk_file_chooser_get_select_multiple)(GtkFileChooser *);
static XX_gtk_file_chooser_get_select_multiple fl_gtk_file_chooser_get_select_multiple = NULL;

// void gtk_widget_hide(GtkWidget *widget);
typedef void (*XX_gtk_widget_hide)(GtkWidget *);
static XX_gtk_widget_hide fl_gtk_widget_hide = NULL;

// gchar * gtk_file_chooser_get_filename(GtkFileChooser *chooser);
typedef gchar* (*XX_gtk_file_chooser_get_filename)(GtkFileChooser *);
static XX_gtk_file_chooser_get_filename fl_gtk_file_chooser_get_filename = NULL;

// GSList * gtk_file_chooser_get_filenames(GtkFileChooser *chooser);
typedef GSList* (*XX_gtk_file_chooser_get_filenames)(GtkFileChooser *chooser);
static XX_gtk_file_chooser_get_filenames fl_gtk_file_chooser_get_filenames = NULL;

// gboolean gtk_main_iteration(void);
typedef gboolean (*XX_gtk_main_iteration)(void);
static XX_gtk_main_iteration fl_gtk_main_iteration = NULL;

// gboolean gtk_events_pending(void);
typedef gboolean (*XX_gtk_events_pending)(void);
static XX_gtk_events_pending fl_gtk_events_pending = NULL;

// GtkWidget * gtk_file_chooser_dialog_new(const gchar *title, GtkWindow *parent, GtkFileChooserAction action, const gchar *first_button_text, ...);
typedef GtkWidget* (*XX_gtk_file_chooser_dialog_new)(const gchar *, GtkWindow *, GtkFileChooserAction, const gchar *, ...);
static XX_gtk_file_chooser_dialog_new fl_gtk_file_chooser_dialog_new = NULL;

// void gtk_file_chooser_add_filter(GtkFileChooser*, GtkFileFilter*);
typedef void (*XX_gtk_file_chooser_add_filter)(GtkFileChooser*, GtkFileFilter*);
static XX_gtk_file_chooser_add_filter fl_gtk_file_chooser_add_filter = NULL;

// GtkFileFilter* gtk_file_chooser_get_filter(GtkFileChooser*);
typedef GtkFileFilter* (*XX_gtk_file_chooser_get_filter)(GtkFileChooser*);
static XX_gtk_file_chooser_get_filter fl_gtk_file_chooser_get_filter = NULL;

// void gtk_file_chooser_set_filter(GtkFileChooser*, GtkFileFilter*);
typedef void (*XX_gtk_file_chooser_set_filter)(GtkFileChooser*, GtkFileFilter*);
static XX_gtk_file_chooser_set_filter fl_gtk_file_chooser_set_filter = NULL;

// GtkFileFilter * gtk_file_filter_new();
typedef GtkFileFilter* (*XX_gtk_file_filter_new)(void);
static XX_gtk_file_filter_new fl_gtk_file_filter_new = NULL;

// void gtk_file_filter_add_pattern(GtkFileFilter*, const gchar*);
typedef void (*XX_gtk_file_filter_add_pattern)(GtkFileFilter*, const gchar*);
static XX_gtk_file_filter_add_pattern fl_gtk_file_filter_add_pattern = NULL;

// void gtk_file_filter_add_custom(GtkFileFilter *filter, GtkFileFilterFlags needed,
//  GtkFileFilterFunc func, gpointer data, GDestroyNotify notify);
typedef void (*XX_gtk_file_filter_add_custom)(GtkFileFilter *filter, GtkFileFilterFlags needed,
					      GtkFileFilterFunc func, gpointer data, 
					      GDestroyNotify notify);
static XX_gtk_file_filter_add_custom fl_gtk_file_filter_add_custom = NULL;

// void gtk_file_filter_set_name(GtkFileFilter*, const gchar*);
typedef void (*XX_gtk_file_filter_set_name)(GtkFileFilter*, const gchar*);
static XX_gtk_file_filter_set_name fl_gtk_file_filter_set_name = NULL;

// const gchar* gtk_file_filter_get_name(GtkFileFilter*);
typedef const gchar* (*XX_gtk_file_filter_get_name)(GtkFileFilter*);
static XX_gtk_file_filter_get_name fl_gtk_file_filter_get_name = NULL;

// void gtk_file_chooser_set_extra_widget(GtkFileChooser *, GtkWidget *);
typedef void (*XX_gtk_file_chooser_set_extra_widget)(GtkFileChooser *, GtkWidget *);
static XX_gtk_file_chooser_set_extra_widget fl_gtk_file_chooser_set_extra_widget = NULL;

// void gtk_widget_show_now(GtkWidget *);
typedef void (*XX_gtk_widget_show_now)(GtkWidget *);
static XX_gtk_widget_show_now fl_gtk_widget_show_now = NULL;

// GdkWindow* gtk_widget_get_window(GtkWidget *);
typedef GdkWindow* (*XX_gtk_widget_get_window)(GtkWidget *);
static XX_gtk_widget_get_window fl_gtk_widget_get_window = NULL;

// Window gdk_x11_drawable_get_xid(GdkWindow *);
typedef Window (*XX_gdk_x11_drawable_get_xid)(GdkWindow *);
static XX_gdk_x11_drawable_get_xid fl_gdk_x11_drawable_get_xid = NULL;

// GtkWidget *gtk_check_button_new_with_label(const gchar *);
typedef GtkWidget* (*XX_gtk_check_button_new_with_label)(const gchar *);
static XX_gtk_check_button_new_with_label fl_gtk_check_button_new_with_label = NULL;

// gulong g_signal_connect_data(gpointer, const gchar *, GCallback, gpointer, GClosureNotify, GConnectFlags);
typedef gulong (*XX_g_signal_connect_data)(gpointer, const gchar *, GCallback, gpointer, GClosureNotify, GConnectFlags);
static XX_g_signal_connect_data fl_g_signal_connect_data = NULL;

// gboolean gtk_toggle_button_get_active(GtkToggleButton *);
typedef gboolean (*XX_gtk_toggle_button_get_active)(GtkToggleButton*);
static XX_gtk_toggle_button_get_active fl_gtk_toggle_button_get_active = NULL;

// void gtk_file_chooser_set_show_hidden(GtkFileChooser *, gboolean);
typedef void (*XX_gtk_file_chooser_set_show_hidden)(GtkFileChooser *, gboolean);
static XX_gtk_file_chooser_set_show_hidden fl_gtk_file_chooser_set_show_hidden = NULL;

// gboolean gtk_file_chooser_get_show_hidden(GtkFileChooser *);
typedef gboolean (*XX_gtk_file_chooser_get_show_hidden)(GtkFileChooser *);
static XX_gtk_file_chooser_get_show_hidden fl_gtk_file_chooser_get_show_hidden = NULL;

// void gtk_toggle_button_set_active(GtkToggleButton *, gboolean);
typedef void (*XX_gtk_toggle_button_set_active)(GtkToggleButton *, gboolean);
static XX_gtk_toggle_button_set_active fl_gtk_toggle_button_set_active = NULL;


Fl_GTK_File_Chooser::Fl_GTK_File_Chooser(int val) : Fl_FLTK_File_Chooser(-1)
{
  gtkw_ptr   = NULL;    // used to hold a GtkWidget* 
  gtkw_slist = NULL;    // will hold the returned file names in a multi-selection...
  gtkw_count = 0;       // How many items were selected?
  gtkw_filename = NULL; // holds the last name we read back in a single file selection...
  gtkw_title = NULL;    // dialog title
  _btype = val;
  previous_filter = NULL;
}

Fl_GTK_File_Chooser::~Fl_GTK_File_Chooser()
{
  // Should free up resources taken for...
  if(gtkw_ptr) { 
    fl_gtk_widget_destroy (gtkw_ptr);
    gtkw_ptr = NULL;
  }
  if(gtkw_filename) {
    fl_g_free(gtkw_filename);
    gtkw_filename = NULL;
  }
  if(gtkw_slist) {
    GSList *iter = (GSList *)gtkw_slist;
    while(iter) {
      if(iter->data) fl_g_free(iter->data);
      iter = g_slist_next(iter);
    }
    fl_g_slist_free((GSList *)gtkw_slist);
    gtkw_slist = NULL;
  }
  gtkw_count = 0; // assume we have no files selected now
  gtkw_title = strfree(gtkw_title);
}

void Fl_GTK_File_Chooser::type(int val) {
  _btype = val;
}

int Fl_GTK_File_Chooser::count() const {
  return gtkw_count;
}

const char *Fl_GTK_File_Chooser::filename() const
{
  if(gtkw_ptr) {
    if(fl_gtk_file_chooser_get_select_multiple((GtkFileChooser *)gtkw_ptr) == FALSE) {
      return gtkw_filename;
    }
    else {
      GSList *iter = (GSList *)gtkw_slist;
      char *nm = (char *)iter->data;
      return nm;
    }
  }
  return("");
}

const char *Fl_GTK_File_Chooser::filename(int i) const
{
  if(fl_gtk_file_chooser_get_select_multiple((GtkFileChooser *)gtkw_ptr) == FALSE) {
    return gtkw_filename;
  }
  else {
    if ((unsigned)i < gtkw_count) {
      GSList *iter = (GSList *)gtkw_slist;
      char *nm = (char *)fl_g_slist_nth_data(iter, i);
      return nm;
    }
  }
  return("");
}

void Fl_GTK_File_Chooser::title(const char *val)
{
  strfree(gtkw_title);
  gtkw_title = strnew(val);
}

const char* Fl_GTK_File_Chooser::title() const
{
  return gtkw_title;
}

/* changes the extension of the outfile in the chooser according to newly selected filter */
void Fl_GTK_File_Chooser::changed_output_type(const char *filter)
{
  if ( !(options()&Fl_Native_File_Chooser::USE_FILTER_EXT) ) return;
  if (strchr(filter, '(') || strchr(filter, '{') || strchr(filter+1, '*') || strncmp(filter, "*.", 2)) return;
  const char *p = fl_gtk_file_chooser_get_filename((GtkFileChooser*)gtkw_ptr);
  if (!p) return;
  p = fl_filename_name(p);
  const char *q = strrchr(p, '.');
  if (!q) q = p + strlen(p);
  char *r = new char[strlen(p) + strlen(filter)];
  strcpy(r, p);
  strcpy(r + (q - p), filter + 1);
  fl_gtk_file_chooser_set_current_name((GtkFileChooser*)gtkw_ptr, r);
  delete[] r;
}

/* Filters files before display in chooser. 
 Also used to detect when the filter just changed */
gboolean Fl_GTK_File_Chooser::custom_gtk_filter_function(const GtkFileFilterInfo *info, Fl_GTK_File_Chooser::pair* p)
{
  if (p->running->previous_filter != p->filter) {
    p->running->changed_output_type(p->filter);
    p->running->previous_filter = p->filter;
    }
  return (gboolean)fl_filename_match(fl_filename_name(info->filename), p->filter);
}

void Fl_GTK_File_Chooser::free_pair(Fl_GTK_File_Chooser::pair *p)
{
  delete p;
}

static void hidden_files_cb(GtkToggleButton *togglebutton, gpointer user_data)
{
  gboolean state = fl_gtk_toggle_button_get_active(togglebutton);
  fl_gtk_file_chooser_set_show_hidden((GtkFileChooser*)user_data, state);
}

int Fl_GTK_File_Chooser::show()
{
  // The point here is that after running a GTK dialog, the calling program's current locale is modified.
  // To avoid that, we memorize the calling program's current locale, and the locale as modified
  // by GTK after the first dialog use. We restore the calling program's current locale 
  // before returning, and we set the locale as modified by GTK before subsequent GTK dialog uses.
  static bool first = true;
  char *p;
  char *before = NULL;
  static char *gtk_wants = NULL;
  fl_open_display();
  // record in before the calling program's current locale
  p = setlocale(LC_ALL, NULL);
  if (p) before = strdup(p);
  if (gtk_wants) { // set the locale as GTK 'wants it'
    setlocale(LC_ALL, gtk_wants);
  }
  int retval = fl_gtk_chooser_wrapper(); // may change the locale
  if (first) {
    first = false;
    // record in gtk_wants the locale as modified by the GTK dialog
    p = setlocale(LC_ALL, NULL);
    if (p) gtk_wants = strdup(p);
  }
  if (before) {
    setlocale(LC_ALL, before); // restore calling program's current locale
    free(before);
    }
  return retval;
}

static char *extract_dir_from_path(const char *path)
{
  static char *dir = NULL;
  if (fl_filename_isdir(path)) {
    return (char*)path;
  }
  if (*path != '/') return NULL;
  if (dir) free(dir);
  dir = strdup(path);
  do {
    char *p = strrchr(dir, '/');
    if (p == dir) p++;
    *p = 0;
    }
  while (!fl_filename_isdir(dir));
  return dir;
}

static void run_response_handler(GtkDialog *dialog, gint response_id, gpointer data)
{
  gint *ri = (gint *)data;
  *ri = response_id;
}


int Fl_GTK_File_Chooser::fl_gtk_chooser_wrapper()
{
  int result = 1;
  static int have_gtk_init = 0;
  char *p;
  
  if(!have_gtk_init) {
    have_gtk_init = -1;
    int ac = 0;
    fl_gtk_init_check(&ac, NULL);
  }
  
  if(gtkw_ptr) { // discard the previous dialog widget
    fl_gtk_widget_destroy (gtkw_ptr);
    gtkw_ptr = NULL;
  }
  
  // set the dialog action type
  GtkFileChooserAction gtw_action_type;
  switch (_btype) {
    case Fl_Native_File_Chooser::BROWSE_DIRECTORY:
    case Fl_Native_File_Chooser::BROWSE_MULTI_DIRECTORY:
      gtw_action_type = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
      break;
      
    case Fl_Native_File_Chooser::BROWSE_SAVE_FILE:
      gtw_action_type = GTK_FILE_CHOOSER_ACTION_SAVE;
      break;
      
    case Fl_Native_File_Chooser::BROWSE_SAVE_DIRECTORY:
      gtw_action_type = GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER;
      break;
      
    case Fl_Native_File_Chooser::BROWSE_MULTI_FILE:
    case Fl_Native_File_Chooser::BROWSE_FILE:
    default:
      gtw_action_type = GTK_FILE_CHOOSER_ACTION_OPEN;
      break;
  }
  // create a new dialog
  gtkw_ptr = fl_gtk_file_chooser_dialog_new (gtkw_title,
					     NULL, /* parent_window */
					     gtw_action_type,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     gtw_action_type == GTK_FILE_CHOOSER_ACTION_SAVE || gtw_action_type == GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER ? 
					     GTK_STOCK_SAVE : GTK_STOCK_OPEN, 
					     GTK_RESPONSE_ACCEPT,
					     NULL);
  // did we create a valid dialog widget?
  if(!gtkw_ptr) {
    // fail
    return -1;
  }
  
  // set the dialog properties
  switch (_btype) {
    case Fl_Native_File_Chooser::BROWSE_MULTI_DIRECTORY:
    case Fl_Native_File_Chooser::BROWSE_MULTI_FILE:
      fl_gtk_file_chooser_set_select_multiple((GtkFileChooser *)gtkw_ptr, TRUE);
      break;
      
    case Fl_Native_File_Chooser::BROWSE_SAVE_FILE:
      if (_preset_file)fl_gtk_file_chooser_set_current_name ((GtkFileChooser *)gtkw_ptr, fl_filename_name(_preset_file));
      /* FALLTHROUGH */
    case Fl_Native_File_Chooser::BROWSE_SAVE_DIRECTORY:
      fl_gtk_file_chooser_set_create_folders((GtkFileChooser *)gtkw_ptr, TRUE);
      fl_gtk_file_chooser_set_do_overwrite_confirmation ((GtkFileChooser *)gtkw_ptr, (_options & Fl_Native_File_Chooser::SAVEAS_CONFIRM)?TRUE:FALSE);
      break;
      
    case Fl_Native_File_Chooser::BROWSE_DIRECTORY:
    case Fl_Native_File_Chooser::BROWSE_FILE:
    default:
      break;
  }
  
  if (_directory && _directory[0]) {
    p = extract_dir_from_path(_directory);
    if (p) fl_gtk_file_chooser_set_current_folder((GtkFileChooser *)gtkw_ptr, p);
  }
  else if (_preset_file) {
    p = extract_dir_from_path(_preset_file);
    if (p) fl_gtk_file_chooser_set_current_folder((GtkFileChooser *)gtkw_ptr, p);
  }
  
  GtkFileFilter **filter_tab = NULL;
  if (_parsedfilt) {
    filter_tab = new GtkFileFilter*[_nfilters];
    char *filter = strdup(_parsedfilt);
    p = strtok(filter, "\t");
    int count = 0;
    while (p) {
      filter_tab[count] = fl_gtk_file_filter_new();
      fl_gtk_file_filter_set_name(filter_tab[count], p);
      p = strchr(p, '(') + 1;
      char *q = strchr(p, ')'); *q = 0;
      fl_gtk_file_filter_add_custom(filter_tab[count], 
				    GTK_FILE_FILTER_FILENAME, 
				    (GtkFileFilterFunc)Fl_GTK_File_Chooser::custom_gtk_filter_function, 
				    new Fl_GTK_File_Chooser::pair(this, p), 
				    (GDestroyNotify)Fl_GTK_File_Chooser::free_pair);
      fl_gtk_file_chooser_add_filter((GtkFileChooser *)gtkw_ptr, filter_tab[count]);
      p = strtok(NULL, "\t");
      count++;
    }
    free(filter);
    fl_gtk_file_chooser_set_filter((GtkFileChooser *)gtkw_ptr, filter_tab[_filtvalue < _nfilters?_filtvalue:0]);
    previous_filter = NULL;
    if (gtw_action_type == GTK_FILE_CHOOSER_ACTION_OPEN) {
      GtkFileFilter* gfilter = fl_gtk_file_filter_new();
      fl_gtk_file_filter_set_name(gfilter, Fl_File_Chooser::all_files_label);
      fl_gtk_file_filter_add_pattern(gfilter, "*");
      fl_gtk_file_chooser_add_filter((GtkFileChooser *)gtkw_ptr, gfilter);
    }
  }
  
  GtkWidget *toggle = fl_gtk_check_button_new_with_label(Fl_File_Chooser::hidden_label);
  fl_gtk_file_chooser_set_extra_widget((GtkFileChooser *)gtkw_ptr, toggle);
  fl_g_signal_connect_data(toggle, "toggled", G_CALLBACK(hidden_files_cb), gtkw_ptr, NULL, (GConnectFlags) 0);
  Fl_Window* firstw = Fl::first_window();
  fl_gtk_widget_show_now(gtkw_ptr); // map the GTK window on screen
  if (firstw) {
    GdkWindow* gdkw = fl_gtk_widget_get_window(gtkw_ptr);
    Window xw = fl_gdk_x11_drawable_get_xid(gdkw); // get the X11 ref of the GTK window
    XSetTransientForHint(fl_display, xw, fl_xid(firstw)); // set the GTK window transient for the last FLTK win
    }
  gboolean state = fl_gtk_file_chooser_get_show_hidden((GtkFileChooser *)gtkw_ptr);
  fl_gtk_toggle_button_set_active((GtkToggleButton *)toggle, state);
  
  gint response_id = GTK_RESPONSE_NONE;
  fl_g_signal_connect_data(gtkw_ptr, "response", G_CALLBACK(run_response_handler), &response_id, NULL, (GConnectFlags) 0);
  while (response_id == GTK_RESPONSE_NONE) { // loop that shows the GTK dialog window
    fl_gtk_main_iteration(); // one iteration of the GTK event loop
    while (XEventsQueued(fl_display, QueuedAfterReading)) { // emulate modal dialog
      XEvent xevent;
      XNextEvent(fl_display, &xevent);
      Window xid = xevent.xany.window;
      if (xevent.type == ConfigureNotify) xid = xevent.xmaprequest.window;
      if (!fl_find(xid)) continue; // skip events to non-FLTK windows
      // process Expose and ConfigureNotify events
      if ( xevent.type == Expose || xevent.type == ConfigureNotify ) fl_handle(xevent); 
    }
    Fl::flush(); // do the drawings needed after Expose events
  } 
  
  if (response_id == GTK_RESPONSE_ACCEPT) {
    if (_parsedfilt) {
      GtkFileFilter *gfilter = fl_gtk_file_chooser_get_filter((GtkFileChooser *)gtkw_ptr);
      for (_filtvalue = 0; _filtvalue < _nfilters; _filtvalue++) {
	if (filter_tab[_filtvalue] == gfilter) break;
      }
    }
    
    // discard any filenames or lists from previous calls
    if(gtkw_filename) {
      fl_g_free(gtkw_filename);
      gtkw_filename = NULL;
    }
    if(gtkw_slist) {
      GSList *iter = (GSList *)gtkw_slist;
      while(iter) {
        if(iter->data) fl_g_free(iter->data);
        iter = g_slist_next(iter);
      }
      fl_g_slist_free((GSList *)gtkw_slist);
      gtkw_slist = NULL;
    }
    gtkw_count = 0; // assume we have no files selected now
    
    if(fl_gtk_file_chooser_get_select_multiple((GtkFileChooser *)gtkw_ptr) == FALSE) {
      gtkw_filename = fl_gtk_file_chooser_get_filename ((GtkFileChooser *)gtkw_ptr);
      if (gtkw_filename) {
        gtkw_count = 1;
        result = 0;
        //printf("single: %s\n", gtkw_filename);
      }
    }
    else {
      gtkw_slist = fl_gtk_file_chooser_get_filenames((GtkFileChooser *)gtkw_ptr);
      gtkw_count = fl_g_slist_length((GSList *)gtkw_slist);
      if(gtkw_count) result = 0;
      
      //      puts("multiple");
      //      GSList *iter = (GSList *)gtkw_slist;
      //      printf ("Selected %d files\n", gtkw_count);
      //      while(iter) {
      //        char *nm = (char *)iter->data;
      //        printf("%s\n", nm);
      //        iter = g_slist_next(iter);
      //      }
    }
  }
  delete[] filter_tab;
  if ( response_id == GTK_RESPONSE_DELETE_EVENT) gtkw_ptr = NULL;
  else fl_gtk_widget_hide (gtkw_ptr);
  
  // I think this is analogus to doing an Fl::check() - we need this here to make sure
  // the GtkFileChooserDialog is removed from the display correctly
  while (fl_gtk_events_pending ()) fl_gtk_main_iteration (); 
  
  return result;
} // fl_gtk_chooser_wrapper

#if HAVE_DLSYM && HAVE_DLFCN_H
// macro to help with the symbol loading boilerplate...
#  define GET_SYM(SSS, LLL) \
dlerror();    /* Clear any existing error */  \
fl_##SSS = (XX_##SSS)dlsym(LLL, #SSS);        \
if ((pc_dl_error = dlerror()) != NULL)  {     \
fprintf(stderr, "%s\n", pc_dl_error);       \
did_find_GTK_libs = 0;                      \
return; }

static void* fl_dlopen(const char *filename1, const char *filename2)
{
  void *ptr = dlopen(filename1, RTLD_LAZY | RTLD_GLOBAL);
  if (!ptr) ptr = dlopen(filename2, RTLD_LAZY | RTLD_GLOBAL);
  return ptr;
}
#endif

/* 
 * Use dlopen to see if we can load the gtk dynamic libraries that
 * will allow us to create a GtkFileChooserDialog() on the fly,
 * without linking to the GTK libs at compile time.
 */
void Fl_GTK_File_Chooser::probe_for_GTK_libs(void) {
#if HAVE_DLSYM && HAVE_DLFCN_H
  void *ptr_glib    = NULL;
  void *ptr_gtk     = NULL;
  
#   ifdef __APPLE_CC__ // allows testing on Darwin + X11
  ptr_glib    = dlopen("/sw/lib/libglib-2.0.dylib", RTLD_LAZY | RTLD_GLOBAL);
#   else
  ptr_glib    = fl_dlopen("libglib-2.0.so", "libglib-2.0.so.0");
#   endif
  // Try first with GTK2
#   ifdef __APPLE_CC__ // allows testing on Darwin + X11
  ptr_gtk     = dlopen("/sw/lib/libgtk-x11-2.0.dylib", RTLD_LAZY | RTLD_GLOBAL);
#else
  ptr_gtk     = fl_dlopen("libgtk-x11-2.0.so", "libgtk-x11-2.0.so.0");
#endif
  if (ptr_gtk && ptr_glib) {
#ifdef DEBUG
    puts("selected GTK-2\n");
#endif
  }
  else {// Try then with GTK3
    ptr_gtk     = fl_dlopen("libgtk-3.so", "libgtk-3.so.0");
#ifdef DEBUG
    if (ptr_gtk && ptr_glib) {
      puts("selected GTK-3\n");
    }
#endif
  }
  
  if((!ptr_glib) || (!ptr_gtk)) {
#ifdef DEBUG
    puts("Failure to load libglib or libgtk");
#endif
    did_find_GTK_libs = 0;
    return;
  }
  
  char *pc_dl_error; // used to report errors by the GET_SYM macro...
  // items we need from GLib
  GET_SYM(g_free, ptr_glib);
  GET_SYM(g_slist_nth_data, ptr_glib);
  GET_SYM(g_slist_length, ptr_glib);
  GET_SYM(g_slist_free, ptr_glib);
  // items we need from GTK
  GET_SYM(gtk_init_check, ptr_gtk);
  GET_SYM(gtk_widget_destroy, ptr_gtk);
  GET_SYM(gtk_file_chooser_set_select_multiple, ptr_gtk);
  GET_SYM(gtk_file_chooser_set_do_overwrite_confirmation, ptr_gtk);
  GET_SYM(gtk_file_chooser_set_current_name, ptr_gtk);
  GET_SYM(gtk_file_chooser_set_current_folder, ptr_gtk);
  GET_SYM(gtk_file_chooser_set_create_folders, ptr_gtk);
  GET_SYM(gtk_file_chooser_get_select_multiple, ptr_gtk);
  GET_SYM(gtk_widget_hide, ptr_gtk);
  GET_SYM(gtk_file_chooser_get_filename, ptr_gtk);
  GET_SYM(gtk_file_chooser_get_filenames, ptr_gtk);
  GET_SYM(gtk_main_iteration, ptr_gtk);
  GET_SYM(gtk_events_pending, ptr_gtk);
  GET_SYM(gtk_file_chooser_dialog_new, ptr_gtk);
  GET_SYM(gtk_file_chooser_add_filter, ptr_gtk);
  GET_SYM(gtk_file_chooser_get_filter, ptr_gtk);
  GET_SYM(gtk_file_chooser_set_filter, ptr_gtk);
  GET_SYM(gtk_file_filter_new, ptr_gtk);
  GET_SYM(gtk_file_filter_add_pattern, ptr_gtk);
  GET_SYM(gtk_file_filter_add_custom, ptr_gtk);
  GET_SYM(gtk_file_filter_set_name, ptr_gtk);
  GET_SYM(gtk_file_filter_get_name, ptr_gtk);
  GET_SYM(gtk_file_chooser_set_extra_widget, ptr_gtk);
  GET_SYM(gtk_widget_show_now, ptr_gtk);
  GET_SYM(gtk_widget_get_window, ptr_gtk);
  GET_SYM(gdk_x11_drawable_get_xid, ptr_gtk);
  GET_SYM(gtk_check_button_new_with_label, ptr_gtk);
  GET_SYM(g_signal_connect_data, ptr_gtk);
  GET_SYM(gtk_toggle_button_get_active, ptr_gtk);
  GET_SYM(gtk_file_chooser_set_show_hidden, ptr_gtk);
  GET_SYM(gtk_file_chooser_get_show_hidden, ptr_gtk);
  GET_SYM(gtk_toggle_button_set_active, ptr_gtk);
  
  did_find_GTK_libs = 1;
#endif
} // probe_for_GTK_libs

//
// End of "$Id$".
//
