//
// Widget Tracker header file for the Fast Light Tool Kit (FLTK).
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

/** \file FL/Fl_Widget_Tracker.H
  \brief Track widget deletion.
*/

#ifndef Fl_Widget_Tracker_H
#define Fl_Widget_Tracker_H

#include <FL/fl_config.h> // build configuration
#include <FL/Fl_Export.H>

class Fl_Widget;


/**
  This class should be used to control safe widget deletion.

  You can use an Fl_Widget_Tracker object to watch another widget, if you
  need to know whether this widget has been deleted during a callback.

  This simplifies the use of the "safe widget deletion" methods
  Fl::watch_widget_pointer() and Fl::release_widget_pointer() and
  makes their use more reliable, because the destructor automatically
  releases the widget pointer from the widget watch list.

  Fl_Widget_Tracker is intended to be used as an automatic (local/stack)
  variable, such that its destructor is called when the object's
  scope is left. This ensures that no stale widget pointers are
  left in the widget watch list (see example below).

  You can also create Fl_Widget_Tracker objects with \c new, but then it
  is your responsibility to delete the object (and thus remove the
  widget pointer from the watch list) when it is no longer needed.

  Example:

  \code
  int MyClass::handle (int event) {

    if (...) {
      Fl_Widget_Tracker wp(this);           // watch myself
      do_callback();                        // call the callback

      if (wp.deleted()) return 1;           // exit, if deleted

      // Now we are sure that the widget has not been deleted,
      // and it is safe to access the widget:

      box(FL_FLAT_BOX);
      color(FL_WHITE);
      redraw();
    }
  }
  \endcode

*/
class FL_EXPORT Fl_Widget_Tracker {

  Fl_Widget* wp_;

public:

  Fl_Widget_Tracker(Fl_Widget *wi);
  // Rule of five. Note that we *can* implement these when we refactor widget
  // tracking with a C++11 map or unordered_map, for example.
  Fl_Widget_Tracker(const Fl_Widget_Tracker&) = delete;
  Fl_Widget_Tracker(Fl_Widget_Tracker&&) = delete;
  Fl_Widget_Tracker& operator=(const Fl_Widget_Tracker&) = delete;
  Fl_Widget_Tracker& operator=(Fl_Widget_Tracker&&) = delete;
  ~Fl_Widget_Tracker();

  /**
   Clear the widget pointer.
   */
  void clear() { wp_ = nullptr; }

  /**
    Returns a pointer to the watched widget.
    \return nullptr if the widget was deleted.
  */
  Fl_Widget *widget() { return wp_; }

  /**
    Check if the widget was deleted since the tracker was created.
    \return 1 if the watched widget has been deleted, otherwise 0
  */
  int deleted() {return wp_ == nullptr;}

  /**
    Check if the widget exists and was not deleted since the tracker was created.
    \return 1 if the watched widget exists, otherwise 0
  */
  int exists() { return wp_ != nullptr; }

};

namespace Fl {

/** \defgroup fl_del_widget Safe widget deletion support functions

 These functions, declared in <FL/Fl.H>, support deletion of widgets inside callbacks.

 Fl::delete_widget() should be called when deleting widgets
 or complete widget trees (Fl_Group, Fl_Window, ...) inside
 callbacks.

 The other functions are intended for internal use. The preferred
 way to use them is by using the helper class Fl_Widget_Tracker.

 The following is to show how it works ...

 There are three groups of related methods:

 -# scheduled widget deletion
 - Fl::delete_widget() schedules widgets for deletion
 - Fl::do_widget_deletion() deletes all scheduled widgets
 -# widget watch list ("smart pointers")
 - Fl::watch_widget_pointer() adds a widget pointer to the watch list
 - Fl::release_widget_pointer() removes a widget pointer from the watch list
 - Fl::clear_widget_pointer() clears a widget pointer \e in the watch list
 -# the class Fl_Widget_Tracker:
 - the constructor calls Fl::watch_widget_pointer()
 - the destructor calls Fl::release_widget_pointer()
 - the access methods can be used to test, if a widget has been deleted
 \see Fl_Widget_Tracker.

 @{ */

// Widget deletion:
FL_EXPORT extern void delete_widget(Fl_Widget *w);
FL_EXPORT extern void do_widget_deletion();
FL_EXPORT extern void watch_widget_pointer(Fl_Widget *&w);
FL_EXPORT extern void release_widget_pointer(Fl_Widget *&w);
FL_EXPORT extern void clear_widget_pointer(Fl_Widget const *w);

/** @} */

} // namespace Fl

#endif // !Fl_Widget_Tracker_H
