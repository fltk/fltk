//
// Core options header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2025 by Bill Spitzak and others.
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

/** \file FL/core/options.H
  \brief Application and system wide options management.
*/

#ifndef Fl_core_options_H
#define Fl_core_options_H

#include <FL/fl_config.h>             // build configuration
#include <FL/Fl_Export.H>             // for FL_EXPORT


namespace Fl {

/** Enumerator for global FLTK options.
    These options can be set system wide, per user, or for the running
    application only.
    \see Fl::option(Fl_Option, bool)
    \see Fl::option(Fl_Option)
*/
typedef enum {

  /// When switched on, moving the text cursor beyond the start or end of
  /// a text in a text widget will change focus to the next text widget.
  /// (This is considered 'old' behavior)
  ///
  /// When switched off (default), the cursor will stop at the end of the text.
  /// Pressing Tab or Ctrl-Tab will advance the keyboard focus.
  ///
  /// See also: Fl_Input_::tab_nav()
  ///
  OPTION_ARROW_FOCUS = 0,


  // When switched on, FLTK will use the file chooser dialog that comes
  // with your operating system whenever possible. When switched off, FLTK
  // will present its own file chooser.
  // \todo implement me
  // OPTION_NATIVE_FILECHOOSER,

  // When Filechooser Preview is enabled, the FLTK or native file chooser
  // will show a preview of a selected file (if possible) before the user
  // decides to choose the file.
  // \todo implement me
  // OPTION_FILECHOOSER_PREVIEW,

  /// If visible focus is switched on (default), FLTK will draw a dotted rectangle
  /// inside the widget that will receive the next keystroke. If switched
  /// off, no such indicator will be drawn and keyboard navigation
  /// is disabled.
  OPTION_VISIBLE_FOCUS,

  /// If text drag-and-drop is enabled (default), the user can select and drag text
  /// from any text widget. If disabled, no dragging is possible, however
  /// dropping text from other applications still works.
  OPTION_DND_TEXT,

  /// If tooltips are enabled (default), hovering the mouse over a widget with a
  /// tooltip text will open a little tooltip window until the mouse leaves
  /// the widget. If disabled, no tooltip is shown.
  OPTION_SHOW_TOOLTIPS,

  /// When switched on (default), Fl_Native_File_Chooser runs GTK file dialogs
  /// if the GTK library is available on the platform (linux/unix only).
  /// When switched off, GTK file dialogs aren't used even if the GTK library is available.
  OPTION_FNFC_USES_GTK,

  /// Meaningful for the Wayland/X11 platform only. When switched on, the library uses a Zenity-based file dialog.
  /// When switched off (default), no zenity-based file dialog is used.
  OPTION_FNFC_USES_ZENITY,

  /// Meaningful for the Wayland/X11 platform only.
  /// When switched on, the library uses a kdialog-based file dialog if command 'kdialog' is available on the running system.
  /// When switched off (default), no kdialog-based file dialog is used.
  OPTION_FNFC_USES_KDIALOG,

  /// When switched on (default), Fl_Printer runs the GTK printer dialog
  /// if the GTK library is available on the platform (linux/unix only).
  /// When switched off, the GTK printer dialog isn't used even if the GTK library is available.
  OPTION_PRINTER_USES_GTK,

  /// When switched on (default), the library shows in a transient yellow window the zoom factor
  /// value.
  /// When switched off, no such window gets displayed.
  OPTION_SHOW_SCALING,

  /// When switched on and when the keyboard in use has '+' in the shifted position of its key,
  /// pressing that key and ctrl triggers the zoom-in operation.
  /// When switched off (default), the zoom-in operation requires that also the shift key is pressed.
  /// Under macOS, this option has no effect because the OS itself generates ⌘= followed
  /// by ⌘+ when pressing ⌘ and the '=|+' key without pressing shift.
  OPTION_SIMPLE_ZOOM_SHORTCUT,

  // don't change this, leave it always as the last element
  /// For internal use only.
  OPTION_LAST
} Fl_Option;

/*
 Return a global setting for all FLTK applications, possibly overridden
 by a setting specifically for this application.
 */
FL_EXPORT extern bool option(Fl_Option opt);

/*
 Override an option while the application is running.
 */
FL_EXPORT extern void option(Fl_Option opt, bool val);

// Visible focus methods...

/**
  Gets or sets the visible keyboard focus on buttons and other
  non-text widgets. The default mode is to enable keyboard focus
  for all widgets.
*/
FL_EXPORT inline void visible_focus(int v) { option(OPTION_VISIBLE_FOCUS, (v!=0)); }

/**
  Gets or sets the visible keyboard focus on buttons and other
  non-text widgets. The default mode is to enable keyboard focus
  for all widgets.
*/
FL_EXPORT inline int  visible_focus() { return option(OPTION_VISIBLE_FOCUS); }

// Drag-n-drop text operation methods...

/**
  Sets whether drag and drop text operations are supported.
  This specifically affects whether selected text can
  be dragged from text fields or dragged within a text field as a
  cut/paste shortcut.
*/
FL_EXPORT inline void dnd_text_ops(int v) { option(OPTION_DND_TEXT, (v!=0)); }

/**
  Gets whether drag and drop text operations are
  supported. This returns whether selected text can
  be dragged from text fields or dragged within a text field as a
  cut/paste shortcut.
*/
FL_EXPORT inline int  dnd_text_ops() { return option(OPTION_DND_TEXT); }

} // namespace Fl


#endif // !Fl_core_options_H
