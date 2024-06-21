//
// FLTK native file chooser widget wrapper for GTK's GtkFileChooserDialog
//
// Copyright 1998-2023 by Bill Spitzak and others.
// Copyright 2012 IMM
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include <config.h>
#include <FL/Fl_Native_File_Chooser.H>
#include "Fl_Native_File_Chooser_Zenity.H"
#include "Fl_Native_File_Chooser_Kdialog.H"

#if HAVE_DLSYM && HAVE_DLFCN_H
#include <FL/platform.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/fl_draw.H>
#include <FL/fl_string_functions.h>
#include <dlfcn.h>   // for dlopen et al
#include "drivers/Posix/Fl_Posix_System_Driver.H"
#include "drivers/Unix/Fl_Unix_Screen_Driver.H"
#include "Fl_Window_Driver.H"
#include "Fl_Screen_Driver.H"

/* --------------------- Type definitions from GLIB and GTK --------------------- */
/* all of this is from the public gnome API, so unlikely to change */
#ifndef FALSE
#define FALSE   (0)
#endif
#ifndef TRUE
#define TRUE    (!FALSE)
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
#define  g_slist_next(slist)             ((slist) ? (((GSList *)(slist))->next) : NULL)
typedef struct _GtkWidget      GtkWidget;
typedef struct _GtkFileChooser      GtkFileChooser;
typedef struct _GtkDialog        GtkDialog;
typedef struct _GtkWindow          GtkWindow;
typedef struct _GtkFileFilter     GtkFileFilter;
typedef struct _GtkToggleButton       GtkToggleButton;
typedef struct _GdkPixbuf GdkPixbuf;
typedef struct _GtkImage GtkImage;
typedef struct _GtkTable GtkTable;
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
#define G_CALLBACK(f)                    ((GCallback) (f))
typedef int GConnectFlags;
typedef struct _GClosure                 GClosure;
typedef void  (*GClosureNotify)(gpointer data, GClosure *closure);

/* --------------------- End of Type definitions from GLIB and GTK --------------------- */


class Fl_GTK_Native_File_Chooser_Driver : public Fl_Native_File_Chooser_FLTK_Driver {
  friend class Fl_Native_File_Chooser;
private:
  static int have_looked_for_GTK_libs;
  typedef struct _GtkFileFilterInfo GtkFileFilterInfo;
  struct pair {
    Fl_GTK_Native_File_Chooser_Driver* running; // the running Fl_GTK_File_Chooser
    const char *filter; // a filter string of the chooser
    pair(Fl_GTK_Native_File_Chooser_Driver* c, const char *f) {
      running = c;
      filter = fl_strdup(f);
    }
    ~pair() {
      free((char*)filter);
    }
  };
  GtkWidget *gtkw_ptr; // used to hold a GtkWidget* without pulling GTK into everything...
  void *gtkw_slist; // used to hold a GLib GSList...
  unsigned gtkw_count; // number of files read back - if any
  mutable char *gtkw_filename; // last name we read back
  char *gtkw_title; // the title to be applied to the dialog
  const char *previous_filter;

  int fl_gtk_chooser_wrapper(); // method that wraps the GTK widget
  Fl_GTK_Native_File_Chooser_Driver(int val);
  ~Fl_GTK_Native_File_Chooser_Driver() FL_OVERRIDE;
  static int did_find_GTK_libs;
  static void probe_for_GTK_libs(void);
  void type(int) FL_OVERRIDE;
  int count() const FL_OVERRIDE;
  const char *filename() const FL_OVERRIDE;
  const char *filename(int i) const FL_OVERRIDE;
  void title(const char *) FL_OVERRIDE;
  const char* title() const FL_OVERRIDE;
  int show() FL_OVERRIDE;
  void changed_output_type(const char *filter);

  static int custom_gtk_filter_function(const GtkFileFilterInfo*, Fl_GTK_Native_File_Chooser_Driver::pair*);
  static void free_pair(pair *p);
  Fl_Preferences gtk_chooser_prefs;
public:
  static gboolean want_preview; // state of "Preview" button
};

gboolean Fl_GTK_Native_File_Chooser_Driver::want_preview = false;

int Fl_GTK_Native_File_Chooser_Driver::did_find_GTK_libs = 0;

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

// GtkWidget *gtk_check_button_new_with_label(const gchar *);
typedef GtkWidget* (*XX_gtk_file_chooser_set_preview_widget_active)(GtkFileChooser*, gboolean);
static XX_gtk_file_chooser_set_preview_widget_active fl_gtk_file_chooser_set_preview_widget_active = NULL;

// GdkPixbuf* gtk_file_chooser_set_preview_widget(GtkFileChooser *, GtkWidget *);
typedef GdkPixbuf* (*XX_gtk_file_chooser_set_preview_widget) (GtkFileChooser *gtkw_ptr, GtkWidget *preview);
static XX_gtk_file_chooser_set_preview_widget fl_gtk_file_chooser_set_preview_widget = NULL;

// char *gtk_file_chooser_get_preview_filename(GtkFileChooser*); // 2.4
typedef char* (*XX_gtk_file_chooser_get_preview_filename) (GtkFileChooser*);
static XX_gtk_file_chooser_get_preview_filename fl_gtk_file_chooser_get_preview_filename = NULL;

// GdkPixbuf *gdk_pixbuf_new_from_data(const uchar *,GdkColorspace, gboolean, int, int, int, int, void*, void*);
typedef GdkPixbuf* (*XX_gdk_pixbuf_new_from_data)(const uchar *, int/*GdkColorspace*/, gboolean, int, int, int, int, void*, void*);
static XX_gdk_pixbuf_new_from_data fl_gdk_pixbuf_new_from_data = NULL;

// void gtk_image_set_from_pixbuf(GtkImage*, GdkPixbuf*);
typedef GdkPixbuf* (*XX_gtk_image_set_from_pixbuf) (GtkImage*, GdkPixbuf*);
static XX_gtk_image_set_from_pixbuf fl_gtk_image_set_from_pixbuf = NULL;

// GtkWidget *gtk_image_new();
typedef GtkWidget* (*XX_gtk_image_new)(void);
static XX_gtk_image_new fl_gtk_image_new = NULL;

// GtkWidget *gtk_table_new();
typedef GtkTable* (*XX_gtk_table_new)(int, int, gboolean);
static XX_gtk_table_new fl_gtk_table_new = NULL;

// GtkWidget *gtk_table_new();
typedef void (*XX_gtk_widget_show_all)(GtkWidget*);
static XX_gtk_widget_show_all fl_gtk_widget_show_all = NULL;

// void gtk_table_attach_defaults()
typedef void (*XX_gtk_table_attach_defaults)(GtkTable *, GtkWidget *, guint left_attach, guint right_attach,
                           guint top_attach, guint bottom_attach);
static XX_gtk_table_attach_defaults fl_gtk_table_attach_defaults = NULL;

// GtkImage *gtk_file_chooser_get_preview_widget(GtkFileChooser*);
typedef GtkImage*(*XX_gtk_file_chooser_get_preview_widget)(GtkFileChooser*);
static XX_gtk_file_chooser_get_preview_widget fl_gtk_file_chooser_get_preview_widget = NULL;

typedef void (*XX_gtk_widget_set_sensitive)(GtkWidget *, gboolean);
static XX_gtk_widget_set_sensitive fl_gtk_widget_set_sensitive = NULL;

typedef GtkWidget *(*XX_gtk_button_new_with_label)(const char*);
XX_gtk_button_new_with_label fl_gtk_button_new_with_label = NULL;

typedef GtkWidget *(*XX_gtk_widget_get_toplevel)(GtkWidget *);
static XX_gtk_widget_get_toplevel fl_gtk_widget_get_toplevel = NULL;

// void g_object_unref(gpointer);
typedef void (*XX_g_object_unref)(void*);
static XX_g_object_unref fl_g_object_unref = NULL;


int Fl_GTK_Native_File_Chooser_Driver::have_looked_for_GTK_libs = 0;


Fl_GTK_Native_File_Chooser_Driver::Fl_GTK_Native_File_Chooser_Driver(int val) : Fl_Native_File_Chooser_FLTK_Driver(-1),
gtk_chooser_prefs(Fl_Preferences::USER, "fltk.org", "fltk/GTK-file-chooser")
{
  gtkw_ptr   = NULL;    // used to hold a GtkWidget*
  gtkw_slist = NULL;    // will hold the returned file names in a multi-selection...
  gtkw_count = 0;       // How many items were selected?
  gtkw_filename = NULL; // holds the last name we read back in a single file selection...
  gtkw_title = NULL;    // dialog title
  _btype = val;
  previous_filter = NULL;
  if (options() & Fl_Native_File_Chooser::PREVIEW) want_preview = true;
  else gtk_chooser_prefs.get("Preview", want_preview, 0);
}

Fl_GTK_Native_File_Chooser_Driver::~Fl_GTK_Native_File_Chooser_Driver()
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
  if (!(options() & Fl_Native_File_Chooser::PREVIEW))
    gtk_chooser_prefs.set("Preview", want_preview);
}

void Fl_GTK_Native_File_Chooser_Driver::type(int val) {
  _btype = val;
}

int Fl_GTK_Native_File_Chooser_Driver::count() const {
  return gtkw_count;
}

const char *Fl_GTK_Native_File_Chooser_Driver::filename() const
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

const char *Fl_GTK_Native_File_Chooser_Driver::filename(int i) const
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

void Fl_GTK_Native_File_Chooser_Driver::title(const char *val)
{
  strfree(gtkw_title);
  gtkw_title = strnew(val);
}

const char* Fl_GTK_Native_File_Chooser_Driver::title() const
{
  return gtkw_title;
}

/* changes the extension of the outfile in the chooser according to newly selected filter */
void Fl_GTK_Native_File_Chooser_Driver::changed_output_type(const char *filter)
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
gboolean Fl_GTK_Native_File_Chooser_Driver::custom_gtk_filter_function(const GtkFileFilterInfo *info, Fl_GTK_Native_File_Chooser_Driver::pair* p)
{
  if (p->running->previous_filter != p->filter) {
    p->running->changed_output_type(p->filter);
    p->running->previous_filter = p->filter;
    }
  return (gboolean)fl_filename_match(fl_filename_name(info->filename), p->filter);
}

void Fl_GTK_Native_File_Chooser_Driver::free_pair(Fl_GTK_Native_File_Chooser_Driver::pair *p)
{
  delete p;
}

static void hidden_files_cb(GtkToggleButton *togglebutton, gpointer user_data)
{
  gboolean state = fl_gtk_toggle_button_get_active(togglebutton);
  fl_gtk_file_chooser_set_show_hidden((GtkFileChooser*)user_data, state);
}

int Fl_GTK_Native_File_Chooser_Driver::show()
{
  return fl_gtk_chooser_wrapper();
}

static char *extract_dir_from_path(const char *path)
{
  static char *dir = NULL;
  if (fl_filename_isdir(path)) {
    return (char*)path;
  }
  if (*path != '/') return NULL;
  if (dir) free(dir);
  dir = fl_strdup(path);
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

// checks whether the file begins with up to 1000 UTF-8-encoded unicode characters
// if yes, those characters are returned as a UTF-8 string (to be delete[]'d after use)
// if no, NULL is returned
static char *text_file_preview(const char *fname) {
  if (!strcmp(fl_filename_ext(fname), ".svg")) return NULL;
  if (!strcmp(fl_filename_ext(fname), ".xpm")) return NULL;
  FILE *in = fl_fopen(fname, "r");
  if (!in) return NULL;
  char *text = new char[4011];
  int len = fread(text, 1, 4010, in);
  fclose(in);
  text[len] = 0;
  if ((int)strlen(text) < len) text[0] = 0; // presence of null byte in file --> not text
  char *p = text;
  int count = 0;
  const char *end = text + strlen(text);
  while (p < end && count < 1000) {
    if (*p & 0x80) { // what should be a multibyte encoding
      fl_utf8decode(p, end, &len);
      if (len < 2) { // That's not genuine UTF-8
        delete[] text;
        return NULL;
      }
    } else {
      len = 1;
    }
    p += len;
    count++;
  }
  *p = 0;
  if (text[0]==0) {delete[] text; text = NULL;}
  return text;
}

static void delete_rgb_image(uchar *pixels, Fl_RGB_Image *rgb) {
  delete rgb;
}

// Draws to an Fl_RGB_Image a preview of text and image files,
// and uses it to fill the "preview" part of the GTK file chooser
//static int preview_width = 175;
static float preview_zoom = 1.;
static GtkWidget *plus_button, *minus_button;

static void update_preview_cb(GtkFileChooser *file_chooser, GtkImage* gtkimg)
{
  gboolean have_preview = false;
  Fl_Shared_Image *img = NULL;
  char *preview_text = NULL;
  char *filename = NULL;

  fl_gtk_widget_set_sensitive(plus_button, false);
  fl_gtk_widget_set_sensitive(minus_button, false);

  if (Fl_GTK_Native_File_Chooser_Driver::want_preview) filename = fl_gtk_file_chooser_get_preview_filename(file_chooser); // 2.4
  if (filename) {
    if (!fl_filename_isdir(filename)) {
      preview_text = text_file_preview(filename);
      if (!preview_text) {
        img = Fl_Shared_Image::get(filename);
      }
    }
    free(filename);
  }
  if (preview_text || (img && !img->fail())) {
    int width = preview_zoom * 175, height = preview_zoom * 225; // same size as Fl_File_Chooser's preview box
    if (preview_text) height = 225;
    if (img) {
      img->scale(width, height);
      width = img->w(), height = img->h();
    }
    Fl_Image_Surface *surf = new Fl_Image_Surface(width, height);
    Fl_Surface_Device::push_current(surf);
    fl_color(FL_WHITE);
    fl_rectf(0, 0, width, height);
    if (img) img->draw(0, 0);
    else {
      fl_color(FL_BLACK);
      fl_font(FL_COURIER, FL_NORMAL_SIZE - 1);
      fl_draw(preview_text, 0, 0, width, height, FL_ALIGN_TOP|FL_ALIGN_LEFT, NULL, false);
      delete[] preview_text;
    }
    Fl_RGB_Image *rgb = surf->image();
    Fl_Surface_Device::pop_current();
    delete surf;
    GdkPixbuf *pixbuf = fl_gdk_pixbuf_new_from_data(rgb->array,  0/*GDK_COLORSPACE_RGB*/,  rgb->d() == 4,
              8,  rgb->data_w(),  rgb->data_h(), rgb->ld() ? rgb->ld() : rgb->data_w() * rgb->d(),
              (void*)&delete_rgb_image, rgb);
    if (pixbuf) {
      fl_gtk_image_set_from_pixbuf(gtkimg, pixbuf);
      fl_g_object_unref(pixbuf);
      if (preview_zoom < 4) fl_gtk_widget_set_sensitive(plus_button, true);
      if (preview_zoom > 1) fl_gtk_widget_set_sensitive(minus_button, true);
      have_preview = true;
    }
  }
  if (img) img->release();
  fl_gtk_file_chooser_set_preview_widget_active(file_chooser, have_preview); //2.4
}

static void preview_cb(GtkToggleButton *togglebutton, GtkFileChooser *chooser)
{
  Fl_GTK_Native_File_Chooser_Driver::want_preview = fl_gtk_toggle_button_get_active(togglebutton);
  GtkImage *preview = fl_gtk_file_chooser_get_preview_widget(chooser);
  update_preview_cb(chooser, preview);
}

static void plus_cb(GtkWidget *togglebutton, GtkImage *preview) {
  preview_zoom *= 1.5;
  if (preview_zoom > 4) {
    preview_zoom = 4;
  }
  update_preview_cb((GtkFileChooser*)fl_gtk_widget_get_toplevel(togglebutton), preview);
}

static void minus_cb(GtkWidget *togglebutton, GtkImage *preview) {
  preview_zoom /= 1.5;
  if (preview_zoom < 1) {
    preview_zoom = 1;
  }
  update_preview_cb((GtkFileChooser*)fl_gtk_widget_get_toplevel(togglebutton), preview);
}


static int fnfc_dispatch(int /*event*/, Fl_Window* /*win*/) {
  return 0;
}


int Fl_GTK_Native_File_Chooser_Driver::fl_gtk_chooser_wrapper()
{
  int result = 1;
  char *p;

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
    char *filter = fl_strdup(_parsedfilt);
    p = strtok(filter, "\t");
    int count = 0;
    while (p) {
      filter_tab[count] = fl_gtk_file_filter_new();
      fl_gtk_file_filter_set_name(filter_tab[count], p);
      p = strchr(p, '(') + 1;
      char *q = strchr(p, ')'); *q = 0;
      fl_gtk_file_filter_add_custom(filter_tab[count],
                                    GTK_FILE_FILTER_FILENAME,
                                    (GtkFileFilterFunc)Fl_GTK_Native_File_Chooser_Driver::custom_gtk_filter_function,
                                    new Fl_GTK_Native_File_Chooser_Driver::pair(this, p),
                                    (GDestroyNotify)Fl_GTK_Native_File_Chooser_Driver::free_pair);
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

  // extra buttons "Show hidden" [+ "Preview" before it if fl_register_images() was called]
  GtkWidget *show_hidden_button = fl_gtk_check_button_new_with_label(Fl_File_Chooser::hidden_label);
  fl_g_signal_connect_data(show_hidden_button, "toggled", G_CALLBACK(hidden_files_cb), gtkw_ptr, NULL, (GConnectFlags) 0);
  GtkWidget *extra = show_hidden_button;
  if (Fl_Image::register_images_done) {
    GtkTable *table = fl_gtk_table_new(1, 4, true);
    GtkWidget *preview = fl_gtk_image_new();
    fl_gtk_file_chooser_set_preview_widget((GtkFileChooser *)gtkw_ptr, preview); //2.4
    fl_g_signal_connect_data((GtkFileChooser *)gtkw_ptr, "update-preview", G_CALLBACK(update_preview_cb), preview,
                             NULL, (GConnectFlags)0);
    GtkWidget *preview_button = fl_gtk_check_button_new_with_label(Fl_File_Chooser::preview_label);
    fl_gtk_toggle_button_set_active((GtkToggleButton *)preview_button, want_preview);
    fl_g_signal_connect_data(preview_button, "toggled", G_CALLBACK(preview_cb), gtkw_ptr, NULL, (GConnectFlags) 0);
    fl_gtk_table_attach_defaults(table, preview_button, 0, 1, 0, 1);

    plus_button = fl_gtk_button_new_with_label("<--->");
    fl_g_signal_connect_data(plus_button, "clicked", G_CALLBACK(plus_cb), preview, NULL, (GConnectFlags) 0);
    fl_gtk_table_attach_defaults(table, plus_button, 1,2, 0, 1);
    minus_button = fl_gtk_button_new_with_label(">---<");
    fl_g_signal_connect_data(minus_button, "clicked", G_CALLBACK(minus_cb), preview, NULL, (GConnectFlags) 0);
    fl_gtk_table_attach_defaults(table, minus_button, 2,3, 0, 1);

    fl_gtk_table_attach_defaults(table, show_hidden_button, 3, 4, 0, 1);
    extra = (GtkWidget*)table;
  }
  fl_gtk_file_chooser_set_extra_widget((GtkFileChooser *)gtkw_ptr, extra);
  fl_gtk_widget_show_all(extra);
  fl_gtk_widget_show_now(gtkw_ptr); // map the GTK window on screen
  gboolean state = fl_gtk_file_chooser_get_show_hidden((GtkFileChooser *)gtkw_ptr);
  fl_gtk_toggle_button_set_active((GtkToggleButton *)show_hidden_button, state);

  Fl_Event_Dispatch old_dispatch = Fl::event_dispatch();
  // prevent FLTK from processing any event
  Fl::event_dispatch(fnfc_dispatch);
  void *control = ((Fl_Unix_Screen_Driver*)Fl::screen_driver())->control_maximize_button(NULL);
  gint response_id = GTK_RESPONSE_NONE;
  fl_g_signal_connect_data(gtkw_ptr, "response", G_CALLBACK(run_response_handler), &response_id, NULL, (GConnectFlags) 0);
  while (response_id == GTK_RESPONSE_NONE) { // loop that shows the GTK dialog window
    fl_gtk_main_iteration(); // one iteration of the GTK event loop
    while (Fl::ready()) Fl::check(); // queued iterations of the FLTK event loop
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

  // I think this is analogous to doing an Fl::check() - we need this here to make sure
  // the GtkFileChooserDialog is removed from the display correctly
  while (fl_gtk_events_pending ()) fl_gtk_main_iteration ();

  Fl::event_dispatch(old_dispatch);
  if (control) ((Fl_Unix_Screen_Driver*)Fl::screen_driver())->control_maximize_button(control);

  return result;
} // fl_gtk_chooser_wrapper

// macro to help with the symbol loading boilerplate...
#  define GET_SYM(SSS, LLL) \
dlerror();    /* Clear any existing error */  \
fl_##SSS = (XX_##SSS)dlsym(LLL, #SSS);        \
if ((pc_dl_error = dlerror()) != NULL)  {     \
fprintf(stderr, "%s\n", pc_dl_error);       \
did_find_GTK_libs = 0;                      \
return; }


/*
 * Use dlopen to see if we can load the gtk dynamic libraries that
 * will allow us to create a GtkFileChooserDialog() on the fly,
 * without linking to the GTK libs at compile time.
 */
void Fl_GTK_Native_File_Chooser_Driver::probe_for_GTK_libs(void) {
  void  *ptr_gtk;
  if ( !Fl_Posix_System_Driver::probe_for_GTK(2, 4, &ptr_gtk)) {
    did_find_GTK_libs = 0;
    return;
  }
  void *ptr_glib = ptr_gtk;
  char *pc_dl_error; // used to report errors by the GET_SYM macro...
  // items we need from GLib
  GET_SYM(g_free, ptr_glib);
  GET_SYM(g_slist_nth_data, ptr_glib);
  GET_SYM(g_slist_length, ptr_glib);
  GET_SYM(g_slist_free, ptr_glib);
  // items we need from GTK
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
  GET_SYM(gtk_file_chooser_set_preview_widget_active, ptr_gtk);
  GET_SYM(gtk_file_chooser_set_preview_widget, ptr_gtk);
  GET_SYM(gtk_file_chooser_get_preview_widget, ptr_gtk);
  GET_SYM(gtk_widget_set_sensitive, ptr_gtk);
  GET_SYM(gtk_button_new_with_label, ptr_gtk);
  GET_SYM(gtk_widget_get_toplevel, ptr_gtk);
  GET_SYM(gtk_file_chooser_get_preview_filename, ptr_gtk);
  GET_SYM(gdk_pixbuf_new_from_data, ptr_gtk);
  GET_SYM(gtk_image_set_from_pixbuf, ptr_gtk);
  GET_SYM(gtk_image_new, ptr_gtk);
  GET_SYM(gtk_table_new, ptr_gtk);
  GET_SYM(gtk_widget_show_all, ptr_gtk);
  GET_SYM(gtk_table_attach_defaults, ptr_gtk);
  GET_SYM(g_object_unref, ptr_gtk);
  GET_SYM(gtk_check_button_new_with_label, ptr_gtk);
  GET_SYM(g_signal_connect_data, ptr_gtk);
  GET_SYM(gtk_toggle_button_get_active, ptr_gtk);
  GET_SYM(gtk_file_chooser_set_show_hidden, ptr_gtk);
  GET_SYM(gtk_file_chooser_get_show_hidden, ptr_gtk);
  GET_SYM(gtk_toggle_button_set_active, ptr_gtk);

  did_find_GTK_libs = 1;
} // probe_for_GTK_libs

#endif // HAVE_DLSYM && HAVE_DLFCN_H

Fl_Native_File_Chooser::Fl_Native_File_Chooser(int val) {
  // Use zenity if available at run-time even if using the KDE desktop,
  // because its portal integration means the KDE chooser will be used.
  // Else use kdialog if available at run-time and if using the KDE
  // desktop, else, use GTK dialog if available at run-time
  // otherwise, use FLTK file chooser.
  platform_fnfc = NULL;
  fl_open_display();
  if (Fl::option(Fl::OPTION_FNFC_USES_ZENITY)&& val != BROWSE_MULTI_DIRECTORY) {
    if (!Fl_Zenity_Native_File_Chooser_Driver::have_looked_for_zenity) {
      // First Time here, try to find zenity
      FILE *pipe = popen("zenity --version 2> /dev/null", "r");
      if (pipe) {
        char *p, line[100] = "";
        p = fgets(line, sizeof(line), pipe);
        if (p && strlen(line) > 0) Fl_Zenity_Native_File_Chooser_Driver::did_find_zenity = true;
        pclose(pipe);
      }
      Fl_Zenity_Native_File_Chooser_Driver::have_looked_for_zenity = true;
    }
    // if we found zenity, we will use the Fl_Zenity_Native_File_Chooser_Driver
    if (Fl_Zenity_Native_File_Chooser_Driver::did_find_zenity) platform_fnfc = new Fl_Zenity_Native_File_Chooser_Driver(val);
  }
  if (!platform_fnfc && Fl::option(Fl::OPTION_FNFC_USES_KDIALOG) && val != BROWSE_MULTI_DIRECTORY) {
    if (!Fl_Kdialog_Native_File_Chooser_Driver::have_looked_for_kdialog) {
      // First Time here, try to find kdialog
      FILE *pipe = popen("kdialog -v 2> /dev/null", "r");
      if (pipe) {
        char *p, line[100] = "";
        p = fgets(line, sizeof(line), pipe);
        if (p && strlen(line) > 0) Fl_Kdialog_Native_File_Chooser_Driver::did_find_kdialog = true;
        pclose(pipe);
      }
      Fl_Kdialog_Native_File_Chooser_Driver::have_looked_for_kdialog = true;
    }
    // if we found kdialog, we will use the Fl_Kdialog_Native_File_Chooser_Driver
    if (Fl_Kdialog_Native_File_Chooser_Driver::did_find_kdialog) platform_fnfc = new Fl_Kdialog_Native_File_Chooser_Driver(val);
  }
#if HAVE_DLSYM && HAVE_DLFCN_H
  if (!platform_fnfc) {
    if (Fl::option(Fl::OPTION_FNFC_USES_GTK)) {
      if ( Fl_GTK_Native_File_Chooser_Driver::have_looked_for_GTK_libs == 0) {
        // First Time here, try to find the GTK libs if they are installed
        Fl_GTK_Native_File_Chooser_Driver::probe_for_GTK_libs();
        Fl_GTK_Native_File_Chooser_Driver::have_looked_for_GTK_libs = -1;
      }
      // if we found all the GTK functions we need, we will use the GtkFileChooserDialog
      if (Fl_GTK_Native_File_Chooser_Driver::did_find_GTK_libs) platform_fnfc = new Fl_GTK_Native_File_Chooser_Driver(val);
    }
  }
#endif // HAVE_DLSYM && HAVE_DLFCN_H

  if (!platform_fnfc) platform_fnfc = new Fl_Native_File_Chooser_FLTK_Driver(val);
}
