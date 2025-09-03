//
// More Fl_File_Chooser routines.
//
// Copyright 1999-2007 by Michael Sweet.
// Copyright 2008-2024 by Bill Spitzak and others.
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

// The following documentation is placed here because the implementation and
// header files FL/Fl_File_Chooser.H and src/Fl_File_Chooser.cxx are generated
// by fluid from src/Fl_File_Chooser.fl.

// *** BEGIN OUT OF SOURCE DOCUMENTATION ***

/** \defgroup group_comdlg Common Dialog Classes and Functions

  \brief Common dialog functions for file selection, message output, and more.

  @{
*/
/** \class Fl_File_Chooser
  The Fl_File_Chooser widget displays a standard file selection
  dialog that supports various selection modes.

  \image html Fl_File_Chooser.jpg
  \image latex  Fl_File_Chooser.jpg "Fl_File_Chooser" width=12cm

  Features include:

    - Multiple filter patterns can be specified, with parenthesis around filters,
      and tabs to separate each pattern, e.g.:
        \code
        char pattern[] = "Image Files (*.{bmp,gif,jpg,png,xbm,xpm})\t"
                         "Web Files (*.{htm,html,php})\t"
                         "All Files (*)";
        \endcode
    - If no "*" pattern is provided, then an entry for "All Files (*)" is automatically added.
    - An optional file preview box is provided which can be toggled by programmer or user
      showing images, or the first 2048 bytes of printable text.
    - Preview image loading functions can be registered to provide custom file previews.
    - The favorites button shows up to 100 user-saved favorite directories, the user's home
      directory, and a filesystems item.
    - A simple dialog is provided for managing saved directories.
    - Shortcut keys are provided:
         <CENTER><TABLE BORDER=1>
           <TR><TH>Shortcut</TH><TH>Description</TH></TR>
           <TR><TD>Alt+a</TD><TD>Adds a directory to the favorites list</TD></TR>
           <TR><TD>Alt+m</TD><TD>Manages the favorites list</TD></TR>
           <TR><TD>Alt+f</TD><TD>Shows the filesystem list</TD></TR>
           <TR><TD>Alt+h</TD><TD>Go to the home directory</TD></TR>
           <TR><TD>Alt+0..9</TD><TD>going to any of the first 10 favorites</TD></TR>
         </TABLE></CENTER>

  The Fl_File_Chooser widget transmits UTF-8 encoded filenames to its user. It is
  recommended to open files that may have non-ASCII names with the fl_fopen() or
  fl_open() utility functions that handle these names in a cross-platform way
  (whereas the standard fopen()/open() functions fail on the Windows platform
  to open files with a non-ASCII name).

  The Fl_File_Chooser class also exports several static values
  that may be used to localize or customize the appearance of all file chooser
  dialogs:

  <CENTER><TABLE BORDER="1">
  <TR>
        <TH>Member</TH>
        <TH>Default value</TH>
  </TR>
  <TR>
        <TD>add_favorites_label</TD>
        <TD>"Add to Favorites"</TD>
  </TR>
  <TR>
        <TD>all_files_label</TD>
        <TD>"All Files (*)"</TD>
  </TR>
  <TR>
        <TD>custom_filter_label</TD>
        <TD>"Custom Filter"</TD>
  </TR>
  <TR>
        <TD>existing_file_label</TD>
        <TD>"Please choose an existing file!"</TD>
  </TR>
  <TR>
        <TD>favorites_label</TD>
        <TD>"Favorites"</TD>
  </TR>
  <TR>
        <TD>filename_label</TD>
        <TD>"Filename:"</TD>
  </TR>
  <TR>
        <TD>filesystems_label</TD>
        <TD>"My Computer" (Windows)<BR>
        "File Systems" (all others)</TD>
  </TR>
  <TR>
        <TD>hidden_label</TD>
        <TD>"Show hidden files:"</TD>
  </TR>
  <TR>
        <TD>manage_favorites_label</TD>
        <TD>"Manage Favorites"</TD>
  </TR>
  <TR>
        <TD>new_directory_label</TD>
        <TD>"New Directory?"</TD>
  </TR>
  <TR>
        <TD>new_directory_tooltip</TD>
        <TD>"Create a new directory."</TD>
  </TR>
  <TR>
        <TD>preview_label</TD>
        <TD>"Preview"</TD>
  </TR>
  <TR>
        <TD>save_label</TD>
        <TD>"Save"</TD>
  </TR>
  <TR>
        <TD>show_label</TD>
        <TD>"Show:"</TD>
  </TR>
  <TR>
        <TD>sort</TD>
        <TD>fl_numericsort</TD>
  </TR>
  </TABLE></CENTER>

  The Fl_File_Chooser::sort member specifies the sort function that is
  used when loading the contents of a directory and can be customized
  at run-time.

  The Fl_File_Chooser class also exports the Fl_File_Chooser::newButton
  and Fl_File_Chooser::previewButton widgets so that application developers
  can control their appearance and use.
*/
/** @} */

/** \fn Fl_File_Chooser::Fl_File_Chooser(const char *pathname, const char *pattern, int type_val, const char *title)
  The constructor creates the Fl_File_Chooser dialog shown.

  - The \p pathname argument can be a directory name or a
  complete file name (in which case the corresponding file is highlighted
  in the list and in the filename input field.)

  - The \p pattern argument can be a NULL
  string or "*" to list all files, or it can be a
  series of descriptions and filter strings separated by tab
  characters (\\t). The format of filters is either
  "Description text (patterns)" or just "patterns". A file chooser
  that provides filters for HTML and image files might look like:
  \code
  "HTML Files (*.html)\tImage Files (*.{bmp,gif,jpg,png})"
  \endcode
  The file chooser will automatically add the "All Files (*)"
  pattern to the end of the string you pass if you do not provide
  one. The first filter in the string is the default filter.
  \p
  See the FLTK documentation on fl_filename_match()
  for the kinds of pattern strings that are supported.

  - The \p type_val argument can be one of the Fl_File_Chooser::Type values.

  - The \p title argument is used to set the title bar text for the
  Fl_File_Chooser window.
*/

/** \var Fl_File_Chooser::newButton
  The "new directory" button is exported so that application developers
  can control the appearance and use.
*/

/** \var Fl_File_Chooser::previewButton
  The "preview" button is exported so that application developers can
  control the appearance and use.
*/

/** \var Fl_File_Chooser::showHiddenButton
 When checked, hidden files (i.e., filename begins with dot) are displayed.

 The "showHiddenButton" button is exported so that application developers can
 control its appearance.
 */

/** \fn Fl_File_Chooser::~Fl_File_Chooser()
  Destroys the widget and frees all memory used by it.*/

/** \fn void Fl_File_Chooser::color(Fl_Color c)
  Sets the background color of the Fl_File_Browser list.*/

/** \fn Fl_Color Fl_File_Chooser::color()
  Gets the background color of the Fl_File_Browser list.*/

/** \fn void Fl_File_Chooser::directory(const char *pathname)
  Sets the current directory.*/

/** \fn const char *Fl_File_Chooser::directory()
 Gets the current directory.*/

/** \fn const char *Fl_File_Chooser::filter()
 Gets the current filename filter patterns.*/

/** \fn void Fl_File_Chooser::filter_value(int f)
  Sets the current filename filter selection.*/

/** \fn int Fl_File_Chooser::filter_value()
  Gets the current filename filter selection.*/

/** \fn void Fl_File_Chooser::hide()
  Hides the Fl_File_Chooser window.*/

/** \fn void Fl_File_Chooser::iconsize(uchar s)
  Sets the size of the icons in the Fl_File_Browser.  By
  default the icon size is set to 1.5 times the textsize().
*/

/** \fn uchar Fl_File_Chooser::iconsize()
  Gets the size of the icons in the Fl_File_Browser.  By
  default the icon size is set to 1.5 times the textsize().
*/

/** \fn void Fl_File_Chooser::label(const char *l)
  Sets the title bar text for the Fl_File_Chooser.*/

/** \fn const char *Fl_File_Chooser::label()
  Gets the title bar text for the Fl_File_Chooser.*/

/** \fn void Fl_File_Chooser::ok_label(const char *l)
  Sets the label for the "ok" button in the Fl_File_Chooser.
*/

/** \fn const char *Fl_File_Chooser::ok_label()
  Gets the label for the "ok" button in the Fl_File_Chooser.
*/

/** \fn int Fl_File_Chooser::preview() const
   Returns the current state of the preview box. */

/** \fn void Fl_File_Chooser::textcolor(Fl_Color c)
  Sets the current Fl_File_Browser text color.*/

/** \fn Fl_Color Fl_File_Chooser::textcolor()
  Gets the current Fl_File_Browser text color.*/

/** \fn void Fl_File_Chooser::textfont(Fl_Font f)
  Sets the current Fl_File_Browser text font.*/

/** \fn Fl_Font Fl_File_Chooser::textfont()
  Gets the current Fl_File_Browser text font.*/

/** \fn void Fl_File_Chooser::textsize(Fl_Fontsize s)
  Sets the current Fl_File_Browser text size.*/

/** \fn Fl_Fontsize Fl_File_Chooser::textsize()
  Gets the current Fl_File_Browser text size.*/

/** \fn void Fl_File_Chooser::type(int t)
  Sets the current type of Fl_File_Chooser.*/

/** \fn int Fl_File_Chooser::type()
  Gets the current type of Fl_File_Chooser.*/

/** \fn int Fl_File_Chooser::visible()
  Returns 1 if the Fl_File_Chooser window is visible.*/

/** \fn Fl_Widget *Fl_File_Chooser::add_extra(Fl_Widget *extra)
  Adds an extra widget at the bottom of the Fl_File_Chooser window.

  You can use any Fl_Widget or Fl_Group. If you use an Fl_Group, set its (x, y)
  coordinates to (0, 0) and position its children relative to (0, 0) inside
  the Fl_Group container widget. Make sure that all child widgets of the
  Fl_Group are entirely included inside the bounding box of their parents,
  i.e. the Fl_Group widget, and the Fl_File_Chooser window, respectively.

  \note The width of the Fl_File_Chooser window is an undocumented
    implementation detail and may change in the future.

  If \p extra is NULL any previous extra widget is removed.

  \param[in] extra Custom widget or group to be added to the Fl_File_Chooser window.

  \returns Pointer to previous extra widget or NULL if not set previously.

  \note Fl_File_Chooser does \b not delete the extra widget in its destructor!
    The extra widget is removed from the Fl_File_Chooser window before the
    Fl_File_Chooser widget gets destroyed. To prevent memory leakage, don't
    forget to delete unused extra widgets.
*/
  /** \fn int Fl_File_Chooser::shown()
    Returns non-zero if the file chooser main window show() has been called,
    but not hide().
    \see Fl_Window::shown()
  */

  /** \fn void Fl_File_Chooser::callback(void (*cb)(Fl_File_Chooser *, void *), void *d = 0)
    Sets the file chooser callback cb and associated data \p d */

  /** \fn void Fl_File_Chooser::user_data(void *d)
    Sets the file chooser user data \p d */

  /** \fn void * Fl_File_Chooser::user_data() const
    Gets the file chooser user data. */

// *** END OF OUT OF SOURCE DOCUMENTATION ***

// Contents:
//
//   Fl_File_Chooser::count()             - Return the number of selected files.
//   Fl_File_Chooser::directory()         - Set the directory in the file chooser.
//   Fl_File_Chooser::filter()            - Set the filter(s) for the chooser.
//   Fl_File_Chooser::newdir()            - Make a new directory.
//   Fl_File_Chooser::value()             - Return a selected filename.
//   Fl_File_Chooser::rescan()            - Rescan the current directory.
//   Fl_File_Chooser::favoritesButtonCB() - Handle favorites selections.
//   Fl_File_Chooser::fileListCB()        - Handle clicks (and double-clicks)
//                                          in the Fl_File_Browser.
//   Fl_File_Chooser::fileNameCB()        - Handle text entry in the FileBrowser.
//   Fl_File_Chooser::showChoiceCB()      - Handle show selections.
//   compare_dirnames()                   - Compare two directory names.
//   quote_pathname()                     - Quote a pathname for a menu.
//   unquote_pathname()                   - Unquote a pathname from a menu.
//
//   Fl_File_Chooser::add_extra()         - add custom extra widget or group
//

//
// Include necessary headers.
//

#include <FL/Fl_File_Chooser.H>
#include "Fl_System_Driver.H"
#include <FL/Fl.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <FL/platform.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/fl_draw.H>
#include <FL/fl_string_functions.h>

#include <stdio.h>
#include <stdlib.h>
#include "flstring.h"
#include <errno.h>
#include <sys/stat.h>

//
// File chooser label strings and sort function...
//

Fl_Preferences* Fl_File_Chooser::prefs_ = NULL;

const char      *Fl_File_Chooser::add_favorites_label = "Add to Favorites";
const char      *Fl_File_Chooser::all_files_label = "All Files (*)";
const char      *Fl_File_Chooser::custom_filter_label = "Custom Filter";
const char      *Fl_File_Chooser::existing_file_label = "Please choose an existing file!";
const char      *Fl_File_Chooser::favorites_label = "Favorites";
const char      *Fl_File_Chooser::filename_label = "Filename:";
const char      *Fl_File_Chooser::filesystems_label = Fl::system_driver()->filesystems_label();
const char      *Fl_File_Chooser::manage_favorites_label = "Manage Favorites";
const char      *Fl_File_Chooser::new_directory_label = "New Directory?";
const char      *Fl_File_Chooser::new_directory_tooltip = "Create a new directory.";
const char      *Fl_File_Chooser::preview_label = "Preview";
const char      *Fl_File_Chooser::save_label = "Save";
const char      *Fl_File_Chooser::show_label = "Show:";
const char      *Fl_File_Chooser::hidden_label = "Show hidden files";
Fl_File_Sort_F  *Fl_File_Chooser::sort = fl_numericsort;


//
// Local functions...
//

static int      compare_dirnames(const char *a, const char *b);
static void     quote_pathname(char *, const char *, int);
static void     unquote_pathname(char *, const char *, int);

/**  Returns the number of selected files.*/
int                             // O - Number of selected files
Fl_File_Chooser::count() {
  int           i;              // Looping var
  int           fcount;         // Number of selected files
  const char    *filename;      // Filename in input field or list


  filename = fileName->value();

  if (!(type_ & MULTI)) {
    // Check to see if the file name input field is blank...
    if (!filename || !filename[0]) return 0;
    else return 1;
  }

  for (i = 1, fcount = 0; i <= fileList->size(); i ++)
    if (fileList->selected(i)) {
      // See if this file is a directory...
      // matt: why would we do that? It is perfectly legal to select multiple
      // directories in a DIR chooser. They are visually selected and value(i)
      // returns all of them as expected
      //filename = (char *)fileList->text(i);

      //if (filename[strlen(filename) - 1] != '/')
        fcount ++;
    }

  if (fcount) return fcount;
  else if (!filename || !filename[0]) return 0;
  else return 1;
}


/** Sets the current directory.*/
void
Fl_File_Chooser::directory(const char *d)// I - Directory to change to
{
  char  *dirptr;                // Pointer into directory
  char  fixpath[FL_PATH_MAX];   // Path with slashes converted


//  printf("Fl_File_Chooser::directory(\"%s\")\n", d == NULL ? "(null)" : d);

  // NULL == current directory
  if (d == NULL)
    d = ".";

  if (Fl::system_driver()->backslash_as_slash()) {
    // See if the filename contains backslashes...
    char        *slash;                         // Pointer to slashes
    if (strchr(d, '\\')) {
      // Convert backslashes to slashes...
      strlcpy(fixpath, d, sizeof(fixpath));

      for (slash = strchr(fixpath, '\\'); slash; slash = strchr(slash + 1, '\\'))
        *slash = '/';

      d = fixpath;
    }
  }

  if (d[0] != '\0')
  {
    // Make the directory absolute...
    if (d[0] != '/' && d[0] != '\\' && ( !Fl::system_driver()->colon_is_drive() || d[1] != ':' ) )
      fl_filename_absolute(directory_, d);
    else
      strlcpy(directory_, d, sizeof(directory_));

    // Strip any trailing slash...
    dirptr = directory_ + strlen(directory_) - 1;
    if ((*dirptr == '/' || *dirptr == '\\') && dirptr > directory_)
      *dirptr = '\0';

    // See if we have a trailing .. or . in the filename...
    dirptr = directory_ + strlen(directory_) - 3;
    if (dirptr >= directory_ && strcmp(dirptr, "/..") == 0) {
      // Yes, we have "..", so strip the trailing path...
      *dirptr = '\0';
      while (dirptr > directory_) {
        if (*dirptr == '/') break;
        dirptr --;
      }

      if (dirptr >= directory_ && *dirptr == '/')
        *dirptr = '\0';
    } else if ((dirptr + 1) >= directory_ && strcmp(dirptr + 1, "/.") == 0) {
      // Strip trailing "."...
      dirptr[1] = '\0';
    }
  }
  else
    directory_[0] = '\0';

  if (shown()) {
    // Rescan the directory...
    rescan();
  }
}


//
// 'Fl_File_Chooser::favoritesButtonCB()' - Handle favorites selections.
//

void
Fl_File_Chooser::favoritesButtonCB()
{
  int           v;                      // Current selection
  char          pathname[FL_PATH_MAX],  // Pathname
                menuname[FL_PATH_MAX];  // Menu name


  v = favoritesButton->value();

  if (!v) {
    // Add current directory to favorites...
    if (Fl::system_driver()->home_directory_name()) v = favoritesButton->size() - 5;
    else v = favoritesButton->size() - 4;

    snprintf(menuname, FL_PATH_MAX, "favorite%02d", v);

    prefs_->set(menuname, directory_);
    prefs_->flush();

    update_favorites();              // adds item to favorites with Alt-n shortcut

    if (favoritesButton->size() > 104) {
      ((Fl_Menu_Item *)favoritesButton->menu())[0].deactivate();
    }
  } else if (v == 1) {
    // Manage favorites...
    favoritesCB(0);
  } else if (v == 2) {
    // Filesystems/My Computer
    directory("");
  } else {
    unquote_pathname(pathname, favoritesButton->text(v), sizeof(pathname));
    directory(pathname);
  }
}


//
// 'Fl_File_Chooser::favoritesCB()' - Handle favorites dialog.
//

void
Fl_File_Chooser::favoritesCB(Fl_Widget *w)
                                        // I - Widget
{
  int           i;                      // Looping var
  char          name[32],               // Preference name
                pathname[1024];         // Directory in list


  if (!w) {
    // Load the favorites list...
    favList->clear();
    favList->deselect();

    for (i = 0; i < 100; i ++) {
      // Get favorite directory 0 to 99...
      snprintf(name, sizeof(name), "favorite%02d", i);

      prefs_->get(name, pathname, "", sizeof(pathname));

      // Stop on the first empty favorite...
      if (!pathname[0]) break;

      // Add the favorite to the list...
      favList->add(pathname,
                   Fl_File_Icon::find(pathname, Fl_File_Icon::DIRECTORY));
    }

    favUpButton->deactivate();
    favDeleteButton->deactivate();
    favDownButton->deactivate();
    favOkButton->deactivate();

    favWindow->hotspot(favList);
    favWindow->show();
  } else if (w == favList) {
    i = favList->value();
    if (i) {
      if (i > 1) favUpButton->activate();
      else favUpButton->deactivate();

      favDeleteButton->activate();

      if (i < favList->size()) favDownButton->activate();
      else favDownButton->deactivate();
    } else {
      favUpButton->deactivate();
      favDeleteButton->deactivate();
      favDownButton->deactivate();
    }
  } else if (w == favUpButton) {
    i = favList->value();

    favList->insert(i - 1, favList->text(i), favList->data(i));
    favList->remove(i + 1);
    favList->select(i - 1);

    if (i == 2) favUpButton->deactivate();

    favDownButton->activate();

    favOkButton->activate();
  } else if (w == favDeleteButton) {
    i = favList->value();

    favList->remove(i);

    if (i > favList->size()) i --;
    favList->select(i);

    if (i < favList->size()) favDownButton->activate();
    else favDownButton->deactivate();

    if (i > 1) favUpButton->activate();
    else favUpButton->deactivate();

    if (!i) favDeleteButton->deactivate();

    favOkButton->activate();
  } else if (w == favDownButton) {
    i = favList->value();

    favList->insert(i + 2, favList->text(i), favList->data(i));
    favList->remove(i);
    favList->select(i + 1);

    if ((i + 1) == favList->size()) favDownButton->deactivate();

    favUpButton->activate();

    favOkButton->activate();
  } else if (w == favOkButton) {
    // Copy the new list over...
    for (i = 0; i < favList->size(); i ++) {
      // Set favorite directory 0 to 99...
      snprintf(name, sizeof(name), "favorite%02d", i);

      prefs_->set(name, favList->text(i + 1));
    }

    // Clear old entries as necessary...
    for (; i < 100; i ++) {
      // Clear favorite directory 0 to 99...
      snprintf(name, sizeof(name), "favorite%02d", i);

      prefs_->get(name, pathname, "", sizeof(pathname));

      if (pathname[0]) prefs_->set(name, "");
      else break;
    }

    update_favorites();
    prefs_->flush();

    favWindow->hide();
  }
}


//
// 'Fl_File_Chooser::fileListCB()' - Handle clicks (and double-clicks) in the
//                                   Fl_File_Browser.
//

void
Fl_File_Chooser::fileListCB()
{
  char  *filename,                      // New filename
        pathname[FL_PATH_MAX + 4];      // Full pathname to file


  filename = (char *)fileList->text(fileList->value());
  if (!filename)
    return;

  if (!directory_[0]) {
    strlcpy(pathname, filename, sizeof(pathname));
  } else if (strcmp(directory_, "/") == 0) {
    snprintf(pathname, sizeof(pathname), "/%s", filename);
  } else {
    snprintf(pathname, sizeof(pathname), "%s/%s", directory_, filename);
  }

  if (Fl::event_clicks()) {
    int condition = 0;
    if (Fl::system_driver()->colon_is_drive() && strlen(pathname) == 2 && pathname[1] == ':') condition = 1;
    if (!condition) condition = Fl::system_driver()->filename_isdir_quick(pathname);
    if (condition)
   {
      // Change directories...
      directory(pathname);

      // Reset the click count so that a click in the same spot won't
      // be treated as a triple-click.  We use a value of -1 because
      // the next click will increment click count to 0, which is what
      // we really want...
      Fl::event_clicks(-1);
    }
    else
    {
      // Hide the window - picked the file...
      window->hide();
      if (callback_) (*callback_)(this, data_);
    }
  }
  else
  {
    // Check if the user clicks on a directory when picking files;
    // if so, make sure only that item is selected...
    filename = pathname + strlen(pathname) - 1;

    if ((type_ & MULTI) && !(type_ & DIRECTORY)) {
      if (*filename == '/') {
        // Clicked on a directory, deselect everything else...
        int i = fileList->value();
        fileList->deselect();
        fileList->select(i);
      } else {
        // Clicked on a file - see if there are other directories selected...
        int i;
        const char *temp;
        for (i = 1; i <= fileList->size(); i ++) {
          if (i != fileList->value() && fileList->selected(i)) {
            temp = fileList->text(i);
            temp += strlen(temp) - 1;
            if (*temp == '/') break;    // Yes, selected directory
          }
        }

        if (i <= fileList->size()) {
          i = fileList->value();
          fileList->deselect();
          fileList->select(i);
        }
      }
    }
    // Strip any trailing slash from the directory name...
    if (*filename == '/') *filename = '\0';

//    puts("Setting fileName from fileListCB...");
    fileName->value(pathname);

    // Update the preview box...
    Fl::remove_timeout((Fl_Timeout_Handler)previewCB, this);
    Fl::add_timeout(1.0, (Fl_Timeout_Handler)previewCB, this);

    // Do any callback that is registered...
    if (callback_) (*callback_)(this, data_);

    // Activate the OK button as needed...
    if (!Fl::system_driver()->filename_isdir_quick(pathname) || (type_ & DIRECTORY))
      okButton->activate();
    else
      okButton->deactivate();
  }
}


//
// 'Fl_File_Chooser::fileNameCB()' - Handle text entry in the FileBrowser.
//

void
Fl_File_Chooser::fileNameCB()
{
  char          *filename,      // New filename
                *slash,         // Pointer to trailing slash
                pathname[FL_PATH_MAX],  // Full pathname to file
                matchname[FL_PATH_MAX]; // Matching filename
  int           i,              // Looping var
                min_match,      // Minimum number of matching chars
                max_match,      // Maximum number of matching chars
                num_files,      // Number of files in directory
                first_line;     // First matching line
  const char    *file;          // File from directory

//  puts("fileNameCB()");
//  printf("Event: %s\n", fl_eventnames[Fl::event()]);

  // Get the filename from the text field...
  filename = (char *)fileName->value();

  if (!filename || !filename[0]) {
    okButton->deactivate();
    return;
  }

  // Expand ~ and $ variables as needed...
  if (strchr(filename, '~') || strchr(filename, '$')) {
    fl_filename_expand(pathname, sizeof(pathname), filename);
    filename = pathname;
    value(pathname);
  }

  // Make sure we have an absolute path...
  int dirIsRelative = directory_[0] != '\0' && filename[0] != '/';
  if (dirIsRelative && Fl::system_driver()->colon_is_drive()) dirIsRelative = !(isalpha(filename[0] & 255) && (!filename[1] || filename[1] == ':'));
  if (dirIsRelative) {
    fl_filename_absolute(pathname, sizeof(pathname), filename);
    value(pathname);
    int flen = (int)strlen(pathname);
    fileName->insert_position(flen, flen); // no selection after expansion
  } else if (filename != pathname) {
    // Finally, make sure that we have a writable copy...
    strlcpy(pathname, filename, sizeof(pathname));
  }

  filename = pathname;

  // Now process things according to the key pressed...
  if (Fl::event_key() == FL_Enter || Fl::event_key() == FL_KP_Enter) {
    // Enter pressed - select or change directory...
    int condition = 0;
    if (Fl::system_driver()->colon_is_drive()) condition = isalpha(pathname[0] & 255) && pathname[1] == ':' && !pathname[2];
    if (!condition) condition = ( Fl::system_driver()->filename_isdir_quick(pathname) && compare_dirnames(pathname, directory_) );
    if (condition) {
      directory(pathname);
    } else if ((type_ & CREATE) || fl_access(pathname, 0) == 0) {
      if (!Fl::system_driver()->filename_isdir_quick(pathname) || (type_ & DIRECTORY)) {
        // Update the preview box...
        update_preview();

        // Do any callback that is registered...
        if (callback_) (*callback_)(this, data_);

        // Hide the window to signal things are done...
        window->hide();
      }
    } else {
      // File doesn't exist, so beep at and alert the user...
      fl_alert("%s",existing_file_label);
    }
  }
  else if (Fl::event_key() != FL_Delete &&
           Fl::event_key() != FL_BackSpace) {
    // Check to see if the user has entered a directory...
    if ((slash = strrchr(pathname, '/')) == NULL)
      slash = strrchr(pathname, '\\');

    if (!slash) return;

    // Yes, change directories if necessary...
    *slash++ = '\0';
    filename = slash;

    int condition = Fl::system_driver()->case_insensitive_filenames() ?
                    strcasecmp(pathname, directory_) : strcmp(pathname, directory_);
    if (condition && (pathname[0] || strcmp("/", directory_))) {
      int p = fileName->insert_position();
      int m = fileName->mark();

      directory(pathname);

      if (filename[0]) {
        char tempname[FL_PATH_MAX + 4];

        snprintf(tempname, sizeof(tempname), "%s/%s", directory_, filename);
        fileName->value(tempname);
        strlcpy(pathname, tempname, sizeof(pathname));
      }

      fileName->insert_position(p, m);
    }

    // Other key pressed - do filename completion as possible...
    num_files  = fileList->size();
    min_match  = (int) strlen(filename);
    max_match  = min_match + 1;
    first_line = 0;

    for (i = 1; i <= num_files && max_match > min_match; i ++) {
      file = fileList->text(i);

      if ( (Fl::system_driver()->case_insensitive_filenames()?
            strncasecmp(filename, file, min_match) : strncmp(filename, file, min_match)) == 0) {
        // OK, this one matches; check against the previous match
        if (!first_line) {
          // First match; copy stuff over...
          strlcpy(matchname, file, sizeof(matchname));
          max_match = (int) strlen(matchname);

          if (matchname[max_match - 1] == '/' &&        // Strip trailing /, if any...
              matchname[1] != 0 ) {                     // unless entire path is root ("/") -- STR #3500
            max_match --;
            matchname[max_match] = '\0';
          }

          // And then make sure that the item is visible
          fileList->topline(i);
          first_line = i;
        } else {
          // Succeeding match; compare to find maximum string match...
          while (max_match > min_match)
            if ( (Fl::system_driver()->case_insensitive_filenames()?
                  strncasecmp(file, matchname, max_match) : strncmp(file, matchname, max_match)) == 0)
              break;
            else
              max_match --;

          // Truncate the string as needed...
          matchname[max_match] = '\0';
        }
      }
    }

    // If we have any matches, add them to the input field...
    if (first_line > 0 && min_match == max_match &&
        max_match == (int)strlen(fileList->text(first_line))) {
      // This is the only possible match...
      fileList->deselect(0);
      fileList->select(first_line);
      fileList->redraw();
    } else if (max_match > min_match && first_line) {
      // Add the matching portion...
      fileName->replace(
                        (int) (filename - pathname),
                        (int) (filename - pathname + min_match),
                        matchname);

      // Highlight it with the cursor at the end of the selection so
      // s/he can press the right arrow to accept the selection
      // (Tab and End also do this for both cases.)
      fileName->insert_position(
                         (int) (filename - pathname + max_match),
                         (int) (filename - pathname + min_match));
    } else if (max_match == 0) {
      fileList->deselect(0);
      fileList->redraw();
    }

    // See if we need to enable the OK button...
    if (((type_ & CREATE) || !fl_access(fileName->value(), 0)) &&
        (!fl_filename_isdir(fileName->value()) || (type_ & DIRECTORY))) {
      okButton->activate();
    } else {
      okButton->deactivate();
    }
  } else {
    // FL_Delete or FL_BackSpace
    fileList->deselect(0);
    fileList->redraw();
    if (((type_ & CREATE) || !fl_access(fileName->value(), 0)) &&
        (!fl_filename_isdir(fileName->value()) || (type_ & DIRECTORY))) {
      okButton->activate();
    } else {
      okButton->deactivate();
    }
  }
}


/** Sets the current filename filter patterns. The filter
 patterns use fl_filename_match().
 Multiple patterns can be used by separating them with tabs, like
 <tt>"*.jpg\t*.png\t*.gif\t*"</tt>. In addition, you can provide
 human-readable labels with the patterns inside parenthesis, like
 <tt>"JPEG Files (*.jpg)\tPNG Files (*.png)\tGIF Files (*.gif)\tAll Files (*)"
 </tt>.

 Use filter(NULL) to show all files.
 */
void
Fl_File_Chooser::filter(const char *p)          // I - Pattern(s)
{
  char          *copyp,                         // Copy of pattern
                *start,                         // Start of pattern
                *end;                           // End of pattern
  int           allfiles;                       // Do we have a "*" pattern?
  char          temp[FL_PATH_MAX];              // Temporary pattern string


  // Make sure we have a pattern...
  if (!p || !*p) p = "*";

  // Copy the pattern string...
  copyp = fl_strdup(p);

  // Separate the pattern string as necessary...
  showChoice->clear();

  for (start = copyp, allfiles = 0; start && *start; start = end) {
    end = strchr(start, '\t');
    if (end) *end++ = '\0';

    if (strcmp(start, "*") == 0) {
      showChoice->add(all_files_label);
      allfiles = 1;
    } else {
      quote_pathname(temp, start, sizeof(temp));
      showChoice->add(temp);
      if (strstr(start, "(*)") != NULL) allfiles = 1;
    }
  }

  free(copyp);

  if (!allfiles) showChoice->add(all_files_label);

  showChoice->add(custom_filter_label);

  showChoice->value(0);
  showChoiceCB();
}


//
// 'Fl_File_Chooser::newdir()' - Make a new directory.
//

void
Fl_File_Chooser::newdir()
{
  const char    *dir;                           // New directory name
  char          pathname[FL_PATH_MAX + 4];      // Full path of directory


  // Get a directory name from the user
  if ((dir = fl_input("%s", NULL, new_directory_label)) == NULL)
    return;

  // Make it relative to the current directory as needed...
  if (dir[0] != '/' && dir[0] != '\\' && (!Fl::system_driver()->colon_is_drive() || dir[1] != ':') )
    snprintf(pathname, sizeof(pathname), "%s/%s", directory_, dir);
  else
    strlcpy(pathname, dir, sizeof(pathname));

  // Create the directory; ignore EEXIST errors...
  if (fl_mkdir(pathname, 0777))
    if (errno != EEXIST)
    {
      fl_alert("%s", strerror(errno));
      return;
    }

  // Show the new directory...
  directory(pathname);
}



/** Enable or disable the preview tile. 1 = enable preview, 0 = disable preview. */
void Fl_File_Chooser::preview(int e)
{
  previewButton->value(e);
  prefs_->set("preview", e);
  prefs_->flush();

  Fl_Group *p = previewBox->parent();
  if (e) {
    int w = p->w() * 2 / 3;
    fileList->resize(fileList->x(), fileList->y(), w, fileList->h());
    errorBox->resize(errorBox->x(), errorBox->y(), w, errorBox->h());
    previewBox->resize(fileList->x()+w, previewBox->y(), p->w()-w, previewBox->h());
    previewBox->show();
    update_preview();
  } else {
    fileList->resize(fileList->x(), fileList->y(), p->w(), fileList->h());
    errorBox->resize(errorBox->x(), errorBox->y(), p->w(), errorBox->h());
    previewBox->resize(p->x()+p->w(), previewBox->y(), 0, previewBox->h());
    previewBox->hide();
  }
  p->init_sizes();

  fileList->parent()->redraw();
}


//
// 'Fl_File_Chooser::previewCB()' - Timeout handler for the preview box.
//

void
Fl_File_Chooser::previewCB(Fl_File_Chooser *fc) {       // I - File chooser
  fc->update_preview();
}


/** Reloads the current directory in the Fl_File_Browser.*/
void
Fl_File_Chooser::rescan()
{
  char  pathname[FL_PATH_MAX];          // New pathname for filename field


  // Clear the current filename
  strlcpy(pathname, directory_, sizeof(pathname));
  if (pathname[0] && pathname[strlen(pathname) - 1] != '/') {
    strlcat(pathname, "/", sizeof(pathname));
  }
//  puts("Setting fileName in rescan()");
  fileName->value(pathname);

  if (type_ & DIRECTORY)
    okButton->activate();
  else
    okButton->deactivate();

  // Build the file list...
  if ( fileList->load(directory_, sort) <= 0 ) {
    if ( fileList->errmsg() ) errorBox->label(fileList->errmsg());     // show OS errormsg when possible
    else                      errorBox->label("No files found...");
    show_error_box(1);
  } else {
    show_error_box(0);
  }

  if (Fl::system_driver()->dot_file_hidden() && !showHiddenButton->value()) remove_hidden_files();
  // Update the preview box...
  update_preview();
}

/**
  Rescan the current directory  without clearing the filename,
  then select the file if it is in the list
*/
void Fl_File_Chooser::rescan_keep_filename()
{
  // if no filename was set, this is likely a diretory browser
  const char *fn = fileName->value();
  if (!fn || !*fn || fn[strlen(fn) - 1]=='/') {
    rescan();
    return;
  }

  int   i;
  char  pathname[FL_PATH_MAX];          // New pathname for filename field
  strlcpy(pathname, fn, sizeof(pathname));

  // Build the file list...
  if (fileList->load(directory_, sort) <= 0) {
    if ( fileList->errmsg() ) errorBox->label(fileList->errmsg());     // show OS errormsg when possible
    else                      errorBox->label("No files found...");
    show_error_box(1);
  } else {
    show_error_box(0);
  }
  if (Fl::system_driver()->dot_file_hidden() && !showHiddenButton->value()) remove_hidden_files();
  // Update the preview box...
  update_preview();

  // and select the chosen file
  char found = 0;
  char *slash = strrchr(pathname, '/');
  if (slash)
    slash++;
  else
    slash = pathname;
  for (i = 1; i <= fileList->size(); i ++)
    if ( (Fl::system_driver()->case_insensitive_filenames() ? strcasecmp(fileList->text(i), slash) : strcmp(fileList->text(i), slash)) == 0) {
      fileList->topline(i);
      fileList->select(i);
      found = 1;
      break;
    }

  // update OK button activity
  if (found || type_ & CREATE)
    okButton->activate();
  else
    okButton->deactivate();
}


//
// 'Fl_File_Chooser::showChoiceCB()' - Handle show selections.
//

void
Fl_File_Chooser::showChoiceCB()
{
  const char    *item,                  // Selected item
                *patstart;              // Start of pattern
  char          *patend;                // End of pattern
  char          temp[FL_PATH_MAX];      // Temporary string for pattern


  item = showChoice->text(showChoice->value());

  if (strcmp(item, custom_filter_label) == 0) {
    if ((item = fl_input("%s", pattern_, custom_filter_label)) != NULL) {
      strlcpy(pattern_, item, sizeof(pattern_));

      quote_pathname(temp, item, sizeof(temp));
      showChoice->add(temp);
      showChoice->value(showChoice->size() - 2);
    }
  } else if ((patstart = strchr(item, '(')) == NULL) {
    strlcpy(pattern_, item, sizeof(pattern_));
  } else {
    strlcpy(pattern_, patstart + 1, sizeof(pattern_));
    if ((patend = strrchr(pattern_, ')')) != NULL) *patend = '\0';
  }

  fileList->filter(pattern_);

  if (shown()) {
    // Rescan the directory...
    rescan_keep_filename();
  }
}


//
// 'Fl_File_Chooser::update_favorites()' - Update the favorites menu.
//

void
Fl_File_Chooser::update_favorites()
{
  int           i;                      // Looping var
  char          pathname[FL_PATH_MAX],  // Pathname
                menuname[2048];         // Menu name
  const char    *home;                  // Home directory


  favoritesButton->clear();
  favoritesButton->add("bla");
  favoritesButton->clear();
  favoritesButton->add(add_favorites_label, FL_ALT + 'a', 0);
  favoritesButton->add(manage_favorites_label, FL_ALT + 'm', 0, 0, FL_MENU_DIVIDER);
  favoritesButton->add(filesystems_label, FL_ALT + 'f', 0);

  if ((home = Fl::system_driver()->home_directory_name()) != NULL) {
    quote_pathname(menuname, home, sizeof(menuname));
    favoritesButton->add(menuname, FL_ALT + 'h', 0);
  }

  for (i = 0; i < 100; i ++) {
    snprintf(menuname, sizeof(menuname), "favorite%02d", i);
    prefs_->get(menuname, pathname, "", sizeof(pathname));
    if (!pathname[0]) break;

    quote_pathname(menuname, pathname, sizeof(menuname));

    if (i < 10) favoritesButton->add(menuname, FL_ALT + '0' + i, 0);
    else favoritesButton->add(menuname);
  }

  if (i == 100) ((Fl_Menu_Item *)favoritesButton->menu())[0].deactivate();
}


//
// 'Fl_File_Chooser::update_preview()' - Update the preview box...
//

void
Fl_File_Chooser::update_preview()
{
  const char            *filename;      // Current filename
  const char            *newlabel = 0;  // New label text
  Fl_Shared_Image       *image = 0,     // New image
                        *oldimage;      // Old image
  int                   pbw, pbh;       // Width and height of preview box
  int                   w, h;           // Width and height of preview image
  int                   set = 0;        // Set this flag as soon as a decent preview is found

  if (!previewButton->value()) return;

  filename = value();
  if (filename == NULL) {
    // no file name at all, so we have an empty preview
    set = 1;
  } else if (fl_filename_isdir(filename)) {
    // filename is a directory, show a folder icon
    newlabel = "@fileopen";
    set = 1;
  } else {
    struct stat s;
    if (fl_stat(filename, &s)==0) {
      if ((s.st_mode & S_IFREG) == 0) {
        // this is no regular file, probably some kind of device
        newlabel = "@-3refresh"; // a cross
        set = 1;
      } else if (s.st_size==0) {
        // this file is empty
        newlabel = "<empty file>";
        set = 1;
      } else {
        // if this file is an image, try to load it
        window->cursor(FL_CURSOR_WAIT);
        Fl::check();

        image = Fl_Shared_Image::get(filename);

        if (image) {
          window->cursor(FL_CURSOR_DEFAULT);
          Fl::check();
          set = 1;
        }
      }
    }
  }

  oldimage = (Fl_Shared_Image *)previewBox->image();

  if (oldimage) oldimage->release();

  previewBox->image(0);

  if (!set) {
    FILE        *fp;
    int         bytes;
    char        *ptr;

    if (filename) fp = fl_fopen(filename, "rb");
    else fp = NULL;

    if (fp != NULL) {
      // Try reading the first 1k of data for a label...
      bytes = (int) fread(preview_text_, 1, sizeof(preview_text_) - 1, fp);
      preview_text_[bytes] = '\0';
      fclose(fp);
    } else {
      // Assume we can't read any data...
      preview_text_[0] = '\0';
    }

    window->cursor(FL_CURSOR_DEFAULT);
    Fl::check();

    // Scan the buffer for printable UTF-8 chars...
    for (ptr = preview_text_; *ptr; ptr++) {
      uchar c = uchar(*ptr);
      if ( (c&0x80)==0 ) {
        if (!isprint(c&255) && !isspace(c&255)) break;
      } else if ( (c&0xe0)==0xc0 ) {
        if (ptr[1] && (ptr[1]&0xc0)!=0x80) break;
        ptr++;
      } else if ( (c&0xf0)==0xe0 ) {
        if (ptr[1] && (ptr[1]&0xc0)!=0x80) break;
        ptr++;
        if (ptr[1] && (ptr[1]&0xc0)!=0x80) break;
        ptr++;
      } else if ( (c&0xf8)==0xf0 ) {
        if (ptr[1] && (ptr[1]&0xc0)!=0x80) break;
        ptr++;
        if (ptr[1] && (ptr[1]&0xc0)!=0x80) break;
        ptr++;
        if (ptr[1] && (ptr[1]&0xc0)!=0x80) break;
        ptr++;
      }
    }
//         *ptr && (isprint(*ptr & 255) || isspace(*ptr & 255));
//       ptr ++);

    // Scan the buffer for printable characters in 8 bit
    if (*ptr || ptr == preview_text_) {
      for (ptr = preview_text_;
         *ptr && (isprint(*ptr & 255) || isspace(*ptr & 255));
         ptr ++) {/*empty*/}
    }

    if (*ptr || ptr == preview_text_) {
      // Non-printable file, just show a big ?...
      previewBox->label(filename ? "?" : 0);
      previewBox->align(FL_ALIGN_CLIP);
      previewBox->labelsize(75);
      previewBox->labelfont(FL_HELVETICA);
    } else {
      // Show the first 1k of text...
      int size = previewBox->h() / 20;
      if (size < 6) size = 6;
      else if (size > FL_NORMAL_SIZE) size = FL_NORMAL_SIZE;

      previewBox->label(preview_text_);
      previewBox->align((Fl_Align)(FL_ALIGN_CLIP | FL_ALIGN_INSIDE |
                                   FL_ALIGN_LEFT | FL_ALIGN_TOP));
      previewBox->labelsize(size);
      previewBox->labelfont(FL_COURIER);
    }
  } else if (image && ( (image->w() <= 0) ||
                        (image->h() <= 0) ||
                        (image->d() < 0)  ||
                        (image->count() <= 0))) {
    image->release();
    // Image has errors? Show big 'X'
    previewBox->label("X");
    previewBox->align(FL_ALIGN_CLIP);
    previewBox->labelsize(70);
    previewBox->labelfont(FL_HELVETICA);
    previewBox->redraw();
  } else if (image) {
    pbw = previewBox->w() - 20;
    pbh = previewBox->h() - 20;

    if (image->w() > pbw || image->h() > pbh) {
      w   = pbw;
      h   = w * image->h() / image->w();

      if (h > pbh) {
        h = pbh;
        w = h * image->w() / image->h();
      }

      oldimage = (Fl_Shared_Image *)image->copy(w, h);
      previewBox->image((Fl_Image *)oldimage);

      image->release();
    } else {
      previewBox->image((Fl_Image *)image);
    }

    previewBox->align(FL_ALIGN_CLIP);
    previewBox->label(0);
  } else if (newlabel) {
    previewBox->label(newlabel);
    previewBox->align(FL_ALIGN_CLIP);
    previewBox->labelsize(newlabel[0]=='@'?75:12);
    previewBox->labelfont(FL_HELVETICA);
  }

  previewBox->redraw();
}


/** Gets the current value of the selected file(s).
 \p f is a \c 1-based index into a list of
 file names. The number of selected files is returned by
 Fl_File_Chooser::count().

 This sample code loops through all selected files:
 \code
 // Get list of filenames user selected from a MULTI chooser
 for ( int t=1; t<=chooser->count(); t++ ) {
     const char *filename = chooser->value(t);
     ...
 }
 \endcode
 */
const char *                    // O - Filename or NULL
Fl_File_Chooser::value(int f)   // I - File number
{
  int           i;              // Looping var
  int           fcount;         // Number of selected files
  const char    *name;          // Current filename
  static char   pathname[FL_PATH_MAX + 4]; // Filename + directory


  name = fileName->value();

  if (!(type_ & MULTI)) {
    // Return the filename in the filename field...
    if (!name || !name[0]) return NULL;
    else return name;
  }

  // Return a filename from the list...
  for (i = 1, fcount = 0; i <= fileList->size(); i ++)
    if (fileList->selected(i)) {
      // See if this file is a selected file/directory...
      name = fileList->text(i);

      fcount ++;

      if (fcount == f) {
        if (directory_[0]) {
          snprintf(pathname, sizeof(pathname), "%s/%s", directory_, name);
        } else {
          strlcpy(pathname, name, sizeof(pathname));
        }

        return pathname;
      }
    }

  // If nothing is selected, use the filename field...
  if (!name || !name[0]) return NULL;
  else return name;
}


/** Sets the current value of the selected file.

  If a relative path is provided in \c filename it is converted to
  an absolute path.

  If \c NULL or an empty string is provided, the working directory is
  changed to the user's current directory and the filename is set to "".

  After assigning the filename the entire string (if any) is selected, i.e.
  - insert_position() is 0 (zero)
  - mark() is strlen(\<expanded filename>).

  \note The selection of the entire string may not always be desired but
    it is kept for backwards compatibility.

  \param[in] filename   relative or absolute filename, may be NULL or ""
 */
void
Fl_File_Chooser::value(const char *filename)
                                        // I - Filename + directory
{
  int   i,                              // Looping var
        fcount;                         // Number of items in list
  char  *slash;                         // Directory separator
  char  pathname[FL_PATH_MAX];          // Local copy of filename


//  printf("Fl_File_Chooser::value(\"%s\")\n", filename == NULL ? "(null)" : filename);

  // See if the filename is the "My System" directory...
  if (filename == NULL || !filename[0]) {
    // Yes, just change the current directory...
    directory(filename);
    fileName->value("");
    okButton->deactivate();
    return;
  }

  char fixpath[FL_PATH_MAX];                   // Path with slashes converted
  if (Fl::system_driver()->backslash_as_slash()) {
    // See if the filename contains backslashes...
    if (strchr(filename, '\\')) {
      // Convert backslashes to slashes...
      strlcpy(fixpath, filename, sizeof(fixpath));

      for (slash = strchr(fixpath, '\\'); slash; slash = strchr(slash + 1, '\\'))
        *slash = '/';

      filename = fixpath;
    }
  }

  // See if there is a directory in there...
  fl_filename_absolute(pathname, sizeof(pathname), filename);

  if ((slash = strrchr(pathname, '/')) != NULL) {
    // Yes, change the display to the directory...
    if (!fl_filename_isdir(pathname)) *slash++ = '\0';

    directory(pathname);
    if (*slash == '/') slash = pathname;
  } else {
    directory(".");
    slash = pathname;
  }

  // Set the input field to the absolute path...
  if (slash > pathname) slash[-1] = '/';

  fileName->value(pathname);
  fileName->insert_position(0, (int) strlen(pathname));
  okButton->activate();

  // Then find the file in the file list and select it...
  fcount = fileList->size();

  fileList->deselect(0);
  fileList->redraw();

  for (i = 1; i <= fcount; i ++)
    if ( ( Fl::system_driver()->case_insensitive_filenames() ? strcasecmp(fileList->text(i), slash) : strcmp(fileList->text(i), slash)) == 0) {
//      printf("Selecting line %d...\n", i);
      fileList->topline(i);
      fileList->select(i);
      break;
    }
}

/** Shows the Fl_File_Chooser window.*/
void Fl_File_Chooser::show()
{
  window->hotspot(fileList);
  window->show();
  Fl::flush();
  fl_cursor(FL_CURSOR_WAIT);
  rescan_keep_filename();
  fl_cursor(FL_CURSOR_DEFAULT);
  fileName->take_focus();
  if (!Fl::system_driver()->dot_file_hidden()) showHiddenButton->hide();
}

void Fl_File_Chooser::hide() {
  Fl::remove_timeout((Fl_Timeout_Handler)previewCB, this);
  Fl_Shared_Image *oldimage = (Fl_Shared_Image*)previewBox->image();
  if (oldimage) oldimage->release();
  previewBox->image(NULL);
  window->hide();
}

void Fl_File_Chooser::showHidden(int value)
{
  if (value) {
    fileList->load(directory());
  } else {
    remove_hidden_files();
    fileList->redraw();
  }
}

void Fl_File_Chooser::remove_hidden_files()
{
  int count = fileList->size();
  for(int num = count; num >= 1; num--) {
    const char *p = fileList->text(num);
    if (*p == '.' && strcmp(p, "../") != 0) fileList->remove(num);
  }
  fileList->topline(1);
}


//
// 'compare_dirnames()' - Compare two directory names.
//

static int
compare_dirnames(const char *a, const char *b) {
  int alen, blen;

  // Get length of each string...
  alen = (int) (strlen(a) - 1);
  blen = (int) (strlen(b) - 1);

  if (alen < 0 || blen < 0) return alen - blen;

  // Check for trailing slashes...
  if (a[alen] != '/') alen ++;
  if (b[blen] != '/') blen ++;

  // If the lengths aren't the same, then return the difference...
  if (alen != blen) return alen - blen;

  // Do a comparison of the first N chars (alen == blen at this point)...
  return Fl::system_driver()->case_insensitive_filenames() ? strncasecmp(a, b, alen) : strncmp(a, b, alen);
}


//
// 'quote_pathname()' - Quote a pathname for a menu.
//

static void
quote_pathname(char       *dst,         // O - Destination string
               const char *src,         // I - Source string
               int        dstsize)      // I - Size of destination string
{
  dstsize--; // prepare for trailing zero

  while (*src && dstsize > 1) {
    if (*src == '\\') {
      // Convert backslash to forward slash...
      *dst++ = '\\';
      *dst++ = '/';
      dstsize -= 2;
      src ++;
    } else {
      if (*src == '/') {
        *dst++ = '\\';
        dstsize--;
      }
      *dst++ = *src++;
      dstsize--;
    }
  }

  *dst = '\0';
}


//
// 'unquote_pathname()' - Unquote a pathname from a menu.
//

static void
unquote_pathname(char       *dst,       // O - Destination string
                 const char *src,       // I - Source string
                 int        dstsize)    // I - Size of destination string
{
  dstsize--; // prepare for trailing zero

  while (*src && dstsize > 0) {
    if (*src == '\\') src++;
    *dst++ = *src++;
    dstsize--;
  }

  *dst = '\0';
}
